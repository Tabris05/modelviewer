#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <glad/glad.h>

#include <string>

using GLid = GLuint;

class ShaderProgram {
private:
	GLid m_id;

	std::string getShaderSource(const char* path);

public:
	ShaderProgram(const char* vertexFile, const char* fragmentFile);
	~ShaderProgram();
	operator GLuint() const;

	void activate();
	void deactivate();
};

#endif

