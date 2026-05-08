#include "GLSLGenerator.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "MaterialSpec.h"

// ============================================================
// Template directory
// ============================================================
static std::string s_templateDir = "Template";
static bool s_targetVulkan = true;

void SetTemplateDirectory(const std::string& dir)
{
    s_templateDir = dir;
}

void SetTargetVulkan(bool vulkan)
{
    s_targetVulkan = vulkan;
}

static std::string LoadTemplate(const std::string& name)
{
    std::string path = s_templateDir + "/" + name;
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
        throw std::runtime_error("cannot open template file: " + path);

    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    // Strip UTF-8 BOM
    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        content.erase(0, 3);
    }

    // Normalize line endings: \r\n -> \n
    size_t pos = 0;
    while ((pos = content.find("\r\n", pos)) != std::string::npos)
        content.erase(pos, 1);

    return content;
}

static const char* StageSuffix(ShaderStage stage)
{
    return stage == ShaderStage::Vertex ? "vs" : "fs";
}

// ============================================================
// EmitDefine — centralized macro emission
// ============================================================
static void EmitDefine(std::ostringstream& os, const char* name)
{
    os << "#define " << name << "\n";
}

static void EmitDefine(std::ostringstream& os, const char* name, const char* value)
{
    os << "#define " << name << " " << value << "\n";
}

// ============================================================
// VertexAttribute -> GLSL mapping
// ============================================================
static const char* VertexAttributeToGLSLType(VertexAttribute attr)
{
    switch (attr)
    {
    case VertexAttribute::POSITION:      return "vec4";
    case VertexAttribute::TANGENTS:      return "vec4";
    case VertexAttribute::COLOR:         return "vec4";
    case VertexAttribute::UV0:           return "vec2";
    case VertexAttribute::UV1:           return "vec2";
    case VertexAttribute::BONE_INDICES:  return "uvec4";
    case VertexAttribute::BONE_WEIGHTS:  return "vec4";
    default:                             return "vec4";  // CUSTOM0..7
    }
}

static const char* VertexAttributeToName(VertexAttribute attr)
{
    switch (attr)
    {
    case VertexAttribute::POSITION:      return "mesh_position";
    case VertexAttribute::TANGENTS:      return "mesh_tangents";
    case VertexAttribute::COLOR:         return "mesh_color";
    case VertexAttribute::UV0:           return "mesh_uv0";
    case VertexAttribute::UV1:           return "mesh_uv1";
    case VertexAttribute::BONE_INDICES:  return "mesh_bone_indices";
    case VertexAttribute::BONE_WEIGHTS:  return "mesh_bone_weights";
    case VertexAttribute::CUSTOM0:       return "mesh_custom0";
    case VertexAttribute::CUSTOM1:       return "mesh_custom1";
    case VertexAttribute::CUSTOM2:       return "mesh_custom2";
    case VertexAttribute::CUSTOM3:       return "mesh_custom3";
    case VertexAttribute::CUSTOM4:       return "mesh_custom4";
    case VertexAttribute::CUSTOM5:       return "mesh_custom5";
    case VertexAttribute::CUSTOM6:       return "mesh_custom6";
    case VertexAttribute::CUSTOM7:       return "mesh_custom7";
    default:                             return "mesh_custom";
    }
}

static const char* VertexAttributeToMacroName(VertexAttribute attr)
{
    switch (attr)
    {
    case VertexAttribute::POSITION:      return "POSITION";
    case VertexAttribute::TANGENTS:      return "TANGENTS";
    case VertexAttribute::COLOR:         return "COLOR";
    case VertexAttribute::UV0:           return "UV0";
    case VertexAttribute::UV1:           return "UV1";
    case VertexAttribute::BONE_INDICES:  return "BONE_INDICES";
    case VertexAttribute::BONE_WEIGHTS:  return "BONE_WEIGHTS";
    case VertexAttribute::CUSTOM0:       return "CUSTOM0";
    case VertexAttribute::CUSTOM1:       return "CUSTOM1";
    case VertexAttribute::CUSTOM2:       return "CUSTOM2";
    case VertexAttribute::CUSTOM3:       return "CUSTOM3";
    case VertexAttribute::CUSTOM4:       return "CUSTOM4";
    case VertexAttribute::CUSTOM5:       return "CUSTOM5";
    case VertexAttribute::CUSTOM6:       return "CUSTOM6";
    case VertexAttribute::CUSTOM7:       return "CUSTOM7";
    default:                             return "UNKNOWN";
    }
}

