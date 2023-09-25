#include "shadowmapnoise.h"

#include <vector>
#include <random>
#include <numbers>

ShadowmapNoise::ShadowmapNoise(int windowSize, int filterSize) {
	std::vector<float> data;
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
	data.reserve(windowSize * windowSize * filterSize * filterSize * 2);
	for (int i = 0; i < windowSize * windowSize; i++) {
		for (int v = filterSize - 1; v >= 0; v--) {
			for (int u = 0; u < filterSize; u++) {
				float x = (u + 0.5f + distribution(generator)) / filterSize;
				float y = (v + 0.5f + distribution(generator)) / filterSize;
				data.push_back(std::sqrt(y) * std::cos(2 * std::numbers::pi_v<float> * x));
				data.push_back(std::sqrt(y) * std::sin(2 * std::numbers::pi_v<float> * x));
			}
		}
	}
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_texID);
	glBindTexture(GL_TEXTURE_3D, m_texID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RG32F, filterSize * filterSize, windowSize, windowSize, 0, GL_RG, GL_FLOAT, data.data());
}

ShadowmapNoise::~ShadowmapNoise() {
}

void ShadowmapNoise::bind() {
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_texID);
}

