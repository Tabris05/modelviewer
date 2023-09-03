#include "framebuffer.h"

FrameBuffer::FrameBuffer(int width, int height) : 
	m_vBuf{ std::vector<float>{
	1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f } } {
	m_vArr.linkAttribute(m_vBuf, 0, 2, GL_FLOAT, 4 * sizeof(float), 0);
	m_vArr.linkAttribute(m_vBuf, 1, 2, GL_FLOAT, 4 * sizeof(float), 2 * sizeof(float));
	m_vArr.unbind();
	m_vBuf.unbind();

	glGenFramebuffers(1, &m_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_id);

	glGenTextures(1, & m_texID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texID, 0);
}

FrameBuffer::~FrameBuffer() {
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

void FrameBuffer::draw(ShaderProgram& shader) {
	glDisable(GL_DEPTH_TEST);
	shader.activate();
	m_vArr.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}