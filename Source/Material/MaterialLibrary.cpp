#include "MaterialLibrary.h"

#include "Material.h"
#include "MaterialParser.h"

MaterialLibrary& MaterialLibrary::Get()
{
	static MaterialLibrary instance;
	return instance;
}

Ref<Material> MaterialLibrary::BuildMaterial(const std::string& name)
{
	std::string matbPath = "D:/Dev/DeferredRendering/CompiledMaterials/" + name + ".matb";

	MaterialParser parser(matbPath);
	return CreateRef<Material>(parser);
}

Ref<Material> MaterialLibrary::GetMaterial(const std::string& name)
{
	auto it = m_Materials.find(name);
	if (it != m_Materials.end())
		return it->second;

	auto mat = BuildMaterial(name);
	if (mat)
		m_Materials.emplace(name, mat);
	return mat;
}
