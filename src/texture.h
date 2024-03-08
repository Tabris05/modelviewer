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
			int width, 
			int height,
			GLenum internalFormat = GL_RGB8,
			void* data = nullptr,
			GLenum format = GL_RGB,
			GLenum dataType = GL_UNSIGNED_BYTE,
			GLenum minFilter = GL_NEAREST,
			GLenum magFilter = GL_NEAREST,
			GLenum wrapS = GL_CLAMP_TO_EDGE,
			GLenum wrapT = GL_CLAMP_TO_EDGE
		);
		static Texture make3D(
			int width,
			int height,
			int depth,
			GLenum internalFormat = GL_RGB8,
			void* data = nullptr,
			GLenum format = GL_RGB,
			GLenum dataType = GL_UNSIGNED_BYTE,
			GLenum minFilter = GL_NEAREST,
			GLenum magFilter = GL_NEAREST,
			GLenum wrapS = GL_CLAMP_TO_EDGE,
			GLenum wrapT = GL_CLAMP_TO_EDGE,
			GLenum wrapR = GL_CLAMP_TO_EDGE
		);
		~Texture();
	private:
		Texture(GLuint id);

		GLuint m_id;
		std::optional<GLuint64> m_handle;
		RefCounter m_rc;
};

#endif
