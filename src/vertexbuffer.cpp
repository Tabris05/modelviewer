#include "vertexbuffer.h"

VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices) : m_rc{ new size_t(1) } {
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(const std::vector<float>& vertices) : m_rc{ new size_t(1) } {
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(const VertexBuffer& src) : m_id{ src.m_id }, m_rc { src.m_rc } {
	if (m_rc) (*m_rc)++;
}

VertexBuffer::VertexBuffer(VertexBuffer&& src) noexcept : m_id{ src.m_id }, m_rc{ src.m_rc } {
	src.m_id = 0;
	src.m_rc = nullptr;
}

VertexBuffer::~VertexBuffer() {
	if (m_rc) {
		(*m_rc)--;
		if (*m_rc == 0) {
			glDeleteBuffers(1, &m_id);
			delete m_rc;
		}
	}
}

void VertexBuffer::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
