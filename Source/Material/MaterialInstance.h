#pragma once
#include <array>
#include <bitset>
#include <map>
#include <memory>

#include "Common/Handle.h"
#include "Material.h"
#include "UnifromBuffer/UniformBuffer.h"

namespace RHI
{
	struct HwTexture;
	struct HwBufferObject;
}

class TextureSampler;

namespace RHI
{
	class RHIDriver;
}

class Texture;

// ── Per-slot descriptor for binding (Vulkan-style) ─────────────────────────
struct DescriptorDesc
{
	DescriptorDesc() noexcept {}

	union
	{
		struct
		{
			Handle<RHI::HwBufferObject> handle;
			uint32_t         offset;
			uint32_t         size;
		} buffer{};
		struct
		{
			Handle<RHI::HwTexture>   handle;
			RHI::SamplerParams  params;
			uint32_t            padding;
		} texture;
	};
};

// ── Descriptor set instance: stores buffer/texture handles for GPU binding ─
struct DescriptorSetInstance
{
	descriptor_set_t               set = 0;
	std::bitset<16>                activeBindings;
	std::array<DescriptorDesc, 16> descriptors;

	bool HasBinding(descriptor_binding_t binding) const { return binding < 16 && activeBindings.test(binding); }

	// Buffer handle
	void			SetBuffer(descriptor_binding_t binding, BufferObjectHandle h, uint32_t offset = 0, uint32_t size = 0);
	BufferObjectHandle GetBufferHandle(descriptor_binding_t binding) const;

	// Texture handle + sampler params
	void          SetTexture(descriptor_binding_t binding, Handle<RHI::HwTexture> h, const RHI::SamplerParams& params);
	TextureHandle GetTextureHandle(descriptor_binding_t binding) const;
	const RHI::SamplerParams& GetSamplerParams(descriptor_binding_t binding) const;
	void Reset() { activeBindings.reset();}
};

// ── Material instance: owns CPU data (UniformBuffer) + GPU handles (DescriptorSetInstance) ─
class MaterialInstance
{
public:
	MaterialInstance() = default;
	explicit MaterialInstance(Material const* material);

	void BindMaterial(Material const* material);
	const Material* GetMaterial() const { return m_Material; }
	Handle<RHI::HwProgram> GetShader() const;

	void Init();
	bool IsAllocated() const { return m_DescriptorSet.activeBindings.any(); }

	DescriptorSetInstance&       GetDescriptorSet()       { return m_DescriptorSet; }
	const DescriptorSetInstance& GetDescriptorSet() const { return m_DescriptorSet; }

	// Raw buffer data (delegates to UniformBuffer)
	const uint8_t* GetUniformData() const;
	uint32_t       GetUniformDataSize() const;

	// Typed access — delegates to UniformBuffer for field lookup + write
	template<typename T>
	bool SetParameter(const std::string& name, const T& value);

	bool SetParameter(const std::string& name, Handle<RHI::HwTexture> texture, const TextureSampler& sampler);

	template<typename T>
	bool GetParameter(const std::string& name, T& outValue) const;

	bool IsDirty()  const { return m_Dirty; }
	void ClearDirty()     { m_Dirty = false; }

	// Upload CPU data from UniformBuffer to GPU (creates GPU buffer lazily)
	void CommitUniforms(RHI::RHIDriver& driver);

private:
	void SetDefaults();

	Material const*        m_Material = nullptr;
	BufferObjectHandle     m_UniformBufferHandle;
	UniformBuffer          m_UniformBuffer;
	DescriptorSetInstance  m_DescriptorSet;
	bool m_Dirty = false;
};

// ── DescriptorSetInstance inline impls ─────────────────────────────────────

inline void DescriptorSetInstance::SetBuffer(descriptor_binding_t binding, BufferObjectHandle h, uint32_t offset, uint32_t size)
{
	if (binding >= 16) return;
	activeBindings.set(binding);
	descriptors[binding].buffer = {h, offset, size};
}

inline Handle<RHI::HwBufferObject> DescriptorSetInstance::GetBufferHandle(descriptor_binding_t binding) const
{
	if (!HasBinding(binding)) return {};
	return descriptors[binding].buffer.handle;
}

inline void DescriptorSetInstance::SetTexture(descriptor_binding_t binding, Handle<RHI::HwTexture> h, const RHI::SamplerParams& params)
{
	if (binding >= 16) return;
	activeBindings.set(binding);
	descriptors[binding].texture = {h, params, 0};
}

inline Handle<RHI::HwTexture> DescriptorSetInstance::GetTextureHandle(descriptor_binding_t binding) const
{
	if (!HasBinding(binding)) return {};
	return descriptors[binding].texture.handle;
}

inline const RHI::SamplerParams& DescriptorSetInstance::GetSamplerParams(descriptor_binding_t binding) const
{
	static const RHI::SamplerParams kDefault{};
	if (!HasBinding(binding)) return kDefault;
	return descriptors[binding].texture.params;
}

// ── MaterialInstance template impls ────────────────────────────────────────
template<typename T>
bool MaterialInstance::SetParameter(const std::string& name, const T& value)
{
	const FieldInfo* field = m_Material->FindField(name);
	if (!field) return false;
	m_UniformBuffer.SetValue(field->offset, value);
	m_Dirty = true;
	return true;
}

template<typename T>
bool MaterialInstance::GetParameter(const std::string& name, T& outValue) const
{
	if (!m_Material) return false;
	const FieldInfo* field = m_Material->FindField(name);
	if (!field) return false;
	return m_UniformBuffer.GetValue(field->offset, outValue);
}
