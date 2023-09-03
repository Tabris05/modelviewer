#include "indexbuffer.h"

IndexBuffer::IndexBuffer(const std::vector<GLuint>& indices) : m_rc{new size_t(1)} {
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
}

IndexBuffer::IndexBuffer(const IndexBuffer& src) : m_id{ src.m_id }, m_rc{ src.m_rc } {
	if (m_rc) (*m_rc)++;
}

IndexBuffer::IndexBuffer(IndexBuffer&& src) noexcept : m_id{ src.m_id }, m_rc{ src.m_rc } {
	src.m_id = 0;
	src.m_rc = nullptr;
}

IndexBuffer::~IndexBuffer() {
	if (m_rc) {
		(*m_rc)--;
		if (*m_rc == 0) {
			glDeleteBuffers(1, &m_id);
			delete m_rc;
		}
	}
}

void IndexBuffer::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
