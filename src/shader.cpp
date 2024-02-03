#include "shader.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "dbg.h"

void Shader::bind() {
	if (m_boundID != m_id) {
		glUseProgram(m_id);
		m_boundID = m_id;
	}
}

void Shader::unbind() {
	if (m_boundID == m_id) {
		glUseProgram(0);
		m_boundID = 0;
	}
}

GLuint Shader::id() const {
	return m_id;
}

template<>
void Shader::setUniform(const char* uniform, glm::mat4 value) {
	glProgramUniformMatrix4fv(m_id, glGetUniformLocation(m_id, uniform), 1, GL_FALSE, glm::value_ptr(value));
}

template<>
void Shader::setUniform(const char* uniform, glm::vec3 value) {
	glProgramUniform3fv(m_id, glGetUniformLocation(m_id, uniform), 1, glm::value_ptr(value));
}

template<>
void Shader::setUniform(const char* uniform, int value) {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, uniform), value);
}

template<>
void Shader::setUniform(const char* uniform, GLuint value) {
	glProgramUniform1i(m_id, glGetUniformLocation(m_id, uniform), value);
}

template<>
void Shader::setUniform(const char* uniform, float value) {
	glProgramUniform1f(m_id, glGetUniformLocation(m_id, uniform), value);
}

template<typename T>
void Shader::setUniform(const char* uniform, T value) {
	log("passed a type to set uniform for which no overload exists");
}

Shader Shader::make(const char* vsPath, const char* fsPath) {
	validatePath(vsPath);
	validatePath(fsPath);

	std::string vsSource = getShaderSource(vsPath);
	std::string fsSource = getShaderSource(fsPath);
	const char* vsSource_cstr = vsSource.c_str();
	const char* fsSource_cstr = fsSource.c_str();

	GLuint vsID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsID, 1, &vsSource_cstr, NULL);
	glCompileShader(vsID);

	checkCompile(vsID);

	GLuint fsID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsID, 1, &fsSource_cstr, NULL);
	glCompileShader(fsID);

	checkCompile(fsID);

	GLuint id = glCreateProgram();
	glAttachShader(id, vsID);
	glAttachShader(id, fsID);
	glLinkProgram(id);

	glDeleteShader(vsID);
	glDeleteShader(fsID);

	return Shader(id);
}

Shader::~Shader() {
	if (m_rc.count() == 0) glDeleteProgram(m_id);
}

Shader::Shader(GLuint id) : m_id{ id } {}

std::string Shader::getShaderSource(const char* path) {
	std::ifstream infile(path);
	return std::string((std::istreambuf_iterator<char>(infile)), (std::istreambuf_iterator<char>()));
}