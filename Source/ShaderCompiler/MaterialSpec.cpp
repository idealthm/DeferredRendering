#include "MaterialSpec.h"
#include "GLSLGenerator.h"

#include <stdexcept>
#include <unordered_map>
#include <sstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

static VertexAttribute ParseVertexAttribute(const std::string& s)
{
    static const std::unordered_map<std::string, VertexAttribute> map = {
        {"POSITION",     VertexAttribute::POSITION},
        {"TANGENTS",     VertexAttribute::TANGENTS},
        {"COLOR",        VertexAttribute::COLOR},
        {"UV0",          VertexAttribute::UV0},
        {"UV1",          VertexAttribute::UV1},
        {"BONE_INDICES", VertexAttribute::BONE_INDICES},
        {"BONE_WEIGHTS", VertexAttribute::BONE_WEIGHTS},
        {"CUSTOM0",      VertexAttribute::CUSTOM0},
        {"CUSTOM1",      VertexAttribute::CUSTOM1},
        {"CUSTOM2",      VertexAttribute::CUSTOM2},
        {"CUSTOM3",      VertexAttribute::CUSTOM3},
        {"CUSTOM4",      VertexAttribute::CUSTOM4},
        {"CUSTOM5",      VertexAttribute::CUSTOM5},
        {"CUSTOM6",      VertexAttribute::CUSTOM6},
        {"CUSTOM7",      VertexAttribute::CUSTOM7},
    };
    auto it = map.find(s);
    if (it == map.end())
    {
        std::ostringstream oss;
        oss << "unknown VertexAttribute '" << s << "'. Valid: POSITION, TANGENTS, COLOR, "
            << "UV0, UV1, BONE_INDICES, BONE_WEIGHTS, CUSTOM0..CUSTOM7";
        throw std::runtime_error(oss.str());
    }
    return it->second;
}

// Forward declaration
static MaterialSpec ParseMaterialSpecImpl(const json& j);

MaterialSpec ParseMaterialSpec(const std::string& jsonString)
{
    json j = json::parse(jsonString);
    return ParseMaterialSpecImpl(j);
}

