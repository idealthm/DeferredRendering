#include "ParameterProcessor.h"

#include <algorithm>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Common/Material/MaterialBuilder.h"
#include "MaterialEnums.h"

namespace
{
	static const char* JsonString(const JsonishObject* obj, const char* key, const char* defaultVal = nullptr)
	{
		if (!obj->hasKey(key)) return defaultVal;
		auto* v = obj->getValue(key)->toJsonString();
		return v ? v->getString().c_str() : defaultVal;
	}

	template<typename EnumT>
	static bool ParseEnum(const std::unordered_map<std::string, EnumT>& map, const std::string& s, EnumT& out)
	{
		auto it = map.find(s);
		if (it == map.end()) return false;
		out = it->second;
		return true;
	}

	// ── Types in MaterialCommon.h (global scope) ──────────────────────────

	static bool ParseBlending(const std::string& s, BlendingMode& out)
	{
		static const std::unordered_map<std::string, BlendingMode> map = {
			{"opaque", BlendingMode::OPAQUE}, {"transparent", BlendingMode::TRANSPARENT},
			{"add", BlendingMode::ADD}, {"masked", BlendingMode::MASKED},
			{"fade", BlendingMode::FADE}, {"multiply", BlendingMode::MULTIPLY},
			{"screen", BlendingMode::SCREEN}, {"custom", BlendingMode::CUSTOM},
		};
		return ParseEnum(map, s, out);
	}

	static bool ParseVertexDomain(const std::string& s, VertexDomain& out)
	{
		static const std::unordered_map<std::string, VertexDomain> map = {
			{"object", VertexDomain::OBJECT}, {"world", VertexDomain::WORLD},
			{"view",  VertexDomain::VIEW},  {"device", VertexDomain::DEVICE},
		};
		return ParseEnum(map, s, out);
	}

	static bool ParseShading(const std::string& s, Shading& out)
	{
		static const std::unordered_map<std::string, Shading> map = {
			{"unlit", Shading::UNLIT}, {"lit", Shading::LIT},
			{"subsurface", Shading::SUBSURFACE}, {"cloth", Shading::CLOTH},
			{"specularGlossiness", Shading::SPECULAR_GLOSSINESS},
		};
		return ParseEnum(map, s, out);
	}

	static bool ParseDomain(const std::string& s, MaterialDomain& out)
	{
		static const std::unordered_map<std::string, MaterialDomain> map = {
			{"surface", MaterialDomain::SURFACE}, {"postprocess", MaterialDomain::POST_PROCESS},
			{"compute", MaterialDomain::COMPUTE},
		};
		return ParseEnum(map, s, out);
	}

	static bool ParsePipeline(const std::string& s, Pipeline& out)
	{
		static const std::unordered_map<std::string, Pipeline> map = {
			{"deferred", Pipeline::DEFERRED}, {"forward", Pipeline::FORWARD},
			{"lighting", Pipeline::LIGHTING},
		};
		return ParseEnum(map, s, out);
	}

	// ── Types in DriverEnums.h (RHI namespace) ────────────────────────────

	static bool ParseBlendFunction(const std::string& s, RHI::BlendFunction& out)
	{
		static const std::unordered_map<std::string, RHI::BlendFunction> map = {
			{"zero", RHI::BlendFunction::ZERO}, {"one", RHI::BlendFunction::ONE},
			{"srcColor", RHI::BlendFunction::SRC_COLOR}, {"oneMinusSrcColor", RHI::BlendFunction::ONE_MINUS_SRC_COLOR},
			{"dstColor", RHI::BlendFunction::DST_COLOR}, {"oneMinusDstColor", RHI::BlendFunction::ONE_MINUS_DST_COLOR},
			{"srcAlpha", RHI::BlendFunction::SRC_ALPHA}, {"oneMinusSrcAlpha", RHI::BlendFunction::ONE_MINUS_SRC_ALPHA},
			{"dstAlpha", RHI::BlendFunction::DST_ALPHA}, {"oneMinusDstAlpha", RHI::BlendFunction::ONE_MINUS_DST_ALPHA},
			{"srcAlphaSaturate", RHI::BlendFunction::SRC_ALPHA_SATURATE},
		};
		return ParseEnum(map, s, out);
	}

	static bool ParseCulling(const std::string& s, RHI::CullingMode& out)
	{
		static const std::unordered_map<std::string, RHI::CullingMode> map = {
			{"none", RHI::CullingMode::NONE}, {"front", RHI::CullingMode::FRONT},
			{"back", RHI::CullingMode::BACK}, {"frontAndBack", RHI::CullingMode::FRONT_AND_BACK},
		};
		return ParseEnum(map, s, out);
	}

