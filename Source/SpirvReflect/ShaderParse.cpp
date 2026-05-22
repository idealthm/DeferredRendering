#include "ShaderParse.h"

#include <fstream>
#include <vector>

#include "spirv_reflect.h"
#include "Common/Serialization/ChunkContainer.h"
#include "Common/Serialization/MaterialBinaryChunks.h"

// ── Constructors ──────────────────────────────────────────────────────────────

ShaderParser::ShaderParser(const std::string& vertPath, const std::string& fragPath)
{
	auto readFile = [](const std::string& path, std::vector<uint8_t>& out) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
			return false;
		const size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		out.resize(fileSize);
		file.read(reinterpret_cast<char*>(out.data()), fileSize);
		return true;
	};

	if (!vertPath.empty() && readFile(vertPath, m_Info.shaderData[0]))
		m_Valid |= ParseStage(m_Info.shaderData[0], RHI::ShaderStageFlags::VERTEX);

	if (!fragPath.empty() && readFile(fragPath, m_Info.shaderData[1]))
		m_Valid |= ParseStage(m_Info.shaderData[1], RHI::ShaderStageFlags::FRAGMENT);
}

// ── Per-stage parsing ─────────────────────────────────────────────────────────

bool ShaderParser::ParseStage(const std::vector<uint8_t>& spirv, RHI::ShaderStageFlags stage)
{
	if (spirv.empty()) return false;

	const spv_reflect::ShaderModule shaderModule(spirv);
	if (shaderModule.GetResult() != SPV_REFLECT_RESULT_SUCCESS)
		return false;

	const auto& module = shaderModule.GetShaderModule();
	ParseDescriptorSets(module, m_Info);
	ParseInterfaceVariables(module, m_Info, stage);
	return true;
}

// ── Descriptor set parsing (fills descriptorSets + uib + sib) ─────────────────

static RHI::DescriptorType MapDescriptorType(SpvReflectDescriptorType spvType)
{
	switch (spvType)
	{
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return RHI::DescriptorType::SHADER_STORAGE_BUFFER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
	case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return RHI::DescriptorType::SAMPLER;
	default:
		return RHI::DescriptorType::UNIFORM_BUFFER;
	}
}

static bool IsUniformBuffer(SpvReflectDescriptorType spvType)
{
	return spvType == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	       spvType == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	       spvType == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	       spvType == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
}

