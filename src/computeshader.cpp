#include "computeshader.h"
#include "dbg.h"
#include <glm/gtc/type_ptr.hpp>

void ComputeShader::dispatch(GLenum barrierBits, GLuint xDim, GLuint yDim, GLuint zDim) {
	glm::ivec3 invocations = (glm::ivec3(xDim, yDim, zDim) + m_workgroupSize - 1) / m_workgroupSize;
	glDispatchCompute(invocations.x, invocations.y, invocations.z);
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

	checkLink(id);

	glDetachShader(id, csID);
	glDeleteShader(csID);

	glm::ivec3 workgroupSize;
	glGetProgramiv(id, GL_COMPUTE_WORK_GROUP_SIZE, glm::value_ptr(workgroupSize));

	return ComputeShader{ id, workgroupSize };
}

ComputeShader::ComputeShader(GLuint id, glm::ivec3 workgroupSize) : Shader(id), m_workgroupSize{ workgroupSize } {}