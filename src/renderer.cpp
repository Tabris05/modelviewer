#include "renderer.h"
#include "vertexbuffer.h"
#include "vertexarray.h"
#include "shader.h"
#include "dbg.h"

void Renderer::run() {

	Shader modelShader = Shader::make("shaders/model_vert.glsl", "shaders/model_frag.glsl");
	modelShader.bind();

	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		draw();
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

	gladLoadGL();
	glViewport(0, 0, width, height);

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(-1);

	Model model = Model::make("model/scene.gltf");

	return Renderer{ window, width, height, std::move(model) };
}

Renderer::~Renderer() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Renderer::draw() {
	m_model.draw();
	glfwSwapBuffers(m_window);
}

void Renderer::resizeWindow(int width, int height) {
	m_width = width;
	m_height = height;
	glViewport(0, 0, width, height);
	draw();
}

Renderer::Renderer(GLFWwindow* window, int width, int height, Model model) : m_window{ window }, m_width{ width }, m_height{ height }, m_model{ std::move(model) } {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(m_window, [] (GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

