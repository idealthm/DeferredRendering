#include "MaterialBuilder.h"

#include <fstream>

#include "DescriptorSetLayout.h"
#include "DescriptorSets.h"
#include "MaterialInfo.h"
#include "ShaderCompiler/MaterialSpec.h"
#include "Package.h"
#include "Common/Serialization/ChunkContainer.h"
#include "Common/Serialization/MaterialChunks.h"
#include "ShaderCompiler/includes.h"
#include "ShaderCompiler/ShaderGenerator.h"
#include "ShaderCompiler/ShaderInputBuilder.h"
#include "ShaderCompiler/UibGenerator.h"

const char* MaterialBuilder::sPropertyNames[MATERIAL_PROPERTIES_COUNT] = {
    "BASE_COLOR", "ROUGHNESS", "METALLIC", "REFLECTANCE", "AMBIENT_OCCLUSION",
    "CLEAR_COAT", "CLEAR_COAT_ROUGHNESS", "CLEAR_COAT_NORMAL", "ANISOTROPY", "ANISOTROPY_DIRECTION",
    "THICKNESS", "SUBSURFACE_POWER", "SUBSURFACE_COLOR", "SHEEN_COLOR", "SHEEN_ROUGHNESS",
    "SPECULAR_COLOR", "GLOSSINESS", "EMISSIVE", "NORMAL", "POST_LIGHTING_COLOR",
    "POST_LIGHTING_MIX_FACTOR", "CLIP_SPACE_TRANSFORM", "ABSORPTION", "TRANSMISSION",
    "IOR", "MICRO_THICKNESS", "BENT_NORMAL", "SPECULAR_FACTOR", "SPECULAR_COLOR_FACTOR"
};

const MaterialBuilder::AttributeDatabase MaterialBuilder::sAttributeDatabase = {{
    { "position",      AttributeType::FLOAT4, VertexAttribute::POSITION     },
    { "tangents",      AttributeType::FLOAT4, VertexAttribute::TANGENTS     },
    { "color",         AttributeType::FLOAT4, VertexAttribute::COLOR        },
    { "uv0",           AttributeType::FLOAT2, VertexAttribute::UV0          },
    { "uv1",           AttributeType::FLOAT2, VertexAttribute::UV1          },
    { "bone_indices",  AttributeType::UINT4,  VertexAttribute::BONE_INDICES },
    { "bone_weights",  AttributeType::FLOAT4, VertexAttribute::BONE_WEIGHTS },
    { },
    { "custom0",       AttributeType::FLOAT4, VertexAttribute::CUSTOM0      },
    { "custom1",       AttributeType::FLOAT4, VertexAttribute::CUSTOM1      },
    { "custom2",       AttributeType::FLOAT4, VertexAttribute::CUSTOM2      },
    { "custom3",       AttributeType::FLOAT4, VertexAttribute::CUSTOM3      },
    { "custom4",       AttributeType::FLOAT4, VertexAttribute::CUSTOM4      },
    { "custom5",       AttributeType::FLOAT4, VertexAttribute::CUSTOM5      },
    { "custom6",       AttributeType::FLOAT4, VertexAttribute::CUSTOM6      },
    { "custom7",       AttributeType::FLOAT4, VertexAttribute::CUSTOM7      },
}};

MaterialBuilder& MaterialBuilder::name(const std::string& name) noexcept {
    mMaterialName = std::string(name);
    return *this;
}

MaterialBuilder& MaterialBuilder::fileName(const char* fileName) noexcept {
    mFileName = std::string(fileName);
    return *this;
}

MaterialBuilder& MaterialBuilder::material(const char* code, size_t line) noexcept {
    mMaterialFragmentCode.setUnresolved(std::string(code));
    mMaterialFragmentCode.setLineOffset(line);
    return *this;
}

MaterialBuilder& MaterialBuilder::includeCallback(IncludeCallback callback) noexcept {
    mIncludeCallback = std::move(callback);
    return *this;
}

