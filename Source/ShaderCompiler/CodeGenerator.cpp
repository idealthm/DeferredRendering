#include "CodeGenerator.h"

#include <algorithm>
#include <cctype>

#include "BufferInterfaceBlock.h"
#include "MaterialSpec.h"
#include "common/Core.h"


stream& CodeGenerator::generateCommonSamplers(stream& out, SamplerInterfaceBlock::SamplerInfoList const& list) const
{
	if (list.empty()) {
		return out;
	}

	for (auto const& info : list) {
		auto type = info.type;
		char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
		out << "layout(binding = " << (int)getUniqueSamplerBindingPoint() << ") ";
		out << "uniform "  << typeName << " " << info.uniformName.c_str();
		out << ";\n";
	}
	out << "\n";

	return out;
}

stream& CodeGenerator::generateUniforms(stream& out, const BufferInterfaceBlock& uib) const
{
	descriptor_binding_t binding = getUniqueUBOBindingPoint();
	return generateBufferInterfaceBlock(out, binding, uib);
}

static std::string EmitDefine(const std::string& name)
{
	return "#define " + name + "\n";
}

static std::string EmitDefine(const std::string& name, const std::string& value)
{
	return "#define " + name + " " + value + "\n";
}

static std::string UpperCase(const std::string& s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return (char)std::toupper(c); });
	return result;
}

stream& CodeGenerator::generateVaryingDefines(stream& out, const struct MaterialSpec& spec) const
{
	for (size_t i = 0; i < spec.variables.size(); ++i)
	{
		const auto& v = spec.variables[i];
		std::string upper = UpperCase(v.name);
		out << EmitDefine("HAS_VARIABLE_" + upper);
		out << EmitDefine("VARIABLE_CUSTOM" + std::to_string(i), v.name);
		out << EmitDefine("VARIABLE_CUSTOM_AT" + std::to_string(i), "variable_" + v.name);
	}
	return out;
}

stream& CodeGenerator::generateConstantDefines(stream& out, const struct MaterialSpec& spec) const
{
	for (const auto& c : spec.constants)
		out << EmitDefine("CONST_" + UpperCase(c.name), c.name);
	return out;
}

stream& CodeGenerator::generateAttributeDefines(stream& out, const struct MaterialSpec& spec) const
{
	auto attrMacro = [](VertexAttribute attr) -> const char* {
		switch (attr) {
		case VertexAttribute::POSITION:      return "POSITION";
		case VertexAttribute::TANGENTS:      return "TANGENTS";
		case VertexAttribute::COLOR:         return "COLOR";
		case VertexAttribute::UV0:           return "UV0";
		case VertexAttribute::UV1:           return "UV1";
		case VertexAttribute::BONE_INDICES:  return "BONE_INDICES";
		case VertexAttribute::BONE_WEIGHTS:  return "BONE_WEIGHTS";
		case VertexAttribute::CUSTOM0:       return "CUSTOM0";
		case VertexAttribute::CUSTOM1:       return "CUSTOM1";
		case VertexAttribute::CUSTOM2:       return "CUSTOM2";
		case VertexAttribute::CUSTOM3:       return "CUSTOM3";
		case VertexAttribute::CUSTOM4:       return "CUSTOM4";
		case VertexAttribute::CUSTOM5:       return "CUSTOM5";
		case VertexAttribute::CUSTOM6:       return "CUSTOM6";
		case VertexAttribute::CUSTOM7:       return "CUSTOM7";
		default:                             return "UNKNOWN";
		}
	};

	for (VertexAttribute attr : spec.requiredAttributes)
		out << EmitDefine("HAS_ATTRIBUTE_" + std::string(attrMacro(attr)));


	return out;
}

stream& CodeGenerator::generateVaryingDeclarations(stream& out, const struct MaterialSpec& spec, const char* direction) const
{
	if (spec.variables.empty())
		return out;

	for (size_t i = 0; i < spec.variables.size(); ++i)
		out << "LAYOUT_LOCATION(" << (10 + i) << ") " << direction << " vec4 VARIABLE_CUSTOM_AT" << i << ";\n";

	return out;
}

stream& CodeGenerator::generateBufferInterfaceBlock(stream& out, descriptor_binding_t binding, const BufferInterfaceBlock& uib) const
{
	auto const& infos = uib.getFieldInfoList();

	std::string blockName{ uib.getName() };
	std::string instanceName{ uib.getName() };
	blockName.front() = char(std::toupper((unsigned char)blockName.front()));
	instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

	out << "\nlayout(binding = " << +binding << ", ";
	switch (uib.getAlignment()) {
		case BufferInterfaceBlock::Alignment::std140:
			out << "std140";
			break;
		case BufferInterfaceBlock::Alignment::std430:
			out << "std430";
			break;
	}
	out << ") ";

	switch (uib.getTarget()) {
		case BufferInterfaceBlock::Target::UNIFORM:
			out << "uniform ";
			break;
		case BufferInterfaceBlock::Target::SSBO:
			out << "buffer ";
			break;
	}

	out << blockName << " ";

	if (uib.getTarget() == BufferInterfaceBlock::Target::SSBO) {
		uint8_t qualifiers = uib.getQualifier();
		while (qualifiers) {
			uint8_t const mask = (qualifiers & -qualifiers);
			switch (BufferInterfaceBlock::Qualifier(qualifiers & mask)) {
				case BufferInterfaceBlock::Qualifier::COHERENT:  out << "coherent "; break;
				case BufferInterfaceBlock::Qualifier::WRITEONLY: out << "writeonly "; break;
				case BufferInterfaceBlock::Qualifier::READONLY:  out << "readonly "; break;
				case BufferInterfaceBlock::Qualifier::VOLATILE:  out << "volatile "; break;
				case BufferInterfaceBlock::Qualifier::RESTRICT:  out << "restrict "; break;
			}
			qualifiers &= ~mask;
		}
	}

	out << "{\n";

	generateInterfaceFields(out, infos);

	out << "} " << instanceName << ";\n";

	return out;
}

