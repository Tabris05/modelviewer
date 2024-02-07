#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "model.h"
#include "shader.h"


class Renderer {
	public:
		void run();

		static Renderer make();
		~Renderer();

	private:
		void draw();
		void resizeWindow(int width, int height);

		Renderer(GLFWwindow* window, int width, int height, Camera camera, Model model, Shader modelShader);
		
		Camera m_camera;
		Model m_model;
		Shader m_modelShader;
		GLFWwindow* m_window;
		int m_width, m_height;
		double m_curFrame = 0.0f, m_lastFrame = 0.0f;
};

#endif
