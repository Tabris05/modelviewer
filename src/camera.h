#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
	public:
		glm::mat4 getProjMatrix(float fovDeg, float nearPlane, float farPlane) const;
		glm::mat4 getViewMatrix() const;
		void handleInput(float deltaTime);
		void updateSize(int width, int height);

		static Camera make(GLFWwindow* window, int width, int height);

	private:
		Camera(GLFWwindow* window, int width, int height);

		GLFWwindow* m_window;
		int m_width, m_height;
		bool m_firstClick = true;
		float m_speed = 1.44f, m_sensitivity = 100.0f;
		glm::vec3 m_position{ -0.75f, 1.0f, -1.5f }, m_rotation{ 0.5f, -0.5f, 1.0f };
};

#endif