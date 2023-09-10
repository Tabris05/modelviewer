#include "renderer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "dbg.h"

Renderer::Renderer(int width, int height, const char* modelPath) :
	m_window{ width, height, "Model Viewer"},
	m_model{modelPath},
	m_screenQuad{width, height},
	m_postprocessingBuffer{width, height},
	m_msaaBuffer{width, height},
	m_camera{width, height, glm::vec3(-0.75f, 1.0f, -1.5f), glm::vec3(0.5f, -0.5f, 1.0f) } {
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	m_modelShaderBlinnPhong.setUniform("lightDir", m_lightDir);
	m_modelShaderBlinnPhong.setUniform("lightCol", m_lightColorBlinnPhong);

	m_modelShaderPBR.setUniform("lightDir", m_lightDir);
	m_modelShaderPBR.setUniform("lightCol", m_lightColorPBR);

	m_skyboxShader.setUniform("fSunlightIntensity", m_sunlightIntensityPBR);

	m_postProcessingShader.setUniform("fGamma", 2.2f);
	m_postProcessingShader.setUniform("fSunlightIntensity", m_sunlightIntensityPBR);

	m_msaaBuffer.attatchTexture(GL_COLOR_ATTACHMENT0, GL_RGB16F, true);
	m_msaaBuffer.attatchRenderBuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, true);

	m_postprocessingBuffer.attatchTexture(GL_COLOR_ATTACHMENT0, GL_RGB16F);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
}

void Renderer::run() {
	while (!m_window.shouldClose()) {
		beginFrame();
		renderUI();
		m_camera.handleInput(m_window, m_curFrame - m_lastFrame);

		m_lastFrame = m_curFrame;
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::rotate(transform, glm::radians(m_modelPitch), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(m_modelYaw), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(m_modelRoll), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(m_modelScale));

		m_msaaBuffer.bind();
		depthPass(transform);
		lightingPass(transform);
		m_msaaBuffer.blitTo(m_postprocessingBuffer, GL_COLOR_BUFFER_BIT);
		m_postprocessingBuffer.unbind();
		m_postprocessingBuffer.bindTexture(0, 0);
		m_screenQuad.draw(m_postProcessingShader);

		endFrame();
		m_curFrame = glfwGetTime();
		checkErr();
		
	}
}

void Renderer::beginFrame() {
	if (m_vsyncEnabled) glfwSwapInterval(1);
	else glfwSwapInterval(0);
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Renderer::renderUI() {
	ImGui::Begin("Options");
	ImGui::SetWindowSize(ImVec2{ 300.0f, 225.0f });
	ImGui::Text("Transform");
	ImGui::SliderFloat("Pitch", &m_modelPitch, -360.0f, 360.0f);
	ImGui::SliderFloat("Yaw", &m_modelYaw, -360.0f, 360.0f);
	ImGui::SliderFloat("Roll", &m_modelRoll, -360.0f, 360.0f);
	ImGui::SliderFloat("Scale", &m_modelScale, 0.01f, 100.0f);
	ImGui::Text("Graphics Settings");
	ImGui::Checkbox("PBR", &m_pbrEnabled);
	ImGui::SameLine();
	ImGui::Checkbox("VSync", &m_vsyncEnabled);
	ImGui::Text("Performance");
	ImGui::Text("%.0f FPS, %.2fms", 1.0 / (m_curFrame - m_lastFrame), 1000.0 * (m_curFrame - m_lastFrame));
	ImGui::End();
}

void Renderer::depthPass(glm::mat4 transform) {
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glClear(GL_DEPTH_BUFFER_BIT);
	m_model.draw(m_modelShaderDepth, m_camera, transform);
	glDepthFunc(GL_LEQUAL);
	m_skybox.draw(m_skyboxShaderDepth, m_camera);
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);
}

void Renderer::lightingPass(glm::mat4 transform) {
	glClear(GL_COLOR_BUFFER_BIT);
	m_model.draw(m_pbrEnabled ? m_modelShaderPBR : m_modelShaderBlinnPhong, m_camera, transform);
	m_skybox.draw(m_skyboxShader, m_camera);
}

void Renderer::endFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	m_window.swapBuffers();
}

