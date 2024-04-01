#ifndef SKYBOX_H
#define SKYBOX_H

#include <filesystem>
#include "texture.h"

class Skybox {
	public:
		GLuint64 skyboxTexHandle() const;
		GLuint64 irradianceTexHandle() const;
		GLuint64 envmapTexHandle() const;

		static Skybox make();
		static Skybox make(const std::filesystem::path& path);
	private:
		Skybox(Texture skyboxTex, Texture irradianceTex, Texture envmapTex);

		Texture m_skyboxTex;
		Texture m_irradianceTex;
		Texture m_envmapTex;
};

#endif
