#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "RHI/TargetBufferInfo.h"
#include "RHI/VertexBuffer.h"
#include "EngineEnum.h"
#include "ShaderCompiler/IncludeCallbaks.h"

class BufferInterfaceBlock;
class ChunkContainer;
class Package;
struct MaterialInfo;
struct MaterialSpec;

class MaterialBuilder
{
public:
	static constexpr size_t MATERIAL_VARIABLES_COUNT = 4;
	enum class Variable : uint8_t { CUSTOM0, CUSTOM1, CUSTOM2, CUSTOM3 };

	using MaterialDomain = MaterialDomain;
	using VertexAttribute = VertexAttribute;
	using BlendingMode = BlendingMode;
	using BlendFunction = RHI::BlendFunction;
	using Shading = Shading;
	using Pipeline = Pipeline;
	using Interpolation = Interpolation;
	using VertexDomain = VertexDomain;
	using AttributeType = UniformType;
	using UniformType = UniformType;
	using ConstantType = ConstantType;
	using SamplerType = RHI::SamplerType;
	using SamplerFormat = RHI::SamplerFormat;
	using CullingMode = RHI::CullingMode;
	using ShaderStage = ShaderStage;

	enum class VariableQualifier : uint8_t { OUT };
	enum class OutputTarget : uint8_t { COLOR, DEPTH };
	enum class OutputType : uint8_t { FLOAT, FLOAT2, FLOAT3, FLOAT4 };

	struct PreprocessorDefine { std::string name, value; PreprocessorDefine(std::string n, std::string v) : name(std::move(n)), value(std::move(v)) {} };
	using PreprocessorDefineList = std::vector<PreprocessorDefine>;

	MaterialBuilder();

	MaterialBuilder& noSamplerValidation(bool enabled) noexcept;
	MaterialBuilder& name(const std::string& name) noexcept;
	MaterialBuilder& fileName(const char* name) noexcept;
	MaterialBuilder& shading(Shading shading) noexcept;
	MaterialBuilder& pipeline(Pipeline pipeline) noexcept;
	MaterialBuilder& interpolation(Interpolation interpolation) noexcept;
	MaterialBuilder& parameter(const char* name, UniformType type) noexcept;
	MaterialBuilder& parameter(const char* name, size_t size, UniformType type) noexcept;
	MaterialBuilder& parameter(const char* name, SamplerType st, SamplerFormat fmt = SamplerFormat::FLOAT, bool ms = false) noexcept;
	MaterialBuilder& variable(Variable v, const char* name) noexcept;
	MaterialBuilder& require(VertexAttribute attribute) noexcept;
	MaterialBuilder& materialDomain(MaterialDomain materialDomain) noexcept;
	MaterialBuilder& material(const char* code, size_t line = 0) noexcept;
	MaterialBuilder& includeCallback(IncludeCallback callback) noexcept;

	using SpirvCompiler = std::function<bool(const std::string& vertSrc, const std::string& fragSrc, std::vector<uint8_t>& vertSpv, std::vector<uint8_t>& fragSpv)>;
	MaterialBuilder& spirvCompiler(SpirvCompiler compiler) noexcept;

	MaterialBuilder& materialVertex(const char* code, size_t line = 0) noexcept;
	MaterialBuilder& blending(BlendingMode blending) noexcept;
	MaterialBuilder& customBlendFunctions(BlendFunction srcRGB, BlendFunction srcA, BlendFunction dstRGB, BlendFunction dstA) noexcept;
	MaterialBuilder& postLightingBlending(BlendingMode blending) noexcept;
	MaterialBuilder& vertexDomain(VertexDomain domain) noexcept;
	MaterialBuilder& culling(CullingMode culling) noexcept;
	MaterialBuilder& colorWrite(bool enable) noexcept;
	MaterialBuilder& depthWrite(bool enable) noexcept;
	MaterialBuilder& depthCulling(bool enable) noexcept;
	MaterialBuilder& instanced(bool enable) noexcept;
	MaterialBuilder& doubleSided(bool doubleSided) noexcept;
	MaterialBuilder& flipUV(bool flipUV) noexcept;
	MaterialBuilder& customSurfaceShading(bool customSurfaceShading) noexcept;
	MaterialBuilder& printShaders(bool printShaders) noexcept;
	MaterialBuilder& shaderDefine(const char* name, const char* value) noexcept;
	MaterialBuilder& output(VariableQualifier q, OutputTarget t, OutputType ty, const char* name, int loc = -1) noexcept;
	MaterialBuilder& enableFramebufferFetch() noexcept;
	MaterialBuilder& vertexDomainDeviceJittered(bool enabled) noexcept;
	MaterialBuilder& groupSize(const glm::uvec3& groupSize) noexcept;

	Package build();

public:
	struct Parameter {
		Parameter() noexcept: parameterType(INVALID) {}
		Parameter(const char* n, SamplerType t, SamplerFormat f, bool ms) : name(n), size(1), samplerType(t), format(f), parameterType(SAMPLER), multisample(ms) {}
		Parameter(const char* n, UniformType t, size_t sz) : name(n), size(sz), uniformType(t), parameterType(UNIFORM) {}
		std::string name; size_t size; UniformType uniformType; SamplerType samplerType; SamplerFormat format; bool multisample;
		enum { INVALID, UNIFORM, SAMPLER } parameterType;
		bool isSampler() const { return parameterType == SAMPLER; }
		bool isUniform() const { return parameterType == UNIFORM; }
	};
	struct Output {
		Output() noexcept = default;
		Output(const char* n, VariableQualifier q, OutputTarget t, OutputType ty, int l) noexcept : name(n), qualifier(q), target(t), type(ty), location(l) {}
		std::string name; VariableQualifier qualifier; OutputTarget target; OutputType type; int location;
	};
	struct Constant { std::string name; ConstantType type; union { int32_t i; float f; bool b; } defaultValue; };
	struct PushConstant { std::string name; ConstantType type; ShaderStage stage; };
	struct CustomVariable { std::string name; bool hasPrecision = false; };

