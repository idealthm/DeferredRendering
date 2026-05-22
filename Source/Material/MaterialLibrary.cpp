#include "MaterialLibrary.h"

#include "Material.h"
#include "SpirvReflect/ShaderParse.h"

MaterialLibrary& MaterialLibrary::Get()
{
	static MaterialLibrary instance;
	return instance;
}

Ref<Material> MaterialLibrary::BuildMaterial(const std::string& name)
{
	std::string vertSpvPath = "Material/CompiledMaterials/" + name + "/" + name + ".vert.spv";
	std::string fragSpvPath = "Material/CompiledMaterials/" + name + "/" + name + ".frag.spv";

	ShaderParser parser(vertSpvPath, fragSpvPath);
	if (!parser.IsValid()) return nullptr;

	return CreateRef<Material>(parser.GetMaterialInfo());
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
