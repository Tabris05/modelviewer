#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

using GLid = GLuint;

class IndexBuffer {
private:
	GLid m_id;
	size_t* m_rc;
public:
	IndexBuffer(const std::vector<GLuint>& indices);
	IndexBuffer(const IndexBuffer& src);
	IndexBuffer(IndexBuffer&& src) noexcept;
	~IndexBuffer();
	void bind();
	void unbind();
};

#endif

