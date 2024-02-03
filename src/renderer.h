#ifndef RENDERER_H
#define RENDERER_H

#include "GLFW/glfw3.h"

class Renderer {
	public:
		static Renderer make();
		void run();
		~Renderer();
	private:
		Renderer(GLFWwindow* window, int width, int height);
		void resizeWindow(int width, int height);

		GLFWwindow* m_window;
		int m_width, m_height;
};

#endif
