#include "renderer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "dbg.h"

// Texture slots
// slot 0: postprocessing framebuffer
// slot 1: shadow map
// slot 2: pcf noise
// slot 3: skybox cubemap
// slot 4: irradiance cubemap
// slot 5: prefiltered environment map
// slot 6: BRDF integration map
// slot 7+: model textures

Renderer::Renderer(int width, int height, const char* modelPath) :
	m_width { width },
	m_height { height },
	m_window{ width, height, "Model Viewer"},
	m_model{ modelPath },
	m_screenQuad{ width, height },
	m_msaaBuffer{ width, height },
	m_shadowMapBuffer { m_shadowMapResolution, m_shadowMapResolution},
	m_shadowNoise{ m_shadowNoiseWindowSize, m_shadowNoiseFilterSize },
	m_postprocessingBuffer{ width, height },
	m_camera{ width, height, glm::vec3(-0.75f, 1.0f, -1.5f), glm::vec3(0.5f, -0.5f, 1.0f)} {
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	m_modelShaderPBR.activate();
	m_modelShaderPBR.setUniform("sampleRadius", 4.0f);
	m_modelShaderPBR.setUniform("shadowMapTexID", 1);
	m_modelShaderPBR.setUniform("shadowNoiseTexID", 2);
	m_modelShaderPBR.setUniform("irradianceTexID", 4);
	m_modelShaderPBR.setUniform("prefilterTexID", 5);
	m_modelShaderPBR.setUniform("brdfLUTTexID", 6);
	m_modelShaderPBR.setUniform("lightDir", m_lightDir);
	m_modelShaderPBR.setUniform("lightCol", m_lightColorPBR);
	m_modelShaderPBR.setUniform("texelSize", 1.0f / m_shadowMapResolution);
	m_modelShaderPBR.setUniform("shadowNoiseWindowSize", m_shadowNoiseWindowSize);
	m_modelShaderPBR.setUniform("shadowNoiseFilterSize", m_shadowNoiseFilterSize);
	m_modelShaderPBR.setUniform("maxLOD", m_numMipLevels - 1.0f);

	m_skyboxShader.activate();
	m_skyboxShader.setUniform("skybox", 3);

	m_postProcessingShader.activate();
	m_postProcessingShader.setUniform("fTexID", 0);
	m_postProcessingShader.setUniform("fGamma", 2.2f);
	m_postProcessingShader.setUniform("fWhiteLevel", 5.0f);

	m_msaaBuffer.attatchTexture(GL_COLOR_ATTACHMENT0, GL_RGB16F);
	m_msaaBuffer.attatchRenderBuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT);

	m_shadowMapBuffer.attatchTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

	m_postprocessingBuffer.attatchTexture(GL_COLOR_ATTACHMENT0, GL_RGB16F);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
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

		glm::mat4 lightMatrix = calcLightMatrix(transform);

		shadowPass(transform, lightMatrix);
		depthPass(transform);
		lightingPass(transform, lightMatrix);
		postprocessingPass();

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
	ImGui::Checkbox("Specular AA", &m_specAA);
	ImGui::SameLine();
	ImGui::Checkbox("VSync", &m_vsyncEnabled);
	ImGui::Text("Performance");
	ImGui::Text("%.0f FPS, %.2fms", 1.0 / (m_curFrame - m_lastFrame), 1000.0 * (m_curFrame - m_lastFrame));
	ImGui::End();
}

