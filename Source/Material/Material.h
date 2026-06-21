#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "DescriptorSetLayout.h"
#include "Common/Handle.h"
#include "MaterialParser.h"
#include "../../Include/BufferInterfaceBlock.h"

class BufferInterfaceBlock;
class DescriptorSetLayout;

namespace RHI
{
	struct HwProgram;
}

class Program;
class MaterialParser;

class Material
{
public:
	Material() = default;

	using FieldInfo = BufferInterfaceBlock::FieldInfo;
	using SamplerInfo = SamplerInterfaceBlock::SamplerInfo;

	// New — from .matb binary via MaterialParser
	explicit Material(MaterialParser& parser);

	Handle<RHI::HwProgram> GetProgram() const;

	const BufferInterfaceBlock& GetUniformBlock() const { return m_UniformBlock; }

	const SamplerInterfaceBlock& GetSamplerBlock() const { return m_SamplerBlock; }
	const SamplerInfo* FindSampler(const std::string& name) const;

	const FieldInfo* FindField(const std::string& name) const;
	const std::vector<BufferInterfaceBlock::FieldInfo>& GetFields() const;

	RHI::RasterState GetRasterState() const { return m_RasterState; }
	RHI::StencilState GetStencilState() const { return m_StencilState; }
	uint32_t GetRequiredAttributes() const { return m_RequiredAttributes; }

	const DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	const DescriptorSetLayout& GetPerViewDescriptorSetLayout() const { return m_PerViewDescriptorSetLayout; }

private:
	RHI::RasterState m_RasterState;
	RHI::StencilState m_StencilState;
	uint32_t m_RequiredAttributes = 0;

	Program::ShaderSource      m_ShaderData;
	DescriptorSetLayout		   m_PerViewDescriptorSetLayout;
	DescriptorSetLayout		   m_DescriptorSetLayout;

	DescriptorSetInfo		   m_DescriptorSetLayouts;
	mutable Handle<RHI::HwProgram> m_CachedProgram;

	BufferInterfaceBlock m_UniformBlock;
	std::unordered_map<std::string, size_t> m_FieldIndex;
	SamplerInterfaceBlock m_SamplerBlock;
	std::unordered_map<std::string, size_t> m_SamplerIndex;
};
