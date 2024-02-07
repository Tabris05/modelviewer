#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

struct AABB {
	glm::vec3 m_min{ std::numeric_limits<float>::max() };
	glm::vec3 m_max{ std::numeric_limits<float>::min() };
};

#endif
