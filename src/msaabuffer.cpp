#include "msaabuffer.h"
#include "dbg.h"

MSAABuffer::MSAABuffer(int width, int height) :m_width{ width }, m_height{ height } {
	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB16F, width, height, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_texID, 0);

	glGenRenderbuffers(1, &m_rBufID);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rBufID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rBufID);
}

MSAABuffer::~MSAABuffer() {
	glDeleteFramebuffers(1, &m_id);
}

void MSAABuffer::blit(FrameBuffer& dst) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void MSAABuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void MSAABuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
