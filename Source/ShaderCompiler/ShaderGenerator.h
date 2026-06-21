#pragma once
#include <sstream>
#include <string>

#include "CodeGenerator.h"
#include "MaterialSpec.h"
#include "Common/Material/MaterialBuilder.h"

class ShaderGenerator
{
public:
	enum class Stage : uint8_t { Vertex, Fragment, Compute };
	enum class Pass  : uint8_t { Depth, GBuffer, Lighting };

	ShaderGenerator() = default;
	ShaderGenerator(MaterialBuilder::PropertyList const& properties,
		MaterialBuilder::VariableList const& variables, MaterialBuilder::OutputList const& outputs,
		MaterialBuilder::PreprocessorDefineList const& defines, MaterialBuilder::PushConstantList const& pushConstants,
		std::string const& materialCode, size_t lineOffset, std::string const& materialVertexCode, size_t vertexLineOffset,
		MaterialBuilder::MaterialDomain materialDomain);

	void SetTemplateDirectory(const std::string& dir) { m_TemplateDir = dir; }
	void SetTargetVulkan(bool vulkan)                { m_TargetVulkan = vulkan; }

	std::string GenerateShader(Stage stage, Pass pass, const MaterialSpec& spec, const std::string& userCode) const;
	std::string GeneratePostProcessShader(Stage stage, const MaterialSpec& spec, const std::string& userCode) const;
	std::string GenerateComputeShader(const MaterialSpec& spec, const std::string& userCode) const;

private:
	void EmitHeader(std::ostringstream& os, CodeGenerator& cg, const MaterialSpec& spec) const;
	void EmitLightHeader(std::ostringstream& os) const;
	void EmitVertexInputs(std::ostringstream& os, const MaterialSpec& spec) const;
	void EmitVaryingOut(std::ostringstream& os) const;
	void EmitVaryingIn(std::ostringstream& os) const;
	void EmitFragmentOutputs(std::ostringstream& os, const MaterialSpec& spec) const;

	std::string GenDepthVS   (const MaterialSpec& spec, const std::string& code) const;
	std::string GenDepthFS   (const MaterialSpec& spec, const std::string& code) const;
	std::string GenGBufferVS (const MaterialSpec& spec, const std::string& code) const;
	std::string GenGBufferFS (const MaterialSpec& spec, const std::string& code) const;
	std::string GenLightingVS(const MaterialSpec& spec, const std::string& code) const;
	std::string GenLightingFS(const MaterialSpec& spec, const std::string& code) const;
	std::string GenPostProcessVS(const MaterialSpec& spec, const std::string& code) const;
	std::string GenPostProcessFS(const MaterialSpec& spec, const std::string& code) const;

	std::string LoadTemplate(const std::string& name) const;
	std::string MainTemplate(Stage stage, Pass pass, Pipeline pipeline) const;

	std::string m_TemplateDir = "Template";
	bool m_TargetVulkan = true;

	MaterialBuilder::PropertyList m_Properties;
	MaterialBuilder::VariableList m_Variables;
	MaterialBuilder::OutputList m_Outputs;
	MaterialBuilder::MaterialDomain m_MaterialDomain;
	MaterialBuilder::PreprocessorDefineList m_Defines;
	MaterialBuilder::PushConstantList m_PushConstants;
	std::string m_MaterialFragmentCode, m_MaterialVertexCode;
	size_t m_MaterialLineOffset, m_MaterialVertexLineOffset;
	bool m_IsMaterialVertexShaderEmpty;
};
