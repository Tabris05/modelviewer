#ifndef DBG_H
#define DBG_H

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#define LOG(x) std::cout << x << '\n'
#define checkErr() for (GLenum glErr = glGetError(); glErr != 0; glErr = glGetError()) LOG(std::hex << glErr)
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
#else
#define LOG(x)
#define checkErr()
#define checkCompile(x)
#endif

#endif
