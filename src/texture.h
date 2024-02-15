#ifndef TEXTURE_H
#define TEXTURE_H

#include <optional>
#include <glad/glad.h>
#include "refcounter.h"

class Texture {
	public:
		GLuint64 makeBindless();
		GLuint id() const;
		std::optional<GLuint64> handle() const;

		static Texture make2D(unsigned char* data, int width, int height, int nrChannels, GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, bool srgb = false);
		~Texture();
	private:
		Texture(GLuint id);

		GLuint m_id;
		std::optional<GLuint64> m_handle;
		RefCounter m_rc;
};

#endif
