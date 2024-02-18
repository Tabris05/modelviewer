#ifndef TEXTURE_H
#define TEXTURE_H

#include <optional>
#include <glad/glad.h>
#include "refcounter.h"

class Texture {
	public:
		GLuint id() const;
		GLuint64 makeBindless();
		std::optional<GLuint64> handle() const;

		static Texture make2D(
			unsigned char* data,
			int width, 
			int height,
			GLenum internalFormat = GL_RGB8,
			GLenum format = GL_RGB,
			GLenum minFilter = GL_NEAREST,
			GLenum magFilter = GL_NEAREST,
			GLenum wrapS = GL_CLAMP_TO_EDGE,
			GLenum wrapT = GL_CLAMP_TO_EDGE
		);
		~Texture();
	private:
		Texture(GLuint id);

		GLuint m_id;
		std::optional<GLuint64> m_handle;
		RefCounter m_rc;
};

#endif
