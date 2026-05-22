#include "MaterialInstance.h"
#include "Engine.h"
#include "RHI/RHIDriver.h"
#include "RHI/TextureSampler.h"

MaterialInstance::MaterialInstance(Material const* material)
	: m_Material(material)
{
}

void MaterialInstance::BindMaterial(Material const* material)
{
	m_Material = material;
	m_UniformBuffer = {};
	m_DescriptorSet.Reset();
	m_Dirty = false;

	// Uniform Buffer clean
}

void MaterialInstance::SetDefaults()
{
	m_Dirty = true;
}

Handle<RHI::HwProgram> MaterialInstance::GetShader() const
{
	return m_Material->GetProgram();
}

void MaterialInstance::Init()
{
	if (!m_Material) return;

	m_DescriptorSet.Reset();
	m_DescriptorSet.set = 0;

	const descriptor_binding_t ubBinding = m_Material->GetUniformBinding();
	const auto& block = m_Material->GetUniformBlock();

	// MaterialParams UBO — allocate CPU data; GPU handle set by CommitUniforms
	if (block.size > 0)
	{
		m_UniformBuffer.Clear(block.size);
		m_DescriptorSet.activeBindings.set(ubBinding);
	}

	// Samplers — engine sets texture handle + params via SetTexture()
	for (const auto& sampler : m_Material->GetSamplerBlock().mSamplersInfoList)
		m_DescriptorSet.activeBindings.set(sampler.binding);

	SetDefaults();
}

const uint8_t* MaterialInstance::GetUniformData() const
{
	return m_UniformBuffer.GetData();
}

uint32_t MaterialInstance::GetUniformDataSize() const
{
	return m_UniformBuffer.GetSize();
}

bool MaterialInstance::SetParameter(const std::string& name, Handle<RHI::HwTexture> texture,
	const TextureSampler& sampler)
{
	const SamplerInfo* samplerInfo = m_Material->FindSampler(name);
	if (!samplerInfo) return false;
	m_DescriptorSet.SetTexture(samplerInfo->binding, texture, sampler.GetParams());
	return m_Dirty = true;
}

void MaterialInstance::CommitUniforms(RHI::RHIDriver& driver)
{
	if (!m_UniformBuffer.IsDirty()) return;

	// Lazy GPU buffer creation
	if (!m_UniformBufferHandle)
	{
		m_UniformBufferHandle = driver.CreateBufferObject(m_UniformBuffer.GetSize(), RHI::BufferObjectBinding::UNIFORM, RHI::BufferUsage::DYNAMIC);

		// Register buffer handle in descriptor set
		descriptor_binding_t ubBinding = m_Material->GetUniformBinding();
		m_DescriptorSet.SetBuffer(ubBinding, m_UniformBufferHandle, 0, m_UniformBuffer.GetSize());
	}

	// driver.SetBufferData(m_UniformBufferHandle, m_UniformBuffer.GetData(), m_UniformBuffer.GetSize());
	m_UniformBuffer.ClearDirty();
	m_Dirty = false;
}
