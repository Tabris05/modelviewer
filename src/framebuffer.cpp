#include "framebuffer.h"

void FrameBuffer::bind() {
	if (m_boundID != m_id) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);
		m_boundID = m_id;
	}
}

void FrameBuffer::unbind() {
	if (m_boundID == m_id) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_boundID = 0;
	}
}

GLuint FrameBuffer::id() const {
	return m_id;
}

void FrameBuffer::attachTexture(const Texture& texture, GLenum attachment) {
	glNamedFramebufferTexture(m_id, attachment, texture.id(), 0);
}

void FrameBuffer::detachTexture(GLenum attachment) {
	glNamedFramebufferTexture(m_id, attachment, 0, 0);
}

void FrameBuffer::attachRenderBuffer(const RenderBuffer& renderBuffer, GLenum attachment) {
	glNamedFramebufferRenderbuffer(m_id, attachment, GL_RENDERBUFFER, renderBuffer.id());
}

void FrameBuffer::detachRenderBuffer(GLenum attachment) {
	glNamedFramebufferRenderbuffer(m_id, attachment, 0, 0);
}

void FrameBuffer::blitTo(FrameBuffer& dst, GLenum mask, int x, int y) {
	glBlitNamedFramebuffer(m_id, dst.id(), 0, 0, x, y, 0, 0, x, y, mask, GL_NEAREST);
}

FrameBuffer FrameBuffer::make() {
	GLuint id;
	glCreateFramebuffers(1, &id);
	return FrameBuffer{ id };
}

FrameBuffer::~FrameBuffer() {
	if (m_rc.count() == 0) glDeleteFramebuffers(1, &m_id);
}

FrameBuffer::FrameBuffer(GLuint id) : m_id{ id } {}