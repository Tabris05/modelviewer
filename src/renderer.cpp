#include "renderer.h"
#include "dbg.h"

void Renderer::run() {

	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		m_camera.handleInput(m_curFrame - m_lastFrame);
		
		draw();
		m_lastFrame = m_curFrame;
		m_curFrame = glfwGetTime();
		checkErr();
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

	Camera camera = Camera::make(window, width, height);
	Model model = Model::make("model/scene.gltf");
	Shader modelShader = Shader::make("shaders/model.vert", "shaders/model.frag");
	Shader prepassShader = Shader::make("shaders/prepass.vert", "shaders/prepass.frag");
	Shader postprocessingShader = Shader::make("shaders/postprocessing.vert", "shaders/postprocessing.frag");

	FrameBuffer postprocessingBuffer = FrameBuffer::make();
	Texture postprocessingTarget = Texture::make2D(NULL, width, height, GL_RGB16F);
	postprocessingBuffer.attachTexture(postprocessingTarget, GL_COLOR_ATTACHMENT0);
	postprocessingShader.setUniform("inputTex", postprocessingTarget.makeBindless());
	postprocessingShader.setUniform("exposure2", 5.0f * 5.0f);
	postprocessingShader.setUniform("gamma", 2.2f);

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
		std::move(model),
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
	m_multisampledBuffer.bind();

	// depth pass
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_prepassShader.bind();
	m_prepassShader.setUniform("camMatrix", m_camera.getProjMatrix(45.0f, 0.1f, 100.0f) * m_camera.getViewMatrix());
	m_prepassShader.setUniform("modelMatrix", m_model.m_baseTransform);
	m_model.draw();

	// color pass
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	m_modelShader.bind();
	m_modelShader.setUniform("camMatrix", m_camera.getProjMatrix(45.0f, 0.1f, 100.0f) * m_camera.getViewMatrix());
	m_modelShader.setUniform("modelMatrix", m_model.m_baseTransform);
	m_modelShader.setUniform("camPos", m_camera.getPos());
	m_model.draw();

	// post processing pass
	glDisable(GL_DEPTH_TEST);
	m_multisampledBuffer.blitTo(m_postprocessingBuffer, GL_COLOR_BUFFER_BIT, m_width, m_height);
	m_multisampledBuffer.unbind();
	m_postprocessingShader.bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glfwSwapBuffers(m_window);
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
	m_multisampledBuffer.attachRenderBuffer(m_multisampledColorTarget, GL_COLOR_ATTACHMENT0);
	m_multisampledBuffer.attachRenderBuffer(m_multisampledDepthTarget, GL_DEPTH_ATTACHMENT);
	glViewport(0, 0, width, height);
	draw();
}

Renderer::Renderer(
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
) :
	m_window{ window },
	m_width{ width },
	m_height{ height },
	m_camera{ std::move(camera) },
	m_model { std::move(model) },
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

