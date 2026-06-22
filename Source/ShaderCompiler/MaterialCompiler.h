#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class MaterialBuilder;
class MaterialLexeme;
class JsonishValue;
struct IncludeResult;

struct CompilerConfig
{
	std::string inputFile;
	std::string outputDir  = "CompiledMaterials";
	bool outputGlsl        = false;
	std::string templateDir = "Template";
	bool targetVulkan       = true;
	std::vector<std::string> includePaths;
	std::string glslcPath;
	std::string targetEnv   = "vulkan1.2";
	bool compileSpirv       = false;
};

class MaterialCompiler
{
public:
	MaterialCompiler();
	bool Run(const CompilerConfig& config);
	bool Build(MaterialBuilder& builder, const CompilerConfig& config);

private:
	static bool isValidJsonStart(const char* buffer, size_t size) noexcept;
	bool parseMaterialAsJSON(const char* buffer, size_t size, MaterialBuilder& builder, const CompilerConfig& config) const noexcept;
	bool parseMaterial(const char* buffer, size_t size, MaterialBuilder& builder, const CompilerConfig& config) const noexcept;
	bool readFile(const std::string& path, std::string& out) const noexcept;
	bool writeFile(const std::string& path, const uint8_t* data, size_t size) const noexcept;
	void writeGlslFiles(const uint8_t* data, size_t size, const std::string& name, const std::string& outputDir) const noexcept;
	std::string resolveMaterialName(const std::string& inputFile) const noexcept;
	void configureBuilder(MaterialBuilder& builder, const CompilerConfig& config) const noexcept;

	std::function<bool(const std::string&, IncludeResult&)> makeIncludeCallback(const CompilerConfig& config) const noexcept;
	std::function<bool(const std::string&, const std::string&, std::vector<uint8_t>&, std::vector<uint8_t>&)> makeSpirvCompiler(const CompilerConfig& config) const noexcept;

	using MaterialConfigProcessor = bool (MaterialCompiler::*)(const MaterialLexeme&, MaterialBuilder& builder) const;
	std::unordered_map<std::string, MaterialConfigProcessor> mConfigProcessor;
	bool processMaterial(const MaterialLexeme&, MaterialBuilder&) const noexcept;
	bool processVertexShader(const MaterialLexeme&, MaterialBuilder&) const noexcept;
	bool processFragmentShader(const MaterialLexeme&, MaterialBuilder&) const noexcept;
	bool processComputeShader(const MaterialLexeme&, MaterialBuilder&) const noexcept;
	bool ignoreLexeme(const MaterialLexeme&, MaterialBuilder&) const noexcept { return true; }

	using MaterialConfigProcessorJSON = bool (MaterialCompiler::*)(const JsonishValue*, MaterialBuilder& builder) const;
	std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;
	bool processMaterialJSON(const JsonishValue*, MaterialBuilder&) const noexcept;
	bool processVertexShaderJSON(const JsonishValue*, MaterialBuilder&) const noexcept;
	bool processFragmentShaderJSON(const JsonishValue*, MaterialBuilder&) const noexcept;
	bool processComputeShaderJSON(const JsonishValue*, MaterialBuilder&) const noexcept;
	bool ignoreLexemeJSON(const JsonishValue*, MaterialBuilder&) const noexcept { return true; }
};
