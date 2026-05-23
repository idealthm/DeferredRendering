#pragma once

#include <cstdint>
#include <string>

// =============================================================================
// Forward declarations for RHI types (avoid including RHI headers in compiler)
// =============================================================================
namespace RHI {
    enum class SamplerType : uint8_t;
    enum class ElementType : uint8_t;
}

// =============================================================================
// Pipeline type (used by both compiler and runtime)
// =============================================================================
enum class Pipeline : uint8_t {
    DEFERRED = 0,
    FORWARD  = 1,
};

// =============================================================================
// Vertex attributes
// =============================================================================
enum VertexAttribute : uint8_t {
    POSITION        = 0, //!< XYZ position (float3)
    TANGENTS        = 1, //!< tangent, bitangent and normal, encoded as a quaternion (float4)
    COLOR           = 2, //!< vertex color (float4)
    UV0             = 3, //!< texture coordinates (float2)
    UV1             = 4, //!< texture coordinates (float2)
    BONE_INDICES    = 5, //!< indices of 4 bones, as unsigned integers (uvec4)
    BONE_WEIGHTS    = 6, //!< weights of the 4 bones (normalized float4)
    CUSTOM0         = 8,
    CUSTOM1         = 9,
    CUSTOM2         = 10,
    CUSTOM3         = 11,
    CUSTOM4         = 12,
    CUSTOM5         = 13,
    CUSTOM6         = 14,
    CUSTOM7         = 15,
};

// =============================================================================
// Material domain
// =============================================================================
enum class MaterialDomain : uint8_t {
    SURFACE         = 0, //!< shaders applied to renderables
    POST_PROCESS    = 1, //!< shaders applied to rendered buffers
    COMPUTE         = 2, //!< compute shader
};

// =============================================================================
// Uniform property types — map to std140 uniform block members
// =============================================================================
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

// =============================================================================
// Material properties (runtime shading model parameters)
// =============================================================================
static constexpr size_t MATERIAL_PROPERTIES_COUNT = 29;
enum class Property : uint8_t {
    BASE_COLOR,              //!< float4, all shading models
    ROUGHNESS,               //!< float,  lit shading models only
    METALLIC,                //!< float,  all shading models, except unlit and cloth
    REFLECTANCE,             //!< float,  all shading models, except unlit and cloth
    AMBIENT_OCCLUSION,       //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT,              //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT_ROUGHNESS,    //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT_NORMAL,       //!< float,  lit shading models only, except subsurface and cloth
    ANISOTROPY,              //!< float,  lit shading models only, except subsurface and cloth
    ANISOTROPY_DIRECTION,    //!< float3, lit shading models only, except subsurface and cloth
    THICKNESS,               //!< float,  subsurface shading model only
    SUBSURFACE_POWER,        //!< float,  subsurface shading model only
    SUBSURFACE_COLOR,        //!< float3, subsurface and cloth shading models only
    SHEEN_COLOR,             //!< float3, lit shading models only, except subsurface
    SHEEN_ROUGHNESS,         //!< float3, lit shading models only, except subsurface and cloth
    SPECULAR_COLOR,          //!< float3, specular-glossiness shading model only
    GLOSSINESS,              //!< float,  specular-glossiness shading model only
    EMISSIVE,                //!< float4, all shading models
    NORMAL,                  //!< float3, all shading models only, except unlit
    POST_LIGHTING_COLOR,     //!< float4, all shading models
    POST_LIGHTING_MIX_FACTOR,//!< float, all shading models
    CLIP_SPACE_TRANSFORM,    //!< mat4,   vertex shader only
    ABSORPTION,              //!< float3, how much light is absorbed by the material
    TRANSMISSION,            //!< float,  how much light is refracted through the material
    IOR,                     //!< float,  material's index of refraction
    MICRO_THICKNESS,         //!< float, thickness of the thin layer
    BENT_NORMAL,             //!< float3, all shading models only, except unlit
    SPECULAR_FACTOR,         //!< float, lit shading models only, except subsurface and cloth
    SPECULAR_COLOR_FACTOR,   //!< float3, lit shading models only, except subsurface and cloth
};
