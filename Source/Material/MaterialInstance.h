#pragma once
#include <array>
#include <bitset>
#include <map>
#include <memory>

#include "Common/Handle.h"
#include "Material.h"
#include "RHI/DescriptorSet.h"
#include "UnifromBuffer/UniformBuffer.h"

class TextureSampler;

namespace RHI
{
	class RHIDriver;
}

class Texture;

// ── Material instance: owns CPU data (UniformBuffer) + GPU handles (DescriptorSetInstance) ─
class MaterialInstance
{
public:
	MaterialInstance() = default;
	explicit MaterialInstance(const Ref<Material>& material);

	Ref<Material> GetMaterial() const { return m_Material; }
	Handle<RHI::HwProgram> GetShader(MaterialPass pass) const;

	DescriptorSet&       GetDescriptorSet()       { return m_DescriptorSet; }
	const DescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; }

	struct TextureParameter {
		Ref<const Texture> texture;
		RHI::SamplerParams params;
	};

	// Typed access — delegates to UniformBuffer for field lookup + write
	template<typename T, typename = std::enable_if_t<!std::is_same_v<Ref<Texture>, T>, T>>
	bool SetParameter(const std::string& name, const T& value);

	bool SetParameter(const std::string& name, const Ref<Texture>& texture, const TextureSampler& sampler);

	template<typename T>
	T GetParameter(const std::string& name) const;

	bool IsDirty()  const { return m_Dirty; }
	void ClearDirty()     { m_Dirty = false; }

	// Upload CPU data from UniformBuffer to GPU (creates GPU buffer lazily)
	void Commit(RHI::RHIDriver& driver);

	void Use(RHI::RHIDriver& driver);

private:
	// RHI::CullingMode mCulling : 2;
	// RHI::SamplerCompareFunc mDepthFunc : 3;
	// bool mColorWrite : 1;
	// bool mDepthWrite : 1;
	// bool mHasScissor : 1;
	// bool mIsDoubleSided : 1;
	// bool mIsDefaultInstance : 1;
	// uint8_t padding : 3;

	
	std::unordered_map<descriptor_binding_t, TextureParameter> m_TextureParam;
	Ref<Material>		   m_Material = nullptr;
	BufferObjectHandle     m_UniformBufferHandle;
	UniformBuffer          m_UniformBuffer;
	DescriptorSet		   m_DescriptorSet;
	bool m_Dirty = false;
};

// ── MaterialInstance template impls ────────────────────────────────────────
template<typename T, typename = std::enable_if_t<!std::is_same_v<Ref<Texture>, T>, T>>
bool MaterialInstance::SetParameter(const std::string& name, const T& value)
{
	const Material::FieldInfo* field = m_Material->FindField(name);
	m_UniformBuffer.SetValue(field->getBufferOffset(), value);
	m_Dirty = true;
	return true;
}

template<typename T>
T MaterialInstance::GetParameter(const std::string& name) const
{
	const Material::FieldInfo* field = m_Material->FindField(name);
	ASSERT(field);
	return m_UniformBuffer.GetValue<T>(field->getBufferOffset());
}
