#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include "vertex.h"

using GLid = GLuint;

class VertexBuffer {
private:
	GLid m_id;
	size_t* m_rc;
public:
	VertexBuffer(const std::vector<Vertex>& vertices);
	VertexBuffer(const std::vector<float>& vertices);
	VertexBuffer(const VertexBuffer& src);
	VertexBuffer(VertexBuffer&& src) noexcept;
	~VertexBuffer();
	void bind();
	void unbind();
};

#endif

