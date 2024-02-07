#ifndef MODEL_H
#define MODEL_H

#include <fastgltf/parser.hpp>
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"
#include "commandbuffer.h"
#include "aabb.h"

class Model {
	public:
		const AABB m_aabb;
		const glm::mat4 m_baseTransform;

		void draw();

		static Model make(const char* pathStr = nullptr);

	private:
		Model(CommandBuffer cmdBuf, VertexBuffer vBuf, IndexBuffer iBuf, VertexArray vArr, glm::mat4 baseTransform, AABB aabb);

		static inline fastgltf::Parser m_parser;
		CommandBuffer m_cmdBuf;
		VertexBuffer m_vBuf;
		IndexBuffer m_iBuf;
		VertexArray m_vArr;
};

#endif