static bool IsSampler(SpvReflectDescriptorType spvType)
{
	return spvType == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER ||
	       spvType == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
	       spvType == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
	       spvType == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

void ShaderParser::ParseDescriptorSets(const SpvReflectShaderModule& module, MaterialInfo& info)
{
	uint32_t setCount = 0;
	spvReflectEnumerateDescriptorSets(&module, &setCount, nullptr);

	std::vector<SpvReflectDescriptorSet*> sets(setCount);
	spvReflectEnumerateDescriptorSets(&module, &setCount, sets.data());

	uint32_t samplerCount = 0;

	for (uint32_t s = 0; s < setCount; s++)
	{
		const auto* src = sets[s];
		auto& descBindings = info.descriptorSets[src->set];

		for (uint32_t b = 0; b < src->binding_count; b++)
		{
			const auto* binding = src->bindings[b];

			// Skip if this binding already populated by another stage
			bool exists = false;
			for (const auto& existing : descBindings)
			{
				if (existing.binding == binding->binding)
				{
					exists = true;
					break;
				}
			}
			if (exists) continue;

			// ── Flat descriptor for Program ──
			Program::Descriptor flatDesc;
			flatDesc.binding = static_cast<uint8_t>(binding->binding);
			flatDesc.type = MapDescriptorType(binding->descriptor_type);

			if (IsUniformBuffer(binding->descriptor_type))
			{
				BufferInterfaceBlock block;
				block.size = static_cast<uint32_t>(binding->block.size);
				block.binding = static_cast<descriptor_binding_t>(binding->binding);
				block.layout = (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
				                binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
				                   ? MemoryLayout::std_430
				                   : MemoryLayout::std_140;
				block.instanceName = binding->name ? binding->name : "";
				if (binding->type_description && binding->type_description->type_name)
					block.structName = binding->type_description->type_name;

				ParseBlockMembers(binding->block, block.fields);

				flatDesc.name = block.structName.empty() ? block.instanceName : block.structName;

				// If MaterialParams UBO, save to uib
				if (block.structName == "MaterialParams")
					info.uib = block;
			}
			else if (IsSampler(binding->descriptor_type))
			{
				SamplerInfo sampler;
				sampler.name = binding->name ? binding->name : "";
				sampler.binding = static_cast<descriptor_binding_t>(binding->binding);

				switch (binding->image.dim)
				{
				case SpvDim1D:   sampler.sampler = RHI::SamplerType::SAMPLER_CUBEMAP_ARRAY;   break;
				case SpvDim2D:   sampler.sampler = (binding->image.arrayed != 0) ? RHI::SamplerType::SAMPLER_2D_ARRAY : RHI::SamplerType::SAMPLER_2D; break;
				case SpvDim3D:   sampler.sampler = RHI::SamplerType::SAMPLER_3D;   break;
				case SpvDimCube: sampler.sampler = RHI::SamplerType::SAMPLER_CUBEMAP; break;
				default:         sampler.sampler = RHI::SamplerType::SAMPLER_2D;   break;
				}

				flatDesc.name = sampler.name;

				// If MaterialParams_ sampler, add to sib
				if (sampler.name.find("materialParams_") == 0 && samplerCount < info.sib.mSamplersInfoList.size())
					info.sib.mSamplersInfoList[samplerCount++] = sampler;
			}
			else
			{
				continue;
			}

			descBindings.push_back(std::move(flatDesc));
		}
	}
}

// ── Block member parsing ──────────────────────────────────────────────────────

void ShaderParser::ParseBlockMembers(const SpvReflectBlockVariable& block, std::vector<FieldInfo>& fields)
{
	if (block.member_count == 0 || !block.members)
		return;

	fields.reserve(block.member_count);
	for (uint32_t i = 0; i < block.member_count; i++)
	{
		const auto& member = block.members[i];

		FieldInfo field;
		field.name = member.name ? member.name : "";
		field.offset = static_cast<uint16_t>(member.absolute_offset);
		field.type = member.type_description ? ConvertFieldType(*member.type_description) : FieldType::FLOAT;
		fields.push_back(std::move(field));
	}
}

// ── Interface variable parsing (stage-aware) ──────────────────────────────────

void ShaderParser::ParseInterfaceVariables(const SpvReflectShaderModule& module, MaterialInfo& info, RHI::ShaderStageFlags stage)
{
	uint32_t count = 0;

	if (stage == RHI::ShaderStageFlags::VERTEX)
	{
		spvReflectEnumerateInputVariables(&module, &count, nullptr);
		if (count > 0)
		{
			std::vector<SpvReflectInterfaceVariable*> vars(count);
			spvReflectEnumerateInputVariables(&module, &count, vars.data());

			for (uint32_t i = 0; i < count; i++)
			{
				const auto* v = vars[i];
				if (v->location >= info.inputVariables.size()) continue;
				VariableParam& param = info.inputVariables[v->location];
				param.name = v->name ? v->name : "";
				param.location = v->location;
				param.type = v->type_description ? ConvertFieldType(*v->type_description) : FieldType::FLOAT;
			}
		}
	}

	if (stage == RHI::ShaderStageFlags::FRAGMENT)
	{
		spvReflectEnumerateOutputVariables(&module, &count, nullptr);
		if (count > 0)
		{
			std::vector<SpvReflectInterfaceVariable*> vars(count);
			spvReflectEnumerateOutputVariables(&module, &count, vars.data());

			for (uint32_t i = 0; i < count; i++)
			{
				const auto* v = vars[i];
				if (v->location >= info.outputVariables.size()) continue;
				VariableParam& param = info.outputVariables[v->location];
				param.name = v->name ? v->name : "";
				param.location = v->location;
				param.type = v->type_description ? ConvertFieldType(*v->type_description) : FieldType::FLOAT;
			}
		}
	}
}

// ── Type conversion ───────────────────────────────────────────────────────────

FieldType ShaderParser::ConvertFieldType(const SpvReflectTypeDescription& typeDesc)
{
	const auto& traits = typeDesc.traits;
	const auto flags = typeDesc.type_flags;

	if (flags & SPV_REFLECT_TYPE_FLAG_MATRIX)
	{
		const uint32_t cols = traits.numeric.matrix.column_count;
		const uint32_t rows = traits.numeric.matrix.row_count;
		if (cols == 4 && rows == 4) return FieldType::MAT4;
		if (cols == 3 && rows == 3) return FieldType::MAT3;
	}

	if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
	{
		const uint32_t comps = traits.numeric.vector.component_count;

		if (flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
		{
			if (comps == 2) return FieldType::FLOAT2;
			if (comps == 3) return FieldType::FLOAT3;
			if (comps == 4) return FieldType::FLOAT4;
		}
		if (flags & SPV_REFLECT_TYPE_FLAG_INT)
		{
			if (traits.numeric.scalar.signedness)
			{
				if (comps == 2) return FieldType::INT2;
				if (comps == 3) return FieldType::INT3;
				if (comps == 4) return FieldType::INT4;
			}
			else
			{
				if (comps == 2) return FieldType::UINT2;
				if (comps == 3) return FieldType::UINT3;
				if (comps == 4) return FieldType::UINT4;
			}
		}
		if (flags & SPV_REFLECT_TYPE_FLAG_BOOL)
		{
			if (comps == 2) return FieldType::BOOL2;
			if (comps == 3) return FieldType::BOOL3;
			if (comps == 4) return FieldType::BOOL4;
		}
	}

	if (flags & SPV_REFLECT_TYPE_FLAG_FLOAT) return FieldType::FLOAT;
	if (flags & SPV_REFLECT_TYPE_FLAG_BOOL)  return FieldType::BOOL;
	if (flags & SPV_REFLECT_TYPE_FLAG_INT)
		return traits.numeric.scalar.signedness ? FieldType::INT : FieldType::UINT;

	return FieldType::FLOAT;
}

// ── Flat → engine type conversions ─────────────────────────────────────────────

FieldInfo MaterialInfo::FromFlat(const FlatFieldInfo& flat)
{
	FieldInfo f;
	f.name       = flat.name;
	f.offset     = flat.offset;
	f.stride     = flat.stride;
	f.type       = static_cast<FieldType>(flat.fieldType);
	f.isArray    = flat.isArray != 0;
	f.size       = flat.arraySize;
	f.structName = flat.structName;
	f.sizeName   = flat.sizeName;
	return f;
}

SamplerInfo MaterialInfo::FromFlat(const FlatSamplerInfo& flat)
{
	SamplerInfo s;
	s.name    = flat.name;
	s.sampler = static_cast<RHI::SamplerType>(flat.samplerType);
	s.binding = flat.binding;
	return s;
}

VariableParam MaterialInfo::FromFlat(const FlatVariableParam& flat)
{
	VariableParam v;
	v.name     = flat.name;
	v.type     = static_cast<FieldType>(flat.fieldType);
	v.location = flat.location;
	return v;
}

Program::Descriptor MaterialInfo::FromFlat(const FlatDescriptorBinding& flat)
{
	Program::Descriptor d;
	d.name    = flat.name;
	d.type    = static_cast<RHI::DescriptorType>(flat.descriptorType);
	d.binding = flat.binding;
	return d;
}

// ── MaterialInfo::LoadFromBinary ───────────────────────────────────────────────

bool MaterialInfo::LoadFromBinary(FArchive& ar)
{
	ChunkContainer cc;
	uint32_t chunkCount = 0;
	if (!cc.ReadFileHeader(ar, chunkCount))
		return false;

	for (uint32_t i = 0; i < chunkCount; i++)
	{
		ChunkType chunkType;
		uint32_t payloadSize = 0;
		if (!cc.ReadChunkHeader(ar, chunkType, payloadSize))
			return false;

		switch (chunkType)
		{
		case ChunkType::MaterialSpirv:
			{
				MaterialSpirvChunk c;
				c.Serialize(ar);
				shaderData[0] = std::move(c.vertexSpirv);
				shaderData[1] = std::move(c.fragmentSpirv);
			}
			break;

		case ChunkType::MaterialUib:
			{
				MaterialUibChunk c;
				c.Serialize(ar);
				uib.structName   = std::move(c.structName);
				uib.instanceName = std::move(c.instanceName);
				uib.size         = c.blockSize;
				uib.binding      = c.binding;
				uib.layout       = static_cast<MemoryLayout>(c.memoryLayout);
				uib.fields.reserve(c.fields.size());
				for (auto& flat : c.fields)
					uib.fields.push_back(FromFlat(flat));
			}
			break;

		case ChunkType::MaterialSib:
			{
				MaterialSibChunk c;
				c.Serialize(ar);
				sib.mName = std::move(c.blockName);
				for (size_t j = 0; j < c.samplers.size() && j < sib.mSamplersInfoList.size(); j++)
					sib.mSamplersInfoList[j] = FromFlat(c.samplers[j]);
			}
			break;

		case ChunkType::MaterialAttributeInfo:
			{
				MaterialAttributeInfoChunk c;
				c.Serialize(ar);
				for (size_t j = 0; j < c.inputVariables.size() && j < inputVariables.size(); j++)
					inputVariables[j] = FromFlat(c.inputVariables[j]);
				for (size_t j = 0; j < c.outputVariables.size() && j < outputVariables.size(); j++)
					outputVariables[j] = FromFlat(c.outputVariables[j]);
			}
			break;

		case ChunkType::MaterialDescriptorSetLayoutInfo:
			{
				MaterialDescriptorSetLayoutInfoChunk c;
				c.Serialize(ar);
				for (size_t s = 0; s < c.kDescriptorSetCount && s < descriptorSets.size(); s++)
				{
					descriptorSets[s].reserve(c.setBindings[s].size());
					for (auto& flat : c.setBindings[s])
						descriptorSets[s].push_back(FromFlat(flat));
				}
			}
			break;

		default:
			cc.SkipChunkPayload(ar);
			break;
		}

		cc.EndChunk(ar);
	}

	return true;
}

// ── ShaderParser::LoadFromBinary ───────────────────────────────────────────────

bool ShaderParser::LoadFromBinary(const std::string& path, MaterialInfo& outInfo)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		return false;

	const size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0);
	std::vector<uint8_t> buffer(fileSize);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	FArchiveRead ar(buffer.data(), buffer.size());
	outInfo = {};
	return outInfo.LoadFromBinary(ar);
}
