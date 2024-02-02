#ifndef MESH_H
#define MESH_H

#include <vector>
#include <variant>
#include <optional>

#include "vertex.h"
#include "vertexarray.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "camera.h"
#include "texture.h"

class Mesh {
public:
	using ColorData = std::variant<Texture, glm::vec3>;
	using MaterialData = std::variant<Texture, float, std::monostate>;
	using NormalData = std::optional<Texture>;
private:
	size_t m_numIndices;
	ColorData m_diffuse, m_emission;
	NormalData m_normal;
	MaterialData m_ao, m_metalness, m_roughness;

	VertexArray m_vArr;
	VertexBuffer m_vBuf;
	IndexBuffer m_iBuf;

	float m_emissiveIntensity;

public:
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, ColorData diffuse, ColorData emission, float emissiveIntensity, NormalData normal, MaterialData ao, MaterialData metalness, MaterialData roughness);

	void bindTextures(ShaderProgram& shaderProgram);

	void draw(ShaderProgram& shaderProgram, glm::mat4 model = glm::mat4(1.0f));
};

#endif

