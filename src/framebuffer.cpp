#include "framebuffer.h"

Framebuffer::Framebuffer(int width, int height) : m_width{ width }, m_height{ height } {
	glGenFramebuffers(1, &m_id);
}

Framebuffer::~Framebuffer() {
	glDeleteTextures(m_texIDs.size(), m_texIDs.data());
	glDeleteRenderbuffers(m_rBufIDs.size(), m_rBufIDs.data());
	glDeleteFramebuffers(1, &m_id);
}

Framebuffer::operator GLid() {
	return m_id;
}

void Framebuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::attatchTexture(GLenum attatchment, GLenum internalFormat, GLenum format, GLenum size) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	GLid texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, size, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, GL_TEXTURE_2D, texID, 0);
	m_texIDs.push_back(texID);
}

void Framebuffer::attatchRenderBuffer(GLenum attatchment, GLenum internalFormat) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	GLid rBufID;
	glGenRenderbuffers(1, &rBufID);
	glBindRenderbuffer(GL_RENDERBUFFER, rBufID);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attatchment, GL_RENDERBUFFER, rBufID);
	m_rBufIDs.push_back(rBufID);
}

void Framebuffer::bindTexture(size_t index, GLuint slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texIDs[index]);
}

void Framebuffer::unbindTexture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::blitTo(Framebuffer& dst, GLenum bufferType) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, bufferType, GL_NEAREST);
}

void Framebuffer::reallocRenderBuffer(size_t index, int width, int height, GLenum internalFormat) {
	glBindRenderbuffer(GL_RENDERBUFFER, m_rBufIDs[index]);
	glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}