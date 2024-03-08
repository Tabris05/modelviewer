#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <glad/glad.h>
#include "refcounter.h"

class RenderBuffer {
	public:
		GLuint id() const;

		static RenderBuffer make(int width, int height, GLenum internalFormat);
		static RenderBuffer makeMultisampled(int width, int height, GLenum internalFormat);
		~RenderBuffer();
	private:
		RenderBuffer(GLuint id);

		GLuint m_id;
		RefCounter m_rc;
};

#endif
