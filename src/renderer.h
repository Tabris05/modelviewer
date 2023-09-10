#ifndef RENDERER_H
#define RENDERER_H

#include "glwindow.h"
#include "model.h"
#include "screenquad.h"
#include "framebuffer.h"
#include "camera.h"
#include "skybox.h"
#include "shaderprogram.h"


#include <glm/glm.hpp>

class Renderer {
private:
	GLWindow m_window;
	Model m_model;

	ScreenQuad m_screenQuad;
	FrameBuffer m_msaaBuffer;
	FrameBuffer m_postprocessingBuffer;

	Camera m_camera;

	Skybox m_skybox{ "./assets/skybox/", { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" } };

	ShaderProgram m_modelShaderDepth{ "./shaders/modeldepth_vertex.glsl", "./shaders/empty_fragment.glsl" };
	ShaderProgram m_skyboxShaderDepth{ "./shaders/skybox_vertex.glsl", "./shaders/empty_fragment.glsl" };
	ShaderProgram m_modelShaderBlinnPhong{ "./shaders/model_vertex.glsl", "./shaders/blinnphong_fragment.glsl" };
	ShaderProgram m_modelShaderPBR{ "./shaders/model_vertex.glsl", "./shaders/pbr_fragment.glsl" };
	ShaderProgram m_skyboxShader{ "./shaders/skybox_vertex.glsl", "./shaders/skybox_fragment.glsl" };
	ShaderProgram m_postProcessingShader{ "./shaders/postprocessing_vertex.glsl", "./shaders/postprocessing_fragment.glsl" };

	const float m_sunlightIntensityBlinnPhong = 1.5f;
	const float m_sunlightIntensityPBR = 5.0f;
	const glm::vec3 m_lightDir{0.0f, 0.0f, 1.0f};
	const glm::vec3 m_lightColorBlinnPhong{glm::vec3(0.98f, 0.90f, 0.74f) * glm::vec3(m_sunlightIntensityBlinnPhong)};
	const glm::vec3 m_lightColorPBR{glm::vec3(0.98f, 0.90f, 0.74f) * glm::vec3(m_sunlightIntensityPBR)};

	float m_modelPitch = 0.0f, m_modelRoll = 0.0f, m_modelYaw = 0.0f, m_modelScale = 1.0f;
	bool m_pbrEnabled = true, m_vsyncEnabled = true;
	double m_lastFrame = 0.0, m_curFrame = 0.0;

	void beginFrame();
	void renderUI();
	void depthPass(glm::mat4 transform = glm::mat4(1.0f));
	void lightingPass(glm::mat4 transform = glm::mat4(1.0f));
	void endFrame();

public:
	Renderer(int width, int height, const char* modelPath);
	void run();
};

#endif