glm::mat4 Renderer::calcLightMatrix(glm::mat4 transform) {
	aabb modelAABB = m_model.getAABB();
	glm::vec3 vertices[8] = {
		glm::vec3(modelAABB.m_min.x, modelAABB.m_min.y, modelAABB.m_min.z),
		glm::vec3(modelAABB.m_max.x, modelAABB.m_min.y, modelAABB.m_min.z),
		glm::vec3(modelAABB.m_max.x, modelAABB.m_max.y, modelAABB.m_min.z),
		glm::vec3(modelAABB.m_min.x, modelAABB.m_max.y, modelAABB.m_min.z),
		glm::vec3(modelAABB.m_min.x, modelAABB.m_min.y, modelAABB.m_max.z),
		glm::vec3(modelAABB.m_max.x, modelAABB.m_min.y, modelAABB.m_max.z),
		glm::vec3(modelAABB.m_max.x, modelAABB.m_max.y, modelAABB.m_max.z),
		glm::vec3(modelAABB.m_min.x, modelAABB.m_max.y, modelAABB.m_max.z),
	};
	for (auto& i : vertices) i = glm::vec3(transform * glm::vec4(i, 1.0f));
	modelAABB = { glm::vec3(FLT_MAX), glm::vec3(FLT_MIN) };
	for (auto i : vertices) {
		modelAABB.m_min = glm::min(modelAABB.m_min, i);
		modelAABB.m_max = glm::max(modelAABB.m_max, i);
	}
	return glm::ortho(
		-modelAABB.m_max.x,
		-modelAABB.m_min.x,
		modelAABB.m_min.y,
		modelAABB.m_max.y,
		modelAABB.m_min.z,
		modelAABB.m_max.z
	) * glm::lookAt(glm::vec3(0.0f), m_lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Renderer::shadowPass(glm::mat4 transform, glm::mat4 lightMatrix) {
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	m_modelShaderDepth.activate();
	m_modelShaderDepth.setUniform("camMatrix", lightMatrix);
	glViewport(0, 0, m_shadowMapResolution, m_shadowMapResolution);
	m_shadowMapBuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m_model.draw(m_modelShaderDepth, transform);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::depthPass(glm::mat4 transform) {
	m_modelShaderDepth.setUniform("camMatrix", m_camera.getProjMatrix(m_fov, m_nearPlane, m_farPlane) * m_camera.getViewMatrix());
	m_msaaBuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m_model.draw(m_modelShaderDepth, transform);
	glDepthFunc(GL_LEQUAL);
	m_skyboxShaderDepth.activate();
	m_skyboxShaderDepth.setUniform("camMatrix", m_camera.getProjMatrix(m_fov, m_nearPlane, m_farPlane) * glm::mat4(glm::mat3(m_camera.getViewMatrix())));
	m_skybox.draw(m_skyboxShaderDepth);
	glDepthMask(GL_FALSE);
}

void Renderer::lightingPass(glm::mat4 transform, glm::mat4 lightMatrix) {
	
	m_shadowMapBuffer.bindTexture(0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_EQUAL);
	m_modelShaderPBR.activate();
	m_modelShaderPBR.setUniform("lightMatrix", lightMatrix);
	m_modelShaderPBR.setUniform("camMatrix", m_camera.getProjMatrix(m_fov, m_nearPlane, m_farPlane) * m_camera.getViewMatrix());
	m_modelShaderPBR.setUniform("camPos", m_camera.getPos());
	m_modelShaderPBR.setUniform("specAA", (int)m_specAA);
	m_model.draw(m_modelShaderPBR, transform);
	m_skyboxShader.activate();
	m_skyboxShader.setUniform("camMatrix", m_camera.getProjMatrix(m_fov, m_nearPlane, m_farPlane) * glm::mat4(glm::mat3(m_camera.getViewMatrix())));
	m_skybox.draw(m_skyboxShader);
}

void Renderer::postprocessingPass() {
	glDisable(GL_CULL_FACE);
	m_msaaBuffer.blitTo(m_postprocessingBuffer, GL_COLOR_BUFFER_BIT);
	m_postprocessingBuffer.unbind();
	m_postprocessingBuffer.bindTexture(0, 0);
	m_postProcessingShader.activate();
	m_screenQuad.draw(m_postProcessingShader);
	glEnable(GL_CULL_FACE);
}

void Renderer::endFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	m_window.swapBuffers();
}

