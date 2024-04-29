#ifndef RENDERER_H
#define RENDERER_H

#define GLM_ENABLE_EXPERIMENTAL 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "camera.h"
#include "model.h"
#include "shader.h"
#include "texture.h"
#include "skybox.h"
#include "framebuffer.h"
#include "renderbuffer.h"

class Renderer {
	public:
		void run();

		static Renderer make();
		~Renderer();

	private:
		struct ConstructorData {
			GLFWwindow* window;
			int width, height;
			Model model;
			Skybox skybox;
			Camera camera;
			Shader ssaoShader;
			Shader ssaoBlurShader;
			Shader modelShader;
			Shader depthShader;
			Shader skyboxShader;
			Shader postprocessingShader;
			FrameBuffer shadowmapBuffer;
			FrameBuffer multisampledBuffer;
			FrameBuffer resolvedBuffer;
			RenderBuffer multisampledColorTarget;
			RenderBuffer multisampledDepthTarget;
			Texture brdfLUT;
			Texture shadowmapTarget;
			Texture ssaoTarget;
			Texture ssaoBlurTarget;
			Texture resolvedColorTarget;
			Texture resolvedDepthTarget;
			ShaderStorageBuffer poissonDisks;
			ShaderStorageBuffer ssaoNoise;
		} m;

		double m_curFrame = 0.0, m_lastFrame = 0.0;

		size_t m_framesThisSecond = 0;
		double m_lastSecond = 0.0;
		double m_fpsLastSecond = 0.0;

		std::string m_modelPath, m_skyboxPath;
		glm::vec3 m_lightAngle{ 0.0f, 0.0f, -1.0f }, m_lightColor{ 0.98f, 0.90f, 0.74f };
		glm::quat m_modelRotation{ glm::mat4{ 1.0f } };
		float m_lightIntensity = 5.0f, m_fov = 90.0f, m_gamma = 2.2f, m_modelScale = 100.0f;
		bool m_vsyncEnabled = true;

		constexpr static inline int m_brdfLUTSize = 512;
		

		// if anything is modified here, a modification must also be made to the macro definitions in model.frag
		constexpr static inline int m_shadowmapResolution = 2048;
		constexpr static inline int m_poissonDiskWindowSize = 8;
		constexpr static inline int m_poissonDiskFilterSize = 16;

		// if anything is modified here, a modification must also be made to the macro definitions in ssao.frag and ssaoblur.comp
		constexpr static inline int m_kernelSize = 64;
		constexpr static inline int m_ssaoNoiseSize = 4;

		void draw();
		void drawLightMenu(float horizontalScale, float verticalScale);
		void drawModelMenu(float horizontalScale, float verticalScale);
		void drawOptionsMenu(float horizontalScale, float verticalScale);
		void drawAssetMenu(float horizontalScale, float verticalScale);
		void resizeWindow(int width, int height);

		glm::mat4 calcLightMatrix(glm::mat4 modelMatrix);
		static ShaderStorageBuffer makeShadowmapNoise(int windowSize, int filterSize);
		static ShaderStorageBuffer makeSSAONoise();
		static std::array<glm::vec3, m_kernelSize> makeSSAOKernel();
		

		Renderer(ConstructorData data);
};

#endif
