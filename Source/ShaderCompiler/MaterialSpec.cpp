#include "MaterialSpec.h"
#include "MaterialEnums.h"

#include <stdexcept>
#include <unordered_map>
#include <sstream>

#include "Lexer/JsonishLexer.h"
#include "Lexer/MaterialLexer.h"
#include "Parser/JsonishParser.h"

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

static std::string JsonString(const JsonishObject* obj, const char* key, const char* defaultVal = nullptr)
{
    if (!obj->hasKey(key))
    {
        if (defaultVal) return defaultVal;
        throw std::runtime_error(std::string("missing required field '") + key + "'");
    }
    auto* v = obj->getValue(key)->toJsonString();
    if (!v) throw std::runtime_error(std::string("field '") + key + "' must be a string");
    return v->getString();
}

// Forward declaration
static MaterialSpec ParseMaterialSpecImpl(const JsonishObject* j);

using BlockMap = std::unordered_map<std::string, std::string>;

static BlockMap ParseTopLevelBlocks(const std::string& source)
{
    MaterialLexer lexer;
    lexer.Lex(source.c_str(), source.size());
    auto& lexemes = lexer.getLexemes();

    BlockMap blocks;

    for (size_t i = 0; i + 1 < lexemes.size(); )
    {
        if (lexemes[i].getType() != IDENTIFIER) { ++i; continue; }
        if (lexemes[i + 1].getType() != BLOCK)  { ++i; continue; }

        std::string key = lexemes[i].getStringValue();
        auto trimmed = lexemes[i + 1].trimBlockMarkers();
        blocks[key] = trimmed.getStringValue();
        i += 2;
    }

    return blocks;
}

MaterialSpec ParseMaterialSpec(const std::string& source)
{
    auto blocks = ParseTopLevelBlocks(source);

    MaterialSpec spec;

    // --- material block → JsonishParser ---
    auto matIt = blocks.find("material");
    if (matIt != blocks.end())
    {
        std::string wrapped = "{" + matIt->second + "}";
        JsonishLexer lexer;
        lexer.Lex(wrapped.c_str(), wrapped.size());
        JsonishParser parser(lexer.getLexemes());
        auto root = parser.parse();
        if (root) spec = ParseMaterialSpecImpl(root.get());
    }

    // --- code blocks: raw GLSL ---
    auto vcIt = blocks.find("vertexCode");
    if (vcIt != blocks.end()) spec.vertexCode = vcIt->second;

    auto fcIt = blocks.find("fragmentCode");
    if (fcIt != blocks.end()) spec.fragmentCode = fcIt->second;

    return spec;
}

