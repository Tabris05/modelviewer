#include "renderer.h"

Renderer Renderer::make() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int width = mode->width * 3 / 4;
	int height = mode->height * 3 / 4;

	GLFWwindow* window = glfwCreateWindow(width, height, "Model Viewer", NULL, NULL);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwMakeContextCurrent(window);
	return Renderer{ window, width, height };
}

void Renderer::run() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
	}
}

Renderer::Renderer(GLFWwindow* window, int width, int height) : m_window{ window }, m_width{ width }, m_height{ height } {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

void Renderer::resizeWindow(int width, int height) {
	m_width = width;
	m_height = height;
}

Renderer::~Renderer() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}