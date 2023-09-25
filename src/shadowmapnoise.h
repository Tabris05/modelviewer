#ifndef SHADOWNOISE_H
#define SHADOWNOISE_H

#include <glad/glad.h>

using GLid = GLuint;

class ShadowmapNoise {
private:
	GLid m_texID;
public:
	ShadowmapNoise(int windowSize, int filterSize);
	~ShadowmapNoise();

	void bind();
};

#endif