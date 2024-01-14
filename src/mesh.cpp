#include "mesh.h"
#include "metautils.h"

#include <utility>
#include <string>
#include <glm/gtc/type_ptr.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, ColorData diffuse, NormalData normal, MaterialData ao, MaterialData metalness, MaterialData roughness) :
	m_numIndices{ indices.size() }, m_vBuf{ std::move(vertices) }, m_iBuf{ std::move(indices) }, m_diffuse{ std::move(diffuse) }, m_normal{ std::move(normal) }, m_ao{ std::move(ao) }, m_metalness{ std::move(metalness) }, m_roughness{ std::move(roughness) } {
	GLuint slot = 0;
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_pos));
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_normal));
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_tangent));
	m_vArr.linkAttribute(m_vBuf, slot++, 2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_texCoord));
	m_vArr.unbind();
	m_vBuf.unbind();
	m_iBuf.unbind();
}

void Mesh::bindTextures(ShaderProgram& shaderProgram) {
	std::visit(overload{
		[&shaderProgram](Texture& map) {
			map.bind();
			shaderProgram.setUniform("diffuseTexID", map.getSlot());
			shaderProgram.setUniform("hasDiffuse", 1);
		},
		[&shaderProgram](glm::vec3 value) {
			shaderProgram.setUniform("meshColor", value);
			shaderProgram.setUniform("hasDiffuse", 0);
		}
	}, m_diffuse);

	std::visit(overload{
		[&shaderProgram](Texture& map) {
			map.bind();
			shaderProgram.setUniform("aoTexID", map.getSlot());
			shaderProgram.setUniform("hasAO", 1);
		},
		[&shaderProgram](float value) {
			shaderProgram.setUniform("meshAO", value);
			shaderProgram.setUniform("hasAO", 0);
		},
		[&shaderProgram](std::monostate) {
			shaderProgram.setUniform("meshAO", 1.0f);
			shaderProgram.setUniform("hasAO", 0);
		}
	}, m_ao);

	std::visit(overload{
		[&shaderProgram](Texture& map) {
			map.bind();
			shaderProgram.setUniform("metalnessTexID", map.getSlot());
			shaderProgram.setUniform("hasMetalness", 1);
		},
		[&shaderProgram](float value) {
			shaderProgram.setUniform("meshMetalness", value);
			shaderProgram.setUniform("hasMetalness", 0);
		},
		[&shaderProgram](std::monostate) {
			shaderProgram.setUniform("meshMetalness", 0.0f);
			shaderProgram.setUniform("hasMetalness", 0);
		}
	}, m_metalness);

	std::visit(overload{
		[&shaderProgram](Texture& map) {
			map.bind();
			shaderProgram.setUniform("roughnessTexID", map.getSlot());
			shaderProgram.setUniform("hasRoughness", 1);
		},
		[&shaderProgram](float value) {
			shaderProgram.setUniform("meshRoughness", value);
			shaderProgram.setUniform("hasRoughness", 0);
		},
		[&shaderProgram](std::monostate) {
			shaderProgram.setUniform("meshMetalness", 1.0f);
			shaderProgram.setUniform("hasRoughness", 0);
		}
	}, m_roughness);

	if (m_normal) {
		m_normal->bind();
		shaderProgram.setUniform("normalTexID", m_normal->getSlot());
		shaderProgram.setUniform("hasNormal", 1);
	}
	else {
		shaderProgram.setUniform("hasNormal", 0);
	}
}

void Mesh::draw(ShaderProgram& shaderProgram, glm::mat4 model) {
	m_vArr.bind();
	
	bindTextures(shaderProgram);
	shaderProgram.setUniform("model", model);

	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
}