#include "renderer.h"
#include <imGuIZMOquat/imGuIZMOquat.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "dbg.h"

void Renderer::run() {

	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		if (const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
			m_camera.handleInput(m_curFrame - m_lastFrame);
		}

		if (m_vsyncEnabled) glfwSwapInterval(1);
		else glfwSwapInterval(0);

		draw();

		m_lastFrame = m_curFrame;
		m_curFrame = glfwGetTime();

		//checkErr();
	}
}

Renderer Renderer::make() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

#ifdef NDEBUG
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_NO_ERROR);
#endif

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int width = mode->width * 3 / 4;
	int height = mode->height * 3 / 4;

	GLFWwindow* window = glfwCreateWindow(width, height, "Model Viewer", NULL, NULL);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	gladLoadGL();
	glViewport(0, 0, width, height);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");

	Camera camera = Camera::make(window, width, height);
	Shader modelShader = Shader::make("shaders/model.vert", "shaders/model.frag");
	Shader prepassShader = Shader::make("shaders/prepass.vert", "shaders/prepass.frag");
	Shader postprocessingShader = Shader::make("shaders/postprocessing.vert", "shaders/postprocessing.frag");

	FrameBuffer postprocessingBuffer = FrameBuffer::make();
	Texture postprocessingTarget = Texture::make2D(NULL, width, height, GL_RGB16F);
	postprocessingBuffer.attachTexture(postprocessingTarget, GL_COLOR_ATTACHMENT0);
	postprocessingShader.setUniform("inputTex", postprocessingTarget.makeBindless());

	FrameBuffer multisampledBuffer = FrameBuffer::make();
	RenderBuffer multisampledColorTarget = RenderBuffer::makeMultisampled(GL_RGB16F, width, height);
	RenderBuffer multisampledDepthTarget = RenderBuffer::makeMultisampled(GL_DEPTH_COMPONENT, width, height);
	multisampledBuffer.attachRenderBuffer(multisampledColorTarget, GL_COLOR_ATTACHMENT0);
	multisampledBuffer.attachRenderBuffer(multisampledDepthTarget, GL_DEPTH_ATTACHMENT);

	return Renderer{ 
		window,
		width,
		height,
		std::move(camera),
		std::move(modelShader),
		std::move(prepassShader),
		std::move(postprocessingShader),
		std::move(postprocessingBuffer),
		std::move(postprocessingTarget),
		std::move(multisampledBuffer),
		std::move(multisampledColorTarget),
		std::move(multisampledDepthTarget)
	};
}

Renderer::~Renderer() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Renderer::draw() {
	// begin frame
	float horizontalScale = m_width / 1920.0f, verticalScale = m_height / 1080.0f;
	if (m_model.has_value()) {
		glm::mat4 camMatrix = m_camera.getProjMatrix(m_fov / 2.0f, 0.1f, 100.0f) * m_camera.getViewMatrix();
		glm::mat4 modelMatrix = m_model->baseTransform() * glm::toMat4(m_modelRotation) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ m_modelScale / 100.0f });
		m_multisampledBuffer.bind();

		// depth pass
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		m_prepassShader.bind();
		m_prepassShader.setUniform("camMatrix", camMatrix);
		m_prepassShader.setUniform("modelMatrix", modelMatrix);
		m_model->draw();

		// color pass
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);
		m_modelShader.bind();
		m_modelShader.setUniform("camMatrix", camMatrix);
		m_modelShader.setUniform("modelMatrix", modelMatrix);
		m_modelShader.setUniform("camPos", m_camera.getPos());
		m_modelShader.setUniform("lightAngle", m_lightAngle);
		m_modelShader.setUniform("lightColor", m_lightColor);
		m_modelShader.setUniform("lightIntensity", m_lightIntensity);
		m_model->draw();

		// post processing pass
		glDisable(GL_DEPTH_TEST);
		m_multisampledBuffer.blitTo(m_postprocessingBuffer, GL_COLOR_BUFFER_BIT, m_width, m_height);
		m_multisampledBuffer.unbind();
		m_postprocessingShader.bind();
		m_postprocessingShader.setUniform("gamma", m_gamma);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	// UI pass
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	drawLightMenu(horizontalScale, verticalScale);
	drawModelMenu(horizontalScale, verticalScale);
	drawOptionsMenu(horizontalScale, verticalScale);
	drawAssetMenu(horizontalScale, verticalScale);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// end frame
	glfwSwapBuffers(m_window);
}

