#include "camera.h"
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

glm::mat4 Camera::getProjMatrix(float fovDeg, float nearPlane, float farPlane) const {
	return glm::perspective(glm::radians(fovDeg), (float)m_width / (float)m_height, nearPlane, farPlane);
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(m_position, m_position + m_rotation, glm::vec3{ 0.0f, 1.0f, 0.0f });
}

glm::vec3 Camera::getPos() const {
	return m_position;
}

void Camera::handleInput(float deltaTime) {
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
		m_position += m_speed * m_rotation * glm::vec3{ deltaTime };
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
		m_position -= m_speed * m_rotation * glm::vec3{ deltaTime };
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
		m_position += m_speed * glm::normalize(glm::cross(m_rotation, glm::vec3{ 0.0f, 1.0f, 0.0f })) * glm::vec3{ deltaTime };
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		m_position -= m_speed * glm::normalize(glm::cross(m_rotation, glm::vec3{ 0.0f, 1.0f, 0.0f })) * glm::vec3{ deltaTime };
	}
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		m_position += m_speed * glm::vec3{ 0.0f, 1.0f, 0.0f } * glm::vec3{ deltaTime };
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		m_position -= m_speed * glm::vec3{ 0.0f, 1.0f, 0.0f } * glm::vec3{ deltaTime };
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		m_speed = 5.76;
	}
	else if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		m_speed = 1.44f;
	}

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (m_firstClick) {
			m_firstClick = false;
			glfwSetCursorPos(m_window, m_width / 2, m_height / 2);
		}

		double mouseX, mouseY;
		glfwGetCursorPos(m_window, &mouseX, &mouseY);

		float rotX = m_sensitivity * (float)(mouseX - (m_width / 2)) / m_width;
		float rotY = m_sensitivity * (float)(mouseY - (m_height / 2)) / m_height;

		m_rotation = glm::rotate(m_rotation, glm::radians(-rotX), glm::vec3{ 0.0f, 1.0f, 0.0f });
		glm::vec3 newRot = glm::rotate(m_rotation, glm::radians(-rotY), glm::normalize(glm::cross(m_rotation, glm::vec3{ 0.0f, 1.0f, 0.0f })));
		if (!(glm::angle(newRot, glm::vec3{ 0.0f, 1.0f, 0.0f }) <= glm::radians(1.0f) || glm::angle(newRot, glm::vec3{ 0.0f, -1.0f, 0.0f }) <= glm::radians(1.0f))) {
			m_rotation = newRot;
		}
		glfwSetCursorPos(m_window, m_width / 2, m_height / 2);
	}
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		m_firstClick = true;
	}
}

void Camera::updateSize(int width, int height) {
	m_width = width;
	m_height = height;
}

Camera Camera::make(GLFWwindow* window, int width, int height) {
	return Camera{ window, width, height };
}

Camera::Camera(GLFWwindow* window, int width, int height) : m_window{ window }, m_width{ width }, m_height{ height } {}