#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include <glad/glad.h>
#include "refcounter.h"
#include "indexbuffer.h"
#include "vertexbuffer.h"

class VertexArray {
	public:
		void bind();
		void unbind();
		void linkIndexBuffer(const IndexBuffer& iBuf);
		void linkVertexBuffer(const VertexBuffer& vBuf, GLsizei stride);
		void linkAttribute(GLuint location, GLuint numComponents, GLenum type, GLuint offset);

		static VertexArray make();
		VertexArray(const VertexArray& src) = default;
		VertexArray(VertexArray&& src) = default;
		VertexArray& operator=(const VertexArray& src);
		VertexArray& operator=(VertexArray&& src) noexcept;
		~VertexArray();

	private:
		VertexArray(GLuint m_id);

		GLuint m_id;
		RefCounter m_rc;
		static inline GLuint m_boundID = 0;
};

#endif
