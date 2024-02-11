#include "shaderstoragebuffer.h"

void ShaderStorageBuffer::bind(size_t index) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_id);
}

GLuint ShaderStorageBuffer::id() const {
	return m_id;
}

ShaderStorageBuffer ShaderStorageBuffer::make(const std::vector<glm::vec4>& bufferData = {}) {
	GLuint id;
	glCreateBuffers(1, &id);
	glNamedBufferData(id, bufferData.size() * sizeof(glm::vec4), bufferData.data(), GL_STATIC_DRAW);
	return ShaderStorageBuffer{ id };
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
	if (m_rc.count() == 0) glDeleteBuffers(1, &m_id);
}

ShaderStorageBuffer::ShaderStorageBuffer(GLuint id) :m_id{ id } {}
