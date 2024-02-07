#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <vector>
#include <glad/glad.h>
#include "refcounter.h"
#include "drawcommand.h"

class CommandBuffer {
public:
	void bind();
	void unbind();
	GLuint id() const;
	size_t numCommands() const;

	static CommandBuffer make(const std::vector<DrawCommand>& cmds);
	~CommandBuffer();

private:
	CommandBuffer(GLuint id, size_t numCommands);

	GLuint m_id;
	size_t m_numCommands;
	RefCounter m_rc;
	static inline GLuint m_boundID = 0;
};

#endif
