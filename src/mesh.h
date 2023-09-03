#ifndef MESH_H
#define MESH_H

#include <vector>
#include <optional>

#include "vertex.h"
#include "vertexarray.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "camera.h"
#include "texture.h"

class Mesh {
private:
	size_t m_numIndices;
	std::optional<Texture> m_diffuseMap, m_normalMap, m_aoMap, m_metalnessMap, m_roughnessMap;

	VertexArray m_vArr;
	VertexBuffer m_vBuf;
	IndexBuffer m_iBuf;

public:
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::optional<Texture> diffuseMap, std::optional<Texture> normalMap, std::optional<Texture> aoMap, std::optional<Texture> metalnessMap, std::optional<Texture> roughnessMap);

	void bindTextures(ShaderProgram& shaderProgram);
	void draw(ShaderProgram& shaderProgram, Camera& camera, glm::mat4 model = glm::mat4(1.0f));
	void drawTextureless(ShaderProgram& shaderProgram, Camera& camera, glm::mat4 model = glm::mat4(1.0f));
};

#endif

