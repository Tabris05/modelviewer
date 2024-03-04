#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>

struct AABB {
	glm::vec3 m_min{ std::numeric_limits<float>::max() };
	glm::vec3 m_max{ std::numeric_limits<float>::min() };
	AABB transform(glm::mat4 transform) {
		AABB result = *this;
		glm::vec3 vertices[8] = {
		glm::vec3(result.m_min.x, result.m_min.y, result.m_min.z),
		glm::vec3(result.m_max.x, result.m_min.y, result.m_min.z),
		glm::vec3(result.m_max.x, result.m_max.y, result.m_min.z),
		glm::vec3(result.m_min.x, result.m_max.y, result.m_min.z),
		glm::vec3(result.m_min.x, result.m_min.y, result.m_max.z),
		glm::vec3(result.m_max.x, result.m_min.y, result.m_max.z),
		glm::vec3(result.m_max.x, result.m_max.y, result.m_max.z),
		glm::vec3(result.m_min.x, result.m_max.y, result.m_max.z),
		};
		for (auto& i : vertices) i = glm::vec3(transform * glm::vec4(i, 1.0f));
		result = AABB{};
		for (auto i : vertices) {
			result.m_min = glm::min(result.m_min, i);
			result.m_max = glm::max(result.m_max, i);
		}
		return result;
	}
};

#endif
