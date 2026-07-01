#pragma once
#include <string>

#include "Common/Handle.h"

namespace RHI { struct HwProgram; }

class LightingShaderLibrary
{
public:
	LightingShaderLibrary() = default;

	void Init(const std::string& compiledDir);
	Handle<RHI::HwProgram> GetProgram();
	void SetCompiledDir(const std::string& dir) { m_CompiledDir = dir; }

private:
	Handle<RHI::HwProgram> m_Program;
	std::string m_CompiledDir = "CompiledMaterials";
};
