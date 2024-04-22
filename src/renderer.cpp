#include "renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <imGuIZMOquat/imGuIZMOquat.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <stb/stb_image.h>
#include <numbers>
#include <random>
#include "dbg.h"

void Renderer::run() {
	while (!glfwWindowShouldClose(m.window)) {
		glfwPollEvents();
		if (const ImGuiIO& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
			m.camera.handleInput(m_curFrame - m_lastFrame);
		}

		if (m_vsyncEnabled) glfwSwapInterval(1);
		else glfwSwapInterval(0);

		draw();

		m_lastFrame = m_curFrame;
		m_curFrame = glfwGetTime();

		m_framesThisSecond++;
		if (m_curFrame - m_lastSecond >= 1.0) {
			m_fpsLastSecond = m_framesThisSecond / (m_curFrame - m_lastSecond);
			m_lastSecond = m_curFrame;
			m_framesThisSecond = 0;
		}
	}
}

Renderer Renderer::make() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int width = mode->width * 3 / 4;
	int height = mode->height * 3 / 4;

	GLFWwindow* window = glfwCreateWindow(width, height, "Model Viewer", NULL, NULL);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
	glViewport(0, 0, width, height);

#ifdef NDEBUG
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_NO_ERROR);
#else
	installDbgCallback();
