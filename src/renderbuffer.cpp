#include "renderbuffer.h"

GLuint RenderBuffer::id() const {
	return m_id;
}

RenderBuffer RenderBuffer::make(GLenum internalFormat, int width, int height) {
	GLuint id;
	glCreateRenderbuffers(1, &id);
	glNamedRenderbufferStorage(id, internalFormat, width, height);
	return RenderBuffer{ id };
}

RenderBuffer RenderBuffer::makeMultisampled(GLenum internalFormat, int width, int height) {
	GLuint id;
	glCreateRenderbuffers(1, &id);
	glNamedRenderbufferStorageMultisample(id, 8, internalFormat, width, height);
	return RenderBuffer{ id };
}

RenderBuffer::~RenderBuffer() {
	if(m_rc.count() == 0) glDeleteRenderbuffers(1, &m_id);
}

RenderBuffer::RenderBuffer(GLuint id) : m_id{ id } {}