#include "ShaderInputBuilder.h"

#include <cstring>

#include "CodeGenerator.h"
#include "MaterialSpec.h"

FieldType GLSLTypeToFieldType(const char* glslType)
{
	if (std::strcmp(glslType, "vec2")  == 0) return FieldType::FLOAT2;
	if (std::strcmp(glslType, "uvec4") == 0) return FieldType::UINT4;
	return FieldType::FLOAT4;
}

std::vector<VariableParam> BuildVertexInputs(const MaterialSpec& spec)
{
	std::vector<VariableParam> inputs;

	// POSITION is always at location 0 for surface domain
	if (spec.domain == MaterialDomain::SURFACE)
	{
		inputs.push_back({ "mesh_position", FieldType::FLOAT4, 0 });
	}

	for (VertexAttribute attr : spec.requiredAttributes)
	{
		// Skip POSITION if already added for surface domain
		if (spec.domain == MaterialDomain::SURFACE && attr == VertexAttribute::POSITION)
			continue;

		VariableParam v;
		v.name     = VertexAttributeToName(attr);
		v.location = static_cast<uint8_t>(VertexAttributeToLocation(attr));
		v.type     = GLSLTypeToFieldType(VertexAttributeToGLSLType(attr));
		inputs.push_back(v);
	}

	return inputs;
}

std::vector<VariableParam> BuildFragmentOutputs(const MaterialSpec& spec)
{
	std::vector<VariableParam> outputs;

	// Default output
	outputs.push_back({ "fragColor", FieldType::FLOAT4, 0 });

	for (auto& o : spec.outputs)
	{
		FieldType ft = (o.type == "color") ? FieldType::FLOAT4 : FieldType::FLOAT;
		uint8_t location = 0; // spec.outputs don't carry location info currently
		outputs.push_back({ o.name, ft, location });
	}

	return outputs;
}