static int VertexAttributeToLocation(VertexAttribute attr)
{
    return static_cast<int>(attr);
}

static std::string UpperCase(const std::string& s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return (char)std::toupper(c); });
    return result;
}

// ============================================================
// Shared header generation
// ============================================================
void GenerateHeader(std::ostringstream& os, const MaterialSpec& spec)
{
    os << "// =====================================================\n";
    os << "// Auto-generated by matc -- Material: " << spec.name << "\n";
    os << "// Domain: " << spec.domain << "\n";
    os << "// Shading model: " << spec.shadingModel << "\n";
    os << "// =====================================================\n\n";

    os << "#version 460\n\n";

    // --- Uniform block (std140) from uniform-type properties ---
    bool hasUniforms = false;
    for (const auto& p : spec.properties)
    {
        if (p.kind == PropertyParam::Kind::Uniform)
        {
            hasUniforms = true;
            break;
        }
    }

    if (hasUniforms)
    {
        os << "// Material uniform block\n";
        os << "layout(std140, binding = 0) uniform MaterialParams\n";
        os << "{\n";
        for (const auto& p : spec.properties)
        {
            if (p.kind != PropertyParam::Kind::Uniform)
                continue;
            if (p.uniformType == UniformType::STRUCT)
                os << "    " << p.structName << " " << p.name << ";\n";
            else
                os << "    " << UniformTypeToGLSL(p.uniformType) << " " << p.name << ";\n";
        }
        os << "} materialParams;\n\n";
    }

    // --- Sampler uniforms (materialParams_<name>) ---
    int samplerBinding = 0;
    for (const auto& p : spec.properties)
    {
        if (p.kind != PropertyParam::Kind::Sampler)
            continue;
        os << "layout(binding = " << samplerBinding++ << ") uniform "
           << SamplerTypeToGLSL(p.samplerType) << " materialParams_" << p.name << ";\n";
    }
    if (samplerBinding > 0)
        os << '\n';

    // --- Feature macros ---
    os << "// Feature macros for conditional compilation\n";

    // Variable macros + name aliases
    //   VARIABLE_CUSTOM<i>     → expands to user name, also enables #ifdef checks
    //   VARIABLE_CUSTOM_AT<i>  → expands to variable_<name> (the varying)
    for (size_t i = 0; i < spec.variables.size(); ++i)
    {
        const auto& v = spec.variables[i];
        std::string upper = UpperCase(v.name);
        EmitDefine(os, ("HAS_VARIABLE_" + upper).c_str());
        EmitDefine(os, ("VARIABLE_CUSTOM" + std::to_string(i)).c_str(), v.name.c_str());
        EmitDefine(os, ("VARIABLE_CUSTOM_AT" + std::to_string(i)).c_str(),
                   ("variable_" + v.name).c_str());
    }

    for (const auto& c : spec.constants)
        EmitDefine(os, ("CONST_" + UpperCase(c.name)).c_str(), c.name.c_str());

    // HAS_ATTRIBUTE_<NAME> for required attributes
    for (VertexAttribute attr : spec.requiredAttributes)
        EmitDefine(os, ("HAS_ATTRIBUTE_" + std::string(VertexAttributeToMacroName(attr))).c_str());

    // HAS_ATTRIBUTE_POSITION is implied for surface domain
    if (spec.domain == "surface")
        EmitDefine(os, "HAS_ATTRIBUTE_POSITION");

    os << '\n';
}

// ============================================================
// Common defines (type aliases from Filament)
// ============================================================
static void GenerateCommonDefines(std::ostringstream& os)
{
    os << "// =====================================================\n";
    os << "// Template: common_defines.glsl\n";
    os << "// =====================================================\n";
    os << LoadTemplate("common_defines.glsl") << '\n';
}

// ============================================================
// Builtin stubs (Filament compatibility layer)
// ============================================================
static void GenerateBuiltins(std::ostringstream& os)
{
    os << "// =====================================================\n";
    os << "// Template: surface_builtins.glsl\n";
    os << "// =====================================================\n";
    os << LoadTemplate("surface_builtins.glsl") << '\n';
}

