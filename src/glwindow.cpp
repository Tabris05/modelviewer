#include "glwindow.h"

GLWindow::GLWindow(int width, int height, const char* title) {
	// init OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	// init window
	m_window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwMakeContextCurrent(m_window);
	gladLoadGL(); // must load glad after creating context or else null function pointers :(
	glViewport(0, 0, width, height);
}

GLWindow::~GLWindow() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

GLWindow::operator GLFWwindow*() const {
	return m_window;
}

void GLWindow::swapBuffers() {
	glfwSwapBuffers(m_window);
}

void GLWindow::setTitle(const char* title) {
	glfwSetWindowTitle(m_window, title);
}

bool GLWindow::shouldClose() {
	return glfwWindowShouldClose(m_window);
}
