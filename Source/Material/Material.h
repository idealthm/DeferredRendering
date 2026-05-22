#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Common/Handle.h"
#include "SpirvReflect/ShaderParse.h"

namespace RHI
{
	struct HwProgram;
}

class Program;

class Material
{
public:
	Material() = default;
	Material(const MaterialInfo& info);

	Handle<RHI::HwProgram> GetProgram() const;
	// Build from SPIR-V reflection; only MaterialParams UBO + samplers
	void BuildFromReflection(const MaterialInfo& info);

	const BufferInterfaceBlock& GetUniformBlock() const { return m_UniformBlock; }
	descriptor_binding_t GetUniformBinding() const { return m_UniformBlock.binding; }

	const SamplerInterfaceBlock& GetSamplerBlock() const { return m_SamplerBlock; }
	const SamplerInfo* FindSampler(const std::string& name) const;

	const FieldInfo* FindField(const std::string& name) const;
	const std::vector<FieldInfo>& GetFields() const { return m_UniformBlock.fields; }

	RHI::RasterState GetRasterState() const { return m_RasterState; }
	RHI::StencilState GetStencilState() const { return m_StencilState; }

private:
	RHI::RasterState m_RasterState;
	RHI::StencilState m_StencilState;

	MaterialInfo             m_Info;
	mutable Handle<RHI::HwProgram>   m_CachedProgram;

	BufferInterfaceBlock m_UniformBlock;
	std::unordered_map<std::string, size_t> m_FieldIndex;
	SamplerInterfaceBlock m_SamplerBlock;
	std::unordered_map<std::string, size_t> m_SamplerIndex;
};
