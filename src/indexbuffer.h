#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include <vector>
#include <glad/glad.h>
#include "refcounter.h"

class IndexBuffer {
public:
	void bind();
	void unbind();
	GLuint id() const;

	static IndexBuffer make(const std::vector<GLuint>& indices = std::vector<GLuint>{});
	IndexBuffer(const IndexBuffer& src) = default;
	IndexBuffer(IndexBuffer&& src) = default;
	IndexBuffer& operator=(const IndexBuffer& src);
	IndexBuffer& operator=(IndexBuffer&& src) noexcept;
	~IndexBuffer();
private:
	IndexBuffer(GLuint id);

	GLuint m_id;
	RefCounter m_rc;
	inline static GLuint m_boundID = 0;
};

#endif
