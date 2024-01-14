#ifndef RENDERER_H
#define RENDERER_H

#include "glwindow.h"
#include "model.h"
#include "screenquad.h"
#include "multisampledframebuffer.h"
#include "framebuffer.h"
#include "camera.h"
#include "skybox.h"
#include "shadowmapnoise.h"
#include "shaderprogram.h"


#include <glm/glm.hpp>

class Renderer {
private:
	GLWindow m_window;
	Model m_model;

	ScreenQuad m_screenQuad;
	MultisampledFramebuffer m_msaaBuffer;
	Framebuffer m_shadowMapBuffer;
	Framebuffer m_postprocessingBuffer;

	Camera m_camera;

	float m_numMipLevels;
	Skybox m_skybox{ m_numMipLevels };

	ShadowmapNoise m_shadowNoise;

	ShaderProgram m_modelShaderDepth{ "./shaders/modeldepth_vertex.glsl", "./shaders/empty_fragment.glsl" };
	ShaderProgram m_skyboxShaderDepth{ "./shaders/skybox_vertex.glsl", "./shaders/empty_fragment.glsl" };
	ShaderProgram m_modelShaderPBR{ "./shaders/model_vertex.glsl", "./shaders/pbr_fragment.glsl" };
	ShaderProgram m_skyboxShader{ "./shaders/skybox_vertex.glsl", "./shaders/skybox_fragment.glsl" };
	ShaderProgram m_postProcessingShader{ "./shaders/postprocessing_vertex.glsl", "./shaders/postprocessing_fragment.glsl" };

	const int m_width, m_height;

	constexpr static int m_shadowNoiseWindowSize = 8;
	constexpr static int m_shadowNoiseFilterSize = 16;
	constexpr static int m_shadowMapResolution = 2048;
	constexpr static float m_fov = 45.0f;
	constexpr static float m_nearPlane = 0.1f;
	constexpr static float m_farPlane = 100.0f;
	constexpr static glm::vec3 m_lightDir{ 0.0f, 0.0f, 1.0f };
	constexpr static glm::vec3 m_lightColorPBR{ glm::vec3(0.98f, 0.90f, 0.74f) * 5.0f };

	float m_modelPitch = 0.0f, m_modelRoll = 0.0f, m_modelYaw = 0.0f, m_modelScale = 1.0f;
	bool m_iblEnabled = true, m_vsyncEnabled = true;
	double m_lastFrame = 0.0, m_curFrame = 0.0;

	void beginFrame();
	void renderUI();
	glm::mat4 calcLightMatrix(glm::mat4 translation);
	void shadowPass(glm::mat4 transform, glm::mat4 lightMatrix);
	void depthPass(glm::mat4 transform);
	void lightingPass(glm::mat4 transform, glm::mat4 lightMatrix);
	void postprocessingPass();
	void endFrame();

public:
	Renderer(int width, int height, const char* modelPath);
	void run();
};

#endif