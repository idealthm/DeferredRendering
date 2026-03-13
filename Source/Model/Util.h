#pragma once
#include <iostream>

#include "MeshSection.h"

#include <vector>
#include <string>
#include <fstream>
#include <map>

// Assimp includes
#include "assimp/material.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <nlohmann/json.hpp>

#include "VertexBuffer/VertexBufferLayout.h"


struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;
class StaticMesh;
class MeshSection;

namespace Util {

    using json = nlohmann::json;

    class MeshLoader {
        friend class StaticMesh;
    public:
        MeshLoader(const std::string& path);
        // 最终加载出来的结果存放处
        std::vector<Ref<MeshSection>> LoadedSections;

        void LoadFromConfig(const std::string& configPath);

        static Ref<StaticMesh> LoadAsset(const string& path);

    private:
        void LoadModelGeometry(const std::string& path);

        Ref<MeshSection> ProcessMesh(aiMesh* mesh, const aiScene* scene);

        void ProcessNode(aiNode* node, const aiScene* scene);
    };

}
