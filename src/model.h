#ifndef MODEL_H
#define MODEL_H

#include <fastgltf/parser.hpp>
#include "texture.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"
#include "commandbuffer.h"
#include "shaderstoragebuffer.h"
#include "aabb.h"

class Model {
	public:
		const AABB m_aabb;
		const glm::mat4 m_baseTransform;

		void draw();

		static Model make(const char* pathStr = nullptr);

	private:
		Model(
			std::vector<Texture> textures,
			ShaderStorageBuffer materialBuf,
			ShaderStorageBuffer materialIndexBuf,
			CommandBuffer cmdBuf,
			VertexBuffer vBuf,
			IndexBuffer iBuf,
			VertexArray vArr,
			glm::mat4 baseTransform,
			AABB aabb
		);

		static inline fastgltf::Parser m_parser;

		std::vector<Texture> m_textures;
		ShaderStorageBuffer m_materialBuf;
		ShaderStorageBuffer m_materialIndexBuf;
		CommandBuffer m_cmdBuf;
		VertexBuffer m_vBuf;
		IndexBuffer m_iBuf;
		VertexArray m_vArr;
};

#endif
