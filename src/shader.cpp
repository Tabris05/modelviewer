#include "shader.h"
#include <fstream>
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

void Shader::setUniform(const char* uniform, glm::mat4 value) {
	for (auto& [name, location, oldVal] : m_mat4Cache) {
		if (name == uniform) {
			if (value != oldVal) {
				glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
				oldVal = value;
			}
			return;
		}
	}
	GLint location = glGetUniformLocation(m_id, uniform);
	glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
	m_mat4Cache.emplace_back(uniform, location, value);
}

void Shader::setUniform(const char* uniform, glm::vec3 value) {
	for (auto& [name, location, oldVal] : m_vec3Cache) {
		if (name == uniform) {
			if (value != oldVal) {
				glProgramUniform3fv(m_id, location, 1, glm::value_ptr(value));
				oldVal = value;
			}
			return;
		}
	}
	GLint location = glGetUniformLocation(m_id, uniform);
	glProgramUniform3fv(m_id, location, 1, glm::value_ptr(value));
	m_vec3Cache.emplace_back(uniform, location, value);
}

void Shader::setUniform(const char* uniform, GLuint value) {
	for (auto& [name, location, oldVal] : m_uintCache) {
		if (name == uniform) {
			if (value != oldVal) {
				glProgramUniform1i(m_id, location, value);
				oldVal = value;
			}
			return;
		}
	}
	GLint location = glGetUniformLocation(m_id, uniform);
	glProgramUniform1i(m_id, location, value);
	m_uintCache.emplace_back(uniform, location, value);
}

void Shader::setUniform(const char* uniform, float value) {
	for (auto& [name, location, oldVal] : m_floatCache) {
		if (name == uniform) {
			if (value != oldVal) {
				glProgramUniform1f(m_id, location, value);
				oldVal = value;
			}
			return;
		}
	}
	GLint location = glGetUniformLocation(m_id, uniform);
	glProgramUniform1f(m_id, location, value);
	m_floatCache.emplace_back(uniform, location, value);
}

void Shader::setUniform(const char* uniform, int value) {
	for (auto& [name, location, oldVal] : m_intCache) {
		if (name == uniform) {
			if (value != oldVal) {
				glProgramUniform1i(m_id, location, value);
				oldVal = value;
			}
			return;
		}
	}
	GLint location = glGetUniformLocation(m_id, uniform);
	glProgramUniform1i(m_id, location, value);
	m_intCache.emplace_back(uniform, location, value);
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

	return Shader{ id };
}

Shader::~Shader() {
	if (m_rc.count() == 0) glDeleteProgram(m_id);
}

Shader::Shader(GLuint id) : m_id{ id } {}

std::string Shader::getShaderSource(const char* path) {
	std::ifstream infile(path);
	return std::string((std::istreambuf_iterator<char>(infile)), (std::istreambuf_iterator<char>()));
}