static MaterialSpec ParseMaterialSpecImpl(const JsonishObject* j)
{
    MaterialSpec spec;

    // --- name (required) ---
    spec.name = JsonString(j, "name");

    // --- pipeline (optional, defaults to "deferred") ---
    {
        std::string pipelineStr = j->hasKey("pipeline") ? JsonString(j, "pipeline") : "deferred";
        if (pipelineStr == "deferred")
            spec.pipeline = Pipeline::DEFERRED;
        else if (pipelineStr == "forward")
            spec.pipeline = Pipeline::FORWARD;
        else if (pipelineStr == "lighting")
            spec.pipeline = Pipeline::LIGHTING;
        else
            throw std::runtime_error("material '" + spec.name + "': invalid pipeline '" +
                pipelineStr + "'. Valid: deferred, forward, lighting");
    }

    // --- shadingModel (optional, defaults to "unlit") ---
    spec.shadingModel = j->hasKey("shadingModel") ? JsonString(j, "shadingModel") : "unlit";

    // --- domain (required) ---
    {
        std::string domainStr = JsonString(j, "domain");
        if (domainStr == "surface")
            spec.domain = MaterialDomain::SURFACE;
        else if (domainStr == "postprocess")
            spec.domain = MaterialDomain::POST_PROCESS;
        else if (domainStr == "compute")
            spec.domain = MaterialDomain::COMPUTE;
        else
            throw std::runtime_error("material '" + spec.name + "': invalid domain '" + domainStr +
                "'. Valid: surface, postprocess, compute");
    }

    // --- require (optional) ---
    if (j->hasKey("require"))
    {
        auto* v = j->getValue("require");
        auto* arr = v ? v->toJsonArray() : nullptr;
        if (!arr)
            throw std::runtime_error("material '" + spec.name + "': 'require' must be an array");

        for (size_t i = 0; i < arr->getElements().size(); ++i)
        {
            auto* str = arr->getElements()[i]->toJsonString();
            if (!str)
                throw std::runtime_error("material '" + spec.name +
                    "': require[" + std::to_string(i) + "] must be a string");
            spec.requiredAttributes.push_back(ParseVertexAttribute(str->getString()));
        }
    }

    // --- constant (optional) ---
    if (j->hasKey("constant"))
    {
        auto* v = j->getValue("constant");
        auto* arr = v ? v->toJsonArray() : nullptr;
        if (!arr)
            throw std::runtime_error("material '" + spec.name + "': 'constant' must be an array");

        for (size_t i = 0; i < arr->getElements().size(); ++i)
        {
            auto* obj = arr->getElements()[i]->toJsonObject();
            if (!obj || !obj->hasKey("type") || !obj->hasKey("name"))
                throw std::runtime_error("material '" + spec.name +
                    "': constant[" + std::to_string(i) + "] must have 'type' and 'name'");
            ConstantParam cp;
            cp.type = JsonString(obj, "type");
            cp.name = JsonString(obj, "name");
            spec.constants.push_back(std::move(cp));
        }
    }

    // --- variable (optional) ---
    if (j->hasKey("variables"))
    {
        auto* v = j->getValue("variables");
        auto* arr = v ? v->toJsonArray() : nullptr;
        if (!arr)
            throw std::runtime_error("material '" + spec.name + "': 'variables' must be an array");

        for (size_t i = 0; i < arr->getElements().size(); ++i)
        {
            auto* str = arr->getElements()[i]->toJsonString();
            if (!str)
                throw std::runtime_error("material '" + spec.name +
                    "': variables[" + std::to_string(i) + "] must be a string");
            spec.variables.push_back({ str->getString() });
        }
    }

    // --- property (optional) ---
    if (j->hasKey("properties"))
    {
        auto* v = j->getValue("properties");
        auto* arr = v ? v->toJsonArray() : nullptr;
        if (!arr)
            throw std::runtime_error("material '" + spec.name + "': 'properties' must be an array");

        for (size_t i = 0; i < arr->getElements().size(); ++i)
        {
            auto* p = arr->getElements()[i]->toJsonObject();
            if (!p || !p->hasKey("type") || !p->hasKey("name"))
                throw std::runtime_error("material '" + spec.name +
                    "': property[" + std::to_string(i) + "] must have 'type' and 'name'");

            std::string typeStr = JsonString(p, "type");
            std::string propName = JsonString(p, "name");

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
                    pp.structName = p->hasKey("structName") ? JsonString(p, "structName") : "";
            }
            else
            {
                throw std::runtime_error("material '" + spec.name +
                    "': property[" + std::to_string(i) + "]: unknown type '" + typeStr +
                    "'. Valid uniform types: BOOL, BOOL2..4, FLOAT, FLOAT2..4, INT, INT2..4, "
                    "UINT, UINT2..4, MAT3, MAT4, STRUCT. "
                    "Valid sampler types: SAMPLER_2D, SAMPLER_2D_ARRAY, SAMPLER_CUBEMAP, "
                    "SAMPLER_3D, SAMPLER_CUBEMAP_ARRAY");
            }

            spec.properties.push_back(std::move(pp));
        }
    }

    // --- output (optional) ---
    if (j->hasKey("outputs"))
    {
        auto* v = j->getValue("outputs");
        auto* arr = v ? v->toJsonArray() : nullptr;
        if (!arr)
            throw std::runtime_error("material '" + spec.name + "': 'outputs' must be an array");

        for (size_t i = 0; i < arr->getElements().size(); ++i)
        {
            auto* o = arr->getElements()[i]->toJsonObject();
            if (!o || !o->hasKey("type") || !o->hasKey("name"))
                throw std::runtime_error("material '" + spec.name +
                    "': output[" + std::to_string(i) + "] must have 'type' and 'name'");

            OutputParam op;
            op.type = JsonString(o, "type");
            op.name = JsonString(o, "name");

            if (op.type != "color" && op.type != "depth")
                throw std::runtime_error("material '" + spec.name +
                    "': output[" + std::to_string(i) + "]: invalid type '" + op.type +
                    "'. Valid: color, depth");

            spec.outputs.push_back(std::move(op));
        }
    }

    return spec;
}
