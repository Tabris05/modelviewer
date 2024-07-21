#include "computeshader.h"
#include "dbg.h"

void ComputeShader::dispatch(GLenum barrierBits, GLuint xDim, GLuint yDim, GLuint zDim) {
	// for now, assume local group size of 8 for x and y and 1 for z
	glDispatchCompute((xDim + 7 / 8), (yDim + 7 / 8), zDim );
	glMemoryBarrier(barrierBits);
}

ComputeShader ComputeShader::make(const char* csPath) {
	validatePath(csPath);

	std::string csSource = getShaderSource(csPath);
	const char* csSource_cstr = csSource.c_str();

	GLuint csID = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(csID, 1, &csSource_cstr, NULL);
	glCompileShader(csID);

	checkCompile(csID);

	GLuint id = glCreateProgram();
	glAttachShader(id, csID);
	glLinkProgram(id);

	glDetachShader(id, csID);
	glDeleteShader(csID);

	return ComputeShader{ id };
}

ComputeShader::ComputeShader(GLuint id) : Shader(id) {}