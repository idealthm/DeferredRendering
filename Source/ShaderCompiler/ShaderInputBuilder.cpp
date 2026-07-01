#include "ShaderInputBuilder.h"

#include <cstring>

#include "CodeGenerator.h"
#include "Common/Material/MaterialBuilder.h"

FieldType GLSLTypeToFieldType(const char* glslType)
{
	if (std::strcmp(glslType, "vec2")  == 0) return FieldType::FLOAT2;
	if (std::strcmp(glslType, "uvec4") == 0) return FieldType::UINT4;
	return FieldType::FLOAT4;
}

std::vector<VariableParam> BuildVertexInputs(MaterialDomain domain,
	const RHI::AttributeBitset& requiredAttributes)
{
	std::vector<VariableParam> inputs;

	if (domain == MaterialDomain::SURFACE)
		inputs.push_back({ "mesh_position", FieldType::FLOAT4, 0 });

	requiredAttributes.forEachSetBit([&](size_t bit) {
		VertexAttribute attr = static_cast<VertexAttribute>(bit);
		if (domain == MaterialDomain::SURFACE && attr == VertexAttribute::POSITION)
			return;

		VariableParam v;
		v.name     = VertexAttributeToName(attr);
		v.location = static_cast<uint8_t>(VertexAttributeToLocation(attr));
		v.type     = GLSLTypeToFieldType(VertexAttributeToGLSLType(attr));
		inputs.push_back(v);
	});

	return inputs;
}

std::vector<VariableParam> BuildFragmentOutputs(
	const MaterialBuilder::OutputList& outputs)
{
	std::vector<VariableParam> result;
	result.push_back({ "fragColor", FieldType::FLOAT4, 0 });

	for (auto& o : outputs)
	{
		FieldType ft = FieldType::FLOAT4;
		switch (o.type)
		{
		case MaterialBuilder::OutputType::FLOAT:  ft = FieldType::FLOAT;  break;
		case MaterialBuilder::OutputType::FLOAT2: ft = FieldType::FLOAT2; break;
		case MaterialBuilder::OutputType::FLOAT3: ft = FieldType::FLOAT3; break;
		case MaterialBuilder::OutputType::FLOAT4: ft = FieldType::FLOAT4; break;
		}
		result.push_back({ o.name, ft, (uint8_t)o.location });
	}

	return result;
}
