#ifndef DBG_H
#define DBG_H

#ifdef NDEBUG
	#define print(x)
	#define checkErr()
	#define checkCompile(x)
	#define checkLink(x)
	#define validatePath(x)
#else
	#include <iostream>
	#include <iomanip>
	#include <filesystem>
	#include <glad/glad.h>
    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
	#define print(x) std::cout << x << '\n'
	#define checkCompile(shader) {\
		GLint success = 0;\
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);\
		if (success == GL_FALSE) {\
			GLint logSize = 0;\
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);\
			GLchar* buf = new GLchar[logSize];\
			glGetShaderInfoLog(shader, logSize, &logSize, buf);\
			std::cout << buf << std::endl;\
			delete[] buf;\
		}\
	}
#define checkLink(shader) {\
		GLint success = 0;\
		glGetProgramiv(shader, GL_LINK_STATUS, &success);\
		if (success == GL_FALSE) {\
			GLint logSize = 0;\
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logSize);\
			GLchar* buf = new GLchar[logSize];\
			glGetProgramInfoLog(shader, logSize, &logSize, buf);\
			std::cout << buf << std::endl;\
			delete[] buf;\
		}\
	}
	#define validatePath(x) {\
		if(!std::filesystem::exists(x)) std::cout << "Error: path " << x << " does not exist!\n";\
	}
	#define installDbgCallback() {\
		glEnable(GL_DEBUG_OUTPUT); \
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); \
		glDebugMessageCallback(glDebugOutput, nullptr); \
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE); \
	}
#endif

#endif