#endif

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);

	glDepthFunc(GL_GEQUAL); // reverse z
	glClearDepth(0.0f);
	glPolygonOffset(-10.0f, -1.0f);
	glCullFace(GL_BACK);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");


	Model model = Model::make();
	Skybox skybox = Skybox::make();

	Camera camera = Camera::make(window, width, height);

	Shader ssaoBlurShader = Shader::makeCompute("shaders/ssaoblur.comp");
	Shader ssaoShader = Shader::makeGraphics("shaders/screenquad.vert", "shaders/ssao.frag");
	Shader modelShader = Shader::makeGraphics("shaders/model.vert", "shaders/model.frag");
	Shader depthShader = Shader::makeGraphics("shaders/depth.vert", "shaders/depth.frag");
	Shader skyboxShader = Shader::makeGraphics("shaders/cubemap.vert", "shaders/cubemap.frag");
	Shader postprocessingShader = Shader::makeGraphics("shaders/screenquad.vert", "shaders/postprocessing.frag");

	FrameBuffer shadowmapBuffer = FrameBuffer::make();
	Texture shadowmapTarget = Texture::make2D(m_shadowmapResolution, m_shadowmapResolution, GL_DEPTH_COMPONENT32);
	shadowmapBuffer.attachTexture(shadowmapTarget, GL_DEPTH_ATTACHMENT);
	modelShader.setUniform("shadowmapTex", shadowmapTarget.handle());

	FrameBuffer ssaoBuffer = FrameBuffer::make();
	Texture ssaoTarget = Texture::make2D(width / 2, height / 2, GL_R8);
	ssaoBuffer.attachTexture(ssaoTarget, GL_COLOR_ATTACHMENT0);

	Texture ssaoBlurTarget = Texture::make2D(width / 2, height / 2, GL_R8, nullptr, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR);
	modelShader.setUniform("ssaoTex", ssaoBlurTarget.handle());

	FrameBuffer multisampledBuffer = FrameBuffer::make();
	RenderBuffer multisampledColorTarget = RenderBuffer::makeMultisampled(width, height, GL_RGB16F);
	RenderBuffer multisampledDepthTarget = RenderBuffer::makeMultisampled(width, height, GL_DEPTH_COMPONENT32);
	multisampledBuffer.attachRenderBuffer(multisampledColorTarget, GL_COLOR_ATTACHMENT0);
	multisampledBuffer.attachRenderBuffer(multisampledDepthTarget, GL_DEPTH_ATTACHMENT);

	FrameBuffer resolvedBuffer = FrameBuffer::make();
	Texture resolvedColorTarget = Texture::make2D(width, height, GL_RGB16F);
	Texture resolvedDepthTarget = Texture::make2D(width, height, GL_DEPTH_COMPONENT32);
	resolvedBuffer.attachTexture(resolvedColorTarget, GL_COLOR_ATTACHMENT0);
	resolvedBuffer.attachTexture(resolvedDepthTarget, GL_DEPTH_ATTACHMENT);
	postprocessingShader.setUniform("inputTex", resolvedColorTarget.handle());
	ssaoShader.setUniform("inputTex", resolvedDepthTarget.handle());

	Texture brdfLUT = Texture::make2D(m_brdfLUTSize, m_brdfLUTSize, GL_RG16F, nullptr, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
	modelShader.setUniform("brdfLUTex", brdfLUT.handle());

	Shader brdfIntegral = Shader::makeCompute("shaders/brdfintegral.comp");
	brdfIntegral.bind();
	glBindImageTexture(0, brdfLUT.id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	glDispatchCompute((m_brdfLUTSize + 7) / 8, (m_brdfLUTSize + 7) / 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindImageTexture(0, ssaoTarget.id(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
	glBindImageTexture(1, ssaoBlurTarget.id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	ShaderStorageBuffer poissonDisks = makeShadowmapNoise(m_poissonDiskWindowSize, m_poissonDiskFilterSize);
	poissonDisks.bind(1);

	std::array<glm::vec3, m_kernelSize> kernel = makeSSAOKernel();
	glProgramUniform3fv(ssaoShader.id(), glGetUniformLocation(ssaoShader.id(), "kernel"), m_kernelSize, glm::value_ptr(kernel[0]));

	Texture ssaoNoise = makeSSAONoise();
	ssaoShader.setUniform("noiseTex", ssaoNoise.handle());

	modelShader.setUniform("viewportSize", glm::vec2(width, height));
	modelShader.setUniform("ssao", 0);

	return Renderer{ {
		window,
		width,
		height,
		std::move(model),
		std::move(skybox),
		std::move(camera),
		std::move(ssaoShader),
		std::move(ssaoBlurShader),
		std::move(modelShader),
		std::move(depthShader),
		std::move(skyboxShader),
		std::move(postprocessingShader),
		std::move(shadowmapBuffer),
		std::move(ssaoBuffer),
		std::move(multisampledBuffer),
		std::move(resolvedBuffer),
		std::move(multisampledColorTarget),
		std::move(multisampledDepthTarget),
		std::move(brdfLUT),
		std::move(ssaoNoise),
		std::move(shadowmapTarget),
		std::move(ssaoTarget),
		std::move(ssaoBlurTarget),
		std::move(resolvedColorTarget),
		std::move(resolvedDepthTarget),
		std::move(poissonDisks)
	} };
}

Renderer::~Renderer() {
	glfwDestroyWindow(m.window);
	glfwTerminate();
}

void Renderer::draw() {
	float horizontalScale = m.width / 1920.0f, verticalScale = m.height / 1080.0f;
	glm::mat4 projMatrix = m.camera.getProjMatrix(m_fov / 2.0f, 0.1f);
	glm::mat4 camMatrix{ projMatrix * m.camera.getViewMatrix() };
	glm::mat4 camMatrixNoTranslation{ projMatrix * glm::mat4{ glm::mat3{ m.camera.getViewMatrix() } } };
	glm::mat4 modelMatrix{ m.model.baseTransform() * glm::toMat4(m_modelRotation) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ m_modelScale / 100.0f }) };
	glm::mat4 lightMatrix{ calcLightMatrix(modelMatrix) };
	glm::mat3 normalMatrix{ glm::transpose(glm::inverse(modelMatrix)) };


	// depth prepass
	glDepthMask(GL_TRUE);
	m.multisampledBuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m.depthShader.bind();
	m.depthShader.setUniform("camMatrix", camMatrix);
	m.model.draw();

	// shadow pass
	glEnable(GL_POLYGON_OFFSET_FILL);
	glViewport(0, 0, m_shadowmapResolution, m_shadowmapResolution);
	m.shadowmapBuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m.depthShader.bind();
	m.depthShader.setUniform("camMatrix", lightMatrix);
	m.depthShader.setUniform("modelMatrix", modelMatrix);
	m.model.draw();
	glDisable(GL_POLYGON_OFFSET_FILL);


	// ssao pass
	glViewport(0, 0, m.width / 2, m.height / 2);
	m.multisampledBuffer.blitTo(m.resolvedBuffer, GL_DEPTH_BUFFER_BIT, m.width, m.height);
	m.ssaoBuffer.bind();
	m.ssaoShader.bind();
	m.ssaoShader.setUniform("projMatrix", projMatrix);
	m.ssaoShader.setUniform("invProjMatrix", glm::inverse(projMatrix));
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// ssao blur pass
	m.multisampledBuffer.bind();
	m.ssaoBlurShader.bind();
	glDispatchCompute((m.width / 2 + 7) / 8, (m.height / 2 + 7) / 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	// color pass
	glViewport(0, 0, m.width, m.height);
	glDepthMask(GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT);
	m.modelShader.bind();
	m.modelShader.setUniform("camMatrix", camMatrix);
	m.modelShader.setUniform("modelMatrix", modelMatrix);
	m.modelShader.setUniform("lightMatrix", lightMatrix);
	m.modelShader.setUniform("normalMatrix", normalMatrix);
	m.modelShader.setUniform("camPos", m.camera.getPos());
	m.modelShader.setUniform("lightAngle", m_lightAngle);
	m.modelShader.setUniform("lightColor", m_lightColor);
	m.modelShader.setUniform("lightIntensity", m_lightIntensity);
	m.model.draw();
	
	// skybox pass
	m.skyboxShader.bind();
	m.skyboxShader.setUniform("camMatrix", camMatrixNoTranslation);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	// post processing pass
	m.multisampledBuffer.blitTo(m.resolvedBuffer, GL_COLOR_BUFFER_BIT, m.width, m.height);
	m.multisampledBuffer.unbind();
	m.postprocessingShader.bind();
	m.postprocessingShader.setUniform("gamma", m_gamma);
	glDrawArrays(GL_TRIANGLES, 0, 3);

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

	glfwSwapBuffers(m.window);
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
	ImGui::SetWindowSize(ImVec2{ 300.0f * horizontalScale, 205.0f * verticalScale });
	ImGui::SliderFloat("Model Scale", &m_modelScale, 20.0f, 500.0f);
	ImGui::gizmo3D("Model Rotation", inout, 125.0f * horizontalScale);
	if (ImGui::Button("Reset Rotation")) {
		inout = vgm::Quat{ 1.0f, 0.0f, 0.0f, 0.0f };
	}
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
	static bool ssao = 0;
	ImGui::SameLine();
	ImGui::Checkbox("SSAO", & ssao);
	m.modelShader.setUniform("ssao", (int)ssao);
	ImGui::NewLine();
	ImGui::Text("Performance");
	ImGui::Text("%.0f FPS, %.2fms", m_fpsLastSecond, 1000.0 / m_fpsLastSecond);
	ImGui::End();
}

void Renderer::drawAssetMenu(float horizontalScale, float verticalScale) {
	ImGui::Begin("Load Assets");
	ImGui::SetWindowPos(ImVec2{ 825.0f * horizontalScale, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 500.0f * horizontalScale, 75.0f * verticalScale });
	if (ImGui::Button("Load model")) {
		std::filesystem::path modelPath{ m_modelPath };
		if (std::filesystem::exists(modelPath) && modelPath.extension() == ".gltf") {
			m.model = std::move(Model::make(modelPath));
		}
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(400.0f * horizontalScale);
	ImGui::InputText("##modelload", &m_modelPath);
	if (ImGui::Button("Load skybox")) {
		std::filesystem::path skyboxPath{ m_skyboxPath };
		if (std::filesystem::exists(skyboxPath) && skyboxPath.extension() == ".hdr") {
			m.skybox = std::move(Skybox::make(skyboxPath));
			m.skyboxShader.setUniform("skyboxTex", m.skybox.skyboxTexHandle());
			m.modelShader.setUniform("irradianceTex", m.skybox.irradianceTexHandle());
			m.modelShader.setUniform("envMapTex", m.skybox.envmapTexHandle());
			m.modelShader.setUniform("maxMip", static_cast<float>(m.skybox.numMipLevels() - 1));
		}
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(400.0f * horizontalScale);
	ImGui::InputText("##skyboxload", &m_skyboxPath);
	
	
	ImGui::End();
}

void Renderer::resizeWindow(int width, int height) {
	m.width = width;
	m.height = height;
	m.camera.updateSize(width, height);
	m.modelShader.setUniform("viewportSize", glm::vec2(width, height));

	m.ssaoTarget = Texture::make2D(width / 2, height / 2, GL_R8);
	m.ssaoBuffer.attachTexture(m.ssaoTarget, GL_COLOR_ATTACHMENT0);

	m.ssaoBlurTarget = Texture::make2D(width / 2, height / 2, GL_R8);
	m.modelShader.setUniform("ssaoTex", m.ssaoBlurTarget.handle());

	glBindImageTexture(0, m.ssaoTarget.id(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
	glBindImageTexture(1, m.ssaoBlurTarget.id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);

	m.multisampledColorTarget = RenderBuffer::makeMultisampled(width, height, GL_RGB16F);
	m.multisampledDepthTarget = RenderBuffer::makeMultisampled(width, height, GL_DEPTH_COMPONENT);
	m.multisampledBuffer.attachRenderBuffer(m.multisampledColorTarget, GL_COLOR_ATTACHMENT0);
	m.multisampledBuffer.attachRenderBuffer(m.multisampledDepthTarget, GL_DEPTH_ATTACHMENT);

	m.resolvedColorTarget = Texture::make2D(width, height, GL_RGB16F);
	m.resolvedDepthTarget = Texture::make2D(width, height, GL_DEPTH_COMPONENT32);
	m.resolvedBuffer.attachTexture(m.resolvedColorTarget, GL_COLOR_ATTACHMENT0);
	m.resolvedBuffer.attachTexture(m.resolvedDepthTarget, GL_DEPTH_ATTACHMENT);
	m.postprocessingShader.setUniform("inputTex", m.resolvedColorTarget.handle());
	m.ssaoShader.setUniform("inputTex", m.resolvedDepthTarget.handle());

	glViewport(0, 0, width, height);
	draw();
}

// very messily calculates a somewhat conservative orthographic projection volume to fit the entire model in regardless of its transform or the light's position
glm::mat4 Renderer::calcLightMatrix(glm::mat4 modelMatrix) {
	// converts the light's position to spherical coordinates to transform the model relative to the light's viewing angle
	float azimuth = atan2f(sqrt(m_lightAngle.x * m_lightAngle.x + m_lightAngle.z * m_lightAngle.z), m_lightAngle.y) - std::numbers::pi_v<float> / 2.0f;
	float polar = atan2f(m_lightAngle.x, m_lightAngle.z) - std::numbers::pi_v<float>;
	AABB worldSpaceAABB = m.model.aabb().transform(
		modelMatrix
		* glm::rotate(glm::mat4{ 1.0f }, azimuth, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::mat4{ 1.0f }, polar, glm::vec3(0.0f, 1.0f, 0.0f))
	);
	// the model will always fit within a bounding cube derived from the maximum dimension of its aabb
	// not as conservative as I would like but I'm not good enough at math to find the actual correct formula
	glm::vec3 maxBounds{ glm::max(glm::abs(worldSpaceAABB.m_min), glm::abs(worldSpaceAABB.m_max)) };
	float maxElem = std::max(maxBounds.x, std::max(maxBounds.y, maxBounds.z));
	// pass in positive for z min and negative for z max for reverse z projection
	return glm::ortho(-maxElem, maxElem, -maxElem, maxElem, maxElem, -maxElem) * glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, -m_lightAngle, glm::vec3{ 0.0f, 1.0f, 0.0f });
}

ShaderStorageBuffer Renderer::makeShadowmapNoise(int windowSize, int filterSize) {
	std::vector<float> samples;
	samples.reserve(windowSize * windowSize * filterSize * filterSize * 2);

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);

	for (int v = filterSize - 1; v >= 0; v--) {
		for (int u = 0; u < filterSize; u++) {
			for (int i = 0; i < windowSize * windowSize; i++) {
				float x = (u + 0.5f + distribution(generator)) / filterSize;
				float y = (v + 0.5f + distribution(generator)) / filterSize;
				samples.push_back(std::sqrtf(y) * std::cosf(2.0f * std::numbers::pi_v<float> * x));
				samples.push_back(std::sqrtf(y) * std::sinf(2.0f * std::numbers::pi_v<float> * x));
			}
		}
	}

	return ShaderStorageBuffer::make(samples);
}

// use trailing return since m_kernelSize is not in scope at beginning of line
auto Renderer::makeSSAOKernel() -> std::array<glm::vec3, m_kernelSize>  {
	std::array<glm::vec3, m_kernelSize> kernel;

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

	for (int i = 0; i < m_kernelSize; i++) {
		float scale = (float)i / (float)m_kernelSize;
		scale = 0.1f + scale * scale * (0.9f);

		glm::vec3 sample{ distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f, distribution(generator) };

		sample = glm::normalize(sample);
		sample *= distribution(generator);
		sample *= scale;

		kernel[i] = sample;
	}

	return kernel;
}

Texture Renderer::makeSSAONoise() {
	std::array<glm::vec2, m_ssaoNoiseSize * m_ssaoNoiseSize> samples;

	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

	for (int i = 0; i < m_ssaoNoiseSize * m_ssaoNoiseSize; i++) {
		samples[i] = glm::vec2{ distribution(generator),  distribution(generator) };
	}

	return Texture::make2D(m_ssaoNoiseSize, m_ssaoNoiseSize, GL_RG16F, samples.data(), GL_RG, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
}

Renderer::Renderer(ConstructorData data) : m{ std::move(data) } {
	glfwSetWindowUserPointer(m.window, this);
	glfwSetFramebufferSizeCallback(m.window, [] (GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

