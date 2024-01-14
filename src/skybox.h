#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <array>

#include "shaderprogram.h"
#include "camera.h"
#include "vertexarray.h"
#include "vertexbuffer.h"

using GLid = GLuint;

class Skybox {
private:
	GLid m_skyboxTexID, m_irradianceTexID, m_prefilterTexID, m_brdfLUTTexID;
	VertexArray m_vArr;
	VertexBuffer m_vBuf;
public:
	Skybox(float& outNumMiplevels);
	~Skybox();
	void draw(ShaderProgram& shaderProgram);
};

#endif

