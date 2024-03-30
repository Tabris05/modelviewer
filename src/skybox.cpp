#include "skybox.h"
#include "framebuffer.h"
#include "shader.h"
#include <stb/stb_image.h>
#include <utility>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

GLuint64 Skybox::skyboxTexHandle() const {
    return m_skyboxTex.handle();
}

Skybox Skybox::make() {
    return Skybox{ Texture::makeCube(1, 1) };
}

Skybox Skybox::make(const std::filesystem::path& path) {
	int width, height, nrChannels;
	float* data = stbi_loadf(path.string().c_str(), &width, &height, &nrChannels, 0);
	Texture equirectangular = Texture::make2D(width, height, GL_RGB16F, data, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
	stbi_image_free(data);

	int cubemapSize = height / 2;
	Texture skyboxTex = Texture::makeCube(cubemapSize, cubemapSize, GL_RGB16F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);

    // render equirectangular map to cubemap faces
    FrameBuffer captureBuffer = FrameBuffer::make();
    captureBuffer.attachTexture(skyboxTex, GL_COLOR_ATTACHMENT0);

    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 views[] = {
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 1.0f,  0.0f,  0.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ -1.0f,  0.0f,  0.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f,  1.0f,  0.0f }, glm::vec3{ 0.0f,  0.0f,  1.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }, glm::vec3{ 0.0f,  0.0f, -1.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f,  0.0f,  1.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f,  0.0f, -1.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f })
    };

    Shader skyboxConversionShader = Shader::make("shaders/skyboxconversion.vert", "shaders/skyboxconversion.frag");
    skyboxConversionShader.setUniform("equirectangularMap", equirectangular.handle());

    captureBuffer.bind();
    skyboxConversionShader.bind();
    glViewport(0, 0, cubemapSize, cubemapSize);
    for (int i = 0; i < 6; i++) {
        skyboxConversionShader.setUniform("camMatrix", projection * views[i]);
        skyboxConversionShader.setUniform("layer", i);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    captureBuffer.detachTexture(GL_COLOR_ATTACHMENT0);

    return Skybox{ std::move(skyboxTex) };
}

Skybox::Skybox(Texture skyboxTex) : m_skyboxTex{ std::move(skyboxTex) }  {}