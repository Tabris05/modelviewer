#include "renderer.h"
#include "dbg.h"

void Renderer::run() {

	m_modelShader.bind();

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

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int width = mode->width * 3 / 4;
	int height = mode->height * 3 / 4;

	GLFWwindow* window = glfwCreateWindow(width, height, "Model Viewer", NULL, NULL);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	gladLoadGL();
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	Camera camera = Camera::make(window, width, height);
	Model model = Model::make("model/scene.gltf");
	Shader modelShader = Shader::make("shaders/model_vert.glsl", "shaders/model_frag.glsl");

	return Renderer{ window, width, height, std::move(camera), std::move(model), std::move(modelShader) };
}

Renderer::~Renderer() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Renderer::draw() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_modelShader.setUniform("transform", m_camera.getProjMatrix(90.0f, 0.1f, 100.0f) * m_camera.getViewMatrix() * m_model.m_baseTransform);
	m_model.draw();
	glfwSwapBuffers(m_window);
}

void Renderer::resizeWindow(int width, int height) {
	m_width = width;
	m_height = height;
	m_camera.updateSize(width, height);
	glViewport(0, 0, width, height);
	draw();
}

Renderer::Renderer(GLFWwindow* window, int width, int height, Camera camera, Model model, Shader modelShader) :
	m_window{ window },
	m_width{ width },
	m_height{ height },
	m_camera{ std::move(camera) },
	m_model { std::move(model) },
	m_modelShader{ std::move(modelShader) } {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(m_window, [] (GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

