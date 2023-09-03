#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class GLWindow {
private:
	GLFWwindow* m_window;
public:
	GLWindow(int width, int height, const char* title);
	~GLWindow();
	operator GLFWwindow*() const;

	void swapBuffers();
	void setTitle(const char* title);
	bool shouldClose();
};

#endif

