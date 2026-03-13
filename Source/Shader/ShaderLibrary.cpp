#include "ShaderLibrary.h"

#include "Shader.h"
#include "Material/Material.h"
#include "ShaderPreprocessor/ShaderLoader.h"

ShaderLibrary& ShaderLibrary::Get()
{
	static ShaderLibrary instance;
	return instance;
}

Ref<Shader> ShaderLibrary::GetShader(EMaterialDomain type)
{
	switch (type)
	{
	case EMaterialDomain::Surface: return GetShader("Shaders/Basic", nullptr);
	case EMaterialDomain::PostProcess: return nullptr;
	case EMaterialDomain::Compute: return nullptr;
	case EMaterialDomain::UI: return nullptr;
		default: return nullptr;
	}
	return {};
}

/*Ref<Shader> ShaderLibrary::GetShader(Ref<Material> mat)
{
	return GetShader(mat->GetShaderType());
}*/

Ref<Shader> ShaderLibrary::GetShader(const std::string& path, DUI* dui)
{
	// compile path & dui to hash.
	if (!path.empty())
	{
		if (auto it = m_PathToShader.find(path); it != m_PathToShader.end())
		{
			return it->second;
		}
		else
		{
			return m_PathToShader[path] = CreateRef<Shader>(path, 10, dui);
		}
	}

	ASSERT(false);
	return nullptr;
}
