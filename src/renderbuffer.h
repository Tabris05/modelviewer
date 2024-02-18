#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <glad/glad.h>

class RenderBuffer {
	public:
		GLuint id() const;

		static RenderBuffer make(GLenum internalFormat, int width, int height);
		static RenderBuffer makeMultisampled(GLenum internalFormat, int width, int height);
		~RenderBuffer();
	private:
		RenderBuffer(GLuint id);

		GLuint m_id;
};

#endif