	// ── Processors ─────────────────────────────────────────────────────────

	static bool processName(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		builder.name(s->getString().c_str());
		return true;
	}

	static bool processParameters(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* arr = value.toJsonArray(); if (!arr) return false;
		for (size_t i = 0; i < arr->getElements().size(); ++i)
		{
			auto* obj = arr->getElements()[i]->toJsonObject(); if (!obj) return false;
			const char* name = JsonString(obj, "name"); if (!name) return false;

			// Legacy "type" key — value determines sampler vs uniform
			if (obj->hasKey("type"))
			{
				std::string t = JsonString(obj, "type");
				if (IsValidSamplerType(t))
					builder.parameter(name, ParseSamplerType(t));
				else if (IsValidUniformType(t))
				{
					auto ut = ParseUniformType(t);
					if (obj->hasKey("size")) {
						auto* sz = obj->getValue("size")->toJsonNumber();
						builder.parameter(name, sz ? static_cast<size_t>(sz->getFloat()) : 1, ut);
					} else builder.parameter(name, ut);
				}
				else { std::cerr << "Parameter '" << name << "': unknown type '" << t << "'" << std::endl; return false; }
				continue;
			}
			// Explicit "samplerType" / "uniformType"
			if (obj->hasKey("samplerType")) {
				std::string s = JsonString(obj, "samplerType");
				if (IsValidSamplerType(s)) { builder.parameter(name, ParseSamplerType(s)); continue; }
				std::cerr << "Unknown sampler: " << s << std::endl; return false;
			}
			if (obj->hasKey("uniformType")) {
				std::string s = JsonString(obj, "uniformType");
				if (IsValidUniformType(s)) {
					auto ut = ParseUniformType(s);
					if (obj->hasKey("size")) {
						auto* sz = obj->getValue("size")->toJsonNumber();
						builder.parameter(name, sz ? static_cast<size_t>(sz->getFloat()) : 1, ut);
					} else builder.parameter(name, ut);
					continue;
				}
				std::cerr << "Unknown uniform: " << s << std::endl; return false;
			}
			std::cerr << "Parameter '" << name << "': needs 'type', 'samplerType', or 'uniformType'" << std::endl;
			return false;
		}
		return true;
	}

	static bool processConstants(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* arr = value.toJsonArray(); if (!arr) return false;
		for (size_t i = 0; i < arr->getElements().size(); ++i)
		{
			auto* obj = arr->getElements()[i]->toJsonObject(); if (!obj) return false;
			const char* name = JsonString(obj, "name"); if (!name) return false;
			builder.shaderDefine(name, JsonString(obj, "value", ""));
		}
		return true;
	}

	static bool processVariables(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* arr = value.toJsonArray(); if (!arr) return false;
		for (size_t i = 0; i < arr->getElements().size() && i < MaterialBuilder::MATERIAL_VARIABLES_COUNT; ++i)
		{
			auto* s = arr->getElements()[i]->toJsonString(); if (!s) return false;
			builder.variable(static_cast<MaterialBuilder::Variable>(i), s->getString().c_str());
		}
		return true;
	}

	static bool processRequires(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* arr = value.toJsonArray(); if (!arr) return false;
		static const std::unordered_map<std::string, VertexAttribute> map = {
			{"POSITION", VertexAttribute::POSITION}, {"TANGENTS", VertexAttribute::TANGENTS},
			{"COLOR",    VertexAttribute::COLOR},    {"UV0",      VertexAttribute::UV0},
			{"UV1",      VertexAttribute::UV1},      {"BONE_INDICES", VertexAttribute::BONE_INDICES},
			{"BONE_WEIGHTS", VertexAttribute::BONE_WEIGHTS},
			{"CUSTOM0",  VertexAttribute::CUSTOM0},  {"CUSTOM1",  VertexAttribute::CUSTOM1},
			{"CUSTOM2",  VertexAttribute::CUSTOM2},  {"CUSTOM3",  VertexAttribute::CUSTOM3},
			{"CUSTOM4",  VertexAttribute::CUSTOM4},  {"CUSTOM5",  VertexAttribute::CUSTOM5},
			{"CUSTOM6",  VertexAttribute::CUSTOM6},  {"CUSTOM7",  VertexAttribute::CUSTOM7},
		};
		for (size_t i = 0; i < arr->getElements().size(); ++i) {
			auto* s = arr->getElements()[i]->toJsonString(); if (!s) return false;
			auto it = map.find(s->getString());
			if (it == map.end()) { std::cerr << "Unknown attribute: " << s->getString() << std::endl; return false; }
			builder.require(it->second);
		}
		return true;
	}

