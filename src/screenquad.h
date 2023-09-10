#ifndef SCREENQUAD_H
#define SCREENQUAD_H

#include <glad/glad.h>

#include "shaderprogram.h"
#include "vertexarray.h"
#include "vertexbuffer.h"

using GLid = GLuint;

class ScreenQuad {
private:
	GLid m_id;
	GLid m_texID;
	VertexArray m_vArr;
	VertexBuffer m_vBuf;
public:
	ScreenQuad(int width, int height);
	ScreenQuad(const ScreenQuad& src) = delete;
	ScreenQuad(ScreenQuad&& src) = delete;

	void draw(ShaderProgram& shader);
};

#endif

