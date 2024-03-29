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

	template<typename T>
	static ShaderStorageBuffer make(const std::vector<T>& bufferData) {
		GLuint id;
		glCreateBuffers(1, &id);
		glNamedBufferData(id, bufferData.size() * sizeof(T), bufferData.data(), GL_STATIC_DRAW);
		return ShaderStorageBuffer{ id };
	}
	~ShaderStorageBuffer();

private:
	ShaderStorageBuffer(GLuint id);

	GLuint m_id;
	RefCounter m_rc;
};

#endif
