#include "model.h"
#include <filesystem>
#include <stb/stb_image.h>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "material.h"
#include "texture.h"
#include "dbg.h"

void Model::draw() {
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, m_cmdBuf.numCommands(), 0);
}

Model Model::make(const char* pathStr) {
	//if (!pathStr) return Model{ VertexBuffer::make(std::vector<Vertex>{}), IndexBuffer::make(std::vector<GLuint>{}), VertexArray::make(), 0, glm::mat4{ 1.0f }, AABB{} };

	const std::filesystem::path path(pathStr);
	fastgltf::GltfDataBuffer data;
	data.loadFromFile(path);
	const fastgltf::Options options =
		fastgltf::Options::GenerateMeshIndices |
		fastgltf::Options::LoadExternalBuffers |
		fastgltf::Options::LoadExternalImages;
	const fastgltf::Asset asset{ std::move(m_parser.loadGLTF(&data, path.parent_path(), options).get()) };

	std::vector<Texture> textures;
	std::vector<std::optional<Texture>> maybeTextures;
	std::vector<Material> materials;
	std::vector<GLuint> materialIndices;
	std::vector<DrawCommand> cmds;
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
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
	maybeTextures.resize(asset.textures.size());
	materials.reserve(asset.materials.size());
	materialIndices.reserve(numPrimitives);
	cmds.reserve(numPrimitives);
	vertices.reserve(verticesSize);
	indices.reserve(indicesSize);

	AABB aabb{};
	
	auto processTexture = [&](size_t index, bool srgb = false) -> GLuint64 {
		if (maybeTextures[index].has_value()) return maybeTextures[index].value().handle().value();
		const fastgltf::Texture& curTexture = asset.textures[index];
		fastgltf::Sampler curSampler{
			.wrapS = fastgltf::Wrap::Repeat,
			.wrapT = fastgltf::Wrap::Repeat
		};
		if (curTexture.samplerIndex.has_value()) {
			curSampler = asset.samplers[curTexture.samplerIndex.value()];
		}
		const fastgltf::sources::Vector& data = std::get<fastgltf::sources::Vector>(asset.images[curTexture.imageIndex.value()].data);
		int width, height, nrChannels;
		unsigned char* bytes = stbi_load_from_memory(data.bytes.data(), data.bytes.size(), &width, &height, &nrChannels, 0);
		maybeTextures[index] = Texture::make2D(
			bytes,
			width,
			height,
			nrChannels,
			static_cast<GLenum>(curSampler.minFilter.value_or(fastgltf::Filter::LinearMipMapLinear)),
			static_cast<GLenum>(curSampler.magFilter.value_or(fastgltf::Filter::Linear)),
			static_cast<GLenum>(curSampler.wrapS),
			static_cast<GLenum>(curSampler.wrapT),
			srgb
		);
		maybeTextures[index]->makeBindless();
		stbi_image_free(bytes);
		return maybeTextures[index].value().handle().value();
	};

	for (const fastgltf::Material& curMaterial : asset.materials) {
		unsigned char dummyAlbedo[] { 255, 255, 255, 255 };
		unsigned char dummyMetallicRoughness[] { 255, 255, 255 };
		unsigned char dummyNormal[] { 0, 0, 255 };

		Material val{
			.m_baseColor{ glm::make_vec4(curMaterial.pbrData.baseColorFactor.data()) },
			.m_metallicRoughness { 0.0f, curMaterial.pbrData.roughnessFactor, curMaterial.pbrData.metallicFactor, 0.0f }
		};

		if (curMaterial.pbrData.baseColorTexture.has_value()) {
			val.m_albedoHandle = processTexture(curMaterial.pbrData.baseColorTexture.value().textureIndex, true);
		}
		else {
			maybeTextures.emplace_back(Texture::make2D(dummyAlbedo, 1, 1, 4, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
			val.m_albedoHandle = maybeTextures.back()->makeBindless();
		}

		if (curMaterial.pbrData.metallicRoughnessTexture.has_value()) {
			val.m_metallicRoughnessHandle = processTexture(curMaterial.pbrData.metallicRoughnessTexture.value().textureIndex);
		}
		else {
			maybeTextures.emplace_back(Texture::make2D(dummyMetallicRoughness, 1, 1, 3, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
			val.m_albedoHandle = maybeTextures.back()->makeBindless();
		}

		if (curMaterial.normalTexture.has_value()) {
			val.m_normalHandle = processTexture(curMaterial.normalTexture.value().textureIndex);
		}
		else {
			maybeTextures.emplace_back(Texture::make2D(dummyNormal, 1, 1, 3, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
			val.m_normalHandle = maybeTextures.back()->makeBindless();
		}

		materials.emplace_back(val);
	}

	for (const std::optional<Texture>& curTexture : maybeTextures) {
		textures.emplace_back(curTexture.value());
	}

	auto processNode = [&](this auto& self, size_t index, glm::mat4 transform) -> void {
		const fastgltf::Node& curNode = asset.nodes[index];
		transform *= std::visit(fastgltf::visitor{
			[](fastgltf::Node::TransformMatrix matrix) {
				return glm::make_mat4(matrix.data());
			},
			[](fastgltf::Node::TRS trs) {
				return glm::translate(glm::mat4{ 1.0f }, glm::make_vec3(trs.translation.data()))
				* glm::toMat4(glm::make_quat(trs.rotation.data()))
				* glm::scale(glm::mat4{ 1.0f }, glm::make_vec3(trs.scale.data()));
			}
		}, curNode.transform);

		if (curNode.meshIndex.has_value()) {
			const glm::mat4 normalTransform = glm::transpose(glm::inverse(transform));
			const fastgltf::Mesh& curMesh = asset.meshes[curNode.meshIndex.value()];
			for (const fastgltf::Primitive& curPrimitive : curMesh.primitives) {
				size_t oldVerticesSize = vertices.size();
				size_t oldIndicesSize = indices.size();

				const fastgltf::Accessor& positionAccessor = asset.accessors[curPrimitive.findAttribute("POSITION")->second];
				fastgltf::iterateAccessor<glm::vec3>(asset, positionAccessor, [&vertices, &aabb, transform](glm::vec3 pos) {
					glm::vec3 vertex = glm::vec3(transform * glm::vec4(pos, 1.0f));
					aabb.m_min = glm::min(aabb.m_min, vertex);
					aabb.m_max = glm::max(aabb.m_max, vertex);
					vertices.emplace_back(vertex);
				});

				const fastgltf::Accessor& normalAccessor = asset.accessors[curPrimitive.findAttribute("NORMAL")->second];
				fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, normalAccessor, [&vertices, oldVerticesSize, normalTransform](glm::vec3 normal, size_t index) {
					vertices[index + oldVerticesSize].m_normal = glm::normalize(glm::vec3(normalTransform * glm::vec4(normal, 0.0f)));
				});

				const fastgltf::Primitive::attribute_type* uvAccessorIndex;
				if ((uvAccessorIndex = curPrimitive.findAttribute("TEXCOORD_0")) != curPrimitive.attributes.cend()) {
					const fastgltf::Accessor& uvAccessor = asset.accessors[uvAccessorIndex->second];
					fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, uvAccessor, [&vertices, oldVerticesSize](glm::vec2 uv, size_t index) {
						vertices[index + oldVerticesSize].m_uv = uv;
					});
				}

				const fastgltf::Accessor& indexAccessor = asset.accessors[curPrimitive.indicesAccessor.value()];
				fastgltf::iterateAccessor<GLuint>(asset, indexAccessor, [&indices](GLuint index) {
					indices.emplace_back(index);
				});

				materialIndices.emplace_back(curPrimitive.materialIndex.value());
				cmds.emplace_back(indices.size() - oldIndicesSize, 1, oldIndicesSize, oldVerticesSize, 0);
			}
		}
		for (size_t i : curNode.children) self(i, transform);
	};

	for (size_t i : asset.scenes[asset.defaultScene.value_or(0)].nodeIndices) {
		glm::mat4 transform{ 1.0f };
		processNode(i, transform);
	}

	ShaderStorageBuffer materialBuf = ShaderStorageBuffer::make(materials);
	ShaderStorageBuffer materialIndexBuf = ShaderStorageBuffer::make(materialIndices);
	CommandBuffer cmdBuf = CommandBuffer::make(cmds);
	VertexBuffer vBuf = VertexBuffer::make(vertices);
	IndexBuffer iBuf = IndexBuffer::make(indices);
	VertexArray vArr = VertexArray::make();
	
	materialBuf.bind(0);
	materialIndexBuf.bind(1);
	vArr.linkVertexBuffer(vBuf, sizeof(Vertex));
	vArr.linkIndexBuffer(iBuf);
	vArr.linkAttribute(0, 3, GL_FLOAT, offsetof(Vertex, m_position));
	vArr.linkAttribute(1, 3, GL_FLOAT, offsetof(Vertex, m_normal));
	vArr.linkAttribute(2, 2, GL_FLOAT, offsetof(Vertex, m_uv));
	vArr.bind();
	cmdBuf.bind();

	glm::vec3 size = aabb.m_max - aabb.m_min;
	float scale = 1.0f / std::max(size.x, std::max(size.y, size.z));
	glm::mat4 baseTransform = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	baseTransform = glm::scale(baseTransform, glm::vec3(scale));
	glm::vec3 corners[8] = {
		glm::vec3(aabb.m_min.x, aabb.m_min.y, aabb.m_min.z),
		glm::vec3(aabb.m_max.x, aabb.m_min.y, aabb.m_min.z),
		glm::vec3(aabb.m_max.x, aabb.m_max.y, aabb.m_min.z),
		glm::vec3(aabb.m_min.x, aabb.m_max.y, aabb.m_min.z),
		glm::vec3(aabb.m_min.x, aabb.m_min.y, aabb.m_max.z),
		glm::vec3(aabb.m_max.x, aabb.m_min.y, aabb.m_max.z),
		glm::vec3(aabb.m_max.x, aabb.m_max.y, aabb.m_max.z),
		glm::vec3(aabb.m_min.x, aabb.m_max.y, aabb.m_max.z),
	};
	for (glm::vec3& i : corners) i = glm::vec3(baseTransform * glm::vec4(i, 1.0f));
	aabb = AABB{};
	for (glm::vec3 i : corners) {
		aabb.m_min = glm::min(aabb.m_min, i);
		aabb.m_max = glm::max(aabb.m_max, i);
	}

	return Model{ 
		std::move(textures),
		std::move(materialBuf),
		std::move(materialIndexBuf),
		std::move(cmdBuf),
		std::move(vBuf),
		std::move(iBuf),
		std::move(vArr),
		baseTransform,
		aabb
	};
}

Model::Model(std::vector<Texture> textures, ShaderStorageBuffer materialBuf, ShaderStorageBuffer materialIndexBuf, CommandBuffer cmdBuf, VertexBuffer vBuf, IndexBuffer iBuf, VertexArray vArr, glm::mat4 baseTransform, AABB aabb) :
	m_textures{ std::move(textures) },
	m_materialBuf{ std::move(materialBuf) },
	m_materialIndexBuf{ std::move(materialIndexBuf) },
	m_cmdBuf{ std::move(cmdBuf) },
	m_vBuf{ std::move(vBuf) },
	m_iBuf{ std::move(iBuf) },
	m_vArr{ std::move(vArr) },
	m_baseTransform{ baseTransform },
	m_aabb{ aabb } {}