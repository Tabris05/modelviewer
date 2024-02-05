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
	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, NULL);
}

Model Model::make(const char* pathStr) {
	if (!pathStr) return Model{ VertexBuffer::make(std::vector<Vertex>{}), IndexBuffer::make(std::vector<GLuint>{}), VertexArray::make(), 0 };

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

	for (size_t i : asset.scenes[asset.defaultScene.value_or(0)].nodeIndices) {
		glm::mat4 transform{ 1.0f };
		processNode(asset, i, transform, vertices, indices);
	}

	VertexBuffer vBuf = VertexBuffer::make(vertices);
	IndexBuffer iBuf = IndexBuffer::make(indices);
	VertexArray vArr = VertexArray::make();

	vArr.linkVertexBuffer(vBuf, sizeof(Vertex));
	vArr.linkIndexBuffer(iBuf);
	vArr.linkAttribute(0, 3, GL_FLOAT, 0);

	return Model{ std::move(vBuf), std::move(iBuf), std::move(vArr), static_cast<GLuint>(indices.size()) };
}

Model::Model(VertexBuffer vBuf, IndexBuffer iBuf, VertexArray vArr, GLuint numIndices) : m_vBuf{ std::move(vBuf) }, m_iBuf{ std::move(iBuf) }, m_vArr{ std::move(vArr) }, m_numIndices{ numIndices } {}

void Model::processNode(const fastgltf::Asset& asset, size_t index, glm::mat4 transform, std::vector<Vertex>&vertices, std::vector<GLuint>& indices) {
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
	
	struct S {};

	if (curNode.meshIndex.has_value()) {
		const fastgltf::Mesh& curMesh = asset.meshes[curNode.meshIndex.value()];
		for (const fastgltf::Primitive& curPrimitive : curMesh.primitives) {
			size_t indexOffset = vertices.size();

			const fastgltf::Accessor& positionAccessor = asset.accessors[curPrimitive.findAttribute("POSITION")->second];
			fastgltf::iterateAccessor<glm::vec3>(asset, positionAccessor, [&vertices, transform] (glm::vec3 pos) {
				vertices.emplace_back(glm::vec3(transform * glm::vec4(pos, 1.0f)));
			});

			const fastgltf::Accessor& indexAccessor = asset.accessors[curPrimitive.indicesAccessor.value()];
			fastgltf::iterateAccessor<GLuint>(asset, indexAccessor, [&indices, indexOffset] (GLuint index) {
				indices.emplace_back(index + indexOffset);
			});

			indices.emplace_back(-1);
		}
	}

	for (size_t i : curNode.children) processNode(asset, i, transform, vertices, indices);
}