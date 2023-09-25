#ifndef MULTISAMPLEDFRAMEBUFFER_H
#define MULTISAMPLEDFRAMEBUFFER_H

#include "framebuffer.h"
class MultisampledFramebuffer : public Framebuffer {
public:
	MultisampledFramebuffer(int width, int height);
	MultisampledFramebuffer(const MultisampledFramebuffer& src) = delete;
	MultisampledFramebuffer(MultisampledFramebuffer&& src) = delete;

	void attatchTexture(GLenum attatchment, GLenum internalFormat);
	void attatchRenderBuffer(GLenum attatchment, GLenum internalFormat);
};

#endif
