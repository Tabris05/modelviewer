#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include "refcounter.h"
#include "texture.h"
#include "renderbuffer.h"

class FrameBuffer {
	public:
		void bind();
		void unbind();
		GLuint id() const;
		void attachTexture(const Texture& texture, GLenum attachment, int mip = 0);
		void detachTexture(GLenum attachment, int mip = 0);
		void attachRenderBuffer(const RenderBuffer& renderBuffer, GLenum attachment);
		void detachRenderBuffer(GLenum attachment);
		void blitTo(FrameBuffer& dst, GLenum mask, int x, int y);
	
		static FrameBuffer make();
		~FrameBuffer();
	
	private:
		FrameBuffer(GLuint id);
		
		GLuint m_id;
		RefCounter m_rc;
		static inline GLuint m_boundID = 0;
};

#endif