	static bool processBlending(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		BlendingMode m;
		if (!ParseBlending(s->getString(), m)) { std::cerr << "Unknown blending: " << s->getString() << std::endl; return false; }
		builder.blending(m); return true;
	}

	static bool processBlendFunction(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* obj = value.toJsonObject(); if (!obj) return false;
		RHI::BlendFunction srcRGB, srcA, dstRGB, dstA;
		if (!ParseBlendFunction(JsonString(obj, "srcRGB", "one"), srcRGB) ||
		    !ParseBlendFunction(JsonString(obj, "srcA", "one"), srcA) ||
		    !ParseBlendFunction(JsonString(obj, "dstRGB", "zero"), dstRGB) ||
		    !ParseBlendFunction(JsonString(obj, "dstA", "zero"), dstA))
			{ std::cerr << "Bad blend function" << std::endl; return false; }
		builder.customBlendFunctions(srcRGB, srcA, dstRGB, dstA);
		return true;
	}

	static bool processVertexDomain(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		VertexDomain d;
		if (!ParseVertexDomain(s->getString(), d)) { std::cerr << "Unknown vertexDomain: " << s->getString() << std::endl; return false; }
		builder.vertexDomain(d); return true;
	}

	static bool processCulling(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		RHI::CullingMode m;
		if (!ParseCulling(s->getString(), m)) { std::cerr << "Unknown culling: " << s->getString() << std::endl; return false; }
		builder.culling(m); return true;
	}

	static bool processBool(MaterialBuilder& (MaterialBuilder::*setter)(bool), const JsonishValue& value)
	{
		auto* b = value.toJsonBool(); if (!b) return false;
		return true;
	}

	#define BOOL_PROCESSOR(name, setter) \
		static bool name(MaterialBuilder& b, const JsonishValue& v) { auto* p = v.toJsonBool(); if (!p) return false; b.setter(p->getBool()); return true; }

	BOOL_PROCESSOR(processColorWrite, colorWrite)
	BOOL_PROCESSOR(processDepthWrite, depthWrite)
	BOOL_PROCESSOR(processDepthCull,  depthCulling)
	BOOL_PROCESSOR(processDoubleSided, doubleSided)
	BOOL_PROCESSOR(processVertexDomainDeviceJittered, vertexDomainDeviceJittered)

	static bool processTransparencyMode(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		BlendingMode m;
		if (!ParseBlending(s->getString(), m)) { std::cerr << "Unknown transparency: " << s->getString() << std::endl; return false; }
		builder.blending(m); return true;
	}

	static bool processShading(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		Shading m;
		if (!ParseShading(s->getString(), m)) { std::cerr << "Unknown shading: " << s->getString() << std::endl; return false; }
		builder.shading(m); return true;
	}

	static bool processDomain(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		MaterialDomain d;
		if (!ParseDomain(s->getString(), d)) { std::cerr << "Unknown domain: " << s->getString() << std::endl; return false; }
		builder.materialDomain(d); return true;
	}

	static bool processPipeline(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* s = value.toJsonString(); if (!s) return false;
		Pipeline p;
		if (!ParsePipeline(s->getString(), p)) { std::cerr << "Unknown pipeline: " << s->getString() << std::endl; return false; }
		builder.pipeline(p); return true;
	}

	static bool processOutputs(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* arr = value.toJsonArray(); if (!arr) return false;
		for (size_t i = 0; i < arr->getElements().size(); ++i)
		{
			auto* obj = arr->getElements()[i]->toJsonObject(); if (!obj) return false;
			const char* name = JsonString(obj, "name"); if (!name) return false;

			const char* targetStr = JsonString(obj, "target", "color");
			MaterialBuilder::OutputTarget target = MaterialBuilder::OutputTarget::COLOR;
			if (std::string(targetStr) == "depth") target = MaterialBuilder::OutputTarget::DEPTH;

			const char* typeStr = JsonString(obj, "type", nullptr);
			if (!typeStr) typeStr = JsonString(obj, "format", "float4");
			std::string fmt = typeStr;
			MaterialBuilder::OutputType otype = MaterialBuilder::OutputType::FLOAT4;
			if (fmt == "float") otype = MaterialBuilder::OutputType::FLOAT;
			else if (fmt == "float2") otype = MaterialBuilder::OutputType::FLOAT2;
			else if (fmt == "float3") otype = MaterialBuilder::OutputType::FLOAT3;
			else if (fmt == "float4") otype = MaterialBuilder::OutputType::FLOAT4;
			else if (fmt == "color") target = MaterialBuilder::OutputTarget::COLOR;
			else if (fmt == "depth") { target = MaterialBuilder::OutputTarget::DEPTH; otype = MaterialBuilder::OutputType::FLOAT; }
			else { std::cerr << "Unknown output format: " << fmt << std::endl; return false; }

			int loc = -1;
			if (obj->hasKey("location")) { auto* lv = obj->getValue("location")->toJsonNumber(); if (lv) loc = (int)lv->getFloat(); }
			builder.output(MaterialBuilder::VariableQualifier::OUT, target, otype, name, loc);
		}
		return true;
	}

