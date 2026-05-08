#pragma once

#include <string>
#include <vector>

#include "Material/MaterialEnums.h"
#include "ShaderCompiler/MaterialEnums.h"

struct ConstantParam
{
    std::string type;   // "float", "int", "bool", "vec3", etc.
    std::string name;
};

struct VariableParam
{
    std::string name;
};

struct PropertyParam
{
    enum class Kind : uint8_t { Uniform, Sampler };

    Kind kind;
    UniformType uniformType;   // valid when kind == Uniform
    SamplerType samplerType;   // valid when kind == Sampler
    std::string name;
    std::string structName;    // only when uniformType == STRUCT
};

struct OutputParam
{
    std::string type;   // "color" or "depth"
    std::string name;
};

struct MaterialSpec
{
    std::string name;
    std::string shadingModel;                   // "unlit" | "lit" | ...
    std::string domain;                         // "surface" | "postprocess" | "compute"
    std::vector<VertexAttribute> requiredAttributes;
    std::vector<ConstantParam> constants;
    std::vector<VariableParam> variables;
    std::vector<PropertyParam> properties;
    std::vector<OutputParam> outputs;
    std::string vertexCode;
    std::string fragmentCode;
};

/**
 * Parse a MaterialSpec from a JSON string.
 * Throws std::runtime_error on malformed JSON or invalid material spec.
 */
MaterialSpec ParseMaterialSpec(const std::string& jsonString);
