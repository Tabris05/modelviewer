#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

#include "shaderprogram.h"
#include "vertexarray.h"
#include "vertexbuffer.h"

using GLid = GLuint;

class FrameBuffer {
private:
	GLid m_id;
	std::vector<GLid> m_texIDs;
	std::vector<GLid> m_rBufIDs;
	int m_width, m_height;
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();
	FrameBuffer(const FrameBuffer& src) = delete;
	FrameBuffer(FrameBuffer&& src) = delete;

	operator GLid();

	void bind();
	void unbind();
	void attatchTexture(GLenum attatchment, GLenum internalFormat, bool multisample = false);
	void attatchRenderBuffer(GLenum attatchment, GLenum internalFormat, bool multisample = false);
	void bindTexture(size_t index, GLuint slot);
	void unbindTexture();
	void blitTo(FrameBuffer& dst, GLenum bufferType);
	
};

#endif

