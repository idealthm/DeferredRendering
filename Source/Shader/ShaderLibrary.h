#pragma once
#include <map>
#include <string>

#include "Common/Core.h"

enum class EMaterialDomain;
class Material;
struct DUI;
enum class EShaderType;
class Shader;

class ShaderLibrary
{
public:
	static ShaderLibrary& Get();

	Ref<Shader> GetShader(EMaterialDomain type);
	// Ref<Shader> GetShader(Ref<Material> mat);
	Ref<Shader> GetShader(const std::string& path, DUI* dui);
private:

	Ref<Shader> m_PBR;
	Ref<Shader> m_ShadowMap;
	Ref<Shader> m_SkyBox;
	Ref<Shader> m_Cubemap;

	std::map<std::string, Ref<Shader>> m_PathToShader;
};
