#include "renderer.h"
#include <imGuIZMOquat/imGuIZMOquat.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <stb/stb_image.h>
#include <numbers>
#include <random>
#include "dbg.h"

// SSBOs:
// 0 - materials
// 1 - shadow map noise

// Images:
// 0 - BRDF LUT (initialization) / Multisampled Frame Buffer (runtime)
// 1 - Postprocessing Frame Buffer
// 2 - Bloom dst / Skybox dst

void Renderer::run() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		if (const ImGuiIO& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
			m_camera.handleInput(m_curFrame - m_lastFrame);
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

	GLFWwindow* window = glfwCreateWindow(width, height, "Model Viewer", nullptr, nullptr);
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

	glDepthFunc(GL_GEQUAL); // reverse z
	glClearDepth(0.0f);
	glCullFace(GL_BACK);
	glViewport(0, 0, width, height);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460 core");


	Model model = Model::make();
	Skybox skybox = Skybox::make();

	Camera camera = Camera::make(window, width, height);

	Shader modelShader = Shader::make("shaders/model.vert", "shaders/model.frag");
	Shader depthShader = Shader::make("shaders/depth.vert", "shaders/depth.frag");
	Shader shadowShader = Shader::make("shaders/shadow.vert", "shaders/depth.frag");
	Shader skyboxShader = Shader::make("shaders/cubemap.vert", "shaders/cubemap.frag");
	ComputeShader resolveShader = ComputeShader::make("shaders/multisampleresolve.comp");
	ComputeShader bloomDownsampleShader = ComputeShader::make("shaders/bloomdownsample.comp");
	ComputeShader bloomUpsampleShader = ComputeShader::make("shaders/bloomupsample.comp");
	ComputeShader postprocessingShader = ComputeShader::make("shaders/postprocessing.comp");

	FrameBuffer shadowmapBuffer = FrameBuffer::make();
	Texture shadowmapTarget = Texture::make2D(m_shadowmapResolution, m_shadowmapResolution, GL_DEPTH_COMPONENT32);
	shadowmapBuffer.attachTexture(shadowmapTarget, GL_DEPTH_ATTACHMENT);
	modelShader.setUniform("shadowmapTex", shadowmapTarget.handle());

	FrameBuffer multisampledBuffer = FrameBuffer::make();
	Texture multisampledColorTarget = Texture::make2DMultisampled(width, height, GL_R11F_G11F_B10F);
	Texture multisampledDepthTarget = Texture::make2DMultisampled(width, height, GL_DEPTH_COMPONENT32);
	multisampledBuffer.attachTexture(multisampledColorTarget, GL_COLOR_ATTACHMENT0);
	multisampledBuffer.attachTexture(multisampledDepthTarget, GL_DEPTH_ATTACHMENT);

	FrameBuffer postprocessingBuffer = FrameBuffer::make();
	Texture postprocessingTarget = Texture::make2D(width, height, GL_R11F_G11F_B10F);
	postprocessingBuffer.attachTexture(postprocessingTarget, GL_COLOR_ATTACHMENT0);
	postprocessingTarget.bindImage(1, GL_READ_WRITE);

	Texture brdfLUT = Texture::make2D(m_brdfLUTSize, m_brdfLUTSize, GL_RG16F, nullptr, GL_RG, GL_FLOAT, GL_LINEAR, GL_LINEAR);
	modelShader.setUniform("brdfLUTex", brdfLUT.handle());

	ComputeShader brdfIntegral = ComputeShader::make("shaders/brdfintegral.comp");
	brdfIntegral.bind();
	brdfLUT.bindImage(0, GL_WRITE_ONLY);
	brdfIntegral.dispatch(GL_TEXTURE_FETCH_BARRIER_BIT, m_brdfLUTSize, m_brdfLUTSize);
	multisampledColorTarget.bindImage(0, GL_READ_ONLY);

	ShaderStorageBuffer poissonDisks = makeShadowmapNoise(m_poissonDiskWindowSize, m_poissonDiskFilterSize);
	poissonDisks.bind(1);
	
	int bufferWidth = width;
	int bufferHeight = height;
	size_t numBloomMips = std::min<size_t>(m_bloomDepth, std::floor(std::log2(std::max(width, height))) + 1);
	Texture bloomTarget = Texture::make2D(width, height, GL_R11F_G11F_B10F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	bloomDownsampleShader.setUniform("inputTex", bloomTarget.handle());
	bloomUpsampleShader.setUniform("inputTex", bloomTarget.handle());
	postprocessingShader.setUniform("bloomTex", bloomTarget.handle());
	std::vector<glm::ivec2> bloomMipSizes;
	for (size_t i = 0; i < numBloomMips; i++) {
		bloomMipSizes.emplace_back(bufferWidth, bufferHeight);
		bufferWidth /= 2;
		bufferHeight /= 2;
	}

	return Renderer { 
		window,
		width,
		height,
		std::move(model),
		std::move(skybox),
		std::move(camera),
		std::move(modelShader),
		std::move(depthShader),
		std::move(shadowShader),
		std::move(skyboxShader),
		std::move(resolveShader),
		std::move(bloomDownsampleShader),
		std::move(bloomUpsampleShader),
		std::move(postprocessingShader),
		std::move(shadowmapBuffer),
		std::move(multisampledBuffer),
		std::move(postprocessingBuffer),
		std::move(multisampledColorTarget),
		std::move(multisampledDepthTarget),
		std::move(brdfLUT),
		std::move(bloomTarget),
		std::move(shadowmapTarget),
		std::move(postprocessingTarget),
		std::move(poissonDisks),
		std::move(bloomMipSizes)
	};
}

Renderer::~Renderer() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Renderer::draw() {
	float horizontalScale = m_width / 1920.0f, verticalScale = m_height / 1080.0f;
	glm::mat4 camMatrix{ m_camera.getProjMatrix(m_fov / 2.0f, 0.1f) * m_camera.getViewMatrix() };
	glm::mat4 camMatrixNoTranslation{ m_camera.getProjMatrix(m_fov / 2.0f, 0.1f) * glm::mat4{ glm::mat3{ m_camera.getViewMatrix() } } };
	glm::mat4 modelMatrix{ m_model.baseTransform() * glm::toMat4(m_modelRotation) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ m_modelScale / 100.0f }) };
	glm::mat4 lightMatrix{ calcLightMatrix(modelMatrix) };
	glm::mat3 normalMatrix{ glm::transpose(glm::inverse(modelMatrix)) };

	// depth pass
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	m_multisampledBuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m_depthShader.bind();
	m_depthShader.setUniform("camMatrix", camMatrix);
	m_depthShader.setUniform("modelMatrix", modelMatrix);
	m_model.drawOpaque();

	// shadow pass
	glViewport(0, 0, m_shadowmapResolution, m_shadowmapResolution);
	m_shadowmapBuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	m_shadowShader.bind();
	m_shadowShader.setUniform("camMatrix", lightMatrix);
	m_shadowShader.setUniform("modelMatrix", modelMatrix);
	m_shadowShader.setUniform("normalMatrix", normalMatrix);
	m_model.drawOpaque();
	glViewport(0, 0, m_width, m_height);

	// color pass
	glDepthMask(GL_FALSE);
	glViewport(0, 0, m_width, m_height);
	m_multisampledBuffer.bind();
	m_modelShader.bind();
	m_modelShader.setUniform("camMatrix", camMatrix);
	m_modelShader.setUniform("modelMatrix", modelMatrix);
	m_modelShader.setUniform("lightMatrix", lightMatrix);
	m_modelShader.setUniform("normalMatrix", normalMatrix);
	m_modelShader.setUniform("camPos", m_camera.getPos());
	m_modelShader.setUniform("lightAngle", m_lightAngle);
	m_modelShader.setUniform("lightColor", m_lightColor);
	m_modelShader.setUniform("lightIntensity", m_lightIntensity);
	m_model.drawOpaque();

	// skybox pass
	m_skyboxShader.bind();
	m_skyboxShader.setUniform("camMatrix", camMatrixNoTranslation);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// transparent pass
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	m_modelShader.bind();
	m_model.drawTransparent();
	glDisable(GL_BLEND);

	glTextureBarrier();

	// resolve pass
	// MSAA only works well in ldr color space, so we do a custom resolve that tonemaps each sample and then combines them
	// we then inverse tonemap the result since the bloom buffer needs to be composited in hdr color space
	// this shader also resolves into mip 0 of the bloom buffer, but does not tonemap/inverse tonemap since any aliasing will be resolved by the blurring
	m_bloomTarget.bindImage(2, GL_WRITE_ONLY);
	m_resolveShader.bind();
	m_resolveShader.dispatch(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, m_width, m_height);

	// bloom pass
	m_multisampledBuffer.unbind();
	if (m_bloomEnabled) {
		m_bloomDownsampleShader.bind();
		for (int i = 1; i < m_bloomMipSizes.size(); i++) {
			m_bloomTarget.bindImage(2, GL_WRITE_ONLY, i);
			m_bloomDownsampleShader.setUniform("inputMip", i - 1);
			m_bloomDownsampleShader.dispatch(GL_TEXTURE_FETCH_BARRIER_BIT, m_bloomMipSizes[i].x, m_bloomMipSizes[i].y);
		}
		m_bloomUpsampleShader.bind();
		for (int i = m_bloomMipSizes.size() - 1; i >= 1; i--) {
			m_bloomTarget.bindImage(2, GL_READ_WRITE, i - 1);
			m_bloomUpsampleShader.setUniform("inputMip", i);
			m_bloomUpsampleShader.dispatch(GL_TEXTURE_FETCH_BARRIER_BIT, m_bloomMipSizes[i - 1].x, m_bloomMipSizes[i - 1].y);
		}
	}

	// post processing pass
	m_postprocessingShader.bind();
	m_postprocessingShader.setUniform("gamma", m_gamma);
	m_postprocessingShader.dispatch(GL_FRAMEBUFFER_BARRIER_BIT, m_width, m_height);
	m_postprocessingBuffer.blitToDefault(GL_COLOR_BUFFER_BIT, m_width, m_height);

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
	ImGui::SameLine();
	ImGui::Checkbox("Bloom", &m_bloomEnabled);
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
		if (std::filesystem::exists(modelPath) && (modelPath.extension() == ".gltf" || modelPath.extension() == ".glb")) {
			m_model = std::move(Model::make(modelPath));
		}
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(400.0f * horizontalScale);
	ImGui::InputText("##modelload", &m_modelPath);
	if (ImGui::Button("Load skybox")) {
		std::filesystem::path skyboxPath{ m_skyboxPath };
		if (std::filesystem::exists(skyboxPath) && skyboxPath.extension() == ".hdr") {
			m_skybox = std::move(Skybox::make(skyboxPath));
			m_skyboxShader.setUniform("skyboxTex", m_skybox.skyboxTexHandle());
			m_modelShader.setUniform("irradianceTex", m_skybox.irradianceTexHandle());
			m_modelShader.setUniform("envMapTex", m_skybox.envmapTexHandle());
			m_modelShader.setUniform("maxMip", static_cast<float>(m_skybox.numMipLevels() - 1));
		}
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(400.0f * horizontalScale);
	ImGui::InputText("##skyboxload", &m_skyboxPath);
	
	
	ImGui::End();
}

