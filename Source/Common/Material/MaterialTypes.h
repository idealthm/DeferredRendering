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

struct FieldInfo
{
	std::string name;
	uint16_t    offset = 0;
	uint8_t     stride = 0;
	FieldType   type = FieldType::FLOAT;
	bool        isArray = false;
	uint32_t    size = 0;
	std::string structName;
	std::string sizeName;
};

struct BufferInterfaceBlock
{
	std::string            instanceName;
	std::string            structName;
	size_t                 size = 0;
	MemoryLayout           layout = MemoryLayout::std_140;
	descriptor_binding_t   binding = 0;
	std::vector<FieldInfo> fields;
};

struct SamplerInfo
{
	std::string           name;
	RHI::SamplerType      sampler = RHI::SamplerType::SAMPLER_2D;
	RHI::Format           format  = RHI::Format::Unknown;
	descriptor_binding_t  binding = 0;
};

struct SamplerInterfaceBlock
{
	std::string                 mName;
	RHI::ShaderStageFlags       mStageFlags{};
	std::array<SamplerInfo, 16> mSamplersInfoList;
};

struct VariableParam
{
	std::string name;
	FieldType   type = FieldType::FLOAT;
	uint8_t     location = 0;
};
