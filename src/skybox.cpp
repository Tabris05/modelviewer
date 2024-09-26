#include "skybox.h"
#include "framebuffer.h"
#include "computeshader.h"
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

Skybox Skybox::make(const std::filesystem::path& path) {
	int width, height, nrChannels;
	float* data = stbi_loadf(path.string().c_str(), &width, &height, &nrChannels, 0);
	Texture equirectangular = Texture::make2D(width, height, GL_R11F_G11F_B10F, data, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
	stbi_image_free(data);
    
    // render equirectangular map to cubemap faces
    int cubemapSize = height / 2;
    Texture skyboxTex = Texture::makeCube(cubemapSize, cubemapSize, GL_R11F_G11F_B10F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);

    ComputeShader skyboxConversionShader = ComputeShader::make("shaders/skyboxconversion.comp");
    skyboxConversionShader.setUniform("equirectangularMap", equirectangular.handle());
    skyboxConversionShader.bind();

    skyboxTex.bindImage(2, GL_WRITE_ONLY, 0);
    skyboxConversionShader.dispatch(GL_TEXTURE_FETCH_BARRIER_BIT, width, height, 6);

    // the texture class doesn't provide an abstraction for copying texture data nor for deferring mipmap generation
    // this is a pathological case, so I'm opting to simply forgo the abstraction rather than trying to shoehorn in logic that will never be needed anywhere else
    GLuint skyboxMipMapID;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skyboxMipMapID);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(skyboxMipMapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(skyboxMipMapID, std::log2(cubemapSize) + 1, GL_R11F_G11F_B10F, cubemapSize, cubemapSize);
    glCopyImageSubData(skyboxTex.id(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, skyboxMipMapID, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, cubemapSize, cubemapSize, 6);
    glGenerateTextureMipmap(skyboxMipMapID);
    GLuint64 skyboxMipMapHandle = glGetTextureHandleARB(skyboxMipMapID);
    glMakeTextureHandleResidentARB(skyboxMipMapHandle);

    // convolve skybox cubemap to create irradiance map (diffuse ibl)
    static constexpr int irradianceMapSize = 32;
    Texture irradianceTex = Texture::makeCube(irradianceMapSize, irradianceMapSize, GL_R11F_G11F_B10F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR, GL_LINEAR);
    
    ComputeShader skyboxConvolutionShader = ComputeShader::make("shaders/skyboxconvolution.comp");
    skyboxConvolutionShader.setUniform("skyboxTex", skyboxMipMapHandle);
    skyboxConvolutionShader.bind();

    irradianceTex.bindImage(2, GL_WRITE_ONLY, 0);
    skyboxConvolutionShader.dispatch(GL_TEXTURE_FETCH_BARRIER_BIT, irradianceMapSize, irradianceMapSize, 6);

    // convolve skybox cubemap to create mip chain of environment maps (specular ibl)
    static constexpr int envMapSize = 512;
    const int numMipLevels = std::log2(envMapSize) + 1;
    Texture envMapTex = Texture::makeCube(envMapSize, envMapSize, GL_R11F_G11F_B10F, nullptr, GL_RGB, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    ComputeShader skyboxPrefilterShader = ComputeShader::make("shaders/skyboxprefilter.comp");
    skyboxPrefilterShader.setUniform("skyboxTex", skyboxMipMapHandle);
    skyboxPrefilterShader.setUniform("resolution", static_cast<float>(cubemapSize));
    skyboxPrefilterShader.bind();

    int curMipSize = envMapSize;
    // unfortunately we cannot do all mip levels in a single dispatch
    // since there is no way to bind a variable amount of images of variable sizes to a shader
    for (int mip = 0; mip < numMipLevels; mip++) {
        glViewport(0, 0, curMipSize, curMipSize);
        envMapTex.bindImage(2, GL_WRITE_ONLY, mip);
        skyboxPrefilterShader.setUniform("roughness", mip / static_cast<float>(numMipLevels - 1));
        skyboxConvolutionShader.dispatch(GL_TEXTURE_FETCH_BARRIER_BIT, curMipSize, curMipSize, 6);
        curMipSize /= 2;
    }

    glMakeTextureHandleNonResidentARB(skyboxMipMapHandle);
    glDeleteTextures(1, &skyboxMipMapID);

    return Skybox{ std::move(skyboxTex), std::move(irradianceTex), std::move(envMapTex), numMipLevels };
}

Skybox::Skybox(Texture skyboxTex, Texture irradianceTex, Texture envmapTex, int numMipLevels) :
    m_skyboxTex{ std::move(skyboxTex) },
    m_irradianceTex{ std::move(irradianceTex) },
    m_envmapTex{ std::move(envmapTex) },
    m_numMipLevels{ numMipLevels } {}