#pragma once
#include <string>
#include <unordered_map>

#include "Common/Core.h"

class Material;
class MaterialInstance;

class MaterialLibrary
{
public:
	static MaterialLibrary& Get();

	Ref<Material> GetMaterial(const std::string& name);

private:
	MaterialLibrary() = default;

	static Ref<Material> BuildMaterial(const std::string& name);

	std::unordered_map<std::string, Ref<Material>> m_Materials;
};
