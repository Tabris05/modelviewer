#ifndef SHADER_H
#define SHADER_H

#include <tuple>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "refcounter.h"

class Shader {
	public:
		void bind();
		void unbind();
		GLuint id() const;

		void setUniform(const char* uniform, glm::mat4 value);
		void setUniform(const char* uniform, glm::mat3 value);
		void setUniform(const char* uniform, glm::vec3 value);
		void setUniform(const char* uniform, glm::vec2 value);
		void setUniform(const char* uniform, GLuint64 value);
		void setUniform(const char* uniform, GLuint value);
		void setUniform(const char* uniform, float value);
		void setUniform(const char* uniform, int value);

		static Shader makeGraphics(const char* vsPath, const char* fsPath);
		static Shader makeCompute(const char* csPath);
		Shader(const Shader& src) = default;
		Shader(Shader&& src) = default;
		Shader& operator=(const Shader& src);
		Shader& operator=(Shader&& src) noexcept;
		~Shader();

	private:
		Shader(GLuint id);

		static std::string getShaderSource(const char* path);

		GLuint m_id;
		RefCounter m_rc;
		static inline GLuint m_boundID = 0;

		// shader should never have more than a few uniforms so linear, cache coherent traversal should be fastest
		std::vector<std::tuple<const char*, GLint, glm::mat4>> m_mat4Cache;
		std::vector<std::tuple<const char*, GLint, glm::mat3>> m_mat3Cache;
		std::vector<std::tuple<const char*, GLint, glm::vec3>> m_vec3Cache;
		std::vector<std::tuple<const char*, GLint, glm::vec2>> m_vec2Cache;
		std::vector<std::tuple<const char*, GLint, GLuint64>> m_ulongCache;
		std::vector<std::tuple<const char*, GLint, GLuint>> m_uintCache;
		std::vector<std::tuple<const char*, GLint, float>> m_floatCache;
		std::vector<std::tuple<const char*, GLint, int>> m_intCache;
};

#endif