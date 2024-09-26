#include "vertexbuffer.h"

void VertexBuffer::bind() {
	if (m_boundID != m_id) {
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		m_boundID = m_id;
	}
}

void VertexBuffer::unbind() {
	if (m_boundID == m_id) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_boundID = 0;
	}
}

GLuint VertexBuffer::id() const {
	return m_id;
}

VertexBuffer VertexBuffer::make(const std::vector<Vertex>& vertices) {
	GLuint id;
	glCreateBuffers(1, &id);
	if(vertices.size() > 0) glNamedBufferStorage(id, vertices.size() * sizeof(Vertex), vertices.data(), 0);
	return VertexBuffer{ id };
}

VertexBuffer& VertexBuffer::operator=(const VertexBuffer& src) {
	this->~VertexBuffer();
	new (this) VertexBuffer{ src };
	return *this;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& src) noexcept {
	this->~VertexBuffer();
	new (this) VertexBuffer{ std::move(src) };
	return *this;
}

VertexBuffer::~VertexBuffer() {
	if (m_rc.count() == 0) glDeleteBuffers(1, &m_id);
}

VertexBuffer::VertexBuffer(GLuint id) : m_id{ id } {}