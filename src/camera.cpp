#include "camera.h"

Camera::Camera(int width, int height, glm::vec3 position, glm::vec3 rotation) : m_width{ width }, m_height{ height }, m_pos{ position }, m_rot{ rotation } {}

glm::mat4 Camera::getProjMatrix(float fovDeg, float nearPlane, float farPlane) const {
	return glm::perspective(glm::radians(fovDeg), (float)m_width / (float)m_height, nearPlane, farPlane);
}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(m_pos, m_pos + m_rot, m_upDir);
}

glm::vec3 Camera::getPos() const {
	return m_pos;
}

void Camera::handleInput(GLWindow& window, double deltaTime) {
	// kb movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		m_pos += m_speed * m_rot * glm::vec3(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		m_pos -= m_speed * m_rot * glm::vec3(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		m_pos += m_speed * glm::normalize(glm::cross(m_rot, m_upDir)) * glm::vec3(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		m_pos -= m_speed * glm::normalize(glm::cross(m_rot, m_upDir)) * glm::vec3(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		m_pos += m_speed * m_upDir * glm::vec3(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		m_pos -= m_speed * m_upDir * glm::vec3(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		m_speed = 5.76;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		m_speed = 1.44f;
	}

	// mouselook
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (m_firstClick) {
			m_firstClick = false;
			glfwSetCursorPos(window, m_width / 2, m_height / 2);
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = m_sensitivity * (float)(mouseX - (m_width / 2)) / m_width;
		float rotY = m_sensitivity * (float)(mouseY - (m_height / 2)) / m_height;

		m_rot = glm::rotate(m_rot, glm::radians(-rotX), m_upDir);
		glm::vec3 newRot = glm::rotate(m_rot, glm::radians(-rotY), glm::normalize(glm::cross(m_rot, m_upDir)));
		if (!(glm::angle(newRot, m_upDir) <= glm::radians(1.0f) || glm::angle(newRot, -m_upDir) <= glm::radians(1.0f))) {
			m_rot = newRot;
		}
		glfwSetCursorPos(window, m_width / 2, m_height / 2);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		m_firstClick = true;
	}
}