MaterialBuilder& MaterialBuilder::spirvCompiler(SpirvCompiler compiler) noexcept {
    mSpirvCompiler = std::move(compiler);
    return *this;
}

MaterialBuilder& MaterialBuilder::materialVertex(const char* code, size_t line) noexcept {
    mMaterialVertexCode.setUnresolved(std::string(code));
    mMaterialVertexCode.setLineOffset(line);
    return *this;
}

MaterialBuilder& MaterialBuilder::shading(Shading shading) noexcept {
    mShading = shading;
    return *this;
}

MaterialBuilder& MaterialBuilder::pipeline(Pipeline pipeline) noexcept {
    mPipeline = pipeline;
    return *this;
}

MaterialBuilder& MaterialBuilder::interpolation(Interpolation interpolation) noexcept {
    mInterpolation = interpolation;
    return *this;
}

MaterialBuilder& MaterialBuilder::variable(Variable v, const char* name) noexcept {
    switch (v) {
        case Variable::CUSTOM0:
        case Variable::CUSTOM1:
        case Variable::CUSTOM2:
        case Variable::CUSTOM3:
            assert(size_t(v) < MATERIAL_VARIABLES_COUNT);
            mVariables[size_t(v)] = { std::string(name), false };
            break;
    }
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(const char* name, size_t size, UniformType type) noexcept {
    ASSERT(mParameterCount < MAX_PARAMETERS_COUNT);
    mParameters[mParameterCount++] = { name, type, size };
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(const char* name, UniformType type) noexcept {
    return parameter(name, 1, type);
}


MaterialBuilder& MaterialBuilder::parameter(const char* name, SamplerType samplerType,
        SamplerFormat format, bool multisample) noexcept {
    ASSERT(!multisample ||
            (format != SamplerFormat::SHADOW &&
                    (samplerType == SamplerType::SAMPLER_2D ||
                            samplerType == SamplerType::SAMPLER_2D_ARRAY)))

    ASSERT(mParameterCount < MAX_PARAMETERS_COUNT);
    mParameters[mParameterCount++] = { name, samplerType, format, multisample };
    return *this;
}

MaterialBuilder& MaterialBuilder::require(VertexAttribute attribute) noexcept {
    mRequiredAttributes.set(attribute);
    return *this;
}

MaterialBuilder& MaterialBuilder::groupSize(const glm::uvec3& groupSize) noexcept {
    mGroupSize = groupSize;
    return *this;
}

MaterialBuilder& MaterialBuilder::materialDomain(
        MaterialBuilder::MaterialDomain materialDomain) noexcept {
    mMaterialDomain = materialDomain;
    if (mMaterialDomain == MaterialDomain::COMPUTE) {
    }
    return *this;
}

MaterialBuilder& MaterialBuilder::blending(BlendingMode blending) noexcept {
    mBlendingMode = blending;
    return *this;
}

MaterialBuilder& MaterialBuilder::customBlendFunctions(
        BlendFunction srcRGB, BlendFunction srcA,
        BlendFunction dstRGB, BlendFunction dstA) noexcept {
    mCustomBlendFunctions[0] = srcRGB;
    mCustomBlendFunctions[1] = srcA;
    mCustomBlendFunctions[2] = dstRGB;
    mCustomBlendFunctions[3] = dstA;
    return *this;
}

MaterialBuilder& MaterialBuilder::postLightingBlending(BlendingMode blending) noexcept {
    mPostLightingBlendingMode = blending;
    return *this;
}

MaterialBuilder& MaterialBuilder::vertexDomain(VertexDomain domain) noexcept {
    mVertexDomain = domain;
    return *this;
}

MaterialBuilder& MaterialBuilder::culling(CullingMode culling) noexcept {
    mCullingMode = culling;
    return *this;
}

MaterialBuilder& MaterialBuilder::colorWrite(bool enable) noexcept {
    mColorWrite = enable;
    return *this;
}

MaterialBuilder& MaterialBuilder::depthWrite(bool enable) noexcept {
    mDepthWrite = enable;
    mDepthWriteSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::depthCulling(bool enable) noexcept {
    mDepthTest = enable;
    return *this;
}

MaterialBuilder& MaterialBuilder::instanced(bool enable) noexcept {
    mInstanced = enable;
    return *this;
}

MaterialBuilder& MaterialBuilder::doubleSided(bool doubleSided) noexcept {
    mDoubleSided = doubleSided;
    mDoubleSidedCapability = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::flipUV(bool flipUV) noexcept {
    mFlipUV = flipUV;
    return *this;
}

MaterialBuilder& MaterialBuilder::customSurfaceShading(bool customSurfaceShading) noexcept {
    mCustomSurfaceShading = customSurfaceShading;
    return *this;
}

MaterialBuilder& MaterialBuilder::shaderDefine(const char* name, const char* value) noexcept {
    mDefines.emplace_back(name, value);
    return *this;
}

bool MaterialBuilder::hasSamplerType(SamplerType samplerType) const noexcept {
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler() && param.samplerType == samplerType) {
            return  true;
        }
    }
    return false;
}

void MaterialBuilder::prepareToBuild(MaterialInfo& info) noexcept {

    // Populate mProperties by matching parameter names against known Property names
    for (size_t pi = 0; pi < MATERIAL_PROPERTIES_COUNT; pi++)
    {
        // Convert UPPER_SNAKE_CASE → camelCase
        std::string camelCase;
        bool nextUpper = false;
        for (char c : std::string(sPropertyNames[pi]))
        {
            if (c == '_') { nextUpper = true; continue; }
            camelCase += camelCase.empty() ? char(std::tolower(c))
                      : nextUpper ? c : char(std::tolower(c));
            nextUpper = false;
        }

        for (size_t i = 0; i < mParameterCount; i++)
        {
            if (mParameters[i].name == camelCase)
                { mProperties[pi] = true; break; }
        }
    }

    // Build the per-material sampler block and uniform block.
    SamplerInterfaceBlock::Builder sbb;
    BufferInterfaceBlock::Builder ibb;
    // sampler bindings start at 1, 0 is the ubo
    for (size_t i = 0, binding = 1, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler()) {
            sbb.add({ param.name.data(), param.name.size() },
                    binding++, param.samplerType, param.format, param.multisample);
        } else if (param.isUniform()) {
            ibb.add({{{ param.name.data(), param.name.size() },
                      uint32_t(param.size == 1u ? 0u : param.size), param.uniformType}});
        }
    }

    if (mSpecularAntiAliasing) {
        ibb.add({
                { "_specularAntiAliasingVariance",  0, UniformType::FLOAT },
                { "_specularAntiAliasingThreshold", 0, UniformType::FLOAT },
        });
    }

    if (mBlendingMode == BlendingMode::MASKED) {
        ibb.add({{ "_maskThreshold", 0, UniformType::FLOAT}});
    }

    if (mDoubleSidedCapability) {
        ibb.add({{ "_doubleSided", 0, UniformType::BOOL}});
    }

    mRequiredAttributes.set(VertexAttribute::POSITION);
    if (mShading != Shading::UNLIT || mShadowMultiplier) {
        mRequiredAttributes.set(VertexAttribute::TANGENTS);
    }

    info.sib = sbb.name("MaterialParams").build();
    info.uib = ibb.name("MaterialParams").build();

    info.isLit = isLit();
    info.hasDoubleSidedCapability = mDoubleSidedCapability;
    info.has3dSamplers = hasSamplerType(SamplerType::SAMPLER_3D);
    info.flipUV = mFlipUV;
    info.requiredAttributes = mRequiredAttributes;
    info.blendingMode = mBlendingMode;
    info.postLightingBlendingMode = mPostLightingBlendingMode;
    info.shading = mShading;
    info.groupSize = mGroupSize;
}

bool MaterialBuilder::ShaderCode::resolveIncludes(IncludeCallback callback,
        const std::string& fileName) noexcept {
    if (!mCode.empty()) {
        ResolveOptions options { true, true };
        IncludeResult source { fileName, mCode, getLineOffset(), std::string("")
        };
        if (!::resolveIncludes(source, std::move(callback), options)) {
            return false;
        }
        mCode = source.text;
    }

    mIncludesResolved = true;
    return true;
}

bool MaterialBuilder::generateShaders(ChunkContainer& container, const MaterialInfo& info) const {

    ShaderGenerator sg(mMaterialName, mPipeline, mShading, mMaterialDomain, mVertexDomain,
            info.uib, info.sib, mRequiredAttributes, mProperties, mVariables, mOutputs, mDefines, mPushConstants,
            mMaterialFragmentCode.getResolved(), mMaterialFragmentCode.getLineOffset(),
            mMaterialVertexCode.getResolved(), mMaterialVertexCode.getLineOffset());

    std::string vertCode = mMaterialVertexCode.getResolved();
    std::string fragCode = mMaterialFragmentCode.getResolved();

    ChunkGlsl::Container glslEntries;
    ChunkSpirv::Container spirvEntries;

    auto addPass = [&](MaterialPass pass) {
        std::string vs = sg.GenerateShader(ShaderGenerator::Stage::Vertex, pass, vertCode);
        std::string fs = sg.GenerateShader(ShaderGenerator::Stage::Fragment, pass, fragCode);
        glslEntries.push_back({ pass, vs, fs });

        if (mSpirvCompiler) {
            std::vector<uint8_t> vertSpv, fragSpv;
            if (mSpirvCompiler(vs, fs, vertSpv, fragSpv))
                spirvEntries.push_back({ pass, std::move(vertSpv), std::move(fragSpv) });
        }
    };

    if (mMaterialDomain == MaterialDomain::POST_PROCESS) {
        std::string vs = sg.GeneratePostProcessShader(ShaderGenerator::Stage::Vertex, vertCode);
        std::string fs = sg.GeneratePostProcessShader(ShaderGenerator::Stage::Fragment, fragCode);
        glslEntries.push_back({ MaterialPass::PostProcess, vs, fs });
        if (mSpirvCompiler) {
            std::vector<uint8_t> vertSpv, fragSpv;
            if (mSpirvCompiler(vs, fs, vertSpv, fragSpv))
                spirvEntries.push_back({ MaterialPass::PostProcess, std::move(vertSpv), std::move(fragSpv) });
        }
    } else if (mPipeline == Pipeline::LIGHTING) {
        addPass(MaterialPass::Lighting);
    } else {
        {
            std::string vs = sg.GenerateShader(ShaderGenerator::Stage::Vertex, MaterialPass::Depth, vertCode);
            std::string fs = sg.GenerateShader(ShaderGenerator::Stage::Fragment, MaterialPass::Depth, std::string{});
            glslEntries.push_back({ MaterialPass::Depth, vs, fs });
            if (mSpirvCompiler) {
                std::vector<uint8_t> vertSpv, fragSpv;
                if (mSpirvCompiler(vs, fs, vertSpv, fragSpv))
                    spirvEntries.push_back({ MaterialPass::Depth, std::move(vertSpv), std::move(fragSpv) });
            }
        }
        addPass(MaterialPass::Surface);
    }

    container.push<ChunkGlsl>(std::move(glslEntries));
    if (!spirvEntries.empty())
        container.push<ChunkSpirv>(std::move(spirvEntries));

    return true;
}

MaterialBuilder& MaterialBuilder::output(VariableQualifier qualifier, OutputTarget target,
        OutputType type, const char* name, int location) noexcept {
    ASSERT(target != OutputTarget::DEPTH || type == OutputType::FLOAT)
    ASSERT(target != OutputTarget::DEPTH || qualifier == VariableQualifier::OUT)

    ASSERT(location >= -1)

    // A location value of -1 signals using the default location. We'll simply take the previous
    // output's location and add 1.
    if (location == -1) {
        location = mOutputs.empty() ? 0 : mOutputs.back().location + 1;
    }

    // Unconditionally add this output, then we'll check if we've maxed on on any particular target.
    mOutputs.emplace_back(name, qualifier, target, type, location);

    uint8_t colorOutputCount = 0;
    uint8_t depthOutputCount = 0;
    for (const auto& output : mOutputs) {
        if (output.target == OutputTarget::COLOR) {
            colorOutputCount++;
        }
        if (output.target == OutputTarget::DEPTH) {
            depthOutputCount++;
        }
    }

    ASSERT(colorOutputCount <= MAX_COLOR_OUTPUT)
    ASSERT(depthOutputCount <= MAX_DEPTH_OUTPUT)

    assert(mOutputs.size() <= MAX_COLOR_OUTPUT + MAX_DEPTH_OUTPUT);

    return *this;
}

MaterialBuilder& MaterialBuilder::enableFramebufferFetch() noexcept {
    // This API is temporary, it is used to enable EXT_framebuffer_fetch for GLSL shaders,
    // this is used sparingly by filament's post-processing stage.
    mEnableFramebufferFetch = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::vertexDomainDeviceJittered(bool enabled) noexcept {
    mVertexDomainDeviceJittered = enabled;
    return *this;
}

Package MaterialBuilder::build() {
    bool success;

    // Add a default color output.
    if (mMaterialDomain == MaterialDomain::POST_PROCESS && mOutputs.empty()) {
        output(VariableQualifier::OUT,
                OutputTarget::COLOR, OutputType::FLOAT4, "color");
    }

    // Resolve #include directives if a callback is provided.
    // When no callback, the code has no includes to resolve — mark as resolved.
    if (mIncludeCallback) {
        if (!mMaterialFragmentCode.resolveIncludes(mIncludeCallback, mFileName) ||
            !mMaterialVertexCode.resolveIncludes(mIncludeCallback, mFileName)) {
            return Package::invalidPackage();
        }
    } else {
        mMaterialFragmentCode.resolveIncludes(
            [](const std::string&, IncludeResult&) { return false; }, mFileName);
        mMaterialVertexCode.resolveIncludes(
            [](const std::string&, IncludeResult&) { return false; }, mFileName);
    }

    if (mCustomSurfaceShading && mShading != Shading::LIT) {
        return Package::invalidPackage();
    }

    MaterialInfo info{};
    prepareToBuild(info);

    // Generate shaders first — needs info.uib / info.sib intact.
    ChunkContainer container;
    success = generateShaders(container, info);

    // Write metadata chunks (moves info.uib / info.sib).
    writeCommonChunks(container, info);
    if (mMaterialDomain == MaterialDomain::SURFACE) {
        writeSurfaceChunks(container);
    }
    if (!success) {
        // Return an empty package to signal a failure to build the material.
        return Package::invalidPackage();
    }

    // Dry-run to measure size, then serialize.
    FArchiveWrite dryAr;
    container.Serialize(dryAr);

    Package package(dryAr.Tell());
    FArchiveWrite ar(package.getData(), package.getSize());
    container.Serialize(ar);
    return package;
}

static const char* to_string(RHI::ShaderStageFlags stageFlags) noexcept {
    switch (stageFlags) {
        case RHI::ShaderStageFlags::NONE:                    return "{ }";
        case RHI::ShaderStageFlags::VERTEX:                  return "{ vertex }";
        case RHI::ShaderStageFlags::FRAGMENT:                return "{ fragment }";
        case RHI::ShaderStageFlags::COMPUTE:                 return "{ compute }";
        case RHI::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS:  return "{ vertex | fragment | compute }";
    }
    return nullptr;
}

bool MaterialBuilder::hasCustomVaryings() const noexcept {
    for (const auto& variable : mVariables) {
        if (!variable.name.empty()) {
            return true;
        }
    }
    return false;
}

bool MaterialBuilder::needsStandardDepthProgram() const noexcept {
    const bool hasEmptyVertexCode = mMaterialVertexCode.getResolved().empty();
    return !hasEmptyVertexCode ||
           hasCustomVaryings() ||
           mBlendingMode == BlendingMode::MASKED ||
           (mTransparentShadow &&
            (mBlendingMode == BlendingMode::TRANSPARENT ||
             mBlendingMode == BlendingMode::FADE));
}

void MaterialBuilder::writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept
{
    container.push<ChunkName>(mMaterialName);
    container.push<ChunkDomain>(static_cast<uint8_t>(mMaterialDomain));

    using Container = std::vector<std::pair<std::string, uint8_t>>;
    Container attributes;
    attributes.reserve(sAttributeDatabase.size());
    for (auto const& attribute: sAttributeDatabase) {
        std::string name("mesh_");
        name.append(attribute.name);
        attributes.emplace_back(std::string{ name.data(), name.size() }, attribute.location);
    }
    container.push<ChunkMaterialAttributesInfo>(std::move(attributes));

    // Vertex inputs (from required attributes) + fragment outputs
    {
        ChunkAttributeInputOutput::Container attrIO;
        attrIO.inputs  = BuildVertexInputs(mMaterialDomain, mRequiredAttributes);
        attrIO.outputs = BuildFragmentOutputs(mOutputs);
        container.push<ChunkAttributeInputOutput>(std::move(attrIO));
    }

    // User parameters (UBO)
    container.push<ChunkUib>(std::move(info.uib));

    // Descriptor bindings (name→binding mapping) — all sets
    {
        DescriptorSetInfo bindings{};

        // PER_VIEW (set 0)
        auto& setView = bindings[+DescriptorSetBindingPoints::PER_VIEW];
        setView.push_back({ "FrameUniforms.frameUniforms", RHI::DescriptorType::UNIFORM_BUFFER,
            +PerViewBindingPoints::FRAME_UNIFORM });
        setView.push_back({ "LightData.lightData", RHI::DescriptorType::UNIFORM_BUFFER,
            +PerViewBindingPoints::LIGHT_DATA });
        setView.push_back({ "irradianceMap", RHI::DescriptorType::SAMPLER,
            +PerViewBindingPoints::IBL_IRRADIANCE });
        setView.push_back({ "prefilterMap", RHI::DescriptorType::SAMPLER,
            +PerViewBindingPoints::IBL_PREFILTER });
        setView.push_back({ "brdfLut", RHI::DescriptorType::SAMPLER,
            +PerViewBindingPoints::BRDF_LUT });

        // PER_RENDERABLE (set 1) — non-lighting pipelines only
        if (mPipeline != Pipeline::LIGHTING)
        {
            auto& setRenderable = bindings[+DescriptorSetBindingPoints::PER_RENDERABLE];
            setRenderable.push_back({ "ObjectUniforms.objectUniforms", RHI::DescriptorType::UNIFORM_BUFFER,
                +PerRenderableBindingPoints::OBJECT_UNIFORM });
        }

        // PER_MATERIAL (set 2)
        auto& setMat = bindings[+DescriptorSetBindingPoints::PER_MATERIAL];
        setMat.push_back({ "MaterialParams.materialParams", RHI::DescriptorType::UNIFORM_BUFFER,
            +PerMaterialBindingPoint::MATERIAL_UNIFORM });
        for (auto& s : info.sib.getSamplerInfoList())
            setMat.push_back({ "materialParams_" + s.name, RHI::DescriptorType::SAMPLER, s.binding });

        // G_BUFFER (set 3) — lighting pipeline only
        if (mPipeline == Pipeline::LIGHTING)
        {
            auto& setGBuffer = bindings[+DescriptorSetBindingPoints::G_BUFFER];
            setGBuffer.push_back({ "gDepth",    RHI::DescriptorType::SAMPLER, +GBufferBindingPoint::G_BUFFER_DEPTH });
            setGBuffer.push_back({ "gNormal",   RHI::DescriptorType::SAMPLER, +GBufferBindingPoint::G_BUFFER_NORMAL });
            setGBuffer.push_back({ "gAlbedo",   RHI::DescriptorType::SAMPLER, +GBufferBindingPoint::G_BUFFER_ALBEDO });
            setGBuffer.push_back({ "gMaterial", RHI::DescriptorType::SAMPLER, +GBufferBindingPoint::G_BUFFER_MATERIAL });
        }

        container.push<ChunkMaterialDescriptorBindings>(std::move(bindings));
    }

    // Descriptor set layout (GPU pipeline layout) — all sets
    {
        ChunkMaterialDescriptorSetLayout::Container layouts{};

        // PER_VIEW (set 0): FrameUniforms + LightData
        {
            auto& layout = layouts[+DescriptorSetBindingPoints::PER_VIEW].bindings;
            RHI::DescriptorSetLayoutBinding frameUBO{};
            frameUBO.type = RHI::DescriptorType::UNIFORM_BUFFER;
            frameUBO.binding = +PerViewBindingPoints::FRAME_UNIFORM;
            frameUBO.stageFlags = RHI::ShaderStageFlags::VERTEX | RHI::ShaderStageFlags::FRAGMENT;
            frameUBO.count = 1;
            layout.push_back(frameUBO);

            RHI::DescriptorSetLayoutBinding lightUBO{};
            lightUBO.type = RHI::DescriptorType::UNIFORM_BUFFER;
            lightUBO.binding = +PerViewBindingPoints::LIGHT_DATA;
            lightUBO.stageFlags = RHI::ShaderStageFlags::FRAGMENT;
            lightUBO.count = 1;
            layout.push_back(lightUBO);

            auto addViewSampler = [&](uint8_t binding) {
                RHI::DescriptorSetLayoutBinding b{};
                b.type = RHI::DescriptorType::SAMPLER;
                b.binding = binding;
                b.stageFlags = RHI::ShaderStageFlags::FRAGMENT;
                b.count = 1;
                layout.push_back(b);
            };
            addViewSampler(+PerViewBindingPoints::IBL_IRRADIANCE);
            addViewSampler(+PerViewBindingPoints::IBL_PREFILTER);
            addViewSampler(+PerViewBindingPoints::BRDF_LUT);
        }

        // PER_RENDERABLE (set 1): ObjectUniforms — non-lighting only
        if (mPipeline != Pipeline::LIGHTING)
        {
            auto& layout = layouts[+DescriptorSetBindingPoints::PER_RENDERABLE].bindings;
            RHI::DescriptorSetLayoutBinding objUBO{};
            objUBO.type = RHI::DescriptorType::UNIFORM_BUFFER;
            objUBO.binding = +PerRenderableBindingPoints::OBJECT_UNIFORM;
            objUBO.stageFlags = RHI::ShaderStageFlags::VERTEX | RHI::ShaderStageFlags::FRAGMENT;
            objUBO.count = 1;
            layout.push_back(objUBO);
        }

        // PER_MATERIAL (set 2): UBO + samplers
        {
            auto& layout = layouts[+DescriptorSetBindingPoints::PER_MATERIAL].bindings;
            {
                RHI::DescriptorSetLayoutBinding ubo{};
                ubo.type = RHI::DescriptorType::UNIFORM_BUFFER;
                ubo.binding = +PerMaterialBindingPoint::MATERIAL_UNIFORM;
                ubo.stageFlags = RHI::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
                ubo.count = 1;
                layout.push_back(ubo);
            }
            for (auto& s : info.sib.getSamplerInfoList())
            {
                RHI::DescriptorSetLayoutBinding b{};
                b.type = RHI::DescriptorType::SAMPLER;
                b.binding = s.binding;
                b.stageFlags = RHI::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
                b.count = 1;
                layout.push_back(b);
            }
        }

        // G_BUFFER (set 3): GBuffer textures — lighting pipeline only
        if (mPipeline == Pipeline::LIGHTING)
        {
            auto& layout = layouts[+DescriptorSetBindingPoints::G_BUFFER].bindings;
            auto addSampler = [&](uint8_t binding) {
                RHI::DescriptorSetLayoutBinding b{};
                b.type = RHI::DescriptorType::SAMPLER;
                b.binding = binding;
                b.stageFlags = RHI::ShaderStageFlags::FRAGMENT;
                b.count = 1;
                layout.push_back(b);
            };
            addSampler(+GBufferBindingPoint::G_BUFFER_DEPTH);
            addSampler(+GBufferBindingPoint::G_BUFFER_NORMAL);
            addSampler(+GBufferBindingPoint::G_BUFFER_ALBEDO);
            addSampler(+GBufferBindingPoint::G_BUFFER_MATERIAL);
        }

        container.push<ChunkMaterialDescriptorSetLayout>(std::move(layouts));
    }

    // User texture parameters (moves sib, must be last)
    container.push<ChunkSib>(std::move(info.sib));

    if (mMaterialDomain != MaterialDomain::COMPUTE) {
        // User Subpass
        container.push<ChunkDoubleSided>(mDoubleSided);
        container.push<ChunkBlendingMode>(static_cast<uint8_t>(mBlendingMode));

        if (mBlendingMode == BlendingMode::CUSTOM) {
            uint32_t const blendFunctions =
                    (uint32_t(mCustomBlendFunctions[0]) << 24) |
                    (uint32_t(mCustomBlendFunctions[1]) << 16) |
                    (uint32_t(mCustomBlendFunctions[2]) <<  8) |
                    (uint32_t(mCustomBlendFunctions[3]) <<  0);
            container.push<ChunkBlendFunction>(blendFunctions);
        }

        container.push<ChunkColorWrite>(mColorWrite);
        container.push<ChunkDepthWriteSet>(mDepthWriteSet);
        container.push<ChunkDepthWrite>(mDepthWrite);
        container.push<ChunkDepthTest>(mDepthTest);
        container.push<ChunkCullingMode>(static_cast<uint8_t>(mCullingMode));

        std::vector<FlatProperty> properties;
        for (size_t i = 0; i < MATERIAL_PROPERTIES_COUNT; i++) {
            if (mProperties[i]) {
                FlatProperty fp;
                fp.name = std::to_string(i);
                fp.propertyId = static_cast<uint8_t>(i);
                properties.push_back(std::move(fp));
            }
        }
        container.push<ChunkProperties>(std::move(properties));
    }
}

void MaterialBuilder::writeSurfaceChunks(ChunkContainer& container) const noexcept {
    if (mBlendingMode == BlendingMode::MASKED) {
        container.push<ChunkMaskThreshold>(mMaskThreshold);
    }

    container.push<ChunkShading>(std::string(mShading == Shading::LIT ? "lit" : "unlit"));

    if (mShading == Shading::UNLIT) {
        container.push<ChunkShadowMultiplier>(mShadowMultiplier);
    }

    container.push<ChunkRequiredAttrs>(mRequiredAttributes.getValue());
}

MaterialBuilder::MaterialBuilder()
    : mMaterialName("unnamed")
{
    std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);
}

MaterialBuilder& MaterialBuilder::noSamplerValidation(bool enabled) noexcept {
    mNoSamplerValidation = enabled;
    return *this;
}
