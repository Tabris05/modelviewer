#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/glad.h>
#include "refcounter.h"

class Shader {
	public:
		void bind();
		void unbind();
		GLuint id() const;

		template<typename T>
		void setUniform(const char* uniform, T value);

		static Shader make(const char* vsPath, const char* fsPath);
		~Shader();

	private:
		Shader(GLuint id);

		static std::string getShaderSource(const char* path);

		GLuint m_id;
		RefCounter m_rc;
		static inline GLuint m_boundID = 0;
};

#endif