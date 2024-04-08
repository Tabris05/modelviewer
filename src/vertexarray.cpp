#include "vertexarray.h"

void VertexArray::bind() {
	if (m_boundID != m_id) {
		glBindVertexArray(m_id);
		m_boundID = m_id;
	}
}

void VertexArray::unbind() {
	if (m_boundID == m_id) {
		glBindVertexArray(0);
		m_boundID = 0;
	}
}

void VertexArray::linkIndexBuffer(const IndexBuffer& iBuf) {
	glVertexArrayElementBuffer(m_id, iBuf.id());
}

void VertexArray::linkVertexBuffer(const VertexBuffer& vBuf, GLsizei stride) {
	glVertexArrayVertexBuffer(m_id, 0, vBuf.id(), 0, stride);
}

void VertexArray::linkAttribute(GLuint location, GLuint numComponents, GLenum type, GLuint offset) {
	glEnableVertexArrayAttrib(m_id, location);
	glVertexArrayAttribBinding(m_id, location, 0);
	glVertexArrayAttribFormat(m_id, location, numComponents, type, GL_FALSE, offset);
}

VertexArray VertexArray::make() {
	GLuint id;
	glCreateVertexArrays(1, &id);
	return VertexArray{ id };
}

VertexArray& VertexArray::operator=(const VertexArray& src) {
	this->~VertexArray();
	new (this) VertexArray{ src };
	return *this;
}

VertexArray& VertexArray::operator=(VertexArray&& src) noexcept {
	this->~VertexArray();
	new (this) VertexArray{ std::move(src) };
	return *this;
}

VertexArray::~VertexArray() {
	if (m_rc.count() == 0) glDeleteVertexArrays(1, &m_id);
}

VertexArray::VertexArray(GLuint id) : m_id{ id } {}