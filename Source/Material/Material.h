#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "DescriptorSetLayout.h"
#include "Common/Handle.h"
#include "MaterialParser.h"

class DescriptorSetLayout;

namespace RHI
{
	struct HwProgram;
}

struct MaterialInfo;
class Program;
class MaterialParser;

class Material
{
public:
	Material() = default;

	// New — from .matb binary via MaterialParser
	explicit Material(MaterialParser& parser);

	Handle<RHI::HwProgram> GetProgram() const;

	const BufferInterfaceBlock& GetUniformBlock() const { return m_UniformBlock; }

	const SamplerInterfaceBlock& GetSamplerBlock() const { return m_SamplerBlock; }
	const SamplerInfo* FindSampler(const std::string& name) const;

	const FieldInfo* FindField(const std::string& name) const;
	const std::vector<FieldInfo>& GetFields() const { return m_UniformBlock.fields; }

	RHI::RasterState GetRasterState() const { return m_RasterState; }
	RHI::StencilState GetStencilState() const { return m_StencilState; }

	const DescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	const DescriptorSetLayout& GetPerViewDescriptorSetLayout() const { return m_PerViewDescriptorSetLayout; }

private:
	RHI::RasterState m_RasterState;
	RHI::StencilState m_StencilState;

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
