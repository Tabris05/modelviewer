#ifndef MODEL_H
#define MODEL_H

#include <filesystem>
#include <unordered_map>
#include <fastgltf/core.hpp>
#include "texture.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "vertexarray.h"
#include "commandbuffer.h"
#include "shaderstoragebuffer.h"
#include "aabb.h"

class Model {
	public:
		void draw();
		AABB aabb() const;
		glm::mat4 baseTransform() const;

		static Model make();
		static Model make(const std::filesystem::path& path);

	private:
		Model(
			std::unordered_map<size_t, Texture> textures,
			ShaderStorageBuffer materialBuf,
			CommandBuffer cmdBuf,
			VertexBuffer vBuf,
			IndexBuffer iBuf,
			VertexArray vArr,
			glm::mat4 baseTransform,
			AABB aabb
		);

		static inline fastgltf::Parser m_parser{ fastgltf::Extensions::KHR_materials_emissive_strength };

		AABB m_aabb;
		glm::mat4 m_baseTransform;
		std::unordered_map<size_t, Texture> m_textures;
		ShaderStorageBuffer m_materialBuf;
		CommandBuffer m_cmdBuf;
		VertexBuffer m_vBuf;
		IndexBuffer m_iBuf;
		VertexArray m_vArr;
};

#endif
