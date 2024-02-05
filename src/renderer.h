#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "model.h"

class Renderer {
	public:
		void run();

		static Renderer make();
		~Renderer();

	private:
		void draw();
		void resizeWindow(int width, int height);

		Renderer(GLFWwindow* window, int width, int height, Model m_model);
		
		Model m_model;
		GLFWwindow* m_window;
		int m_width, m_height;
};

#endif