	using Property = Property;
	using PropertyList = bool[MATERIAL_PROPERTIES_COUNT];
	static const char* sPropertyNames[MATERIAL_PROPERTIES_COUNT];
	using VariableList = std::array<CustomVariable,MATERIAL_VARIABLES_COUNT>;
	using OutputList = std::vector<Output>;
	static constexpr size_t MAX_COLOR_OUTPUT = MAX_SUPPORTED_RENDER_TARGET_COUNT;
	static constexpr size_t MAX_DEPTH_OUTPUT = 1;
	static constexpr size_t MAX_PARAMETERS_COUNT = 48;
	static constexpr size_t MAX_SUBPASS_COUNT = 1;
	static constexpr size_t MAX_BUFFERS_COUNT = 4;
	using ParameterList = Parameter[MAX_PARAMETERS_COUNT];
	using SubpassList = Parameter[MAX_SUBPASS_COUNT];
	using BufferList = std::vector<std::unique_ptr<BufferInterfaceBlock>>;
	using ConstantList = std::vector<Constant>;
	using PushConstantList = std::vector<PushConstant>;

	uint8_t getParameterCount() const noexcept { return mParameterCount; }
	const ParameterList& getParameters() const noexcept { return mParameters; }
	uint8_t getSubpassCount() const noexcept { return mSubpassCount; }

	struct Attribute { std::string_view name; AttributeType type; VertexAttribute location; };
	using AttributeDatabase = std::array<Attribute, MAX_VERTEX_ATTRIBUTE_COUNT>;
	static inline AttributeDatabase const& getAttributeDatabase() noexcept { return sAttributeDatabase; }

	bool hasSamplerType(SamplerType samplerType) const noexcept;

private:
	static const AttributeDatabase sAttributeDatabase;
	void prepareToBuild(MaterialInfo& info) noexcept;
	void writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept;
	void writeSurfaceChunks(ChunkContainer& container) const noexcept;
	bool generateShaders(ChunkContainer& container, const MaterialInfo& info) const;
	bool hasCustomVaryings() const noexcept;
	bool needsStandardDepthProgram() const noexcept;
	bool isLit() const noexcept { return mShading != Shading::UNLIT; }

	class ShaderCode {
	public:
		void setLineOffset(size_t offset) noexcept { mLineOffset = offset; }
		void setUnresolved(const std::string& code) noexcept { mIncludesResolved = false; mCode = code; }
		bool resolveIncludes(IncludeCallback callback, const std::string& fileName) noexcept;
		const std::string& getResolved() const noexcept { return mCode; }
		size_t getLineOffset() const noexcept { return mLineOffset; }
	private:
		std::string mCode; size_t mLineOffset = 0; bool mIncludesResolved = true;
	};

	ShaderCode mMaterialFragmentCode, mMaterialVertexCode;
	IncludeCallback mIncludeCallback = nullptr;
	SpirvCompiler mSpirvCompiler;
	PropertyList mProperties{}; ParameterList mParameters{}; PushConstantList mPushConstants;
	VariableList mVariables{}; OutputList mOutputs;
	BlendingMode mBlendingMode = BlendingMode::OPAQUE;
	BlendingMode mPostLightingBlendingMode = BlendingMode::TRANSPARENT;
	std::array<BlendFunction, 4> mCustomBlendFunctions{};
	CullingMode mCullingMode = CullingMode::BACK;
	Shading mShading = Shading::LIT;
	Pipeline mPipeline = Pipeline::DEFERRED;
	MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
	Interpolation mInterpolation = Interpolation::SMOOTH;
	VertexDomain mVertexDomain = VertexDomain::OBJECT;
	uint8_t mStereoscopicEyeCount = 2;
	RHI::AttributeBitset mRequiredAttributes;
	float mMaskThreshold = 0.4f;
	glm::uvec3 mGroupSize = {1,1,1};
	bool mShadowMultiplier = false, mTransparentShadow = false;
	uint8_t mParameterCount = 0, mSubpassCount = 0;
	bool mDoubleSided = false, mDoubleSidedCapability = false, mColorWrite = true, mDepthTest = true;
	bool mInstanced = false, mDepthWrite = true, mDepthWriteSet = false;
	bool mAlphaToCoverage = false, mAlphaToCoverageSet = false;
	bool mSpecularAntiAliasing = false, mClearCoatIorChange = true;
	bool mFlipUV = true, mMultiBounceAO = false, mMultiBounceAOSet = false;
	bool mSpecularAOSet = false, mCustomSurfaceShading = false;
	bool mEnableFramebufferFetch = false, mVertexDomainDeviceJittered = false;
	PreprocessorDefineList mDefines;
	bool mNoSamplerValidation = false;
	std::string mMaterialName, mFileName;
};