void Renderer::drawLightMenu(float horizontalScale, float verticalScale) {
	vgm::Vec3 inout{ -m_lightAngle.x, -m_lightAngle.y, -m_lightAngle.z };
	ImGui::Begin("Lighting");
	ImGui::SetWindowPos(ImVec2{ 0.0f, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 350.0f * horizontalScale, 375.0f * verticalScale });
	ImGui::PushItemWidth(200.0f * horizontalScale);
	ImGui::ColorPicker3("Light Color", &m_lightColor.x, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB);
	ImGui::SliderFloat("Light Intensity", &m_lightIntensity, 1.0f, 10.0f);
	ImGui::gizmo3D("Light Angle", inout, 100.0f * horizontalScale);
	ImGui::End();
	m_lightAngle = glm::vec3(-inout.x, -inout.y, -inout.z);
}

void Renderer::drawModelMenu(float horizontalScale, float verticalScale) {
	vgm::Quat inout{ m_modelRotation.w, m_modelRotation.x, m_modelRotation.y, m_modelRotation.z };
	ImGui::Begin("Model");
	ImGui::SetWindowPos(ImVec2{ 350.0f * horizontalScale, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 300.0f * horizontalScale, 200.0f * verticalScale });
	ImGui::SliderFloat("Model Scale", &m_modelScale, 20.0f, 500.0f);
	ImGui::gizmo3D("Model Rotation", inout, 125.0f * horizontalScale);
	ImGui::End();
	m_modelRotation = glm::quat(inout.w, inout.x, inout.y, inout.z);
}

void Renderer::drawOptionsMenu(float horizontalScale, float verticalScale) {
	ImGui::Begin("Options");
	ImGui::SetWindowPos(ImVec2{ 650.0f * horizontalScale, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 175.0f * horizontalScale, 175.0f * verticalScale });
	ImGui::Text("Graphics Settings");
	ImGui::SliderFloat("FOV", &m_fov, 60.0f, 120.0f);
	ImGui::SliderFloat("Gamma", &m_gamma, 1.0f, 4.4f);
	ImGui::Checkbox("VSync", &m_vsyncEnabled);
	ImGui::NewLine();
	ImGui::Text("Performance");
	ImGui::Text("%.0f FPS, %.2fms", 1.0 / (m_curFrame - m_lastFrame), 1000.0 * (m_curFrame - m_lastFrame));
	ImGui::End();
}

void Renderer::drawAssetMenu(float horizontalScale, float verticalScale) {
	ImGui::Begin("Load Assets");
	ImGui::SetWindowPos(ImVec2{ 825.0f * horizontalScale, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 500.0f * horizontalScale, 75.0f * verticalScale });
	ImGui::PushItemWidth(400.0f);
	ImGui::InputText("##modelload", &m_modelPath);
	ImGui::SameLine();
	if (ImGui::Button("Load model")) {
		std::filesystem::path modelPath{ m_modelPath };
		if (std::filesystem::exists(modelPath) && modelPath.extension() == ".gltf") {
			m_model = std::move(Model::make(modelPath));
		}
	}
	ImGui::End();
}

void Renderer::resizeWindow(int width, int height) {
	m_width = width;
	m_height = height;
	m_camera.updateSize(width, height);
	m_postprocessingTarget = Texture::make2D(NULL, width, height);
	m_postprocessingBuffer.attachTexture(m_postprocessingTarget, GL_COLOR_ATTACHMENT0);
	m_postprocessingShader.setUniform("inputTex", m_postprocessingTarget.makeBindless());
	m_multisampledColorTarget = RenderBuffer::makeMultisampled(GL_RGB16F, width, height);
	m_multisampledDepthTarget = RenderBuffer::makeMultisampled(GL_DEPTH_COMPONENT, width, height);
	m_multisampledBuffer.attachRenderBuffer(m_multisampledColorTarget, GL_COLOR_ATTACHMENT0); // this line causes invalid operation errors
	m_multisampledBuffer.attachRenderBuffer(m_multisampledDepthTarget, GL_DEPTH_ATTACHMENT);
	glViewport(0, 0, width, height);
	draw();
}

Renderer::Renderer(
	GLFWwindow* window,
	int width,
	int height,
	Camera camera,
	Shader modelShader,
	Shader prepassShader,
	Shader postprocessingShader,
	FrameBuffer postprocessingBuffer,
	Texture postprocessingTarget,
	FrameBuffer multisampledBuffer,
	RenderBuffer multisampledColorTarget,
	RenderBuffer multisampledDepthTarget
) :
	m_window{ window },
	m_width{ width },
	m_height{ height },
	m_camera{ std::move(camera) },
	m_modelShader{ std::move(modelShader) },
	m_prepassShader{ std::move(prepassShader) },
	m_postprocessingShader{ std::move(postprocessingShader) },
	m_postprocessingBuffer{ std::move(postprocessingBuffer) },
	m_postprocessingTarget{ std::move(postprocessingTarget) },
	m_multisampledBuffer{ std::move(multisampledBuffer) },
	m_multisampledColorTarget{ std::move(multisampledColorTarget) },
	m_multisampledDepthTarget{ std::move(multisampledDepthTarget) } {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(m_window, [] (GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

