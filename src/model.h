#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <optional>
#include "mesh.h"
#include "aabb.h"

class Model {
private:
	std::vector<Mesh> m_meshes;
	std::vector<glm::mat4> m_transformations;
	std::unordered_map<std::string, std::optional<Texture>> m_loadedTextures;
	std::string m_directory;
	glm::mat4 m_baseTransform;
	aabb m_aabb;

	GLuint m_texSlots = 0;

	void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform = glm::mat4(1.0f));
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::optional<Texture> loadMaterialTexture(aiMaterial* mat, aiTextureType type);
	glm::mat4 aiMat4ToGLM(const aiMatrix4x4* from);
public:
	Model(std::string path);
	void draw(ShaderProgram& shader, Camera& camera, glm::mat4 transformation = glm::mat4(1.0f));
};

#endif