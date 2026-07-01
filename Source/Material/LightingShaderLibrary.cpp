#include "LightingShaderLibrary.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "Common/Serialization/ChunkContainer.h"
#include "Common/Serialization/MaterialChunks.h"
#include "Engine.h"
#include "RHI/RHIDriver.h"
#include "Shader/Program.h"

void LightingShaderLibrary::Init(const std::string& compiledDir)
{
	m_CompiledDir = compiledDir;
}

Handle<RHI::HwProgram> LightingShaderLibrary::GetProgram()
{
	if (m_Program)
		return m_Program;

	std::string path = m_CompiledDir + "/Lighting.matb";

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "LightingShaderLibrary: missing " << path << std::endl;
		return {};
	}

	const size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0);
	std::vector<uint8_t> buffer(size);
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

	FArchiveRead ar(buffer.data(), buffer.size());
	ChunkContainer cc;
	cc.Deserialize(ar);

	ChunkSpirv::Container spirvData;
	if (!cc.Get<ChunkSpirv>(spirvData) || spirvData.empty())
	{
		std::cerr << "LightingShaderLibrary: no SPIR-V in " << path << std::endl;
		return {};
	}

	auto& entry = spirvData[0];
	if (entry.vertexSpirv.empty() || entry.fragmentSpirv.empty())
	{
		std::cerr << "LightingShaderLibrary: empty SPIR-V in " << path << std::endl;
		return {};
	}

	DescriptorSetInfo descInfo{};
	cc.Get<ChunkDescriptorSetBindings>(descInfo);

	Program::ShaderSource source = { entry.vertexSpirv, entry.fragmentSpirv };
	m_Program = gEngine->GetDriver().CreateProgram(Program{ source, descInfo });

	std::cout << "LightingShaderLibrary: loaded Lighting.matb (" << size << " bytes)" << std::endl;
	return m_Program;
}
