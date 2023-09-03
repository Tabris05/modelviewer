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
		glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexID"), m_diffuseMap->getSlot());
		glUniform1i(glGetUniformLocation(shaderProgram, "hasDiffuse"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shaderProgram, "hasDiffuse"), 0);
	}

	if (m_normalMap) {
		glUniform1i(glGetUniformLocation(shaderProgram, "normalTexID"), m_normalMap->getSlot());
		glUniform1i(glGetUniformLocation(shaderProgram, "hasNormal"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shaderProgram, "hasNormal"), 0);
	}

	if (m_aoMap) {
		glUniform1i(glGetUniformLocation(shaderProgram, "aoTexID"), m_aoMap->getSlot());
		glUniform1i(glGetUniformLocation(shaderProgram, "hasAO"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shaderProgram, "hasAO"), 0);
	}

	if (m_metalnessMap) {
		glUniform1i(glGetUniformLocation(shaderProgram, "metalnessTexID"), m_metalnessMap->getSlot());
		glUniform1i(glGetUniformLocation(shaderProgram, "hasMetalness"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shaderProgram, "hasMetalness"), 0);
	}

	if (m_roughnessMap) {
		glUniform1i(glGetUniformLocation(shaderProgram, "roughnessTexID"), m_roughnessMap->getSlot());
		glUniform1i(glGetUniformLocation(shaderProgram, "hasRoughness"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shaderProgram, "hasRoughness"), 0);
	}
}

void Mesh::draw(ShaderProgram& shaderProgram, Camera& camera, glm::mat4 model) {
	shaderProgram.activate();
	m_vArr.bind();
	
	bindTextures(shaderProgram);

	glm::vec3 cameraPos = camera.getPos();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera.getProjMatrix(45.0f, 0.1f, 100.0f) * camera.getViewMatrix()));
	glUniform3f(glGetUniformLocation(shaderProgram, "camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
}

void Mesh::drawTextureless(ShaderProgram& shaderProgram, Camera& camera, glm::mat4 model) {
	shaderProgram.activate();
	m_vArr.bind();

	glm::vec3 cameraPos = camera.getPos();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera.getProjMatrix(45.0f, 0.1f, 100.0f) * camera.getViewMatrix()));
	glUniform3f(glGetUniformLocation(shaderProgram, "camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
}