#ifndef MSAABUFFER_H
#define MSAABUFFER_H

#include <glad/glad.h>

#include "framebuffer.h"

using GLid = GLuint;

class MSAABuffer {
private:
	GLid m_id;
	GLid m_texID;
	GLid m_rBufID;
	int m_width, m_height;
public:
	MSAABuffer(int width, int height);
	~MSAABuffer();
	MSAABuffer(const MSAABuffer& src) = delete;
	MSAABuffer(MSAABuffer&& src) = delete;

	void blit(FrameBuffer& dst);
	void bind();
	void unbind();
};

#endif

