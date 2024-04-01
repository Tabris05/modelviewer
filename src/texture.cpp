#include "texture.h"
#include <cmath>

GLuint Texture::id() const {
	return m_id;
}

std::optional<GLuint64> Texture::handle() const {
	return m_handle;
}

void Texture::makeBindless() {
	GLuint64 handle = glGetTextureHandleARB(m_id);
	glMakeTextureHandleResidentARB(handle);
	m_handle = handle;
}

void Texture::generateMipMaps() {
	glGenerateTextureMipmap(m_id);
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

	return Texture{ id };
}

Texture Texture::make3D(
	int width,
	int height,
	int depth,
	GLenum internalFormat,
	void* data,
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
	glCreateTextures(GL_TEXTURE_3D, 1, &id);

	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrapS);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrapT);
	glTextureParameteri(id, GL_TEXTURE_WRAP_R, wrapR);

	glTextureStorage3D(id, hasMipmaps ? std::floor(std::log2(std::max(width, height))) + 1 : 1, internalFormat, width, height, depth);
	if (data) {
		glTextureSubImage3D(id, 0, 0, 0, 0, width, height, depth, format, dataType, data);
		if (hasMipmaps) {
			glGenerateTextureMipmap(id);
			float anisotropy;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
			glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
		}
	}

	return Texture{ id };
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

	return Texture{ id };
}

Texture Texture::make2DBindless(
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
	Texture ret = Texture::make2D(width, height, internalFormat, data, format, dataType, minFilter, magFilter, wrapS, wrapT);
	ret.makeBindless();

	return ret;
}

Texture Texture::make3DBindless(
	int width,
	int height,
	int depth,
	GLenum internalFormat,
	void* data,
	GLenum format,
	GLenum dataType,
	GLenum minFilter,
	GLenum magFilter,
	GLenum wrapS,
	GLenum wrapT,
	GLenum wrapR
) {
	Texture ret = Texture::make3D(width, height, depth, internalFormat, data, format, dataType, minFilter, magFilter, wrapS, wrapT, wrapR);
	ret.makeBindless();

	return ret;
}

Texture Texture::makeCubeBindless(
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
	Texture ret = Texture::makeCube(width, height, internalFormat, data, format, dataType, minFilter, magFilter, wrapS, wrapT, wrapR);
	ret.makeBindless();

	return ret;
}

Texture::~Texture() {
	if (m_rc.count() == 0) {
		if(m_handle.has_value()) glMakeTextureHandleNonResidentARB(m_handle.value());
		glDeleteTextures(1, &m_id);
	}
}

Texture::Texture(GLuint id) : m_id{ id } {}