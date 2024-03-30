#ifndef SKYBOX_H
#define SKYBOX_H

#include <filesystem>
#include "texture.h"

class Skybox {
	public:
		GLuint64 skyboxTexHandle() const;

		static Skybox make();
		static Skybox make(const std::filesystem::path& path);
	private:
		Skybox(Texture skyboxTex);

		Texture m_skyboxTex;
};

#endif
