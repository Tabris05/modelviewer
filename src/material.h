#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>

// if anything is modified here, a modification must also be made to the macro definitions in model.frag
enum class TextureBitfield : GLuint {
	NONE = 0x00,
	HAS_ALBEDO = 0x01,
	HAS_NORMAL = 0x02,
	HAS_METALLIC_ROUGHNESS = 0x04,
	HAS_OCCLUSION = 0x08,
	HAS_EMISSIVE = 0x10
};

inline TextureBitfield& operator|=(TextureBitfield& lhs, TextureBitfield rhs) {
	return (TextureBitfield&)((GLuint&)(lhs) |= (GLuint)(rhs));
}

struct Material {
	glm::vec4 m_baseColor;
	glm::vec4 m_emissiveColor;
	GLuint64 m_albedoHandle;
	GLuint64 m_normalHandle;
	GLuint64 m_occlusionHandle;
	GLuint64 m_metallicRoughnessHandle;
	GLuint64 m_emissiveHandle;
	float m_metalness;
	float m_roughness;
	TextureBitfield m_textureBitfield;
	GLuint64 _padding;
};

#endif