char const* CodeGenerator::getUniformTypeName(BufferInterfaceBlock::FieldInfo const& info) noexcept
{
	using Type = BufferInterfaceBlock::Type;
	switch (info.type) {
	case Type::STRUCT: return info.structName.c_str();
	default:            return getTypeName(info.type);
	}
}

char const* CodeGenerator::getTypeName(UniformType type)
{
	switch (type) {
	case UniformType::BOOL:   return "bool";
	case UniformType::BOOL2:  return "bvec2";
	case UniformType::BOOL3:  return "bvec3";
	case UniformType::BOOL4:  return "bvec4";
	case UniformType::FLOAT:  return "float";
	case UniformType::FLOAT2: return "vec2";
	case UniformType::FLOAT3: return "vec3";
	case UniformType::FLOAT4: return "vec4";
	case UniformType::INT:    return "int";
	case UniformType::INT2:   return "ivec2";
	case UniformType::INT3:   return "ivec3";
	case UniformType::INT4:   return "ivec4";
	case UniformType::UINT:   return "uint";
	case UniformType::UINT2:  return "uvec2";
	case UniformType::UINT3:  return "uvec3";
	case UniformType::UINT4:  return "uvec4";
	case UniformType::MAT3:   return "mat3";
	case UniformType::MAT4:   return "mat4";
	case UniformType::STRUCT: return "";
	}
	ASSERT(false);
	return nullptr;
}

char const* CodeGenerator::getSamplerTypeName(RHI::SamplerType type, RHI::SamplerFormat format, bool multisample)
{
	using RHI::SamplerType;
	using RHI::SamplerFormat;

	switch (type) {
		case SamplerType::SAMPLER_2D:
			switch (format) {
				case SamplerFormat::INT:    return multisample ? "isampler2DMS" : "isampler2D";
				case SamplerFormat::UINT:   return multisample ? "usampler2DMS" : "usampler2D";
				case SamplerFormat::FLOAT:  return multisample ? "sampler2DMS" : "sampler2D";
				case SamplerFormat::SHADOW: return "sampler2DShadow";
			}
		case SamplerType::SAMPLER_3D:
			assert(format != SamplerFormat::SHADOW);
			switch (format) {
				case SamplerFormat::INT:    return "isampler3D";
				case SamplerFormat::UINT:   return "usampler3D";
				case SamplerFormat::FLOAT:  return "sampler3D";
				case SamplerFormat::SHADOW: return nullptr;
			}
		case SamplerType::SAMPLER_2D_ARRAY:
			switch (format) {
				case SamplerFormat::INT:    return multisample ? "isampler2DMSArray": "isampler2DArray";
				case SamplerFormat::UINT:   return multisample ? "usampler2DMSArray": "usampler2DArray";
				case SamplerFormat::FLOAT:  return multisample ? "sampler2DMSArray": "sampler2DArray";
				case SamplerFormat::SHADOW: return "sampler2DArrayShadow";
			}
		case SamplerType::SAMPLER_CUBEMAP:
			switch (format) {
				case SamplerFormat::INT:    return "isamplerCube";
				case SamplerFormat::UINT:   return "usamplerCube";
				case SamplerFormat::FLOAT:  return "samplerCube";
				case SamplerFormat::SHADOW: return "samplerCubeShadow";
			}
		case SamplerType::SAMPLER_CUBEMAP_ARRAY:
			switch (format) {
				case SamplerFormat::INT:    return "isamplerCubeArray";
				case SamplerFormat::UINT:   return "usamplerCubeArray";
				case SamplerFormat::FLOAT:  return "samplerCubeArray";
				case SamplerFormat::SHADOW: return "samplerCubeArrayShadow";
			}
	}
	ASSERT(0);
	return nullptr;
}

stream& CodeGenerator::generateInterfaceFields(stream& out, std::vector<BufferInterfaceBlock::FieldInfo> const& infos) const
{
	for (auto const& info : infos) {
		char const* const type = getUniformTypeName(info);
		out << "\t" << type << " " << info.name.c_str();
		if (info.isArray) {
			if (info.sizeName.empty()) {
				if (info.size) {
					out << "[" << info.size << "]";
				} else {
					out << "[]";
				}
			} else {
				out << "[" << info.sizeName.c_str() << "]";
			}
		}
		out << ";\n";
	}
	return out;
}
