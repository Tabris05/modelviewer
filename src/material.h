#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>

struct Material {
	glm::vec4 m_baseColor;
	glm::vec4 m_metallicRoughness; // std430 moment
	GLuint64 m_albedoHandle;
	GLuint64 m_normalHandle;
	GLuint64 m_occlusionHandle;
	GLuint64 m_metallicRoughnessHandle;
};

#endif

