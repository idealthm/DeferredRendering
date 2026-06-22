#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Common/Handle.h"
#include "Common/Serialization/MaterialChunks.h"
#include "DescriptorSetLayout.h"
#include "MaterialParser.h"
#include "../../Include/BufferInterfaceBlock.h"

class BufferInterfaceBlock;
class DescriptorSetLayout;
class Program;

namespace RHI { struct HwProgram; }

class MaterialParser;

class Material
{
public:
	Material() = default;

	using FieldInfo   = BufferInterfaceBlock::FieldInfo;
	using SamplerInfo = SamplerInterfaceBlock::SamplerInfo;

	explicit Material(MaterialParser& parser);

	Handle<RHI::HwProgram> GetProgram(MaterialPass pass = MaterialPass::Surface) const;

	const BufferInterfaceBlock& GetUniformBlock() const { return m_UniformBlock; }
	const SamplerInterfaceBlock& GetSamplerBlock() const { return m_SamplerBlock; }
	const SamplerInfo* FindSampler(const std::string& name) const;
	const FieldInfo*  FindField(const std::string& name) const;
	const std::vector<BufferInterfaceBlock::FieldInfo>& GetFields() const;

	RHI::RasterState  GetRasterState()       const { return m_RasterState; }
	RHI::StencilState GetStencilState()      const { return m_StencilState; }
	uint32_t          GetRequiredAttributes() const { return m_RequiredAttributes; }

	const DescriptorSetLayout& GetDescriptorSetLayout()          const { return m_DescriptorSetLayout; }
	const DescriptorSetLayout& GetPerViewDescriptorSetLayout()   const { return m_PerViewDescriptorSetLayout; }

private:
	const SpirvEntry* FindPass(MaterialPass pass) const;

	RHI::RasterState  m_RasterState;
	RHI::StencilState m_StencilState;
	uint32_t          m_RequiredAttributes = 0;

	ChunkSpirv::Container m_ShaderData;
	struct PassHash { size_t operator()(MaterialPass p) const noexcept { return static_cast<size_t>(p); } };
	mutable std::unordered_map<MaterialPass, Handle<RHI::HwProgram>, PassHash> m_CachedProgram;

	DescriptorSetLayout m_PerViewDescriptorSetLayout;
	DescriptorSetLayout m_DescriptorSetLayout;

	DescriptorSetInfo m_DescriptorSetLayouts;

	BufferInterfaceBlock m_UniformBlock;
	std::unordered_map<std::string, size_t> m_FieldIndex;
	SamplerInterfaceBlock m_SamplerBlock;
	std::unordered_map<std::string, size_t> m_SamplerIndex;
};
