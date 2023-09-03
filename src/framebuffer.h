#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>

#include "shaderprogram.h"
#include "vertexarray.h"
#include "vertexbuffer.h"

using GLid = GLuint;

class FrameBuffer {
private:
	GLid m_id;
	GLid m_texID;
	VertexArray m_vArr;
	VertexBuffer m_vBuf;
public:
	FrameBuffer(int width, int height);
	~FrameBuffer();
	FrameBuffer(const FrameBuffer& src) = delete;
	FrameBuffer(FrameBuffer&& src) = delete;

	operator GLid();

	void bind();
	void unbind();
	void draw(ShaderProgram& shader);
};

#endif

