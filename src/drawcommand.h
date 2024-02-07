#ifndef DRAWCOMMAND_H
#define DRAWCOMMAND_H

#include <glad/glad.h>

struct DrawCommand {
	GLuint count, instanceCount, firstIndex, baseVertex, baseInstance;
};

#endif
