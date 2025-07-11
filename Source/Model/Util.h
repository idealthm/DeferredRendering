#pragma once
#include <iostream>

#include "MeshSection.h"
#include "assimp/material.h"


struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;
class StaticMesh;
class MeshSection;

namespace Util
{
	class MeshLoader
	{
		friend class StaticMesh;
	public:
		static Ref<StaticMesh> LoadAsset(const string& path);
	private:
		MeshLoader(const string& path);

		void LoadMesh(const std::string& path);

		void ProcessNode(aiNode* node, const aiScene* scene);

		Ref<MeshSection> ProcessNode(aiMesh* mesh, const aiScene* scene);

		void loadMaterialTextures(aiMaterial* mat, aiTextureType type, vector<Ref<Texture2D>>& outTextures);
	private:
		std::string m_Directory;
		std::string m_Path;

		std::vector<Ref<MeshSection>>			m_LoadedMeshes;
		std::map<std::string, Ref<Texture2D>>	m_PathToTexture;
	};
}
