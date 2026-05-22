#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#ifndef SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#endif
#include <array>

#include "spirv_reflect.h"
#include "RHI/DriverEnums.h"
#include "Shader/Program.h"

class FArchive;

struct FlatFieldInfo;
struct FlatSamplerInfo;
struct FlatVariableParam;
struct FlatDescriptorBinding;

enum class FieldType : uint8_t
{
	BOOL,
	BOOL2,
	BOOL3,
	BOOL4,
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	INT,
	INT2,
	INT3,
	INT4,
	UINT,
	UINT2,
	UINT3,
	UINT4,
	MAT3,
	MAT4,
	STRUCT
};

template<FieldType T>
struct FieldTypeTraits;

#define GLM_FIELD_TYPE_TRAITS(ENUM, GLM_TYPE) \
template<> struct FieldTypeTraits<FieldType::ENUM> { using type = GLM_TYPE; }

GLM_FIELD_TYPE_TRAITS(BOOL,   bool);
GLM_FIELD_TYPE_TRAITS(BOOL2,  glm::bvec2);
GLM_FIELD_TYPE_TRAITS(BOOL3,  glm::bvec3);
GLM_FIELD_TYPE_TRAITS(BOOL4,  glm::bvec4);
GLM_FIELD_TYPE_TRAITS(FLOAT,  float);
GLM_FIELD_TYPE_TRAITS(FLOAT2, glm::vec2);
GLM_FIELD_TYPE_TRAITS(FLOAT3, glm::vec3);
GLM_FIELD_TYPE_TRAITS(FLOAT4, glm::vec4);
GLM_FIELD_TYPE_TRAITS(INT,    int32_t);
GLM_FIELD_TYPE_TRAITS(INT2,   glm::ivec2);
GLM_FIELD_TYPE_TRAITS(INT3,   glm::ivec3);
GLM_FIELD_TYPE_TRAITS(INT4,   glm::ivec4);
GLM_FIELD_TYPE_TRAITS(UINT,   uint32_t);
GLM_FIELD_TYPE_TRAITS(UINT2,  glm::uvec2);
GLM_FIELD_TYPE_TRAITS(UINT3,  glm::uvec3);
GLM_FIELD_TYPE_TRAITS(UINT4,  glm::uvec4);
GLM_FIELD_TYPE_TRAITS(MAT3,   glm::mat3);
GLM_FIELD_TYPE_TRAITS(MAT4,   glm::mat4);

#undef GLM_FIELD_TYPE_TRAITS

template<FieldType T>
constexpr size_t GetFieldTypeSize()
{
	return sizeof(typename FieldTypeTraits<T>::type);
}

template<typename T>
struct FieldTypeOf { static constexpr FieldType value = static_cast<FieldType>(0xFF); };

#define FIELD_TYPE_OF(CPP_TYPE, ENUM) \
	template<> struct FieldTypeOf<CPP_TYPE> { static constexpr FieldType value = FieldType::ENUM; }

FIELD_TYPE_OF(bool,        BOOL);
FIELD_TYPE_OF(glm::bvec2,  BOOL2);
FIELD_TYPE_OF(glm::bvec3,  BOOL3);
FIELD_TYPE_OF(glm::bvec4,  BOOL4);
FIELD_TYPE_OF(float,       FLOAT);
FIELD_TYPE_OF(glm::vec2,   FLOAT2);
FIELD_TYPE_OF(glm::vec3,   FLOAT3);
FIELD_TYPE_OF(glm::vec4,   FLOAT4);
FIELD_TYPE_OF(int32_t,     INT);
FIELD_TYPE_OF(glm::ivec2,  INT2);
FIELD_TYPE_OF(glm::ivec3,  INT3);
FIELD_TYPE_OF(glm::ivec4,  INT4);
FIELD_TYPE_OF(uint32_t,    UINT);
FIELD_TYPE_OF(glm::uvec2,  UINT2);
FIELD_TYPE_OF(glm::uvec3,  UINT3);
FIELD_TYPE_OF(glm::uvec4,  UINT4);
FIELD_TYPE_OF(glm::mat3,   MAT3);
FIELD_TYPE_OF(glm::mat4,   MAT4);

#undef FIELD_TYPE_OF

enum class MemoryLayout : uint8_t
{
	std_140,
	std_430
};


struct FieldInfo {
	std::string name;
	uint16_t offset;            // byte offset of this field in the buffer
	uint8_t stride;             // byte stride to the next array element
	FieldType type;
	bool isArray = false;
	uint32_t size = 0;          // array element count, 0 if not an array
	std::string structName;
	std::string sizeName;

	inline size_t getBufferOffset(size_t index = 0) const {
		ASSERT(index < std::max(1u, size));
		return offset + stride * index;
	}
};

struct BufferInterfaceBlock
{
	std::string instanceName;
	std::string structName;
	size_t      size = 0;
	MemoryLayout layout = MemoryLayout::std_140;
	descriptor_binding_t binding = 0;
	std::vector<FieldInfo> fields;
};

struct SamplerInfo
{
	std::string name;
	RHI::SamplerType sampler = RHI::SamplerType::SAMPLER_2D;
	RHI::Format format = RHI::Format::Unknown;
	descriptor_binding_t binding = 0;
};

struct SamplerInterfaceBlock
{
	std::string mName;
	RHI::ShaderStageFlags mStageFlags{};
	std::array<SamplerInfo, 16> mSamplersInfoList;
};


struct VariableParam
{
	std::string name;
	FieldType type = FieldType::FLOAT;
	uint8_t location = 0;
};

struct MaterialInfo
{
	std::array<VariableParam, 16> inputVariables;
	std::array<VariableParam, 16> outputVariables;
	Program::ShaderSource shaderData;
	Program::DescriptorSetInfo descriptorSets;
	BufferInterfaceBlock uib;
	SamplerInterfaceBlock sib;

	// Populate this MaterialInfo from a .matb binary archive
	bool LoadFromBinary(FArchive& ar);

	// Conversion helpers from flat serialized types
	static FieldInfo       FromFlat(const FlatFieldInfo& flat);
	static SamplerInfo     FromFlat(const FlatSamplerInfo& flat);
	static VariableParam   FromFlat(const FlatVariableParam& flat);
	static Program::Descriptor FromFlat(const FlatDescriptorBinding& flat);
};

class ShaderParser
{
public:
	ShaderParser() = default;

	ShaderParser(const std::string& vertPath, const std::string& fragPath);

	bool IsValid() const { return m_Valid; }
	const MaterialInfo& GetMaterialInfo() const { return m_Info; }

	// Load from a .matb binary file (replaces runtime spirv_reflect)
	static bool LoadFromBinary(const std::string& path, MaterialInfo& outInfo);

private:
	bool ParseStage(const std::vector<uint8_t>& spirv, RHI::ShaderStageFlags stage);

	static void ParseDescriptorSets(const SpvReflectShaderModule& module, MaterialInfo& info);
	static void ParseBlockMembers(const SpvReflectBlockVariable& block, std::vector<FieldInfo>& fields);
	static void ParseInterfaceVariables(const SpvReflectShaderModule& module, MaterialInfo& info, RHI::ShaderStageFlags stage);
	static FieldType ConvertFieldType(const SpvReflectTypeDescription& typeDesc);

	MaterialInfo m_Info;
	bool m_Valid = false;
};
