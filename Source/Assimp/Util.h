#pragma once

#include <string>


#include <nlohmann/json.hpp>

#include "Common/Core.h"

class Texture;
class MaterialInstance;
class StaticMesh;

namespace Util {

    using json = nlohmann::json;

    class MeshLoader {
        friend class StaticMesh;
    public:
        static Ref<StaticMesh> LoadAsset(const std::string& path);

    private:
        static Ref<StaticMesh> LoadAssetInternal(const std::string& path, Ref<MaterialInstance> material);
    };

    Ref<Texture> LoadTexture(const std::string& path, bool srgb, bool generateMipmap); 
}
