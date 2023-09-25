#include "multisampledframebuffer.h"

MultisampledFramebuffer::MultisampledFramebuffer(int width, int height) : Framebuffer(width, height) {}

void MultisampledFramebuffer::attatchTexture(GLenum attatchment, GLenum internalFormat) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	GLid texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, internalFormat, m_width, m_height, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, GL_TEXTURE_2D_MULTISAMPLE, texID, 0);
	m_texIDs.push_back(texID);
}

void MultisampledFramebuffer::attatchRenderBuffer(GLenum attatchment, GLenum internalFormat) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	GLid rBufID;
	glGenRenderbuffers(1, &rBufID);
	glBindRenderbuffer(GL_RENDERBUFFER, rBufID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, internalFormat, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attatchment, GL_RENDERBUFFER, rBufID);
	m_rBufIDs.push_back(rBufID);
}