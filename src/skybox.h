#ifndef SKYBOX_H
#define SKYBOX_H

#include "texture.h"

class Skybox {
	public:
		static Skybox make(const char* path);
	private:
		Skybox(Texture skyboxTex);

		Texture m_skyboxTex;
};

#endif
