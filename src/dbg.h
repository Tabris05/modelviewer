#ifndef DBG_H
#define DBG_H

#ifdef NDEBUG
	#define log(x)
	#define checkErr()
	#define checkCompile(x)
#else
	#include <iostream>
	#include <iomanip>
	#include <filesystem>
	#define log(x) std::cout << x << '\n'
	#define checkErr() for (GLenum glErr = glGetError(); glErr != 0; glErr = glGetError()) log(std::hex << glErr)
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
	#define validatePath(path) {\
		if(!std::filesystem::exists(path)) std::cout << "Error: path " << path << " does not exist!\n";\
	}
#endif

#endif
