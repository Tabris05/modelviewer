#include "texture.h"

#include <stb/stb_image.h>

Texture::Texture() : m_widthImg{ 0 }, m_heightImg{ 0 }, m_numColCh{ 0 }, m_texID{ 0 }, m_slot{ 0 }, m_rc{ nullptr } {}

Texture::Texture(const char* filename, GLuint slot, GLenum colorspace) : m_slot{ slot }, m_rc{ new size_t(1) } {
	unsigned char* bytes = stbi_load(filename, &m_widthImg, &m_heightImg, &m_numColCh, 0);

	glGenTextures(1, &m_texID);
	glActiveTexture(GL_TEXTURE0 + m_slot);
	glBindTexture(GL_TEXTURE_2D, m_texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLenum format = GL_RGBA;
	switch (m_numColCh) {
		case 4:
			format = GL_RGBA;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 2:
			format = GL_RG;
			break;
		case 1:
			format = GL_RED;
			break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, colorspace, m_widthImg, m_heightImg, 0, format, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytes);
}

Texture::Texture(const Texture& src) noexcept : m_widthImg{ src.m_widthImg }, m_heightImg{ src.m_heightImg }, m_numColCh{ src.m_numColCh }, m_texID{ src.m_texID }, m_slot{ src.m_slot }, m_rc{ src.m_rc } {
	if(m_rc) (*m_rc)++;
}

Texture::Texture(Texture&& src) noexcept : m_widthImg{ src.m_widthImg }, m_heightImg{ src.m_heightImg }, m_numColCh{ src.m_numColCh }, m_texID{ src.m_texID }, m_slot{ src.m_slot }, m_rc{ src.m_rc } {
	src.m_texID = 0;
	src.m_rc = nullptr;
}

Texture& Texture::operator=(const Texture& src) noexcept {
	m_widthImg = src.m_widthImg;
	m_heightImg = src.m_heightImg;
	m_numColCh = src.m_numColCh;
	m_texID = src.m_texID;
	m_slot = src.m_slot;
	m_rc = src.m_rc;
	if(m_rc) (*m_rc)++;
	return *this;
}

void Texture::bind() {
	glActiveTexture(GL_TEXTURE0 + m_slot);
	glBindTexture(GL_TEXTURE_2D, m_texID);
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	if (m_rc) {
		(*m_rc)--;
		if (*m_rc == 0) {
			glDeleteTextures(1, &m_texID);
			delete m_rc;
		}
	}
}

GLid Texture::getSlot() const {
	return m_slot;
}