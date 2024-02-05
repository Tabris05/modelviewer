#ifndef DBG_H
#define DBG_H

#ifdef NDEBUG
	#define print(x)
	#define checkErr()
	#define checkCompile(x)
	#define validatePath(x)
#else
	#include <iostream>
	#include <iomanip>
	#include <filesystem>
	#define print(x) std::cout << x << '\n'
	#define checkErr() for (GLenum glErr = glGetError(); glErr != 0; glErr = glGetError()) std::cout << std::hex << glErr << "\n";
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
	#define validatePath(x) {\
		if(!std::filesystem::exists(x)) std::cout << "Error: path " << x << " does not exist!\n";\
	}
#endif

#endif
