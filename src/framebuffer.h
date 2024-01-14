#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

using GLid = GLuint;

class Framebuffer {
protected:
	GLid m_id;
	std::vector<GLid> m_texIDs;
	std::vector<GLid> m_rBufIDs;
	int m_width, m_height;

public:
	Framebuffer(int width, int height);
	~Framebuffer();
	Framebuffer(const Framebuffer& src) = delete;
	Framebuffer(Framebuffer&& src) = delete;

	operator GLid();

	void bind();
	void unbind();
	void attatchTexture(GLenum attatchment, GLenum internalFormat, GLenum format = GL_RGB, GLenum size = GL_UNSIGNED_BYTE);
	void attatchRenderBuffer(GLenum attatchment, GLenum internalFormat);
	void bindTexture(size_t index, GLuint slot);
	void unbindTexture();
	void blitTo(Framebuffer& dst, GLenum bufferType);
	void reallocRenderBuffer(size_t index, int width, int height, GLenum internalFormat);
};

#endif
