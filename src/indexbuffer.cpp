#include "indexbuffer.h"

void IndexBuffer::bind() {
	if (m_boundID != m_id) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		m_boundID = m_id;
	}
}

void IndexBuffer::unbind() {
	if (m_boundID == m_id) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		m_boundID = 0;
	}
}

GLuint IndexBuffer::id() const {
	return m_id;
}

IndexBuffer IndexBuffer::make(const std::vector<GLuint>& indices) {
	GLuint id;
	glCreateBuffers(1, &id);
	glNamedBufferStorage(id, indices.size() * sizeof(GLuint), indices.data(), NULL);
	return IndexBuffer{ id };
}

IndexBuffer::~IndexBuffer() {
	if (m_rc.count() == 0) glDeleteBuffers(1, &m_id);
}

IndexBuffer::IndexBuffer(GLuint id) : m_id{ id } {}