static MaterialSpec ParseMaterialSpecImpl(const json& j)
{
    MaterialSpec spec;

    // --- name (required) ---
    if (!j.contains("name"))
        throw std::runtime_error("missing required field 'name'");
    spec.name = j["name"].get<std::string>();

    // --- pipeline (optional, defaults to "deferred") ---
    {
        std::string pipelineStr = j.value("pipeline", "deferred");
        if (pipelineStr == "deferred")
            spec.pipeline = Pipeline::Deferred;
        else if (pipelineStr == "forward")
            spec.pipeline = Pipeline::Forward;
        else
            throw std::runtime_error("material '" + spec.name + "': invalid pipeline '" +
                pipelineStr + "'. Valid: deferred, forward");
    }

    // --- shadingModel (optional, defaults to "unlit") ---
    spec.shadingModel = j.value("shadingModel", "unlit");

    // --- domain (required) ---
    if (!j.contains("domain"))
        throw std::runtime_error("material '" + spec.name + "': missing required field 'domain'");
    spec.domain = j["domain"].get<std::string>();
    if (spec.domain != "surface" && spec.domain != "postprocess" && spec.domain != "compute")
        throw std::runtime_error("material '" + spec.name + "': invalid domain '" + spec.domain +
            "'. Valid: surface, postprocess, compute");

    // --- require (optional) ---
    if (j.contains("require"))
    {
        const auto& req = j["require"];
        if (!req.is_array())
            throw std::runtime_error("material '" + spec.name + "': 'require' must be an array");

        for (size_t i = 0; i < req.size(); ++i)
        {
            if (!req[i].is_string())
                throw std::runtime_error("material '" + spec.name +
                    "': require[" + std::to_string(i) + "] must be a string");
            try
            {
                spec.requiredAttributes.push_back(ParseVertexAttribute(req[i].get<std::string>()));
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("material '" + spec.name +
                    "': require[" + std::to_string(i) + "]: " + e.what());
            }
        }
    }

    // --- constant (optional) ---
    if (j.contains("constant"))
    {
        const auto& arr = j["constant"];
        if (!arr.is_array())
            throw std::runtime_error("material '" + spec.name + "': 'constant' must be an array");

        for (size_t i = 0; i < arr.size(); ++i)
        {
            const auto& c = arr[i];
            if (!c.contains("type") || !c.contains("name"))
                throw std::runtime_error("material '" + spec.name +
                    "': constant[" + std::to_string(i) + "] must have 'type' and 'name'");
            ConstantParam cp;
            cp.type = c["type"].get<std::string>();
            cp.name = c["name"].get<std::string>();
            spec.constants.push_back(std::move(cp));
        }
    }

    // --- variable (optional) ---
    if (j.contains("variables"))
    {
        const auto& arr = j["variables"];
        if (!arr.is_array())
            throw std::runtime_error("material '" + spec.name + "': 'variables' must be an array");

        for (size_t i = 0; i < arr.size(); ++i)
        {
            const auto& v = arr[i];
            spec.variables.push_back({ v.get<std::string>() });
        }
    }

    // --- property (optional) ---
    if (j.contains("properties"))
    {
        const auto& arr = j["properties"];
        if (!arr.is_array())
            throw std::runtime_error("material '" + spec.name + "': 'property' must be an array");

        for (size_t i = 0; i < arr.size(); ++i)
        {
            const auto& p = arr[i];
            if (!p.contains("type") || !p.contains("name"))
                throw std::runtime_error("material '" + spec.name +
                    "': property[" + std::to_string(i) + "] must have 'type' and 'name'");

            std::string typeStr = p["type"].get<std::string>();
            std::string propName = p["name"].get<std::string>();

            PropertyParam pp;
            pp.name = propName;

            if (IsValidSamplerType(typeStr))
            {
                pp.kind = PropertyParam::Kind::Sampler;
                pp.samplerType = ParseSamplerType(typeStr);
            }
            else if (IsValidUniformType(typeStr))
            {
                pp.kind = PropertyParam::Kind::Uniform;
                pp.uniformType = ParseUniformType(typeStr);
                if (pp.uniformType == UniformType::STRUCT)
                    pp.structName = p.value("structName", "");
            }
            else
            {
                throw std::runtime_error("material '" + spec.name +
                    "': property[" + std::to_string(i) + "]: unknown type '" + typeStr +
                    "'. Valid uniform types: BOOL, BOOL2..4, FLOAT, FLOAT2..4, INT, INT2..4, "
                    "UINT, UINT2..4, MAT3, MAT4, STRUCT. "
                    "Valid sampler types: SAMPLER_2D, SAMPLER_2D_ARRAY, SAMPLER_CUBEMAP, SAMPLER_3D, SAMPLER_CUBEMAP_ARRAY");
            }

            spec.properties.push_back(std::move(pp));
        }
    }

    // --- output (optional) ---
    if (j.contains("outputs"))
    {
        const auto& arr = j["outputs"];
        if (!arr.is_array())
            throw std::runtime_error("material '" + spec.name + "': 'output' must be an array");

        for (size_t i = 0; i < arr.size(); ++i)
        {
            const auto& o = arr[i];
            if (!o.contains("type") || !o.contains("name"))
                throw std::runtime_error("material '" + spec.name +
                    "': output[" + std::to_string(i) + "] must have 'type' and 'name'");

            OutputParam op;
            op.type = o["type"].get<std::string>();
            op.name = o["name"].get<std::string>();

            if (op.type != "color" && op.type != "depth")
                throw std::runtime_error("material '" + spec.name +
                    "': output[" + std::to_string(i) + "]: invalid type '" + op.type +
                    "'. Valid: color, depth");

            spec.outputs.push_back(std::move(op));
        }
    }

    // --- vertexCode (optional) ---
    spec.vertexCode = j.value("vertexCode", "");

    // --- fragmentCode (optional) ---
    spec.fragmentCode = j.value("fragmentCode", "");

    return spec;
}
