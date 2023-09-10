#include "mesh.h"

#include <utility>
#include <string>
#include <glm/gtc/type_ptr.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::optional<Texture> diffuseMap, std::optional<Texture> normalMap, std::optional<Texture> aoMap, std::optional<Texture> metalnessMap, std::optional<Texture> roughnessMap) :
	m_numIndices{ indices.size() }, m_vBuf{ std::move(vertices) }, m_iBuf{ std::move(indices) }, m_diffuseMap{ std::move(diffuseMap) }, m_normalMap{ std::move(normalMap) }, m_aoMap{ std::move(aoMap) }, m_metalnessMap{ std::move(metalnessMap) }, m_roughnessMap{ std::move(roughnessMap) } {
	GLuint slot = 0;
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_pos));
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_color));
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_normal));
	m_vArr.linkAttribute(m_vBuf, slot++, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_tangent));
	m_vArr.linkAttribute(m_vBuf, slot++, 2, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, m_texCoord));
	m_vArr.unbind();
	m_vBuf.unbind();
	m_iBuf.unbind();
}

void Mesh::bindTextures(ShaderProgram& shaderProgram) {
	if (m_diffuseMap) {
		m_diffuseMap->bind();
		shaderProgram.setUniform("diffuseTexID", m_diffuseMap->getSlot());
		shaderProgram.setUniform("hasDiffuse", 1);
	}
	else {
		shaderProgram.setUniform("hasDiffuse", 0);
	}

	if (m_normalMap) {
		m_normalMap->bind();
		shaderProgram.setUniform("normalTexID", m_normalMap->getSlot());
		shaderProgram.setUniform("hasNormal", 1);
	}
	else {
		shaderProgram.setUniform("hasNormal", 0);
	}

	if (m_aoMap) {
		m_aoMap->bind();
		shaderProgram.setUniform("aoTexID", m_aoMap->getSlot());
		shaderProgram.setUniform("hasAO", 1);
	}
	else {
		shaderProgram.setUniform("hasAO", 1);
	}

	if (m_metalnessMap) {
		m_metalnessMap->bind();
		shaderProgram.setUniform("metalnessTexID", m_metalnessMap->getSlot());
		shaderProgram.setUniform("hasMetalness", 1);
	}
	else {
		shaderProgram.setUniform("hasMetalness", 0);
	}

	if (m_roughnessMap) {
		m_roughnessMap->bind();
		shaderProgram.setUniform("roughnessTexID", m_roughnessMap->getSlot());
		shaderProgram.setUniform("hasRoughness", 1);
	}
	else {
		shaderProgram.setUniform("hasRoughness", 0);
	}
}

void Mesh::draw(ShaderProgram& shaderProgram, Camera& camera, glm::mat4 model) {
	shaderProgram.activate();
	m_vArr.bind();
	
	bindTextures(shaderProgram);

	glm::vec3 cameraPos = camera.getPos();
	shaderProgram.setUniform("model", model);
	shaderProgram.setUniform("camMatrix", camera.getProjMatrix(45.0f, 0.1f, 100.0f) * camera.getViewMatrix());
	shaderProgram.setUniform("camPos", cameraPos);
	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
}