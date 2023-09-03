#include "vertexarray.h"

VertexArray::VertexArray() : m_rc{new size_t(1)} {
	glGenVertexArrays(1, &m_id);
	glBindVertexArray(m_id);
}

VertexArray::VertexArray(const VertexArray& src) : m_id{ src.m_id }, m_rc{ src.m_rc } {
	if (m_rc) (*m_rc)++;
}

VertexArray::VertexArray(VertexArray&& src) noexcept : m_id{ src.m_id }, m_rc{ src.m_rc } {
	src.m_id = 0;
	src.m_rc = nullptr;
}

VertexArray::~VertexArray() {
	if (m_rc) {
		(*m_rc)--;
		if (*m_rc == 0) {
			glDeleteVertexArrays(1, &m_id);
			delete m_rc;
		}
	}
}

void VertexArray::bind() {
	glBindVertexArray(m_id);
}

void VertexArray::unbind() {
	glBindVertexArray(0);
}

void VertexArray::linkAttribute(VertexBuffer& vBuf, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, GLsizeiptr offset) {
	vBuf.bind();
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, (void*)offset);
	glEnableVertexAttribArray(layout);
	vBuf.unbind();
}
