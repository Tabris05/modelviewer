#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "refcounter.h"

class ShaderStorageBuffer {
public:
	void bind(size_t index);
	GLuint id() const;

	static ShaderStorageBuffer make(const std::vector<glm::vec4>& bufferData);
	~ShaderStorageBuffer();

private:
	ShaderStorageBuffer(GLuint id);

	GLuint m_id;
	RefCounter m_rc;
};

#endif
