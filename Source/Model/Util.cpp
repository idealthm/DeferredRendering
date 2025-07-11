#include "Util.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Texture.h"
#include "VertexBuffer/VertexBufferLayout.h"
#include "VertexArray/VertexArray.h"
#include "VertexBuffer/VertexBuffer.h"
#include "IndexBuffer/IndexBuffer.h"
#include "Model/StaticMesh.h"
#include "Shapes/MeshBuilder.h"


Ref<StaticMesh> Util::MeshLoader::LoadAsset(const string& path)
{
	Util::MeshLoader loader(path);
	return CreateRef<StaticMesh>(loader);
}

Util::MeshLoader::MeshLoader(const string& path)
{
	LoadMesh(path);
}

void Util::MeshLoader::LoadMesh(const std::string& path)
{
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	// retrieve the directory path of the filepath
	m_Directory = path.substr(0, path.find_last_of('/'));

	// process ASSIMP's root node recursively
	ProcessNode(scene->mRootNode, scene);
}

void Util::MeshLoader::ProcessNode(aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_LoadedMeshes.push_back(ProcessNode(mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Ref<MeshSection> Util::MeshLoader::ProcessNode(aiMesh* mesh, const aiScene* scene)
{
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::map<TextureType, vector<Ref<Texture2D>>> textures;

	// walk through each of the mesh's vertices
	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// positions
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		// normals
		if (mesh->HasNormals())
			vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		// texture coordinates
		if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			// tangent
			vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			// bitangent
			vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}

	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);        
	}

	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
			
	// 1. diffuse maps
	loadMaterialTextures(material, aiTextureType_DIFFUSE, textures[TextureType::DIFFUSE]);
	// 2. specular maps
	loadMaterialTextures(material, aiTextureType_SPECULAR, textures[TextureType::SPECULAR]);
	// 3. normal maps
	loadMaterialTextures(material, aiTextureType_HEIGHT, textures[TextureType::NORMAL]);
	// 4. height maps
	loadMaterialTextures(material, aiTextureType_AMBIENT, textures[TextureType::HEIGHT]);

	BufferLayout layout = {
		{ShaderDataType::Float3, "aPosition"},
		{ShaderDataType::Float3, "aNormal"},
		{ShaderDataType::Float2, "aTexCoords"},
		{ShaderDataType::Float3, "aTangent"},
		{ShaderDataType::Float3, "aBitangent"},
		{ShaderDataType::Int4, "aBonsIDs"},
		{ShaderDataType::Float4, "aWeights"},
	};

	// return a mesh object created from the extracted mesh data
	auto section = MeshBuilder::BuildSection(vertices, indices, layout);
	section->SetTexture(textures);
	return section;
}

void Util::MeshLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, vector<Ref<Texture2D>>& outTextures)
{
	for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		auto cached = m_PathToTexture.find(str.C_Str());
		if (cached == m_PathToTexture.end())
		{
			cout << str.C_Str() << std::endl;
			Ref<Texture2D> tex = Texture2D::Create(m_Directory + '/' + str.C_Str());
			outTextures.push_back(tex);
			m_PathToTexture[str.C_Str()] = tex;
		}
		else
		{
			outTextures.push_back(cached->second);
		}
	}
}
