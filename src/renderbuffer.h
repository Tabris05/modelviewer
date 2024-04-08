#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <glad/glad.h>
#include "refcounter.h"

class RenderBuffer {
	public:
		GLuint id() const;

		static RenderBuffer make(int width, int height, GLenum internalFormat);
		static RenderBuffer makeMultisampled(int width, int height, GLenum internalFormat);
		RenderBuffer(const RenderBuffer& src) = default;
		RenderBuffer(RenderBuffer&& src) = default;
		RenderBuffer& operator=(const RenderBuffer& src);
		RenderBuffer& operator=(RenderBuffer&& src) noexcept;
		~RenderBuffer();
	private:
		RenderBuffer(GLuint id);

		GLuint m_id;
		RefCounter m_rc;
};

#endif
