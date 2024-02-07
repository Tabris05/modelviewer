#include "model.h"
#include <filesystem>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "dbg.h"



void Model::draw() {
	m_vArr.bind();
	m_cmdBuf.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, m_cmdBuf.numCommands(), 0);
}

Model Model::make(const char* pathStr) {
	//if (!pathStr) return Model{ VertexBuffer::make(std::vector<Vertex>{}), IndexBuffer::make(std::vector<GLuint>{}), VertexArray::make(), 0, glm::mat4{ 1.0f }, AABB{} };

	const std::filesystem::path path(pathStr);
	fastgltf::GltfDataBuffer data;
	data.loadFromFile(path);
	const fastgltf::Asset asset{ std::move(m_parser.loadGLTF(&data, path.parent_path(), fastgltf::Options::GenerateMeshIndices | fastgltf::Options::LoadExternalBuffers).get()) };

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	size_t verticesSize = 0;
	size_t indicesSize = 0;
	for (const fastgltf::Mesh curMesh : asset.meshes) {
		for (size_t i = 0; i < curMesh.primitives.size(); i++) {
			verticesSize += asset.accessors[curMesh.primitives[i].findAttribute("POSITION")->second].count;
			indicesSize += asset.accessors[curMesh.primitives[i].indicesAccessor.value()].count + 1;
		}
	}
	vertices.reserve(verticesSize);
	indices.reserve(indicesSize);

	std::vector<DrawCommand> cmds;

	AABB aabb{};

	auto processNode = [&asset, &vertices, &indices, &cmds, &aabb](this auto& self, size_t index, glm::mat4 transform) -> void {
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
				const fastgltf::Accessor& indexAccessor = asset.accessors[curPrimitive.indicesAccessor.value()];
				fastgltf::iterateAccessor<GLuint>(asset, indexAccessor, [&indices](GLuint index) {
					indices.emplace_back(index);
				});
				cmds.emplace_back(indices.size() - oldIndicesSize, 1, oldIndicesSize, oldVerticesSize, 0);
			}
		}
		for (size_t i : curNode.children) self(i, transform);
	};

	for (size_t i : asset.scenes[asset.defaultScene.value_or(0)].nodeIndices) {
		glm::mat4 transform{ 1.0f };
		processNode(i, transform);
	}

	CommandBuffer cmdBuf = CommandBuffer::make(cmds);
	VertexBuffer vBuf = VertexBuffer::make(vertices);
	IndexBuffer iBuf = IndexBuffer::make(indices);
	VertexArray vArr = VertexArray::make();
	

	vArr.linkVertexBuffer(vBuf, sizeof(Vertex));
	vArr.linkIndexBuffer(iBuf);
	vArr.linkAttribute(0, 3, GL_FLOAT, 0);

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

	return Model{ std::move(cmdBuf), std::move(vBuf), std::move(iBuf), std::move(vArr), baseTransform, aabb};
}

Model::Model(CommandBuffer cmdBuf, VertexBuffer vBuf, IndexBuffer iBuf, VertexArray vArr, glm::mat4 baseTransform, AABB aabb) :
	m_cmdBuf{ std::move(cmdBuf) },
	m_vBuf{ std::move(vBuf) },
	m_iBuf{ std::move(iBuf) },
	m_vArr{ std::move(vArr) },
	m_baseTransform{ baseTransform },
	m_aabb{ aabb } {}