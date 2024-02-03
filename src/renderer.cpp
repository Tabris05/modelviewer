#include "renderer.h"
#include "vertexbuffer.h"
#include "vertexarray.h"
#include "shader.h"
#include "dbg.h"

void Renderer::run() {
	std::vector<Vertex> vertices{
		Vertex{ glm::vec3(-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f) },
		Vertex{ glm::vec3(0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f) },
		Vertex{ glm::vec3(0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f) }
	};

	Shader triShader = Shader::make("shaders/tri_vert.glsl", "shaders/tri_frag.glsl");
	VertexBuffer vBuf = VertexBuffer::make(vertices);
	VertexArray vArr = VertexArray::make();
	vArr.linkVertexBuffer(vBuf, sizeof(Vertex));
	vArr.linkAttribute(0, 3, GL_FLOAT, 0);
	vArr.bind();
	triShader.bind();

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

	return Renderer{ window, width, height };
}

Renderer::~Renderer() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

Renderer::Renderer(GLFWwindow* window, int width, int height) : m_window{ window }, m_width{ width }, m_height{ height } {
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int x, int y) { static_cast<Renderer*>(glfwGetWindowUserPointer(window))->resizeWindow(x, y); });
}

void Renderer::draw() {
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glfwSwapBuffers(m_window);
}

void Renderer::resizeWindow(int width, int height) {
	m_width = width;
	m_height = height;
	glViewport(0, 0, width, height);
	draw();
}