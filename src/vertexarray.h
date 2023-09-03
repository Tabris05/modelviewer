#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "vertexbuffer.h"
#include "indexbuffer.h"

using GLid = GLuint;

class VertexArray {
private:
	GLid m_id;
	size_t* m_rc;
public:
	VertexArray();
	VertexArray(const VertexArray& src);
	VertexArray(VertexArray&& src) noexcept;
	~VertexArray();
	void bind();
	void unbind();
	void linkAttribute(VertexBuffer& vBuf, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, GLsizeiptr offset);
};

#endif
