#include "renderbuffer.h"
#include <new>
#include <utility>

GLuint RenderBuffer::id() const {
	return m_id;
}

RenderBuffer RenderBuffer::make(int width, int height, GLenum internalFormat) {
	GLuint id;
	glCreateRenderbuffers(1, &id);
	glNamedRenderbufferStorage(id, internalFormat, width, height);
	return RenderBuffer{ id };
}

RenderBuffer RenderBuffer::makeMultisampled(int width, int height, GLenum internalFormat) {
	GLuint id;
	glCreateRenderbuffers(1, &id);
	glNamedRenderbufferStorageMultisample(id, 8, internalFormat, width, height);
	return RenderBuffer{ id };
}

RenderBuffer& RenderBuffer::operator=(const RenderBuffer& src) {
	this->~RenderBuffer();
	new (this) RenderBuffer{ src };
	return *this;
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& src) noexcept {
	this->~RenderBuffer();
	new (this) RenderBuffer{ std::move(src) };
	return *this;
}

RenderBuffer::~RenderBuffer() {
	if(m_rc.count() == 0) glDeleteRenderbuffers(1, &m_id);
}

RenderBuffer::RenderBuffer(GLuint id) : m_id{ id } {}