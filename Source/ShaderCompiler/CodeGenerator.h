#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "BufferInterfaceBlock.h"
#include "EngineEnum.h"
#include "SamplerInterfaceBlock.h"
#include "RHI/DriverEnums.h"

inline const char* VertexAttributeToGLSLType(VertexAttribute attr)
{
	switch (attr)
	{
	case VertexAttribute::POSITION:      return "vec4";
	case VertexAttribute::TANGENTS:      return "vec4";
	case VertexAttribute::COLOR:         return "vec4";
	case VertexAttribute::UV0:           return "vec2";
	case VertexAttribute::UV1:           return "vec2";
	case VertexAttribute::BONE_INDICES:  return "uvec4";
	case VertexAttribute::BONE_WEIGHTS:  return "vec4";
	default:                             return "vec4";
	}
}

inline const char* VertexAttributeToName(VertexAttribute attr)
{
	switch (attr)
	{
	case VertexAttribute::POSITION:      return "mesh_position";
	case VertexAttribute::TANGENTS:      return "mesh_tangents";
	case VertexAttribute::COLOR:         return "mesh_color";
	case VertexAttribute::UV0:           return "mesh_uv0";
	case VertexAttribute::UV1:           return "mesh_uv1";
	case VertexAttribute::BONE_INDICES:  return "mesh_bone_indices";
	case VertexAttribute::BONE_WEIGHTS:  return "mesh_bone_weights";
	case VertexAttribute::CUSTOM0:       return "mesh_custom0";
	case VertexAttribute::CUSTOM1:       return "mesh_custom1";
	case VertexAttribute::CUSTOM2:       return "mesh_custom2";
	case VertexAttribute::CUSTOM3:       return "mesh_custom3";
	case VertexAttribute::CUSTOM4:       return "mesh_custom4";
	case VertexAttribute::CUSTOM5:       return "mesh_custom5";
	case VertexAttribute::CUSTOM6:       return "mesh_custom6";
	case VertexAttribute::CUSTOM7:       return "mesh_custom7";
	default:                             return "mesh_custom";
	}
}

inline int VertexAttributeToLocation(VertexAttribute attr)
{
	return static_cast<int>(attr);
}

struct BufferInterfaceBlock;
struct MaterialSpec;

using stream = std::ostringstream;

class CodeGenerator
{
public:

	// ── Sampler ──────────────────────────────────────────────────────

	stream& generateCommonSamplers(stream& out, SamplerInterfaceBlock::SamplerInfoList const& list) const;
	stream& generateCommonSamplers(stream& out, const SamplerInterfaceBlock& sib) const {
		return generateCommonSamplers(out, sib.getSamplerInfoList());
	}

	// ── Uniform / SSBO ───────────────────────────────────────────────

	stream& generateUniforms(stream& out, const BufferInterfaceBlock& uib) const;
	stream& generateBufferInterfaceBlock(stream& out, descriptor_binding_t binding, const BufferInterfaceBlock& uib) const;

	static char const* getUniformTypeName(BufferInterfaceBlock::FieldInfo const& info) noexcept;
	static char const* getTypeName(UniformType type);
	static char const* getSamplerTypeName(RHI::SamplerType type, RHI::SamplerFormat format, bool multisample);

	// ── Feature macros ───────────────────────────────────────────────

	/// HAS_VARIABLE_xxx / VARIABLE_CUSTOM<i> / VARIABLE_CUSTOM_AT<i>
	stream& generateVaryingDefines(stream& out, const struct MaterialSpec& spec) const;

	/// CONST_xxx
	stream& generateConstantDefines(stream& out, const struct MaterialSpec& spec) const;

	/// MATERIAL_HAS_xxx — one for each property
	stream& generatePropertyDefines(stream& out, const struct MaterialSpec& spec) const;

	/// HAS_ATTRIBUTE_xxx + implied HAS_ATTRIBUTE_POSITION
	stream& generateAttributeDefines(stream& out, const struct MaterialSpec& spec) const;

	// ── Stage-specific declarations ──────────────────────────────────

	/// LAYOUT_LOCATION(10+i) <direction> vec4 VARIABLE_CUSTOM_AT<i>
	stream& generateVaryingDeclarations(stream& out, const struct MaterialSpec& spec, const char* direction) const;

private:
	uint8_t getUniqueSamplerBindingPoint() const {return m_UniqueSamplerBindingPoint++;};
	uint8_t getUniqueUBOBindingPoint() const {return m_UniqueUBOBindingPoint++;};
	uint8_t getUniqueSSBOBindingPoint() const {return m_UniqueSSBOBindingPoint++;};

	stream& generateInterfaceFields(stream& out, std::vector<BufferInterfaceBlock::FieldInfo> const& infos) const;

	mutable uint8_t m_UniqueUBOBindingPoint = 0;
	mutable uint8_t m_UniqueSamplerBindingPoint = 0;
	mutable uint8_t m_UniqueSSBOBindingPoint = 0;
};
