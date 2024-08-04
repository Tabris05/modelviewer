#include "texture.h"
#include <cmath>

GLuint Texture::id() const {
	return m_id;
}

GLuint64 Texture::handle() const {
	return m_handle;
}

void Texture::bindForCompute(GLuint binding, GLenum access) const {
	glBindImageTexture(binding, m_id, 0, GL_FALSE, 0, access, m_internalFormat);
}

Texture Texture::make2D(
		int width,
		int height,
		GLenum internalFormat,
		void* data,
		GLenum format,
		GLenum dataType,
		GLenum minFilter,
		GLenum magFilter,
		GLenum wrapS,
		GLenum wrapT
	) {
	bool hasMipmaps = (minFilter & 0xFF00) == 0x2700;
	
	GLuint id;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);

	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrapS);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrapT);

	glTextureStorage2D(id, hasMipmaps ? std::floor(std::log2(std::max(width, height))) + 1 : 1, internalFormat, width, height);
	if (data) {
		glTextureSubImage2D(id, 0, 0, 0, width, height, format, dataType, data);
		if (hasMipmaps) {
			glGenerateTextureMipmap(id);
			float anisotropy;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
			glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
		}
	}
	GLuint64 handle = glGetTextureHandleARB(id);
	glMakeTextureHandleResidentARB(handle);

	return Texture{ id, handle, internalFormat };
}

Texture Texture::make2DMultisampled(
	int width,
	int height,
	GLenum internalFormat
) {
	GLuint id;
	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &id);

	glTextureStorage2DMultisample(id, 8, internalFormat, width, height, GL_TRUE);

	GLuint64 handle = glGetTextureHandleARB(id);
	glMakeTextureHandleResidentARB(handle);

	return Texture{ id, handle, internalFormat };
}


Texture Texture::makeCube(
	int width,
	int height,
	GLenum internalFormat,
	void** data,
	GLenum format,
	GLenum dataType,
	GLenum minFilter,
	GLenum magFilter,
	GLenum wrapS,
	GLenum wrapT,
	GLenum wrapR
) {
	bool hasMipmaps = (minFilter & 0xFF00) == 0x2700;

	GLuint id;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);

	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrapS);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrapT);
	glTextureParameteri(id, GL_TEXTURE_WRAP_R, wrapR);

	glTextureStorage2D(id, hasMipmaps ? std::floor(std::log2(std::max(width, height))) + 1 : 1, internalFormat, width, height);
	if(data) {
		for (uint8_t face = 0; face < 6; face++) {
			glTextureSubImage3D(id, 0, 0, 0, face, width, height, 1, format, dataType, data[face]);
		}
		if (hasMipmaps) {
			glGenerateTextureMipmap(id);
			float anisotropy;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
			glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
		}
	}

	GLuint64 handle = glGetTextureHandleARB(id);
	glMakeTextureHandleResidentARB(handle);

	return Texture{ id, handle, internalFormat };
}

Texture& Texture::operator=(const Texture& src) {
	this->~Texture();
	new (this) Texture{ src };
	return *this;
}

Texture& Texture::operator=(Texture&& src) noexcept {
	this->~Texture();
	new (this) Texture{ std::move(src) };
	return *this;
}

Texture::~Texture() {
	if (m_rc.count() == 0) {
		glMakeTextureHandleNonResidentARB(m_handle);
		glDeleteTextures(1, &m_id);
	}
}

Texture::Texture(GLuint id, GLuint64 handle, GLenum internalFormat) : m_id{ id }, m_handle{ handle }, m_internalFormat{ internalFormat } {}