// ============================================================
// Surface varying declarations (vertex_* varyings)
// ============================================================
static void GenerateSurfaceVarying(std::ostringstream& os, ShaderStage stage)
{
    os << "// =====================================================\n";
    os << "// Template: surface_varying.glsl\n";
    os << "// =====================================================\n";
    // VARYING maps to 'out' for vertex stage, 'in' for fragment stage
    if (stage == ShaderStage::Vertex)
        EmitDefine(os, "VARYING", "out");
    else
        EmitDefine(os, "VARYING", "in");
    os << LoadTemplate("surface_varying.glsl") << '\n';
}

// ============================================================
// Template-based generators
// ============================================================
void GenerateMaterialInput(std::ostringstream& os, const MaterialSpec& spec, ShaderStage stage)
{
    std::string file = std::string("surface_material_input.") + StageSuffix(stage);
    os << "// =====================================================\n";
    os << "// Template: " << file << "\n";
    os << "// =====================================================\n";
    os << LoadTemplate(file) << '\n';
}

void GenerateGetter(std::ostringstream& os, const MaterialSpec& spec, ShaderStage stage)
{
    std::string file = std::string("surface_getters.") + StageSuffix(stage);
    std::string content = LoadTemplate(file);
    if (content.empty())
        return;
    os << "// =====================================================\n";
    os << "// Template: " << file << "\n";
    os << "// =====================================================\n";
    os << content << '\n';
}

void GenerateMain(std::ostringstream& os, const MaterialSpec& spec, ShaderStage stage)
{
    std::string file = std::string("surface_main.") + StageSuffix(stage);
    os << "// =====================================================\n";
    os << "// Template: " << file << "\n";
    os << "// =====================================================\n";
    os << LoadTemplate(file) << '\n';
}

void GenerateShadingModel(std::ostringstream& os, const MaterialSpec& spec)
{
    std::string file = "surface_shading_" + spec.shadingModel + ".fs";
    os << "// =====================================================\n";
    os << "// Template: " << file << " (shading model: " << spec.shadingModel << ")\n";
    os << "// =====================================================\n";
    os << LoadTemplate(file) << '\n';
}

// ============================================================
// Vertex shader generation
// ============================================================

// clang-format off
// Assembly order:
//   1. Header        (version, uniforms, macros)
//   2. common_defines (type aliases: float2→vec2, LAYOUT_LOCATION, etc.)
//   3. TARGET_VULKAN_ENVIRONMENT / VERTEX_STAGE
//   4. Vertex input attributes (mesh_*) — must precede builtins/getters that reference them
//   5. surface_builtins (Filament compatibility stubs)
//   6. surface_varying (VARYING=out, common vertex_* varyings)
//   7. surface_material_input.vs (MaterialVertexInputs struct)
//   8. surface_getters.vs (getPosition / getCustom*)
//   9. Custom varying outputs (VARIABLE_CUSTOM_AT<i>)
//  10. User vertex code (materialVertex)
//  11. surface_main.vs (main entry point)
// ============================================================
// clang-format on
void GenerateVertexShader(std::ostringstream& os, const MaterialSpec& spec, const std::string& expandedVertexCode)
{
    // 1. Shared header
    GenerateHeader(os, spec);

    // Additional vertex-only macros
    if (spec.domain == "surface")
        EmitDefine(os, "VERTEX_DOMAIN_DEVICE");

    os << '\n';

    // 2. Common defines (type aliases)
    GenerateCommonDefines(os);

    // Vulkan target environment (before templates that check it)
    if (s_targetVulkan)
        EmitDefine(os, "TARGET_VULKAN_ENVIRONMENT");

    EmitDefine(os, "VERTEX_STAGE");
    os << '\n';

    // 3. Vertex input attributes (mesh_* names) — before builtins/getters that reference them
    {
        os << "// Vertex input attributes\n";

        if (spec.domain == "surface")
            os << "layout(location = 0) in vec4 mesh_position;\n";

        for (VertexAttribute attr : spec.requiredAttributes)
        {
            if (spec.domain == "surface" && attr == VertexAttribute::POSITION)
                continue;
            os << "layout(location = " << VertexAttributeToLocation(attr) << ") in "
               << VertexAttributeToGLSLType(attr) << " " << VertexAttributeToName(attr) << ";\n";
        }
        os << '\n';
    }

    // 4. Builtin stubs (Filament compatibility) — after vertex inputs
    GenerateBuiltins(os);

    // 5. Surface varying declarations (VARYING=out)
    GenerateSurfaceVarying(os, ShaderStage::Vertex);

    // 6. Material input struct + init (surface_material_input.vs)
    GenerateMaterialInput(os, spec, ShaderStage::Vertex);

    // 7. Getter functions (surface_getters.vs)
    GenerateGetter(os, spec, ShaderStage::Vertex);

    // 8. Custom varying outputs (VARIABLE_CUSTOM_AT<i>, start at location 10)
    if (!spec.variables.empty())
    {
        os << "// Custom varying outputs\n";
        for (size_t i = 0; i < spec.variables.size(); ++i)
            os << "LAYOUT_LOCATION(" << (10 + i) << ") out vec4 VARIABLE_CUSTOM_AT" << i << ";\n";
        os << '\n';
    }

    // 9. User vertex code (materialVertex function)
    if (!expandedVertexCode.empty())
    {
        os << "// =====================================================\n";
        os << "// User vertex code\n";
        os << "// =====================================================\n";
        os << expandedVertexCode << '\n';
    }

    // 10. Main entry point (surface_main.vs)
    GenerateMain(os, spec, ShaderStage::Vertex);
}

