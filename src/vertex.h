#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 m_position{ 0.0f };
	glm::vec3 m_normal{ 0.0f };
	glm::vec2 m_uv{ 0.0f };
};

#endif
