#ifndef RENDERER_H
#define RENDERER_H

#define GLM_ENABLE_EXPERIMENTAL 1
#include <optional>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
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
		void drawLightMenu(float horizontalScale, float verticalScale);
		void drawModelMenu(float horizontalScale, float verticalScale);
		void drawOptionsMenu(float horizontalScale, float verticalScale);
		void drawAssetMenu(float horizontalScale, float verticalScale);
		void resizeWindow(int width, int height);

		Renderer(
			GLFWwindow* window,
			int width,
			int height,
			Camera camera,
			Shader modelShader,
			Shader prepassShader,
			Shader postprocessingShader,
			FrameBuffer shadowmapBuffer,
			FrameBuffer multisampledBuffer,
			FrameBuffer postprocessingBuffer,
			RenderBuffer multisampledColorTarget,
			RenderBuffer multisampledDepthTarget,
			Texture shadowmapTarget,
			Texture postprocessingTarget
		);
		
		Camera m_camera;
		std::optional<Model> m_model;
		Shader m_modelShader;
		Shader m_depthShader;
		Shader m_postprocessingShader;
		FrameBuffer m_shadowmapBuffer;
		FrameBuffer m_multisampledBuffer;
		FrameBuffer m_postprocessingBuffer;
		RenderBuffer m_multisampledColorTarget;
		RenderBuffer m_multisampledDepthTarget;
		Texture m_shadowmapTarget;
		Texture m_postprocessingTarget;
		GLFWwindow* m_window;
		int m_width, m_height;
		double m_curFrame = 0.0f, m_lastFrame = 0.0f;

		std::string m_modelPath;
		glm::vec3 m_lightAngle{ 0.0f, 0.0f, -1.0f }, m_lightColor{ 0.98f, 0.90f, 0.74f };
		float m_lightIntensity = 5.0f, m_fov = 90.0f, m_gamma = 2.2f, m_modelScale = 100.0f;
		glm::quat m_modelRotation{ glm::mat4{ 1.0f } };
		bool m_vsyncEnabled = true;

		constexpr static inline int m_shadowmapResolution = 2048;
};

#endif