// ============================================================
// Fragment shader generation
// ============================================================

// clang-format off
// Assembly order:
//   1. Header        (version, uniforms, macros)
//   2. common_defines (type aliases)
//   3. TARGET_VULKAN_ENVIRONMENT / FRAGMENT_STAGE
//   4. surface_varying (VARYING=in, common vertex_* varyings)
//   5. surface_material_input.fs (MaterialInputs struct — must precede builtins)
//   6. surface_builtins (Filament compatibility stubs — prepareMaterial needs MaterialInputs)
//   7. surface_getters.fs (getColor / getUV0 / getUV1)
//   8. Custom varying inputs (VARIABLE_CUSTOM_AT<i>)
//   9. Output declarations (if any beyond surface_main.fs's fragColor)
//  10. User fragment code (material)
//  11. surface_shading_<model>.fs (evaluateMaterial — must precede main)
//  12. surface_main.fs (main entry point, calls evaluateMaterial)
// ============================================================
// clang-format on
void GenerateFragmentShader(std::ostringstream& os, const MaterialSpec& spec, const std::string& expandedFragmentCode)
{
    // 1. Shared header
    GenerateHeader(os, spec);

    // 2. Common defines (type aliases)
    GenerateCommonDefines(os);

    // Vulkan target environment (before templates that check it)
    if (s_targetVulkan)
        EmitDefine(os, "TARGET_VULKAN_ENVIRONMENT");

    EmitDefine(os, "FRAGMENT_STAGE");
    os << '\n';

    // 3. Surface varying declarations (VARYING=in)
    GenerateSurfaceVarying(os, ShaderStage::Fragment);

    // 4. Material input struct + init (surface_material_input.fs) — defines MaterialInputs
    GenerateMaterialInput(os, spec, ShaderStage::Fragment);

    // 5. Builtin stubs (Filament compatibility) — after MaterialInputs so prepareMaterial works
    GenerateBuiltins(os);

    // 6. Getter functions (surface_getters.fs)
    GenerateGetter(os, spec, ShaderStage::Fragment);

    // 6. Custom varying inputs (VARIABLE_CUSTOM_AT<i>, start at location 10)
    if (!spec.variables.empty())
    {
        os << "// Custom varying inputs\n";
        for (size_t i = 0; i < spec.variables.size(); ++i)
            os << "LAYOUT_LOCATION(" << (10 + i) << ") in vec4 VARIABLE_CUSTOM_AT" << i << ";\n";
        os << '\n';
    }

    // 7. Output declarations (if any, surface_main.fs already has fragColor)
    if (!spec.outputs.empty())
    {
        os << "// Fragment outputs\n";
        int colorIndex = 0;
        for (const auto& o : spec.outputs)
        {
            if (o.type == "color")
                os << "layout(location = " << colorIndex++ << ") out vec4 " << o.name << ";\n";
            else if (o.type == "depth")
                os << "layout(depth_any) out float " << o.name << ";\n";
        }
        os << '\n';
    }

    // 8. User fragment code (material function)
    if (!expandedFragmentCode.empty())
    {
        os << "// =====================================================\n";
        os << "// User fragment code\n";
        os << "// =====================================================\n";
        os << expandedFragmentCode << '\n';
    }

    // 9. Shading model (evaluateMaterial) — must come before main()
    GenerateShadingModel(os, spec);

    // 10. Main entry point (surface_main.fs, calls evaluateMaterial)
    GenerateMain(os, spec, ShaderStage::Fragment);
}
