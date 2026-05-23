#pragma once

#include <string>
#include <vector>

#include "Common/Material/MaterialCommon.h"

enum class FieldType : uint8_t;

struct ConstantParam
{
    std::string type;   // "float", "int", "bool", "vec3", etc.
    std::string name;
};

struct VariableParam
{
    std::string name;
    FieldType varType;
    uint8_t location = 0;
};

struct PropertyParam
{
    enum class Kind : uint8_t { Uniform, Sampler };

    Kind kind;
    UniformType uniformType;
    RHI::SamplerType samplerType;
    std::string name;
    std::string structName;
};

struct OutputParam
{
    std::string type;   // "color" or "depth"
    std::string name;
};

struct MaterialSpec
{
    std::string name;
    Pipeline pipeline;
    std::string shadingModel;
    MaterialDomain domain;
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
