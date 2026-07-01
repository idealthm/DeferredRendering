#pragma once
#include <sstream>
#include <string>

#include "CodeGenerator.h"
#include "Common/Material/MaterialBuilder.h"
#include "Common/Material/MaterialCommon.h"
#include "BufferInterfaceBlock.h"
#include "SamplerInterfaceBlock.h"
#include "Common/Material/MaterialTypes.h"

struct ConstantParam;
struct MaterialSpec;

class ShaderGenerator
{
public:
	enum class Stage : uint8_t { Vertex, Fragment, Compute };

	ShaderGenerator() = default;
	ShaderGenerator(std::string const& materialName,
	                Pipeline pipeline, Shading shadingModel, MaterialDomain domain,
	                BufferInterfaceBlock const& materialUib, SamplerInterfaceBlock const& materialSib,
	                RHI::AttributeBitset const& requiredAttributes,
	                MaterialBuilder::PropertyList const& properties,
	                MaterialBuilder::VariableList const& variables,
	                MaterialBuilder::OutputList const& outputs,
	                MaterialBuilder::PreprocessorDefineList const& defines,
	                MaterialBuilder::PushConstantList const& pushConstants,
	                std::string const& fragmentCode, size_t fragmentLineOffset,
	                std::string const& vertexCode, size_t vertexLineOffset);

	void SetTemplateDirectory(const std::string& dir) { m_TemplateDir = dir; }
	void SetTargetVulkan(bool vulkan)                { m_TargetVulkan = vulkan; }

	std::string GenerateShader(Stage stage, MaterialPass pass, const std::string& userCode) const;
	std::string GeneratePostProcessShader(Stage stage, const std::string& userCode) const;
	std::string GenerateComputeShader(const std::string& userCode) const;

private:
	void EmitHeader(std::ostringstream& os, CodeGenerator& cg) const;
	void EmitLightHeader(std::ostringstream& os) const;
	void EmitVertexInputs(std::ostringstream& os) const;
	void EmitVaryingOut(std::ostringstream& os) const;
	void EmitVaryingIn(std::ostringstream& os) const;
	void EmitFragmentOutputs(std::ostringstream& os) const;

	std::string MainTemplate(Stage stage, MaterialPass pass) const;

	std::string GenDepthVS     (const std::string& code) const;
	std::string GenDepthFS     (const std::string& code) const;
	std::string GenSurfaceVS   (const std::string& code) const;
	std::string GenSurfaceFS   (const std::string& code) const;
	std::string GenLightingVS  (const std::string& code) const;
	std::string GenLightingFS  (const std::string& code) const;
	std::string GenPostProcessVS(const std::string& code) const;
	std::string GenPostProcessFS(const std::string& code) const;

	std::string LoadTemplate(const std::string& name) const;

	// ── Material data (set at construction) ──────────────────────────
	std::string m_MaterialName;
	Pipeline    m_Pipeline = Pipeline::DEFERRED;
	Shading		m_ShadingModel = Shading::LIT;
	MaterialDomain m_MaterialDomain = MaterialDomain::SURFACE;
	const BufferInterfaceBlock* m_MaterialUib = nullptr;
	const SamplerInterfaceBlock* m_MaterialSib = nullptr;
	RHI::AttributeBitset m_RequiredAttributes;

	// ── User-defined data ────────────────────────────────────────────
	MaterialBuilder::PropertyList               m_Properties;
	MaterialBuilder::VariableList             m_Variables;
	MaterialBuilder::OutputList            m_Outputs;
	MaterialBuilder::PreprocessorDefineList m_Defines;
	MaterialBuilder::PushConstantList      m_PushConstants;

	// ── User code ────────────────────────────────────────────────────
	std::string m_MaterialFragmentCode;
	std::string m_MaterialVertexCode;
	size_t m_MaterialLineOffset = 0;
	size_t m_MaterialVertexLineOffset = 0;
	bool m_IsMaterialVertexShaderEmpty = true;

	// ── Config ───────────────────────────────────────────────────────
	std::string m_TemplateDir = "Template";
	bool m_TargetVulkan = true;
};
