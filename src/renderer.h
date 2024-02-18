#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "model.h"
#include "shader.h"
#include "texture.h"
#include "framebuffer.h"
#include "renderbuffer.h"

class Renderer {
	public:
		void run();

		static Renderer make();
		~Renderer();

	private:
		void draw();
		void resizeWindow(int width, int height);

		Renderer(
			GLFWwindow* window,
			int width,
			int height,
			Camera camera,
			Model model,
			Shader modelShader,
			Shader prepassShader,
			Shader postprocessingShader,
			FrameBuffer postprocessingBuffer,
			Texture postprocessingTarget,
			FrameBuffer multisampledBuffer,
			RenderBuffer multisampledColorTarget,
			RenderBuffer multisampledDepthTarget
		);
		
		Camera m_camera;
		Model m_model;
		Shader m_modelShader;
		Shader m_prepassShader;
		Shader m_postprocessingShader;
		FrameBuffer m_postprocessingBuffer;
		Texture m_postprocessingTarget;
		FrameBuffer m_multisampledBuffer;
		RenderBuffer m_multisampledColorTarget;
		RenderBuffer m_multisampledDepthTarget;
		GLFWwindow* m_window;
		int m_width, m_height;
		double m_curFrame = 0.0f, m_lastFrame = 0.0f;
};

#endif
