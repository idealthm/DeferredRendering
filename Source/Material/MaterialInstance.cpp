#include "MaterialInstance.h"
#include "Engine.h"
#include "EngineEnum.h"
#include "Model/Texture.h"
#include "RHI/RHIDriver.h"
#include "RHI/TextureSampler.h"

MaterialInstance::MaterialInstance(const Ref<Material>& material)
	: m_Material(material), m_DescriptorSet(material->GetDescriptorSetLayout())
{
	RHI::RHIDriver& driver = gEngine->GetDriver();

	if (material->GetUniformBlock().getFieldInfoList().size() > 0) {
		m_UniformBuffer = UniformBuffer(material->GetUniformBlock().getSize());
		m_UniformBufferHandle = driver.CreateBufferObject(m_UniformBuffer.GetSize(),
				RHI::BufferObjectBinding::UNIFORM, RHI::BufferUsage::STATIC);
	}

	// set the UBO, always descriptor 0
	m_DescriptorSet.SetBuffer(0, m_UniformBufferHandle, 0, m_UniformBuffer.GetSize());
}

Handle<RHI::HwProgram> MaterialInstance::GetShader() const
{
	return m_Material->GetProgram();
}

bool MaterialInstance::SetParameter(const std::string& name, const Ref<Texture>& texture,
	const TextureSampler& sampler)
{
	const Material::SamplerInfo* samplerInfo = m_Material->FindSampler(name);

	if (texture && texture->TextureHandleCanMutate())
	{
		m_TextureParam.emplace(samplerInfo->binding, TextureParameter{texture, sampler.GetParams()});
	}
	else
	{
		Handle<RHI::HwTexture> handle{};
		if (texture) {
			handle = texture->GetHandleForSampling();
			ASSERT(handle == texture->GetHandle());
		} else {
			m_TextureParam.erase(samplerInfo->binding);
		}
		m_DescriptorSet.SetTexture(samplerInfo->binding, handle, sampler.GetParams());
	}
	return true;
}

void MaterialInstance::Commit(RHI::RHIDriver& driver)
{
	if (m_UniformBuffer.IsDirty()) {
		driver.updateBufferObject(m_UniformBufferHandle, m_UniformBuffer.toBufferDescriptor(driver), 0);
	}

	if (!m_TextureParam.empty()) {
		for (auto const& [binding, p]: m_TextureParam) {
			ASSERT(p.texture);
			// TODO: Create TextureView
			Handle<RHI::HwTexture> handle = p.texture->GetHandleForSampling();
			ASSERT(handle);
			m_DescriptorSet.SetTexture(binding, handle, p.params);
		}
	}

	// Commit descriptors if needed (e.g. when textures are updated,or the first time)
	m_DescriptorSet.commit(driver, m_Material->GetDescriptorSetLayout());
}

void MaterialInstance::Use(RHI::RHIDriver& driver)
{
	m_DescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_MATERIAL);
}