void Renderer::resizeWindow(int width, int height) {
	m_width = width;
	m_height = height;
	m_camera.updateSize(width, height);

	m_multisampledColorTarget = Texture::make2DMultisampled(width, height, GL_R11F_G11F_B10F);
	m_multisampledDepthTarget = Texture::make2DMultisampled(width, height, GL_DEPTH_COMPONENT32);
	m_multisampledBuffer.attachTexture(m_multisampledColorTarget, GL_COLOR_ATTACHMENT0);
	m_multisampledBuffer.attachTexture(m_multisampledDepthTarget, GL_DEPTH_ATTACHMENT);
	m_multisampledColorTarget.bindImage(0, GL_READ_ONLY);
	m_postprocessingTarget = Texture::make2D(width, height, GL_R11F_G11F_B10F);
	m_postprocessingBuffer.attachTexture(m_postprocessingTarget, GL_COLOR_ATTACHMENT0);
	m_postprocessingTarget.bindImage(1, GL_READ_WRITE);

	int bufferWidth = width;
	int bufferHeight = height;
	size_t numBloomMips = std::min<size_t>(m_bloomDepth, std::floor(std::log2(std::max(width, height))) + 1);
	m_bloomTarget = Texture::make2D(width, height, GL_R11F_G11F_B10F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_bloomDownsampleShader.setUniform("inputTex", m_bloomTarget.handle());
	m_bloomUpsampleShader.setUniform("inputTex", m_bloomTarget.handle());
	m_postprocessingShader.setUniform("bloomTex", m_bloomTarget.handle());
	m_bloomMipSizes.clear();
	for (size_t i = 0; i < numBloomMips; i++) {
		m_bloomMipSizes.emplace_back(bufferWidth, bufferHeight);
		bufferWidth /= 2;
		bufferHeight /= 2;
	}

	glViewport(0, 0, width, height);
	draw();
}

// very messily calculates a somewhat conservative orthographic projection volume to fit the entire model in regardless of its transform or the light's position
glm::mat4 Renderer::calcLightMatrix(glm::mat4 modelMatrix) {
	// converts the light's position to spherical coordinates to transform the model relative to the light's viewing angle
	float azimuth = atan2f(sqrt(m_lightAngle.x * m_lightAngle.x + m_lightAngle.z * m_lightAngle.z), m_lightAngle.y) - std::numbers::pi_v<float> / 2.0f;
	float polar = atan2f(m_lightAngle.x, m_lightAngle.z) - std::numbers::pi_v<float>;
	AABB worldSpaceAABB = m_model.aabb().transform(
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

Renderer::Renderer(
	GLFWwindow* window,
	int width,
	int height,
	Model model,
	Skybox skybox,
	Camera camera,
	Shader modelShader,
	Shader depthShader,
	Shader shadowShader,
	Shader skyboxShader,
	ComputeShader resolveShader,
	ComputeShader bloomDownsampleShader,
	ComputeShader bloomUpsampleShader,
	ComputeShader postprocessingShader,
	FrameBuffer shadowmapBuffer,
	FrameBuffer multisampledBuffer,
	FrameBuffer postprocessingBuffer,
	Texture multisampledColorTarget,
	Texture multisampledDepthTarget,
	Texture brdfLUT,
	Texture bloomTarget,
	Texture shadowmapTarget,
	Texture postprocessingTarget,
	ShaderStorageBuffer poissonDisks,
	std::vector<glm::ivec2> bloomMipSizes
) :
	m_window{ window },
	m_width{ width },
	m_height{ height },
	m_model{ std::move(model) },
	m_skybox{ std::move(skybox) },
	m_camera{ std::move(camera) },
	m_modelShader{ std::move(modelShader) },
	m_depthShader{ std::move(depthShader) },
	m_shadowShader{ std::move(shadowShader) },
	m_skyboxShader{ std::move(skyboxShader) },
	m_resolveShader{ std::move(resolveShader) },
	m_bloomDownsampleShader{ std::move(bloomDownsampleShader) },
	m_bloomUpsampleShader{ std::move(bloomUpsampleShader) },
	m_postprocessingShader{ std::move(postprocessingShader) },
	m_shadowmapBuffer{ std::move(shadowmapBuffer) },
	m_multisampledBuffer{ std::move(multisampledBuffer) },
	m_postprocessingBuffer{ std::move(postprocessingBuffer) },
	m_multisampledColorTarget{ std::move(multisampledColorTarget) },
	m_multisampledDepthTarget{ std::move(multisampledDepthTarget) },
	m_brdfLUT{ std::move(brdfLUT) },
	m_bloomTarget{ std::move(bloomTarget) },
	m_shadowmapTarget{ std::move(shadowmapTarget) },
	m_postprocessingTarget{ std::move(postprocessingTarget) },
	m_poissonDisks{ std::move(poissonDisks) },
	m_bloomMipSizes{ std::move(bloomMipSizes) } {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(m_window, [] (GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

