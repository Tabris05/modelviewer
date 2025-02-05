#include "commandbuffer.h"

void CommandBuffer::bind() {
	if (m_boundID != m_id) {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_id);
		m_boundID = m_id;
	}
}

void CommandBuffer::unbind() {
	if (m_boundID == m_id) {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		m_boundID = 0;
	}
}

GLuint CommandBuffer::id() const {
	return m_id;
}

size_t CommandBuffer::numCommands() const {
	return m_numCommands;
}

CommandBuffer CommandBuffer::make(const std::vector<DrawCommand>& cmds) {
	GLuint id;
	glCreateBuffers(1, &id);
	if(cmds.size() > 0) glNamedBufferStorage(id, cmds.size() * sizeof(DrawCommand), cmds.data(), 0);
	return CommandBuffer{ id, cmds.size()};
}

CommandBuffer& CommandBuffer::operator=(const CommandBuffer& src) {
	this->~CommandBuffer();
	new (this) CommandBuffer{ src };
	return *this;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& src) noexcept {
	this->~CommandBuffer();
	new (this) CommandBuffer{ std::move(src) };
	return *this;
}

CommandBuffer::~CommandBuffer() {
	if (m_rc.count() == 0) glDeleteBuffers(1, &m_id);
}

CommandBuffer::CommandBuffer(GLuint id, size_t numCommands) : m_id{ id }, m_numCommands{ numCommands } {}