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
		glNamedBufferStorage(id, bufferData.size() * sizeof(T), bufferData.data(), NULL);
		return ShaderStorageBuffer{ id };
	}
	static ShaderStorageBuffer make();
	~ShaderStorageBuffer();
	void test(const std::vector<int>& bufferData = std::vector<int>{}) {};
private:
	ShaderStorageBuffer(GLuint id);

	GLuint m_id;
	RefCounter m_rc;
};

#endif
