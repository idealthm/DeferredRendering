#include "Material.h"

#include "Engine.h"
#include "EngineEnum.h"
#include "LightingShaderLibrary.h"
#include "MaterialParser.h"
#include "Shader/Program.h"

Material::Material(MaterialParser& parser)
{
	parser.Get<ChunkUib>(m_UniformBlock);
	parser.Get<ChunkSib>(m_SamplerBlock);
	parser.Get<ChunkSpirv>(m_SpirvData);
	parser.Get<ChunkGlsl>(m_GlslData);
	parser.Get<ChunkDescriptorSetBindings>(m_DescriptorSetLayouts);
	parser.Get<ChunkRequiredAttrs>(m_RequiredAttributes);

	std::string shadingStr;
	if (parser.Get<ChunkShading>(shadingStr))
		m_Shading = ShadingFromString(shadingStr);

	std::array<RHI::DescriptorSetLayout, MAX_DESCRIPTOR_SET_COUNT> descLayouts;
	if (parser.Get<ChunkDescriptorSetLayout>(descLayouts))
	{
		auto& driver = gEngine->GetDriver();
		m_DescriptorSetLayout = { driver, std::move(descLayouts[+DescriptorSetBindingPoints::PER_MATERIAL]) };
	}

	for (size_t i = 0; i < m_UniformBlock.getFieldInfoList().size(); i++)
		m_FieldIndex[m_UniformBlock.getFieldInfoList()[i].name] = i;
	for (size_t i = 0; i < m_SamplerBlock.getSamplerInfoList().size(); i++)
		m_SamplerIndex[m_SamplerBlock.getSamplerInfoList()[i].name] = i;
}

const SpirvEntry* Material::FindSpirvPass(MaterialPass pass) const
{
	for (auto& entry : m_SpirvData)
	{
		if (entry.pass == pass)
			return &entry;
	}
	return m_SpirvData.empty() ? nullptr : &m_SpirvData[0];
}

const GlslEntry* Material::FindGlslPass(MaterialPass pass) const
{
	for (auto& entry : m_GlslData)
	{
		if (entry.pass == pass)
			return &entry;
	}
	return m_GlslData.empty() ? nullptr : &m_GlslData[0];
}

Handle<RHI::HwProgram> Material::GetProgram(MaterialPass pass) const
{
	if (pass == MaterialPass::Lighting)
		return gEngine->GetLightingShaderLibrary().GetProgram();

	auto it = m_CachedProgram.find(pass);
	if (it != m_CachedProgram.end())
		return it->second;

	const SpirvEntry* spirvEntry = FindSpirvPass(pass);
	if (spirvEntry && !spirvEntry->vertexSpirv.empty())
	{
		Program::ShaderSource source = { spirvEntry->vertexSpirv, spirvEntry->fragmentSpirv };
		Handle<RHI::HwProgram> program = gEngine->GetDriver().CreateProgram(
			Program{ source, m_DescriptorSetLayouts });
		m_CachedProgram[pass] = program;
		return program;
	}

	// Fallback: compile from GLSL source
	const GlslEntry* glslEntry = FindGlslPass(pass);
	if (glslEntry && !glslEntry->vertexGlsl.empty())
	{
		Program::ShaderSource source;
		auto toBlob = [](const std::string& s) {
			return std::vector<uint8_t>(s.begin(), s.end());
		};
		source[0] = toBlob(glslEntry->vertexGlsl);
		source[1] = toBlob(glslEntry->fragmentGlsl);

		Handle<RHI::HwProgram> program = gEngine->GetDriver().CreateProgram(
			Program{ source, m_DescriptorSetLayouts });
		m_CachedProgram[pass] = program;
		return program;
	}

	return {};
}


const Material::SamplerInfo* Material::FindSampler(const std::string& name) const
{
	const auto it = m_SamplerIndex.find(name);
	if (it == m_SamplerIndex.end())
		return nullptr;
	return &m_SamplerBlock.getSamplerInfoList()[it->second];
}

const Material::FieldInfo* Material::FindField(const std::string& name) const
{
	const auto it = m_FieldIndex.find(name);
	if (it == m_FieldIndex.end())
		return nullptr;
	return &m_UniformBlock.getFieldInfoList()[it->second];
}
