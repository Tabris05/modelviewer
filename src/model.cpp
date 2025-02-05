#include "model.h"
#include <stb/stb_image.h>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <mikktspace/mikktspace.h>
#include "material.h"
#include "texture.h"
#include "dbg.h"

void Model::drawOpaque() {
	if (m_opaqueCmdBuf.numCommands() > 0) {
		m_opaqueCmdBuf.bind();
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, m_opaqueCmdBuf.numCommands(), 0);
	}
}

void Model::drawTransparent() {
	if (m_transparentCmdBuf.numCommands() > 0) {
		m_transparentCmdBuf.bind();
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, m_transparentCmdBuf.numCommands(), 0);
	}
}

AABB Model::aabb() const {
	return m_aabb;
}

glm::mat4 Model::baseTransform() const {
	return m_baseTransform;
}

Model Model::make() {
	std::unordered_map<size_t, Texture> textures;
	ShaderStorageBuffer materialBuf = ShaderStorageBuffer::make();
	CommandBuffer opaqueCmds = CommandBuffer::make();
	CommandBuffer transparentCmds = CommandBuffer::make();
	VertexBuffer vBuf = VertexBuffer::make();
	IndexBuffer iBuf = IndexBuffer::make();
	VertexArray vArr = VertexArray::make();
	glm::mat4 baseTransform{ 1.0f };
	AABB aabb{ glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
	vArr.linkVertexBuffer(vBuf, sizeof(Vertex));
	vArr.linkIndexBuffer(iBuf);
	vArr.bind();
	return Model{
		std::move(textures),
		std::move(materialBuf),
		std::move(opaqueCmds),
		std::move(transparentCmds),
		std::move(vBuf),
		std::move(iBuf),
		std::move(vArr),
		baseTransform,
		aabb
	};
}

Model Model::make(const std::filesystem::path& path) {
	fastgltf::GltfDataBuffer data;
	data.loadFromFile(path);
	const fastgltf::Options options =
		fastgltf::Options::GenerateMeshIndices |
		fastgltf::Options::LoadExternalBuffers |
		fastgltf::Options::LoadExternalImages;
	const fastgltf::Asset asset{ std::move(m_parser.loadGltf(&data, path.parent_path(), options).get()) };

	std::unordered_map<size_t, Texture> textures;
	std::vector<Material> materials;
	std::vector<DrawCommand> opaqueCmds;
	std::vector<DrawCommand> transparentCmds;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	// count number of primitives, vertices, and indices to preallocate vector with correct size
	size_t numPrimitives = 0;
	size_t verticesSize = 0;
	size_t indicesSize = 0;
	for (const fastgltf::Mesh curMesh : asset.meshes) {
		for (size_t i = 0; i < curMesh.primitives.size(); i++) {
			verticesSize += asset.accessors[curMesh.primitives[i].findAttribute("POSITION")->second].count;
			indicesSize += asset.accessors[curMesh.primitives[i].indicesAccessor.value()].count;
			numPrimitives++;
		}
	}

	textures.reserve(asset.textures.size());
	materials.reserve(asset.materials.size());
	opaqueCmds.reserve(numPrimitives);
	transparentCmds.reserve(numPrimitives);
	vertices.reserve(verticesSize);
	indices.reserve(indicesSize);

	AABB aabb{};
	
	// helper function that returns a handle to the ith texture, or loads it if it hasn't been loaded already
	auto processTexture = [&](size_t index, bool srgb = false) -> GLuint64 {
		if (textures.contains(index)) return textures.at(index).handle();

		const fastgltf::Texture& curTexture = asset.textures[index];

		fastgltf::Sampler curSampler{
			.wrapS = fastgltf::Wrap::Repeat,
			.wrapT = fastgltf::Wrap::Repeat
		};
		if (curTexture.samplerIndex.has_value()) {
			curSampler = asset.samplers[curTexture.samplerIndex.value()];
		}

		const fastgltf::sources::Array& data = std::get<fastgltf::sources::Array>(asset.images[curTexture.imageIndex.value()].data);
		int width, height, nrChannels;
		unsigned char* bytes = stbi_load_from_memory(data.bytes.data(), data.bytes.size(), &width, &height, &nrChannels, 0);
		
		GLenum internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		GLenum format = GL_RGBA;
		switch (nrChannels) {
		case 3:
			internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
			format = GL_RGB;
			break;
		case 2:
			internalFormat = GL_RG8;
			format = GL_RG;
			break;
		case 1:
			internalFormat = GL_R8;
			format = GL_RED;
			break;
		}

		textures.emplace(std::make_pair(index, Texture::make2D(
			width,
			height,
			internalFormat,
			bytes,
			format,
			GL_UNSIGNED_BYTE,
			static_cast<GLenum>(curSampler.minFilter.value_or(fastgltf::Filter::LinearMipMapLinear)),
			static_cast<GLenum>(curSampler.magFilter.value_or(fastgltf::Filter::Linear)),
			static_cast<GLenum>(curSampler.wrapS),
			static_cast<GLenum>(curSampler.wrapT)
		)));

		stbi_image_free(bytes);
		return textures.at(index).handle();
	};

	if (!asset.materials.empty()) {
		for (const fastgltf::Material& curMaterial : asset.materials) {
			Material val{
				.m_baseColor{ glm::make_vec4(curMaterial.pbrData.baseColorFactor.data()) },
				.m_emissiveColor{ glm::vec4(glm::make_vec3(curMaterial.emissiveFactor.data()), curMaterial.emissiveStrength) },
				.m_metalness { curMaterial.pbrData.metallicFactor },
				.m_roughness { curMaterial.pbrData.roughnessFactor },
				.m_textureBitfield{ TextureBitfield::NONE }
			};

			if (curMaterial.pbrData.baseColorTexture.has_value()) {
				val.m_albedoHandle = processTexture(curMaterial.pbrData.baseColorTexture.value().textureIndex, true);
				val.m_textureBitfield |= TextureBitfield::HAS_ALBEDO;
			}

			if (curMaterial.normalTexture.has_value()) {
				val.m_normalHandle = processTexture(curMaterial.normalTexture.value().textureIndex);
				val.m_textureBitfield |= TextureBitfield::HAS_NORMAL;
			}

			if (curMaterial.pbrData.metallicRoughnessTexture.has_value()) {
				val.m_metallicRoughnessHandle = processTexture(curMaterial.pbrData.metallicRoughnessTexture.value().textureIndex);
				val.m_textureBitfield |= TextureBitfield::HAS_METALLIC_ROUGHNESS;
			}

			if (curMaterial.occlusionTexture.has_value()) {
				val.m_occlusionHandle = processTexture(curMaterial.occlusionTexture.value().textureIndex);
				val.m_textureBitfield |= TextureBitfield::HAS_OCCLUSION;
			}

			if (curMaterial.emissiveTexture.has_value()) {
				val.m_emissiveHandle = processTexture(curMaterial.emissiveTexture.value().textureIndex, true);
				val.m_textureBitfield |= TextureBitfield::HAS_EMISSIVE;
			}

			materials.emplace_back(val);
		}
	}
	else {
		Material val{
				.m_baseColor{ 1.0f, 1.0f, 1.0f, 1.0f },
				.m_emissiveColor{ 0.0f, 0.0f, 0.0f, 0.0f },
				.m_metalness { 0.0f },
				.m_roughness { 1.0f },
				.m_textureBitfield{ TextureBitfield::NONE }
		};
		materials.emplace_back(val);
	}

	// helper function to load meshes, called recursively to traverse the tree of nodes
	// mesh vertices are transformed to a common model space before storage to avoid the need to store model matrices per mesh
	// all vertices and indices are stored in a single shared vertex buffer and index buffer for use with multidrawindirect
	// also, I am hijacking the baseinstance parameter of the draw commands to store an index into the materials buffer
	// this is fine since the parameter appears to have no intrinsic meaning to the driver,
	// and allows me to have a reliable way of getting the correct material index stored per-mesh regardless of draw order
	auto processNode = [&](this auto& self, size_t index, glm::mat4 transform) -> void {
		const fastgltf::Node& curNode = asset.nodes[index];
		transform *= std::visit(fastgltf::visitor{
			[](fastgltf::Node::TransformMatrix matrix) {
				return glm::make_mat4(matrix.data());
			},
			[](fastgltf::TRS trs) {
				return glm::translate(glm::mat4{ 1.0f }, glm::make_vec3(trs.translation.data()))
				* glm::toMat4(glm::make_quat(trs.rotation.data()))
				* glm::scale(glm::mat4{ 1.0f }, glm::make_vec3(trs.scale.data()));
			}
		}, curNode.transform);

		if (curNode.meshIndex.has_value()) {
			const glm::mat3 normalTransform{ glm::transpose(glm::inverse(transform)) };
			const fastgltf::Mesh& curMesh = asset.meshes[curNode.meshIndex.value()];
			for (const fastgltf::Primitive& curPrimitive : curMesh.primitives) {
				size_t oldVerticesSize = vertices.size();
				size_t oldIndicesSize = indices.size();

				// really we should not assume we have indices and have an alternate path to deduplicate vertices to create an index buffer as a fallback
				const fastgltf::Accessor& indexAccessor = asset.accessors[curPrimitive.indicesAccessor.value()];
				fastgltf::iterateAccessor<GLuint>(asset, indexAccessor, [&indices](GLuint index) {
					indices.emplace_back(index);
				});

				const fastgltf::Accessor& positionAccessor = asset.accessors[curPrimitive.findAttribute("POSITION")->second];
				fastgltf::iterateAccessor<glm::vec3>(asset, positionAccessor, [&vertices, &aabb, transform](glm::vec3 pos) {
					glm::vec3 vertex = glm::vec3(transform * glm::vec4(pos, 1.0f));
					aabb.m_min = glm::min(aabb.m_min, vertex);
					aabb.m_max = glm::max(aabb.m_max, vertex);
					vertices.emplace_back(vertex);
				});

				// the spec says normals are optional and if they are not provided the implementation should calculate flat normals
				// but I'm not very interested in displaying the kinds of meshes that use flat normals anyways so I'll just assume normals are given for now
				const fastgltf::Accessor& normalAccessor = asset.accessors[curPrimitive.findAttribute("NORMAL")->second];
				fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, normalAccessor, [&vertices, oldVerticesSize, normalTransform](glm::vec3 normal, size_t index) {
					vertices[index + oldVerticesSize].m_normal = glm::normalize(normalTransform * normal);
				});

				const fastgltf::Primitive::attribute_type* uvAccessorIndex;
				if ((uvAccessorIndex = curPrimitive.findAttribute("TEXCOORD_0")) != curPrimitive.attributes.cend()) {
					const fastgltf::Accessor& uvAccessor = asset.accessors[uvAccessorIndex->second];
					fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, uvAccessor, [&vertices, oldVerticesSize](glm::vec2 uv, size_t index) {
						vertices[index + oldVerticesSize].m_uv = uv;
					});
				}

				const fastgltf::Primitive::attribute_type* tangentAccessorIndex;
				if ((tangentAccessorIndex = curPrimitive.findAttribute("TANGENT")) != curPrimitive.attributes.cend()) {
					const fastgltf::Accessor& tangentAccessor = asset.accessors[tangentAccessorIndex->second];
					fastgltf::iterateAccessorWithIndex<glm::vec4>(asset, tangentAccessor, [&vertices, oldVerticesSize, transform](glm::vec4 tangent, size_t index) {
						vertices[index + oldVerticesSize].m_tangent = glm::normalize(glm::vec3{ transform * glm::vec4{ glm::vec3{ tangent }, 0.0f } });
						vertices[index + oldVerticesSize].m_tangentSign = tangent.w;
					});
				}
				// if we are not given tangents but have uvs (and normals) then we can calculate them
				else if (uvAccessorIndex != curPrimitive.attributes.cend()) {
					struct UsrPtr {
						size_t vertexOffset;
						size_t indexOffset;
						std::vector<Vertex>& vertices;
						std::vector<GLuint>& indices;
					} usrPtr{ oldVerticesSize, oldIndicesSize, vertices, indices };

					SMikkTSpaceInterface interface{
						// get number of faces (triangles) in primitive
						[](const SMikkTSpaceContext* ctx) -> int { 
							UsrPtr* data = static_cast<UsrPtr*>(ctx->m_pUserData);
							return (data->indices.size() - data->indexOffset) / 3;
						},
						// get number of vertices in face
						[](const SMikkTSpaceContext*, const int) -> int {
							return 3;
						},
						// get position of specified vertex
						[](const SMikkTSpaceContext* ctx, float outPos[], const int face, const int vert) {
							UsrPtr* data = static_cast<UsrPtr*>(ctx->m_pUserData);
							memcpy(outPos, &data->vertices[data->vertexOffset + data->indices[data->indexOffset + face * 3 + vert]].m_position, sizeof(glm::vec3));
						},
						// get normal of specified vertex
						[](const SMikkTSpaceContext* ctx, float outNorm[], const int face, const int vert) {
							UsrPtr* data = static_cast<UsrPtr*>(ctx->m_pUserData);
							memcpy(outNorm, &data->vertices[data->vertexOffset + data->indices[data->indexOffset + face * 3 + vert]].m_normal, sizeof(glm::vec3));
						},
						// get uv of specified vertex
						[](const SMikkTSpaceContext* ctx, float outUV[], const int face, const int vert) {
							UsrPtr* data = static_cast<UsrPtr*>(ctx->m_pUserData);
							memcpy(outUV, &data->vertices[data->vertexOffset + data->indices[data->indexOffset + face * 3 + vert]].m_uv, sizeof(glm::vec2));
						},
						// set tangent of specified vertex
						[](const SMikkTSpaceContext* ctx, const float inTangent[], const float sign, const int face, const int vert) {
							UsrPtr* data = static_cast<UsrPtr*>(ctx->m_pUserData);
							size_t vertexIndex = data->vertexOffset + data->indices[data->indexOffset + face * 3 + vert];
							memcpy(&data->vertices[vertexIndex].m_tangent, inTangent, sizeof(glm::vec3));
							data->vertices[vertexIndex].m_tangentSign = sign;
						}
					};
					SMikkTSpaceContext ctx{ &interface, &usrPtr };
					genTangSpaceDefault(&ctx);
				}

				if (curPrimitive.materialIndex.has_value()) {
					if (asset.materials[curPrimitive.materialIndex.value()].alphaMode != fastgltf::AlphaMode::Blend) {
						opaqueCmds.emplace_back(indices.size() - oldIndicesSize, 1, oldIndicesSize, oldVerticesSize, curPrimitive.materialIndex.value());
					}
					else {
						transparentCmds.emplace_back(indices.size() - oldIndicesSize, 1, oldIndicesSize, oldVerticesSize, curPrimitive.materialIndex.value());
					}
				}
				else {
					opaqueCmds.emplace_back(indices.size() - oldIndicesSize, 1, oldIndicesSize, oldVerticesSize, curPrimitive.materialIndex.value());
				}
			}
		}
		for (size_t i : curNode.children) self(i, transform);
	};

	for (size_t i : asset.scenes[asset.defaultScene.value_or(0)].nodeIndices) {
		glm::mat4 transform{ 1.0f };
		processNode(i, transform);
	}

	ShaderStorageBuffer materialBuf = ShaderStorageBuffer::make(materials);
	CommandBuffer opaqueCmdBuf = CommandBuffer::make(opaqueCmds);
	CommandBuffer transparentCmdBuf = CommandBuffer::make(transparentCmds);
	VertexBuffer vBuf = VertexBuffer::make(vertices);
	IndexBuffer iBuf = IndexBuffer::make(indices);
	VertexArray vArr = VertexArray::make();
	
	materialBuf.bind(0);
	vArr.linkVertexBuffer(vBuf, sizeof(Vertex));
	vArr.linkIndexBuffer(iBuf);
	vArr.linkAttribute(0, 3, GL_FLOAT, offsetof(Vertex, m_position));
	vArr.linkAttribute(1, 3, GL_FLOAT, offsetof(Vertex, m_normal));
	vArr.linkAttribute(2, 3, GL_FLOAT, offsetof(Vertex, m_tangent));
	vArr.linkAttribute(3, 2, GL_FLOAT, offsetof(Vertex, m_uv));
	vArr.linkAttribute(4, 1, GL_FLOAT, offsetof(Vertex, m_tangentSign));
	vArr.bind();

	glm::vec3 size = aabb.m_max - aabb.m_min;
	float scale = 1.0f / std::max(size.x, std::max(size.y, size.z));
	glm::mat4 baseTransform = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	baseTransform = glm::scale(baseTransform, glm::vec3(scale));

	return Model{ 
		std::move(textures),
		std::move(materialBuf),
		std::move(opaqueCmdBuf),
		std::move(transparentCmdBuf),
		std::move(vBuf),
		std::move(iBuf),
		std::move(vArr),
		baseTransform,
		aabb
	};
}

Model::Model(
	std::unordered_map<size_t, Texture> textures,
	ShaderStorageBuffer materialBuf,
	CommandBuffer opaqueCmdBuf,
	CommandBuffer transparentCmdBuf,
	VertexBuffer vBuf,
	IndexBuffer iBuf,
	VertexArray vArr,
	glm::mat4 baseTransform,
	AABB aabb
) :
	m_textures{ std::move(textures) },
	m_materialBuf{ std::move(materialBuf) },
	m_opaqueCmdBuf{ std::move(opaqueCmdBuf) },
	m_transparentCmdBuf{ std::move(transparentCmdBuf) },
	m_vBuf{ std::move(vBuf) },
	m_iBuf{ std::move(iBuf) },
	m_vArr{ std::move(vArr) },
	m_baseTransform{ baseTransform },
	m_aabb{ aabb } {}