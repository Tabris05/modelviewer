#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec3 m_tangent;
	glm::vec2 m_texCoord;
};

#endif