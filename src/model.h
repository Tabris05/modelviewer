#ifndef MODEL_H
#define MODEL_H

#include <fastgltf/parser.hpp>
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"

class Model {
	public:
		void draw();

		static Model make(const char* pathStr = nullptr);

	private:
		Model(VertexBuffer vBuf, IndexBuffer iBuf, VertexArray vArr, GLuint numIndices);

		static void processNode(const fastgltf::Asset& asset, size_t index, glm::mat4 transform, std::vector<Vertex>& vertices, std::vector<GLuint>& indices);

		static inline fastgltf::Parser m_parser;
		VertexBuffer m_vBuf;
		IndexBuffer m_iBuf;
		VertexArray m_vArr;
		GLuint m_numIndices;
};

#endif
