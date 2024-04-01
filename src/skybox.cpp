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
    return m_skyboxTex.handle().value();
}

GLuint64 Skybox::irradianceTexHandle() const {
    return m_irradianceTex.handle().value();
}

GLuint64 Skybox::envmapTexHandle() const {
    return m_envmapTex.handle().value();
}

Skybox Skybox::make() {
    return Skybox{ Texture::makeCubeBindless(1, 1), Texture::makeCubeBindless(1, 1), Texture::makeCubeBindless(1, 1) };
}

// this should probably be done in compute but its less math for me this way (plus no bindless images)
Skybox Skybox::make(const std::filesystem::path& path) {
    const static glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const static glm::mat4 views[] = {
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ -1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }),
        glm::lookAt(glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f })
    };

	int width, height, nrChannels;
	float* data = stbi_loadf(path.string().c_str(), &width, &height, &nrChannels, 0);
	Texture equirectangular = Texture::make2DBindless(width, height, GL_RGB16F, data, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
	stbi_image_free(data);

    FrameBuffer captureBuffer = FrameBuffer::make();
    captureBuffer.bind();
    
    // render equirectangular map to cubemap faces
    int cubemapSize = height / 2;
    Texture skyboxTex = Texture::makeCube(cubemapSize, cubemapSize, GL_RGB16F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    captureBuffer.attachTexture(skyboxTex, GL_COLOR_ATTACHMENT0);

    Shader skyboxConversionShader = Shader::make("shaders/skyboxpreprocess.vert", "shaders/skyboxconversion.frag");
    skyboxConversionShader.setUniform("equirectangularMap", equirectangular.handle().value());

    skyboxConversionShader.bind();
    glViewport(0, 0, cubemapSize, cubemapSize);
    for (int i = 0; i < 6; i++) {
        skyboxConversionShader.setUniform("camMatrix", projection * views[i]);
        skyboxConversionShader.setUniform("layer", i);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    skyboxTex.generateMipMaps();
    skyboxTex.makeBindless();


    // convolute skybox cubemap to create irradiance map (diffuse ibl)
    static constexpr int irradianceMapSize = 32;
    Texture irradianceTex = Texture::makeCubeBindless(irradianceMapSize, irradianceMapSize, GL_RGB16F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    captureBuffer.attachTexture(irradianceTex, GL_COLOR_ATTACHMENT0);

    Shader skyboxConvolutionShader = Shader::make("shaders/skyboxpreprocess.vert", "shaders/skyboxconvolution.frag");
    skyboxConvolutionShader.setUniform("skyboxTex", skyboxTex.handle().value());

    skyboxConvolutionShader.bind();
    glViewport(0, 0, irradianceMapSize, irradianceMapSize);
    for (int i = 0; i < 6; i++) {
        skyboxConvolutionShader.setUniform("camMatrix", projection * views[i]);
        skyboxConvolutionShader.setUniform("layer", i);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    captureBuffer.detachTexture(GL_COLOR_ATTACHMENT0);

    return Skybox{ std::move(skyboxTex), std::move(irradianceTex), Texture::makeCubeBindless(1, 1) };
}

Skybox::Skybox(Texture skyboxTex, Texture irradianceTex, Texture envmapTex) :
    m_skyboxTex{ std::move(skyboxTex) },
    m_irradianceTex{ std::move(irradianceTex) },
    m_envmapTex{ std::move(envmapTex) } {}