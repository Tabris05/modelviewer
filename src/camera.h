#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "glwindow.h"

class Camera {
private:
	glm::vec3 m_pos;
	glm::vec3 m_rot;
	constexpr static glm::vec3 m_upDir{ 0.0f, 1.0f, 0.0f };

	int m_width;
	int m_height;

	float m_speed = 1.44f;
	float m_sensitivity = 100.0f;

	bool m_firstClick = true;
public:
	Camera(int width, int height, glm::vec3 position, glm::vec3 rotation);

	glm::mat4 getProjMatrix(float fovDeg, float nearPlane, float farPlane) const;
	glm::mat4 getViewMatrix() const;
	glm::vec3 getPos() const;
	void handleInput(GLWindow& window, double deltaTime);
};

#endif