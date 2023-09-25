#include "shaderprogram.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include "dbg.h"

ShaderProgram::ShaderProgram(const char* vertexFile, const char* fragmentFile) {

	// read shader source code from file and convert to c string
	std::string vShaderSource = getShaderSource(vertexFile);
	std::string fShaderSource = getShaderSource(fragmentFile);
	const char* vShaderSource_cstr = vShaderSource.c_str();
	const char* fShaderSource_cstr = fShaderSource.c_str();

	// compile vertex shader
	GLid vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vShaderSource_cstr, NULL);
	glCompileShader(vertexShaderID);

	checkCompile(vertexShaderID);

	// compile fragment shader
	GLid fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShaderID, 1, &fShaderSource_cstr, NULL);
	glCompileShader(fragShaderID);

	checkCompile(fragShaderID);

	// create shader program
	m_id = glCreateProgram();
	glAttachShader(m_id, vertexShaderID);
	glAttachShader(m_id, fragShaderID);
	glLinkProgram(m_id);

	// cleanup shaders
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragShaderID);
	glUseProgram(m_id);
}

std::string ShaderProgram::getShaderSource(const char* path) {
	std::ifstream infile(path);
	return std::string((std::istreambuf_iterator<char>(infile)), (std::istreambuf_iterator<char>()));
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(m_id);
}

void ShaderProgram::activate() {
	glUseProgram(m_id);
}

void ShaderProgram::deactivate() {
	glUseProgram(0);
}

template<>
void ShaderProgram::setUniform(const char* uniform, glm::mat4 value) {
	glUniformMatrix4fv(glGetUniformLocation(m_id, uniform), 1, GL_FALSE, glm::value_ptr(value));
}

template<>
void ShaderProgram::setUniform(const char* uniform, glm::vec3 value) {
	glUniform3fv(glGetUniformLocation(m_id, uniform), 1, glm::value_ptr(value));
}

template<>
void ShaderProgram::setUniform(const char* uniform, int value) {
	glUniform1i(glGetUniformLocation(m_id, uniform), value);
}

template<>
void ShaderProgram::setUniform(const char* uniform, GLuint value) {
	glUniform1i(glGetUniformLocation(m_id, uniform), value);
}

template<>
void ShaderProgram::setUniform(const char* uniform, float value) {
	glUniform1f(glGetUniformLocation(m_id, uniform), value);
}

template<typename T>
void ShaderProgram::setUniform(const char* uniform, T value) {}