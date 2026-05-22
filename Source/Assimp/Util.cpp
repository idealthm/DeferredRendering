#include "Util.h"

#include <fstream>
#include <iostream>

#include "Engine.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Common/Utils.h"
#include "Material/Material.h"
#include "Material/MaterialInstance.h"
#include "Model/StaticMesh.h"
#include "Model/Asset.h"
#include "RHI/PixelBufferDescriptor.h"
#include "stb_images/stb_image.h"

namespace Util
{
    Ref<StaticMesh> MeshLoader::LoadAsset(const std::string& path)
    {
        // Re-read config to get model path
        std::ifstream f(path);
        if (!f.is_open()) return nullptr;

        json data;
        try {
            data = json::parse(f);
        } catch (json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            return nullptr;
        }

        std::string directory = path.substr(0, path.find_last_of("/\\"));
        std::string modelFile = data["model_path"];

        Ref<MaterialInstance> mi = CreateRef<MaterialInstance>(gEngine->GetDefaultModelMaterial().get());
        if (data.contains("textures")) {
            for (auto& [typeStr, fileName] : data["textures"].items()) {
                std::string texPath = directory + "/" + std::string(fileName);
                mi->SetParameter(typeStr, LoadTexture(texPath, typeStr._Equal("albedo"), typeStr._Equal("albedo"), true));
            }
        }

        return LoadAssetInternal(directory + "/" + modelFile, mi);
    }

    Ref<StaticMesh> MeshLoader::LoadAssetInternal(const std::string& path, Ref<MaterialInstance> material)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
                                                aiProcess_Triangulate |
                                                aiProcess_FlipUVs |
                                                aiProcess_CalcTangentSpace |
                                                aiProcess_GenSmoothNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return nullptr;
        }

        Asset asset;
        asset.file = path;

        // Collect all meshes from the scene
        std::vector<aiMesh*> aiMeshes;
        std::function<void(aiNode*)> collectMeshes = [&](aiNode* node) {
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
                aiMeshes.push_back(scene->mMeshes[node->mMeshes[i]]);
            for (unsigned int i = 0; i < node->mNumChildren; i++)
                collectMeshes(node->mChildren[i]);
        };
        collectMeshes(scene->mRootNode);

        // Build SOA arrays
        for (aiMesh* aiMesh : aiMeshes)
        {
            size_t baseVertex = asset.positions.size();
            size_t baseIndex = asset.indices.size();

            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
            {
                // Position
                asset.positions.push_back({
                    aiMesh->mVertices[i].x,
                    aiMesh->mVertices[i].y,
                    aiMesh->mVertices[i].z,
                    1.0f
                });

                // Normal
                if (aiMesh->HasNormals()) {
                    asset.normals.push_back({
                        aiMesh->mNormals[i].x,
                        aiMesh->mNormals[i].y,
                        aiMesh->mNormals[i].z,
                        0.0f
                    });
                } else {
                    asset.normals.push_back({0.0f, 0.0f, 0.0f, 0.0f});
                }

                // Tangent + bitangent sign
                if (aiMesh->HasTangentsAndBitangents()) {
                    glm::vec3 t(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z);
                    glm::vec3 n(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);
                    glm::vec3 b(aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z);
                    // Compute handedness: sign of dot(cross(n, t), b)
                    float sign = glm::dot(glm::cross(n, t), b) >= 0.0f ? 1.0f : -1.0f;
                    asset.tangents.push_back({t.x, t.y, t.z, sign});
                } else {
                    asset.tangents.push_back({1.0f, 0.0f, 0.0f, 1.0f});
                }

                // TexCoords0
                if (aiMesh->mTextureCoords[0]) {
                    float u = glm::clamp(aiMesh->mTextureCoords[0][i].x, 0.0f, 1.0f);
                    float v = glm::clamp(aiMesh->mTextureCoords[0][i].y, 0.0f, 1.0f);
                    asset.texCoords0.push_back({
                        static_cast<uint16_t>(u * 65535.0f),
                        static_cast<uint16_t>(v * 65535.0f)
                    });
                } else {
                    asset.texCoords0.push_back({0, 0});
                }

                // TexCoords1
                if (aiMesh->mTextureCoords[1]) {
                    float u = glm::clamp(aiMesh->mTextureCoords[1][i].x, 0.0f, 1.0f);
                    float v = glm::clamp(aiMesh->mTextureCoords[1][i].y, 0.0f, 1.0f);
                    asset.texCoords1.push_back({
                        static_cast<uint16_t>(u * 65535.0f),
                        static_cast<uint16_t>(v * 65535.0f)
                    });
                }
            }

            // Indices (with vertex offset adjustment)
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
            {
                aiFace face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    asset.indices.push_back(baseVertex + face.mIndices[j]);
            }

            // Build Mesh entry
            Mesh meshEntry;
            meshEntry.offset = baseVertex;
            meshEntry.count = aiMesh->mNumVertices;
            Part part;
            part.offset = baseIndex;
            part.count = 0;
            part.mi = material;
            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
                part.count += aiMesh->mFaces[i].mNumIndices;
            meshEntry.parts.push_back(part);
            asset.meshes.push_back(meshEntry);
        }

        asset.snormUV0 = !asset.texCoords0.empty();
        asset.snormUV1 = !asset.texCoords1.empty();

        return StaticMesh::Create(asset);
    }

    Ref<Texture> LoadTexture(const std::string& filepath, bool srgb, bool hasAlpha, bool generateMipmap)
    {
        Ref<Texture> texture;
        if (!filepath.empty())
        {
            Path path(filepath);
            if (path.exists())
            {
                int w, h, n;
                int numChannels = hasAlpha ? 4 : 3;

                RHI::Format internalFormat;
                if (srgb)
                    internalFormat = hasAlpha ? RHI::Format::SRGBA8 : RHI::Format::SRGB8;
                else
                    internalFormat = hasAlpha ? RHI::Format::RGBA8 : RHI::Format::RGB8;

                RHI::PixelDataFormat outputFormat = hasAlpha ? RHI::PixelDataFormat::RGBA : RHI::PixelDataFormat::RGB;

                uint8_t *data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, numChannels);
                if (data != nullptr)
                {
                    RHI::TextureDesc desc;
                    desc.Width = w;
                    desc.Height = h;
                    desc.MipLevels = 0xff;
                    desc.Format = internalFormat;
                    texture = CreateRef<Texture>(desc);

                    PixelBufferDescriptor buffer(data,
                        size_t(w * h * numChannels),
                        outputFormat,
                        RHI::PixelDataType::UBYTE,
                        (PixelBufferDescriptor::Callback) &stbi_image_free);

                    gEngine->GetDriver().update3DImage(texture->GetHandle(), 0, 0, 0, 0, w, h, 1, std::move(buffer));
                    if (generateMipmap)
                        gEngine->GetDriver().generateMipmap(texture->GetHandle());
                }
            }
            else
            {
                std::cout << "Texture not found: " << filepath << '\n';
            }
        }
        else
        {
            std::cout << "Texture not specified" << '\n';
        }
        return texture;
    }
}
