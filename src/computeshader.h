#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include "shader.h"

class ComputeShader : public Shader {
	public:
		void dispatch(GLenum barrierBits, GLuint xDim = 1, GLuint yDim = 1, GLuint zDim = 1);

		static ComputeShader make(const char* csPath);

	private:
		ComputeShader(GLuint id, glm::ivec3 workgroupSize);

		glm::ivec3 m_workgroupSize;
};

#endif
