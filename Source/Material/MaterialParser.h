#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Common/Serialization/ChunkContainer.h"
#include "Common/Serialization/MaterialChunks.h"
#include "Common/Material/MaterialCommon.h"
#include "Common/Material/MaterialTypes.h"
#include "Shader/Program.h"

// =============================================================================
// glm type mapping (engine-side, used by uniform buffer system)
// =============================================================================

template<FieldType T> struct FieldTypeTraits;

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

template<FieldType T> constexpr size_t GetFieldTypeSize() { return sizeof(typename FieldTypeTraits<T>::type); }

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

// =============================================================================
// MaterialParser
// =============================================================================

class MaterialParser
{
public:
	explicit MaterialParser(const std::string& path);
	explicit MaterialParser(const void* data, size_t size);

	template<typename T>
	bool Get(typename T::Container& out)
	{
		return m_CC.Get<T>(out);
	}

private:
	std::vector<uint8_t> m_Buffer;
	ChunkContainer       m_CC;
};