	static bool processGroupSizes(MaterialBuilder& builder, const JsonishValue& value)
	{
		auto* arr = value.toJsonArray(); if (!arr || arr->getElements().size() != 3) return false;
		glm::uvec3 g{1,1,1};
		for (size_t i = 0; i < 3; ++i) { auto* n = arr->getElements()[i]->toJsonNumber(); if (!n) return false; g[i] = (uint32_t)n->getFloat(); }
		builder.groupSize(g); return true;
	}
}

// =============================================================================
// ParametersProcessor
// =============================================================================

ParametersProcessor::ParametersProcessor()
{
	using T = JsonishValue::Type;
	mParameters["name"]            = { &processName,         T::STRING };
	mParameters["parameters"]      = { &processParameters,   T::ARRAY };
	mParameters["properties"]      = { &processParameters,   T::ARRAY }; // legacy
	mParameters["constants"]       = { &processConstants,    T::ARRAY };
	mParameters["variables"]       = { &processVariables,    T::ARRAY };
	mParameters["requires"]        = { &processRequires,     T::ARRAY };
	mParameters["require"]         = { &processRequires,     T::ARRAY }; // legacy
	mParameters["blending"]        = { &processBlending,     T::STRING };
	mParameters["blendFunction"]   = { &processBlendFunction,T::OBJECT };
	mParameters["vertexDomain"]    = { &processVertexDomain, T::STRING };
	mParameters["culling"]         = { &processCulling,      T::STRING };
	mParameters["colorWrite"]      = { &processColorWrite,   T::BOOL };
	mParameters["depthWrite"]      = { &processDepthWrite,   T::BOOL };
	mParameters["depthCulling"]    = { &processDepthCull,    T::BOOL };
	mParameters["doubleSided"]     = { &processDoubleSided,  T::BOOL };
	mParameters["transparency"]    = { &processTransparencyMode, T::STRING };
	mParameters["shadingModel"]    = { &processShading,      T::STRING };
	mParameters["domain"]          = { &processDomain,       T::STRING };
	mParameters["pipeline"]        = { &processPipeline,     T::STRING };
	mParameters["vertexDomainDeviceJittered"] = { &processVertexDomainDeviceJittered, T::BOOL };
	mParameters["outputs"]         = { &processOutputs,      T::ARRAY };
	mParameters["groupSize"]       = { &processGroupSizes,   T::ARRAY };
}

bool ParametersProcessor::process(MaterialBuilder& builder, const JsonishObject& jsonObject)
{
	for (const auto& entry : jsonObject.getEntries()) {
		const std::string& key = entry.first;
		const JsonishValue* field = entry.second;
		auto it = mParameters.find(key);
		if (it == mParameters.end()) { std::cerr << "Ignoring unknown key: \"" << key << "\"" << std::endl; continue; }
		if (it->second.rootAssert != field->getType()) {
			std::cerr << "Key \"" << key << "\": expected " << JsonishValue::typeToString(it->second.rootAssert)
			          << " got " << JsonishValue::typeToString(field->getType()) << std::endl;
			return false;
		}
		if (!it->second.callback(builder, *field)) { std::cerr << "Error processing key \"" << key << "\"" << std::endl; return false; }
	}
	return true;
}

bool ParametersProcessor::process(MaterialBuilder& builder, const std::string& key, const std::string& value)
{
	auto it = mParameters.find(key);
	if (it == mParameters.end()) { std::cerr << "Unknown key: \"" << key << "\"" << std::endl; return false; }

	std::unique_ptr<JsonishValue> var;
	switch (it->second.rootAssert) {
	case JsonishValue::Type::BOOL: {
		std::string lower; std::transform(value.begin(), value.end(), std::back_inserter(lower), ::tolower);
		var = std::make_unique<JsonishBool>(!(lower.empty() || lower == "false" || lower == "f" || lower == "0"));
		break;
	}
	case JsonishValue::Type::NUMBER: var = std::make_unique<JsonishNumber>(std::stof(value)); break;
	case JsonishValue::Type::STRING: var = std::make_unique<JsonishString>(value); break;
	default: std::cerr << "Unsupported inline type" << std::endl; return false;
	}
	return it->second.callback(builder, *var);
}
