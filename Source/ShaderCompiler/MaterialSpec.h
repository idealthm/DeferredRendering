#pragma once

#include <string>
#include <vector>

#include "Common/Material/MaterialCommon.h"
#include "Common/Material/MaterialTypes.h"

struct ConstantParam
{
    std::string type;
    std::string name;
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
    std::string type;
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

MaterialSpec ParseMaterialSpec(const std::string& jsonString);
