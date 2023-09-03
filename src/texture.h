#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

#include "shaderprogram.h"

using GLid = GLuint;

class Texture {
public:
private:
	int m_widthImg, m_heightImg, m_numColCh;
	GLid m_texID;
	GLuint m_slot;
	size_t* m_rc;
public:
	Texture();
	Texture(const char* filename, GLuint slot, GLenum colorspace);
	Texture(const Texture& src) noexcept;
	Texture(Texture&& src) noexcept;
	~Texture();


	Texture& operator=(const Texture& src) noexcept;

	void bind();
	void unbind();
	GLid getSlot() const;
};

#endif

