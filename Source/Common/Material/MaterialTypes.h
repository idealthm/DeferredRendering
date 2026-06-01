#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "RHI/DriverEnums.h"

// =============================================================================
// Shared data types — used by both ShaderCompiler (matc) and DeferredRendering.
// No glm / spirv-reflect / glad dependency.
// =============================================================================

enum class FieldType : uint8_t
{
	BOOL, BOOL2, BOOL3, BOOL4,
	FLOAT, FLOAT2, FLOAT3, FLOAT4,
	INT, INT2, INT3, INT4,
	UINT, UINT2, UINT3, UINT4,
	MAT3, MAT4, STRUCT
};

enum class MemoryLayout : uint8_t
{
	std_140,
	std_430
};

struct SamplerInfo
{
	std::string           name;
	RHI::SamplerType      sampler = RHI::SamplerType::SAMPLER_2D;
	RHI::Format           format  = RHI::Format::Unknown;
	descriptor_binding_t  binding = 0;
};

struct VariableParam
{
	std::string name;
	FieldType   type = FieldType::FLOAT;
	uint8_t     location = 0;
};

struct Descriptor
{
	std::string           name;
	RHI::DescriptorType   type;
	descriptor_binding_t  binding;
};

using DescriptorSetInfo = std::array<std::vector<Descriptor>, MAX_DESCRIPTOR_SET_COUNT>;