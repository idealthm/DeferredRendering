#include "Util.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Texture.h"
#include "VertexBuffer/VertexBufferLayout.h"
#include "IndexBuffer/IndexBuffer.h"
#include "Material/Material.h"
#include "Model/StaticMesh.h"
#include "Shapes/MeshBuilder.h"

namespace Util
{
	MeshLoader::MeshLoader(const std::string& path)
	{
		LoadFromConfig(path);
	}

	void MeshLoader::LoadFromConfig(const std::string& configPath)
	{
		std::ifstream f(configPath);
		if (!f.is_open()) return;
        
		json data;
		try {
			data = json::parse(f);
		} catch (json::parse_error& e) {
			std::cerr << "JSON Parse Error: " << e.what() << std::endl;
			return;
		}

		std::string directory = configPath.substr(0, configPath.find_last_of("/\\"));
		std::string modelFile = data["model_path"];
        
		// 1. 更加健壮的贴图加载逻辑
		Ref<Material> mat = Material::CreateDefault();
		if (data.contains("textures")) {
			for (auto& [typeStr, fileName] : data["textures"].items()) {
				std::string texPath = directory + "/" + std::string(fileName);
                
				// 提示：sRGB 只有 Albedo/Diffuse 需要开启
				mat->SetTexture2D(typeStr, CreateRef<Texture2D>(texPath, typeStr._Equal("uAlbedo")));
			}
		}

		// 2. 加载几何体
		LoadModelGeometry(directory + "/" + modelFile);

		for (auto& section : LoadedSections)
		{
			section->SetMaterial(mat);
		}
	}

	Ref<StaticMesh> MeshLoader::LoadAsset(const string& path)
	{
		MeshLoader loader(path);
		return CreateRef<StaticMesh>(StaticMeshDesc{path}, loader.LoadedSections);
	}

	void MeshLoader::LoadModelGeometry(const std::string& path)
	{
		Assimp::Importer importer;
		// 关键 Flag: 自动计算切线空间、三角化、翻转 UV
		const aiScene* scene = importer.ReadFile(path, 
												aiProcess_Triangulate | 
												aiProcess_FlipUVs | 
												aiProcess_CalcTangentSpace | 
												aiProcess_GenSmoothNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return;
		}

		ProcessNode(scene->mRootNode, scene);;
	}

	void MeshLoader::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process each mesh located at the current node
		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			LoadedSections.push_back(ProcessMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Ref<MeshSection> MeshLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		BufferLayout layout = {
			{ ShaderDataType::Float3, "aPosition" },
			{ ShaderDataType::Float3, "aNormal"   },
			{ ShaderDataType::Float2, "aTexCoord" },
			{ ShaderDataType::Float3, "aTangent"  },
			{ ShaderDataType::Float3, "aBitangent"},
			{ ShaderDataType::Int4  , "aBonsIDs"  },
			{ ShaderDataType::Float4, "aWeights"  },
		};

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			// Position
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                
			// Normal
			if (mesh->HasNormals()) {
				vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			}

			// TexCoords
			if (mesh->mTextureCoords[0]) {
				vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			} else {
				vertex.TexCoords = { 0.0f, 0.0f };
			}

			// Tangent & Bitangent (PBR 必需)
			if (mesh->HasTangentsAndBitangents()) {
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}

			vertices.push_back(vertex);
		}

		// Indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		return MeshBuilder::BuildSection(vertices, indices, layout);
	}
}