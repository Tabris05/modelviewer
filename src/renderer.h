#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include "GLFW/glfw3.h"

class Renderer {
	public:
		void run();

		static Renderer make();
		~Renderer();

	private:
		Renderer(GLFWwindow* window, int width, int height);

		void draw();
		void resizeWindow(int width, int height);
		

		GLFWwindow* m_window;
		int m_width, m_height;
};

#endif
