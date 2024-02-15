#include "texture.h"

GLuint64 Texture::makeBindless() {
	m_handle = glGetTextureHandleARB(m_id);
	glMakeTextureHandleResidentARB(m_handle.value());
	return m_handle.value();
}

GLuint Texture::id() const {
	return m_id;
}

std::optional<GLuint64> Texture::handle() const {
	return m_handle;
}

Texture Texture::make2D(unsigned char* data, int width, int height, int nrChannels, GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT, bool srgb) {
	GLuint id;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);

	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrapS);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrapT);

	GLenum internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	GLenum format = GL_RGBA;
	switch (nrChannels) {
		case 3:
			internalFormat = srgb ? GL_SRGB8 : GL_RGB8;
			format = GL_RGB;
			break;
		case 2:
			internalFormat = GL_RG8;
			format = GL_RG;
			break;
		case 1:
			internalFormat = GL_R8;
			format = GL_RED;
			break;
	}

	glTextureStorage2D(id, 1, internalFormat, width, height);
	glTextureSubImage2D(id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

	if ((minFilter & 0xFF00) == 0x2700) {
		glGenerateTextureMipmap(id);
		float anisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
		glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
	}
	return Texture{ id };
}

Texture::~Texture() {
	if (m_rc.count() == 0) {
		if(m_handle.has_value()) glMakeTextureHandleNonResidentARB(m_handle.value());
		glDeleteTextures(1, &m_id);
	}
}

Texture::Texture(GLuint id) : m_id{ id } {}