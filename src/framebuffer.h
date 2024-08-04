#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include "refcounter.h"
#include "texture.h"

class FrameBuffer {
	public:
		void bind();
		void unbind();
		GLuint id() const;
		void attachTexture(const Texture& texture, GLenum attachment, int mip = 0);
		void detachTexture(GLenum attachment, int mip = 0);
		void blitTo(FrameBuffer& dst, GLenum mask, int x, int y);
		void blitToDefault(GLenum mask, int x, int y);
	
		static FrameBuffer make();
		FrameBuffer(const FrameBuffer& src) = default;
		FrameBuffer(FrameBuffer&& src) = default;
		FrameBuffer& operator=(const FrameBuffer& src);
		FrameBuffer& operator=(FrameBuffer&& src) noexcept;
		~FrameBuffer();
	
	private:
		FrameBuffer(GLuint id);
		
		GLuint m_id;
		RefCounter m_rc;
		static inline GLuint m_boundID = 0;
};

#endif
