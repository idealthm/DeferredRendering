#pragma once

#include <sstream>
#include <string>

#include "CodeGenerator.h"
#include "EngineEnum.h"
#include "MaterialSpec.h"

struct MaterialSpec;
enum class ShaderStage;

class ShaderGenerator
{
public:
	/// Configure template directory.  Must be called before generation.
	void SetTemplateDirectory(const std::string& dir) { m_TemplateDir = dir; }
	void SetTargetVulkan(bool vulkan)                { m_TargetVulkan = vulkan; }

	std::string GenerateVertexShader(
		const MaterialSpec& spec,
		const std::string& expandedVertexCode) const;

	std::string GenerateFragmentShader(
		const MaterialSpec& spec,
		const std::string& expandedFragmentCode) const;

private:
	// ── Shared header ─────────────────────────────────────────────────
	void EmitHeader(std::ostringstream& os, CodeGenerator& cg,
	                const MaterialSpec& spec) const;

	// ── Template loading ──────────────────────────────────────────────
	std::string LoadTemplate(const std::string& name) const;

	std::string m_TemplateDir = "Template";
	bool m_TargetVulkan = true;
};
