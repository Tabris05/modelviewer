#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <vector>
#include <glad/glad.h>
#include "refcounter.h"
#include "vertex.h"

class VertexBuffer {
	public:
		void bind();
		void unbind();
		GLuint id() const;

		static VertexBuffer make(const std::vector<Vertex>& vertices = std::vector<Vertex>{});
		~VertexBuffer();

	private:
		VertexBuffer(GLuint id);

		GLuint m_id;
		RefCounter m_rc;
		static inline GLuint m_boundID = 0;
};

#endif
