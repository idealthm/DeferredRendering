#include "Material.h"

#include "Renderer.h"
#include "Model/Texture.h"
#include "Shader/ShaderLibrary.h"


void Material::ApplyMaterial(Ref<Shader> shader)
{
	shader->Bind();

	for (auto& [name, prop] : m_FloatProperty)
	{
		switch (prop.size)
		{
		case 1:shader->SetUniform1f(name, GetFloat<float>(prop.offset));break;
		case 2:shader->SetUniform2f(name, GetFloat<glm::vec2>(prop.offset));break;
		case 3:shader->SetUniform3f(name, GetFloat<glm::vec3>(prop.offset));break;
		case 4:shader->SetUniform4f(name, GetFloat<glm::vec4>(prop.offset));break;
			default: ASSERT(false); break;
		}
	}

	uint32_t startIndex = shader->GetFreeSlotIndex();
	for (auto& [name, tex] : m_TextureProperty)
	{
		tex->Bind(startIndex);
		shader->SetUniform1i(name, startIndex++);
	}
}

Ref<Texture2D> Material::GetTexture2D(const std::string& name) const
{
	auto pos = m_TextureProperty.find(name);
	return pos == m_TextureProperty.end() ? nullptr : pos->second;
}

void Material::SetTexture2D(const std::string& name, const Ref<Texture2D>& texture)
{
	m_TextureProperty[name] = texture;
}

Ref<Shader> Material::GetShader(RenderPassType PassType) const
{
	return ShaderLibrary::Get().GetShader(m_Domain);
}

Ref<Material> Material::CreateDefault()
{
	auto mat = CreateRef<Material>(EMaterialDomain::Surface, EBlendMode::Opaque);
	mat->SetTexture2D("uAlbedo", GDefaultTextures.White);
	mat->SetTexture2D("uNormal", GDefaultTextures.Normal);
	mat->SetTexture2D("uRoughness", GDefaultTextures.Black);
	mat->SetTexture2D("uMetallic", GDefaultTextures.Gray);
	mat->SetTexture2D("uAO", GDefaultTextures.White);
	return mat;
}
