#include "framebuffer.h"

FrameBuffer::FrameBuffer(int width, int height) : m_width{ width }, m_height{ height } {
	glGenFramebuffers(1, &m_id);
}

FrameBuffer::~FrameBuffer() {
	glDeleteTextures(m_texIDs.size(), m_texIDs.data());
	glDeleteRenderbuffers(m_rBufIDs.size(), m_rBufIDs.data());
	glDeleteFramebuffers(1, &m_id);
}

FrameBuffer::operator GLid() {
	return m_id;
}

void FrameBuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::attatchTexture(GLenum attatchment, GLenum internalFormat, bool multisample) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	GLid texID;
	glGenTextures(1, &texID);
	if (multisample) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, internalFormat, m_width, m_height, GL_TRUE);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texID, 0);
	m_texIDs.push_back(texID);
}

void FrameBuffer::attatchRenderBuffer(GLenum attatchment, GLenum internalFormat, bool multisample) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	GLid rBufID;
	glGenRenderbuffers(1, &rBufID);
	glBindRenderbuffer(GL_RENDERBUFFER, rBufID);
	if (multisample) {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, internalFormat, m_width, m_height);
	}
	else {
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, m_width, m_height);
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attatchment, GL_RENDERBUFFER, rBufID);
	m_rBufIDs.push_back(rBufID);
}

void FrameBuffer::bindTexture(size_t index, GLuint slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texIDs[index]);
}

void FrameBuffer::unbindTexture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::blitTo(FrameBuffer& dst, GLenum bufferType) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, bufferType, GL_NEAREST);
}