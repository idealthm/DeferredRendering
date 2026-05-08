#pragma once

#include <sstream>
#include <string>

struct MaterialSpec;

enum class ShaderStage
{
    Vertex,
    Fragment
};

enum class Pipeline
{
    Deferred,
    Forward
};

/**
 * Set the template directory from which template .vs/.fs files are loaded.
 * Must be called before GenerateVertexShader / GenerateFragmentShader.
 */
void SetTemplateDirectory(const std::string& dir);

/**
 * Set Vulkan target environment.
 * When true, TARGET_VULKAN_ENVIRONMENT is defined in generated shaders.
 * Must be called before GenerateVertexShader / GenerateFragmentShader.
 */
void SetTargetVulkan(bool vulkan);

/**
 * GenerateHeader produces the shared declarations for both vertex and fragment shaders:
 *   - #version 460
 *   - Uniform struct (std140) from uniform properties
 *   - Sampler uniforms from sampler properties
 *   - HAS_<PROPERTY> macros
 */
void GenerateHeader(std::ostringstream& os, const MaterialSpec& spec);

/**
 * GenerateMaterialInput loads the per-stage material input template
 * (surface_material_input.{vs,fs}) that defines the MaterialInputs struct
 * and its initialization function.
 */
void GenerateMaterialInput(std::ostringstream& os, const MaterialSpec& spec, ShaderStage stage);

/**
 * GenerateGetter loads the per-stage getter template
 * (surface_getters.{vs,fs}) that defines getPosition / getCustom* helpers.
 */
void GenerateGetter(std::ostringstream& os, const MaterialSpec& spec, ShaderStage stage);

/**
 * GenerateMain loads the per-stage main template
 * (surface_main.{vs,fs}) that defines the main() entry point.
 */
void GenerateMain(std::ostringstream& os, const MaterialSpec& spec, ShaderStage stage);

/**
 * GenerateShadingModel loads the shading-model-specific template
 * (surface_shading_<shadingModel>.fs) that defines evaluateMaterial().
 * Only meaningful for the fragment stage (surface domain).
 */
void GenerateShadingModel(std::ostringstream& os, const MaterialSpec& spec);

/**
 * GenerateVertexShader produces a complete GLSL vertex shader from the material spec.
 * The spec.vertexCode must already be include-expanded.
 */
void GenerateVertexShader(std::ostringstream& os, const MaterialSpec& spec, const std::string& expandedVertexCode);

/**
 * GenerateFragmentShader produces a complete GLSL fragment shader from the material spec.
 * The spec.fragmentCode must already be include-expanded.
 */
void GenerateFragmentShader(std::ostringstream& os, const MaterialSpec& spec, const std::string& expandedFragmentCode);

