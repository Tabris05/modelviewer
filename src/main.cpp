#ifdef NDEBUG
#define main WinMain
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

#include "glwindow.h"
#include "shaderprogram.h"
#include "model.h"
#include "camera.h"
#include "skybox.h"
#include "msaabuffer.h"
#include "framebuffer.h"
#include "dbg.h"

using GLid = GLuint;

#ifdef NDEBUG
constexpr int windowW = 1280, windowH = 720;
#else
constexpr int windowW = 1920, windowH = 1080;
#endif


int main() {
	if (__argc != 2) return 0;
	std::string executablePath(__argv[0]);
	std::filesystem::current_path(executablePath.substr(0, executablePath.find_last_of('\\')));

	float sunlightIntensity = 5.0f;
	glm::vec3 lightDir(0.0f, 0.0f, 1.0f);
	glm::vec3 lightColorPhong = glm::vec3(0.98f, 0.90f, 0.74f) * glm::vec3(1.5f);
	glm::vec3 lightColorPBR = glm::vec3(0.98f, 0.90f, 0.74f) * glm::vec3(sunlightIntensity);

	GLWindow window(windowW, windowH, "Model Viewer");

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ShaderProgram modelShaderBlinnPhong("./shaders/model_vertex.glsl", "./shaders/blinnphong_fragment.glsl");
	glUniform3fv(glGetUniformLocation(modelShaderBlinnPhong, "lightDir"), 1, glm::value_ptr(lightDir));
	glUniform3fv(glGetUniformLocation(modelShaderBlinnPhong, "lightCol"), 1, glm::value_ptr(lightColorPhong));

	ShaderProgram modelShaderPBR("./shaders/model_vertex.glsl", "./shaders/pbr_fragment.glsl");
	glUniform3fv(glGetUniformLocation(modelShaderPBR, "lightDir"), 1, glm::value_ptr(lightDir));
	glUniform3fv(glGetUniformLocation(modelShaderPBR, "lightCol"), 1, glm::value_ptr(lightColorPBR));

	ShaderProgram skyboxShader("./shaders/skybox_vertex.glsl", "./shaders/skybox_fragment.glsl");
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);
	glUniform1f(glGetUniformLocation(skyboxShader, "fSunlightIntensity"), sunlightIntensity);

	ShaderProgram postProcessingShader("./shaders/postprocessing_vertex.glsl", "./shaders/postprocessing_fragment.glsl");
	glUniform1i(glGetUniformLocation(postProcessingShader, "fTexID"), 1);
	glUniform1f(glGetUniformLocation(postProcessingShader, "fGamma"), 2.2f);
	glUniform1f(glGetUniformLocation(postProcessingShader, "fSunlightIntensity"), sunlightIntensity);

	Skybox skybox("./assets/skybox/", { "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png" });

	Model model(__argv[1]);

	FrameBuffer frameBuffer(windowW, windowH);
	MSAABuffer msaaBuffer(windowW, windowH);

	Camera camera(windowW, windowH, glm::vec3(-0.75f, 1.0f, -1.5f), glm::vec3(0.5f, -0.5f, 1.0f));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	float pitch = 0.0f, roll = 0.0f, yaw =0.0f, scale = 1.0f;
	bool pbr = true;
	bool vsync = true;

	double lastFrame = 1.0, curFrame = 1.0;

	while (!window.shouldClose()) {

		if (vsync) glfwSwapInterval(1);
		else glfwSwapInterval(0);

		glfwPollEvents();

		camera.handleInput(window, curFrame - lastFrame);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Options");
		ImGui::SetWindowSize(ImVec2{ 300.0f, 225.0f });
		ImGui::Text("Transform");
		ImGui::SliderFloat("Pitch", &pitch, -360.0f, 360.0f);
		ImGui::SliderFloat("Yaw", &yaw, -360.0f, 360.0f);
		ImGui::SliderFloat("Roll", &roll, -360.0f, 360.0f);
		ImGui::SliderFloat("Scale", &scale, 0.01f, 100.0f);
		ImGui::Text("Graphics Settings");
		ImGui::Checkbox("PBR", &pbr);
		ImGui::SameLine();
		ImGui::Checkbox("VSync", &vsync);
		ImGui::Text("Performance");
		ImGui::Text("%.0f FPS, %.2fms", 1.0 / (curFrame - lastFrame), 1000.0 * (curFrame - lastFrame));
		ImGui::End();

		lastFrame = curFrame;

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::rotate(transform, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		transform = glm::rotate(transform, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(scale, scale, scale));

		

		msaaBuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		model.draw(pbr ? modelShaderPBR : modelShaderBlinnPhong, camera, transform);
		skybox.draw(skyboxShader, camera);
		
		msaaBuffer.blit(frameBuffer);
		msaaBuffer.unbind();

		frameBuffer.draw(postProcessingShader);

		checkErr();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();

		curFrame = glfwGetTime();	
	}
	return 0;
}