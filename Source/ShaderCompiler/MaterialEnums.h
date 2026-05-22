#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "RHI/RHIDriver.h"

/**
 * Uniform property types — map to std140 uniform block members
 */
enum class UniformType : uint8_t {
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
    STRUCT,
};

inline const char* UniformTypeToGLSL(UniformType t)
{
    switch (t)
    {
    case UniformType::BOOL:  return "bool";
    case UniformType::BOOL2: return "bvec2";
    case UniformType::BOOL3: return "bvec3";
    case UniformType::BOOL4: return "bvec4";
    case UniformType::FLOAT: return "float";
    case UniformType::FLOAT2: return "vec2";
    case UniformType::FLOAT3: return "vec3";
    case UniformType::FLOAT4: return "vec4";
    case UniformType::INT:   return "int";
    case UniformType::INT2:  return "ivec2";
    case UniformType::INT3:  return "ivec3";
    case UniformType::INT4:  return "ivec4";
    case UniformType::UINT:  return "uint";
    case UniformType::UINT2: return "uvec2";
    case UniformType::UINT3: return "uvec3";
    case UniformType::UINT4: return "uvec4";
    case UniformType::MAT3:  return "mat3";
    case UniformType::MAT4:  return "mat4";
    case UniformType::STRUCT: return "";  // name provided separately
    }
    return "";
}

static const std::unordered_map<std::string, UniformType> sStringToUniformType = {
    {"bool",  UniformType::BOOL},  {"bool2",  UniformType::BOOL2},
    {"bool3", UniformType::BOOL3}, {"bool4",  UniformType::BOOL4},
    {"float", UniformType::FLOAT}, {"float2",  UniformType::FLOAT2},
    {"float3", UniformType::FLOAT3}, {"float4",  UniformType::FLOAT4},
    {"int",   UniformType::INT},   {"int2",   UniformType::INT2},
    {"int3",  UniformType::INT3},  {"int4",   UniformType::INT4},
    {"uint",  UniformType::UINT},  {"uint2",  UniformType::UINT2},
    {"uint3", UniformType::UINT3}, {"uint4",  UniformType::UINT4},
    {"mat3",  UniformType::MAT3},  {"mat4",   UniformType::MAT4},
    {"struct", UniformType::STRUCT},
};

inline UniformType ParseUniformType(const std::string& s)
{
    return sStringToUniformType.at(s);
}

inline bool IsValidUniformType(const std::string& s)
{
    return sStringToUniformType.find(s) != sStringToUniformType.end();
}



inline const char* SamplerTypeToGLSL(RHI::SamplerType t)
{
    switch (t)
    {
    case RHI::SamplerType::SAMPLER_2D:               return "sampler2D";
    case RHI::SamplerType::SAMPLER_2D_ARRAY:         return "sampler2DArray";
    case RHI::SamplerType::SAMPLER_CUBEMAP:          return "samplerCube";
    case RHI::SamplerType::SAMPLER_3D:               return "sampler3D";
    case RHI::SamplerType::SAMPLER_CUBEMAP_ARRAY:    return "samplerCubeArray";
    }
    return "";
}

inline RHI::SamplerType ParseSamplerType(const std::string& s)
{
    static const std::unordered_map<std::string, RHI::SamplerType> map = {
        {"sampler2d",           RHI::SamplerType::SAMPLER_2D},
        {"sampler2dArray",      RHI::SamplerType::SAMPLER_2D_ARRAY},
        {"samplerCubemap",      RHI::SamplerType::SAMPLER_CUBEMAP},
        {"sampler3d",           RHI::SamplerType::SAMPLER_3D},
        {"samplerCubemapArray", RHI::SamplerType::SAMPLER_CUBEMAP_ARRAY},
    };
    return map.at(s);
}

inline bool IsValidSamplerType(const std::string& s)
{
    static const std::unordered_map<std::string, RHI::SamplerType> map = {
        {"sampler2d",           RHI::SamplerType::SAMPLER_2D},
        {"sampler2dArray",      RHI::SamplerType::SAMPLER_2D_ARRAY},
        {"samplerCubemap",      RHI::SamplerType::SAMPLER_CUBEMAP},
        {"sampler3d",           RHI::SamplerType::SAMPLER_3D},
        {"samplerCubemapArray", RHI::SamplerType::SAMPLER_CUBEMAP_ARRAY},
    };
    return map.find(s) != map.end();
}
