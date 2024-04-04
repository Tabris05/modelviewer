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

GLuint64 Skybox::irradianceTexHandle() const {
    return m_irradianceTex.handle();
}

GLuint64 Skybox::envmapTexHandle() const {
    return m_envmapTex.handle();
}

int Skybox::numMipLevels() const {
    return m_numMipLevels;
}

Skybox Skybox::make() {
    return Skybox{ Texture::makeCube(1, 1), Texture::makeCube(1, 1), Texture::makeCube(1, 1), 0 };
}

// this should probably be done in compute but its less math for me this way
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
	Texture equirectangular = Texture::make2D(width, height, GL_RGB16F, data, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
	stbi_image_free(data);

    FrameBuffer captureBuffer = FrameBuffer::make();
    captureBuffer.bind();
    
    // render equirectangular map to cubemap faces
    int cubemapSize = height / 2;
    Texture skyboxTex = Texture::makeCube(cubemapSize, cubemapSize, GL_RGB16F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    captureBuffer.attachTexture(skyboxTex, GL_COLOR_ATTACHMENT0);

    Shader skyboxConversionShader = Shader::makeGraphics("shaders/skyboxpreprocess.vert", "shaders/skyboxconversion.frag");
    skyboxConversionShader.setUniform("equirectangularMap", equirectangular.handle());

    skyboxConversionShader.bind();
    glViewport(0, 0, cubemapSize, cubemapSize);
    for (int i = 0; i < 6; i++) {
        skyboxConversionShader.setUniform("camMatrix", projection * views[i]);
        skyboxConversionShader.setUniform("layer", i);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    GLuint skyboxMipMapID;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skyboxMipMapID);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(skyboxMipMapID, std::log2(cubemapSize) + 1, GL_RGB16F, cubemapSize, cubemapSize);
    glCopyImageSubData(skyboxTex.id(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, skyboxMipMapID, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, cubemapSize, cubemapSize, 6);
    glGenerateTextureMipmap(skyboxMipMapID);
    GLuint64 skyboxMipMapHandle = glGetTextureHandleARB(skyboxMipMapID);
    glMakeTextureHandleResidentARB(skyboxMipMapHandle);

    // convolute skybox cubemap to create irradiance map (diffuse ibl)
    static constexpr int irradianceMapSize = 32;
    Texture irradianceTex = Texture::makeCube(irradianceMapSize, irradianceMapSize, GL_RGB16F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    captureBuffer.attachTexture(irradianceTex, GL_COLOR_ATTACHMENT0);

    Shader skyboxConvolutionShader = Shader::makeGraphics("shaders/skyboxpreprocess.vert", "shaders/skyboxconvolution.frag");
    skyboxConvolutionShader.setUniform("skyboxTex", skyboxMipMapHandle);

    skyboxConvolutionShader.bind();
    glViewport(0, 0, irradianceMapSize, irradianceMapSize);
    for (int i = 0; i < 6; i++) {
        skyboxConvolutionShader.setUniform("camMatrix", projection * views[i]);
        skyboxConvolutionShader.setUniform("layer", i);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // convolute skybox cubemap to create mip chain of environment maps (specular ibl)
    static constexpr int envMapSize = 512;
    const int numMipLevels = std::log2(envMapSize) + 1;
    Texture envMapTex = Texture::makeCube(envMapSize, envMapSize, GL_RGB16F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    Shader skyboxPrefilterShader = Shader::makeGraphics("shaders/skyboxpreprocess.vert", "shaders/skyboxprefilter.frag");
    skyboxPrefilterShader.setUniform("skyboxTex", skyboxMipMapHandle);
    skyboxPrefilterShader.setUniform("resolution", static_cast<float>(cubemapSize));

    int curMipSize = envMapSize;
    skyboxPrefilterShader.bind();
    for (int mip = 0; mip < numMipLevels; mip++) {
        glViewport(0, 0, curMipSize, curMipSize);
        captureBuffer.attachTexture(envMapTex, GL_COLOR_ATTACHMENT0, mip);
        skyboxPrefilterShader.setUniform("roughness", mip / static_cast<float>(numMipLevels - 1));
        for (int i = 0; i < 6; i++) {
            skyboxPrefilterShader.setUniform("camMatrix", projection * views[i]);
            skyboxPrefilterShader.setUniform("layer", i);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        curMipSize /= 2;
    }

    captureBuffer.detachTexture(GL_COLOR_ATTACHMENT0, numMipLevels - 1);
    glMakeTextureHandleNonResidentARB(skyboxMipMapHandle);
    glDeleteTextures(1, &skyboxMipMapID);

    return Skybox{ std::move(skyboxTex), std::move(irradianceTex), std::move(envMapTex), numMipLevels };
}

Skybox::Skybox(Texture skyboxTex, Texture irradianceTex, Texture envmapTex, int numMipLevels) :
    m_skyboxTex{ std::move(skyboxTex) },
    m_irradianceTex{ std::move(irradianceTex) },
    m_envmapTex{ std::move(envmapTex) },
    m_numMipLevels{ numMipLevels } {}