#include "skybox.h"
#include "framebuffer.h"

#include <stb/stb_image.h>
#include <iostream>

#include "dbg.h"

Skybox::Skybox(float& outNumMipLevels) : 
    m_vBuf{ std::vector<float>{
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    } } {
    m_vArr.linkAttribute(m_vBuf, 0, 3, GL_FLOAT, sizeof(float) * 3, 0);

    // load hdr equirectangular map
    GLid hdrTexID;
    int width, height, numColCh;
    stbi_set_flip_vertically_on_load(true);
    float* data = stbi_loadf("./skybox.hdr", &width, &height, &numColCh, 0);
    glGenTextures(1, &hdrTexID);
    glBindTexture(GL_TEXTURE_2D, hdrTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    
    // allocate cubemap
    int cubemapSize = height / 2;
	glGenTextures(1, &m_skyboxTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexID);

	for (size_t i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, cubemapSize, cubemapSize, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // render equirectangular map to cubemap faces
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 views[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    glViewport(0, 0, cubemapSize, cubemapSize);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexID);

    Framebuffer captureBuffer{ cubemapSize, cubemapSize };
    captureBuffer.attatchRenderBuffer(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24);

    ShaderProgram skyboxConversionShader{ "./shaders/skyboxconversion_vertex.glsl", "./shaders/skyboxconversion_fragment.glsl" };
    skyboxConversionShader.setUniform("equirectangular", 0);

    for (size_t i = 0; i < 6; i++) {
        skyboxConversionShader.setUniform("camMatrix", projection * views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_skyboxTexID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 36);

    }
    for (size_t i = 0; i < 6; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0);
    }

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexID);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glDeleteTextures(1, &hdrTexID);

    // generate irradiance map by convoluting skybox cubemap
    static constexpr int irradianceMapSize = 32;
    glGenTextures(1, &m_irradianceTexID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceTexID);
    for (size_t i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceMapSize, irradianceMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glViewport(0, 0, irradianceMapSize, irradianceMapSize);
    

    captureBuffer.reallocRenderBuffer(0, irradianceMapSize, irradianceMapSize, GL_DEPTH_COMPONENT24);

    ShaderProgram convolutionShader{ "./shaders/skyboxconversion_vertex.glsl", "./shaders/skyboxconvolution_fragment.glsl" };
    convolutionShader.setUniform("skybox", 3);

    for (size_t i = 0; i < 6; i++) {
        skyboxConversionShader.setUniform("camMatrix", projection * views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceTexID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 36);

    }
    for (size_t i = 0; i < 6; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0);
    }

    // generate prefiltered environment map
    static constexpr int prefilteredMapSize = 512;
    const size_t numMipLevels = log2(prefilteredMapSize);
    outNumMipLevels = numMipLevels;

    glGenTextures(1, &m_prefilterTexID);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterTexID);
    for (size_t i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilteredMapSize, prefilteredMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    
    ShaderProgram prefilterShader{ "./shaders/skyboxconversion_vertex.glsl", "./shaders/skyboxprefilter_fragment.glsl" };
    prefilterShader.setUniform("skybox", 3);
    prefilterShader.setUniform("resolution", (float)cubemapSize);
    
    int curPrefilteredSize = prefilteredMapSize;
    for (size_t mip = 0; mip < numMipLevels; mip++) {
        captureBuffer.reallocRenderBuffer(0, curPrefilteredSize, curPrefilteredSize, GL_DEPTH_COMPONENT24);
        glViewport(0, 0, curPrefilteredSize, curPrefilteredSize);
        curPrefilteredSize /= 2;

        float roughness = mip / (float)(numMipLevels - 1);
        prefilterShader.setUniform("roughness", roughness);
        for (size_t i = 0; i < 6; i++) {
            prefilterShader.setUniform("camMatrix", projection * views[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterTexID, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    for (size_t i = 0; i < 6; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0);
    }

    m_vArr.unbind();
    m_vBuf.unbind();

    // compute BRDF integration map
    const int brdfLUTSize = 512;
    glGenTextures(1, &m_brdfLUTTexID);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdfLUTSize, brdfLUTSize, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glViewport(0, 0, brdfLUTSize, brdfLUTSize);

    captureBuffer.reallocRenderBuffer(0, brdfLUTSize, brdfLUTSize, GL_DEPTH_COMPONENT24);
    
    ShaderProgram brdfLUTShader{ "./shaders/brdflut_vertex.glsl", "./shaders/brdflut_fragment.glsl" };

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUTTexID, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    VertexArray vArr;
    VertexBuffer vBuf{ std::vector<float>{
        1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f,  1.0f,  1.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    } };
    vArr.linkAttribute(vBuf, 0, 2, GL_FLOAT, 4 * sizeof(float), 0);
    vArr.linkAttribute(vBuf, 1, 2, GL_FLOAT, 4 * sizeof(float), 2 * sizeof(float));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
}

Skybox::~Skybox() {
    glDeleteTextures(1, &m_skyboxTexID);
    glDeleteTextures(1, &m_irradianceTexID);
    glDeleteTextures(1, &m_prefilterTexID);
    glDeleteTextures(1, &m_brdfLUTTexID);
}

void Skybox::draw(ShaderProgram& shaderProgram) {
    m_vArr.bind();
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
