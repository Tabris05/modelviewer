#include "model.h"
#include <algorithm>
#include <climits>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

Model::Model(std::string path) : m_directory{ path.substr(0, path.find_last_of('\\') + 1) }, m_baseTransform{ glm::mat4(1.0f) }, m_aabb { glm::vec3(FLT_MAX), glm::vec3(FLT_MIN) } {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
	processNode(scene->mRootNode, scene);
	m_baseTransform = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 size = m_aabb.m_max - m_aabb.m_min;
	float scale = 1.0f / std::max(size.x, std::max(size.y, size.z));
	m_baseTransform = glm::scale(m_baseTransform, glm::vec3(scale));
	glm::vec3 vertices[8] = {
		glm::vec3(m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z),
		glm::vec3(m_aabb.m_max.x, m_aabb.m_min.y, m_aabb.m_min.z),
		glm::vec3(m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_min.z),
		glm::vec3(m_aabb.m_min.x, m_aabb.m_max.y, m_aabb.m_min.z),
		glm::vec3(m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_max.z),
		glm::vec3(m_aabb.m_max.x, m_aabb.m_min.y, m_aabb.m_max.z),
		glm::vec3(m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z),
		glm::vec3(m_aabb.m_min.x, m_aabb.m_max.y, m_aabb.m_max.z),
	};
	for (auto& i : vertices) i = glm::vec3(m_baseTransform * glm::vec4(i, 1.0f));
	m_aabb = { glm::vec3(FLT_MAX), glm::vec3(FLT_MIN) };
	for (auto i : vertices) {
		m_aabb.m_min = glm::min(m_aabb.m_min, i);
		m_aabb.m_max = glm::max(m_aabb.m_max, i);
	}
}

void Model::processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform) {
	glm::mat4 tmp = aiMat4ToGLM(&node->mTransformation);
	glm::mat4 curTransform = parentTransform * tmp;
	for (size_t i = 0; i < node->mNumMeshes; i++) {
		m_transformations.push_back(curTransform);
		m_meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
	}
	for (size_t i = 0; i < node->mNumChildren; i++) processNode(node->mChildren[i], scene, curTransform);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Mesh::ColorData diffuse = glm::vec3(1.0f);
	Mesh::MaterialData ao = std::monostate{}, metalness = std::monostate{}, roughness = std::monostate{};
	Mesh::NormalData normal;
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiColor4D outVal;
		if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &outVal) == AI_SUCCESS) diffuse = glm::vec3(outVal.r, outVal.g, outVal.b);
		if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &outVal) == AI_SUCCESS) ao = std::max(outVal.r, std::max(outVal.g, std::max(outVal.b, outVal.a)));
		if (aiGetMaterialColor(material, AI_MATKEY_METALLIC_FACTOR, &outVal) == AI_SUCCESS) metalness = outVal.r;
		if (aiGetMaterialColor(material, AI_MATKEY_ROUGHNESS_FACTOR, &outVal) == AI_SUCCESS) roughness = outVal.r;

		std::optional<Texture> diffuseMap = loadMaterialTexture(material, aiTextureType_DIFFUSE);
		if (diffuseMap) diffuse = diffuseMap.value();
		std::optional<Texture> aoMap = loadMaterialTexture(material, aiTextureType_LIGHTMAP);
		if (aoMap) ao = aoMap.value();
		std::optional<Texture> metalnessMap = loadMaterialTexture(material, aiTextureType_METALNESS);
		if (metalnessMap) metalness = metalnessMap.value();
		std::optional<Texture> roughnessMap = loadMaterialTexture(material, aiTextureType_DIFFUSE_ROUGHNESS);
		if (roughnessMap) roughness = roughnessMap.value();
		normal = loadMaterialTexture(material, aiTextureType_NORMALS);
	}

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		glm::vec3 worldSpaceVertex = glm::vec3(m_transformations.back() * glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f));
		m_aabb.m_min = glm::min(m_aabb.m_min, worldSpaceVertex);
		m_aabb.m_max = glm::max(m_aabb.m_max, worldSpaceVertex);
		vertices.push_back(Vertex{
			glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
			glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
			(mesh->mTangents ? glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z) : glm::vec3(0.0f)),
			mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f),
		});
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace curFace = mesh->mFaces[i];
		for (size_t j = 0; j < curFace.mNumIndices; j++) indices.push_back(curFace.mIndices[j]);
	}

	return Mesh(std::move(vertices), std::move(indices), std::move(diffuse), std::move(normal), std::move(ao), std::move(metalness), std::move(roughness));
}

std::optional<Texture> Model::loadMaterialTexture(aiMaterial* mat, aiTextureType type) {
	GLenum colorspace;
	switch (type) {
		case aiTextureType_DIFFUSE:
			colorspace = GL_SRGB_ALPHA;
			break;
		default:
			colorspace = GL_RGBA;
			break;
	}
	aiString str;
	mat->GetTexture(type, 0, &str);
	if (str.length > 0) {
		std::string fullPath = m_directory + std::string(str.C_Str());
		std::optional<Texture> maybeTex = m_loadedTextures[fullPath];
		if (maybeTex)  return *maybeTex;
		Texture result{ fullPath.c_str(), colorspace, m_texSlotStart++ };
		m_loadedTextures[fullPath] = result;
		return result;
	}
	return {};
}

glm::mat4 Model::aiMat4ToGLM(const aiMatrix4x4* from) {
	glm::mat4 to;
	to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
	to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
	to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
	to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;
	return to;
}

aabb Model::getAABB() const {
	return m_aabb;
}

void Model::draw(ShaderProgram& shader, glm::mat4 transformation) {
	for (size_t i = 0; i < m_meshes.size(); i++) m_meshes[i].draw(shader, transformation * m_baseTransform * m_transformations[i]);
}