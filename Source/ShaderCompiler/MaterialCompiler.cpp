#include "MaterialCompiler.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Common/Material/MaterialBuilder.h"
#include "Common/Material/Package.h"
#include "IncludeCallbaks.h"
#include "Lexer/JsonishLexer.h"
#include "Lexer/MaterialLexeme.h"
#include "Lexer/MaterialLexer.h"
#include "ParameterProcessor.h"
#include "Parser/JsonishParser.h"

static constexpr const char* CK_MATERIAL  = "material";
static constexpr const char* CK_VERTEX    = "vertex";
static constexpr const char* CK_FRAGMENT  = "fragment";
static constexpr const char* CK_COMPUTE   = "compute";
static constexpr const char* CK_TOOL      = "tool";

// =============================================================================
// Constructor
// =============================================================================

MaterialCompiler::MaterialCompiler()
{
	mConfigProcessor[CK_MATERIAL]     = &MaterialCompiler::processMaterial;
	mConfigProcessor[CK_VERTEX]       = &MaterialCompiler::processVertexShader;
	mConfigProcessor[CK_FRAGMENT]     = &MaterialCompiler::processFragmentShader;
	mConfigProcessor[CK_COMPUTE]      = &MaterialCompiler::processComputeShader;
	mConfigProcessor[CK_TOOL]         = &MaterialCompiler::ignoreLexeme;

	mConfigProcessorJSON[CK_MATERIAL]  = &MaterialCompiler::processMaterialJSON;
	mConfigProcessorJSON[CK_VERTEX]    = &MaterialCompiler::processVertexShaderJSON;
	mConfigProcessorJSON[CK_FRAGMENT]  = &MaterialCompiler::processFragmentShaderJSON;
	mConfigProcessorJSON[CK_COMPUTE]   = &MaterialCompiler::processComputeShaderJSON;
	mConfigProcessorJSON[CK_TOOL]      = &MaterialCompiler::ignoreLexemeJSON;
}

// =============================================================================
// Helpers
// =============================================================================

bool MaterialCompiler::isValidJsonStart(const char* buffer, size_t size) noexcept
{
	for (size_t i = 0; i < size; ++i)
	{
		char c = buffer[i];
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			continue;
		return c == '{';
	}
	return false;
}

bool MaterialCompiler::readFile(const std::string& path, std::string& out) const noexcept
{
	std::ifstream f(path, std::ios::binary | std::ios::ate);
	if (!f.is_open())
		return false;
	out.resize(static_cast<size_t>(f.tellg()));
	f.seekg(0);
	f.read(out.data(), out.size());
	return true;
}

bool MaterialCompiler::writeFile(const std::string& path, const uint8_t* data, size_t size) const noexcept
{
	std::ofstream f(path, std::ios::binary);
	if (!f.is_open())
		return false;
	f.write(reinterpret_cast<const char*>(data), size);
	return true;
}

void MaterialCompiler::configureBuilder(MaterialBuilder& builder, const CompilerConfig& config) const noexcept
{
	if (!config.includePaths.empty())
		builder.includeCallback(makeIncludeCallback(config));
	if (config.compileSpirv)
		builder.spirvCompiler(makeSpirvCompiler(config));
}

// =============================================================================
// Include resolver
// =============================================================================

std::function<bool(const std::string&, IncludeResult&)>
MaterialCompiler::makeIncludeCallback(const CompilerConfig& config) const noexcept
{
	auto paths = config.includePaths;
	return [paths](const std::string&, IncludeResult& result) -> bool
	{
		for (auto& dir : paths)
		{
			std::string fullPath = dir + "/" + result.includeName;
			std::ifstream f(fullPath, std::ios::binary | std::ios::ate);
			if (!f.is_open())
				continue;

			result.text.resize(static_cast<size_t>(f.tellg()));
			f.seekg(0);
			f.read(result.text.data(), result.text.size());
			result.name = fullPath;
			return true;
		}
		return false;
	};
}

// =============================================================================
// SPIR-V compiler (glslc wrapper)
// =============================================================================

std::function<bool(const std::string&, const std::string&,
                   std::vector<uint8_t>&, std::vector<uint8_t>&)>
MaterialCompiler::makeSpirvCompiler(const CompilerConfig& config) const noexcept
{
	std::string glslcPath = config.glslcPath;
	return [glslcPath](const std::string& vertSrc, const std::string& fragSrc,
	                   std::vector<uint8_t>& vertSpv, std::vector<uint8_t>& fragSpv) -> bool
	{
		static int counter = 0;

		auto compile = [&](const std::string& src, const std::string& stage,
		                   std::vector<uint8_t>& spv) -> bool
		{
			int id = counter++;
			std::string tmpIn  = "_matc_tmp_" + std::to_string(id) + "." + stage;
			std::string tmpOut = tmpIn + ".spv";

			{
				std::ofstream f(tmpIn);
				if (!f)
					return false;
				f << src;
			}

			std::string cmd = glslcPath + " -fshader-stage=" + stage
			                + " --target-env=opengl"
			                + " " + tmpIn + " -o " + tmpOut;

			int ret = system(cmd.c_str());
			if (ret != 0)
				std::cerr << "glslc[" << stage << "]: compilation failed (exit " << ret << ")" << std::endl;

			if (ret == 0)
			{
				std::ifstream f(tmpOut, std::ios::binary | std::ios::ate);
				if (f)
				{
					spv.resize(static_cast<size_t>(f.tellg()));
					f.seekg(0);
					f.read(reinterpret_cast<char*>(spv.data()), spv.size());
				}
			}

			std::remove(tmpIn.c_str());
			std::remove(tmpOut.c_str());
			return ret == 0 && !spv.empty();
		};

		return compile(vertSrc, "vert", vertSpv)
		    && compile(fragSrc, "frag", fragSpv);
	};
}

// =============================================================================
// Run — file-based compilation
// =============================================================================

bool MaterialCompiler::Run(const CompilerConfig& config)
{
	if (config.inputFile.empty())
	{
		std::cerr << "matc: error: no input file specified" << std::endl;
		return false;
	}

	std::string source;
	if (!readFile(config.inputFile, source))
	{
		std::cerr << "matc: error: cannot open input file: " << config.inputFile << std::endl;
		return false;
	}

	std::cout << "matc: reading " << config.inputFile
	          << " (" << source.size() << " bytes)" << std::endl;

	MaterialBuilder builder;
	builder.fileName(config.inputFile.c_str());
	configureBuilder(builder, config);

	bool ok = isValidJsonStart(source.data(), source.size())
		? parseMaterialAsJSON(source.data(), source.size(), builder, config)
		: parseMaterial(source.data(), source.size(), builder, config);

	if (!ok)
	{
		std::cerr << "matc: error: failed to parse material" << std::endl;
		return false;
	}

	Package pkg = Package::invalidPackage();
	try
	{
		pkg = builder.build();
	}
	catch (const std::exception& e)
	{
		std::cerr << "matc: exception: " << e.what() << std::endl;
		return false;
	}

	if (!pkg.isValid())
	{
		std::cerr << "matc: error: failed to build material" << std::endl;
		return false;
	}

	std::string outputDir = config.outputDir.empty() ? "CompiledMaterials" : config.outputDir;
	std::string materialName = config.inputFile;
	{
		auto pos = materialName.find_last_of("/\\");
		if (pos != std::string::npos)
			materialName = materialName.substr(pos + 1);
		pos = materialName.rfind('.');
		if (pos != std::string::npos)
			materialName = materialName.substr(0, pos);
		while (!materialName.empty() && materialName.front() == '_')
			materialName.erase(0, 1);
		while (!materialName.empty() && materialName.back() == '_')
			materialName.pop_back();
	}
	if (materialName.empty())
		materialName = "material";

	std::string matbPath = outputDir + "/" + materialName + ".matb";
	if (!writeFile(matbPath, pkg.getData(), pkg.getSize()))
	{
		std::cerr << "matc: error: cannot write output file: " << matbPath << std::endl;
		return false;
	}

	std::cout << "matc: wrote " << matbPath << " (" << pkg.getSize() << " bytes)" << std::endl;
	return true;
}

// =============================================================================
// Build — programmatic compilation
// =============================================================================

bool MaterialCompiler::Build(MaterialBuilder& builder, const CompilerConfig& config)
{
	configureBuilder(builder, config);

	Package pkg = Package::invalidPackage();
	try
	{
		pkg = builder.build();
	}
	catch (const std::exception& e)
	{
		std::cerr << "matc: exception: " << e.what() << std::endl;
		return false;
	}

	if (!pkg.isValid())
	{
		std::cerr << "matc: error: failed to build material" << std::endl;
		return false;
	}

	std::string outputDir = config.outputDir.empty() ? "CompiledMaterials" : config.outputDir;
	std::string materialName = config.inputFile;
	{
		auto pos = materialName.find_last_of("/\\");
		if (pos != std::string::npos)
			materialName = materialName.substr(pos + 1);
		pos = materialName.rfind('.');
		if (pos != std::string::npos)
			materialName = materialName.substr(0, pos);
		while (!materialName.empty() && materialName.front() == '_')
			materialName.erase(0, 1);
		while (!materialName.empty() && materialName.back() == '_')
			materialName.pop_back();
	}
	if (materialName.empty())
		materialName = "material";

	std::string matbPath = outputDir + "/" + materialName + ".matb";
	if (!writeFile(matbPath, pkg.getData(), pkg.getSize()))
	{
		std::cerr << "matc: error: cannot write output file: " << matbPath << std::endl;
		return false;
	}

	std::cout << "matc: wrote " << matbPath << " (" << pkg.getSize() << " bytes)" << std::endl;
	return true;
}

// =============================================================================
// Block-format parsing
// =============================================================================

bool MaterialCompiler::parseMaterial(const char* buffer, size_t size,
                                     MaterialBuilder& builder,
                                     const CompilerConfig&) const noexcept
{
	MaterialLexer materialLexer;
	materialLexer.Lex(buffer, size);
	auto& lexemes = materialLexer.getLexemes();

	for (auto& lexeme : lexemes)
	{
		if (lexeme.getType() == MaterialType::UNKNOWN)
			return false;
	}

	if (lexemes.size() < 2)
		return false;

	for (size_t i = 0; i < lexemes.size(); i += 2)
	{
		if (i == lexemes.size() - 1)
			return false;

		auto& idLexeme    = lexemes[i];
		auto& blockLexeme = lexemes[i + 1];

		if (idLexeme.getType() != MaterialType::IDENTIFIER)
			return false;
		if (blockLexeme.getType() != MaterialType::BLOCK)
			return false;

		std::string identifier = idLexeme.getStringValue();
		auto it = mConfigProcessor.find(identifier);
		if (it == mConfigProcessor.end())
		{
			std::cerr << "matc: unknown identifier '" << identifier << "'" << std::endl;
			return false;
		}

		if (!(*this.*(it->second))(blockLexeme, builder))
		{
			std::cerr << "matc: error processing block '" << identifier << "'" << std::endl;
			return false;
		}
	}

	return true;
}

bool MaterialCompiler::parseMaterialAsJSON(const char* buffer, size_t size,
                                           MaterialBuilder& builder,
                                           const CompilerConfig&) const noexcept
{
	JsonishLexer jlexer;
	jlexer.Lex(buffer, size, 1);
	JsonishParser parser(jlexer.getLexemes());

	auto json = parser.parse();
	if (!json)
	{
		std::cerr << "matc: could not parse JSON material file" << std::endl;
		return false;
	}

	for (auto& entry : json->getEntries())
	{
		const std::string& key = entry.first;
		auto it = mConfigProcessorJSON.find(key);
		if (it == mConfigProcessorJSON.end())
		{
			std::cerr << "matc: unknown key '" << key << "'" << std::endl;
			return false;
		}

		if (!(*this.*(it->second))(entry.second, builder))
		{
			std::cerr << "matc: error processing key '" << key << "'" << std::endl;
			return false;
		}
	}

	return true;
}

// =============================================================================
// Block-format processors
// =============================================================================

bool MaterialCompiler::processMaterial(const MaterialLexeme& lexeme,
                                       MaterialBuilder& builder) const noexcept
{
	std::string raw = lexeme.getStringValue();

	JsonishLexer jlexer;
	jlexer.Lex(raw.c_str(), raw.size(), lexeme.getLine());

	auto json = JsonishParser(jlexer.getLexemes()).parse();
	if (!json) return false;

	const auto* obj = json->toJsonObject();
	if (!obj) return false;

	ParametersProcessor processor;
	return processor.process(builder, *obj);
}

bool MaterialCompiler::processVertexShader(const MaterialLexeme& lexeme,
                                           MaterialBuilder& builder) const noexcept
{
	auto trimmed = lexeme.trimBlockMarkers();
	builder.materialVertex(trimmed.getStringValue().c_str(), trimmed.getLine());
	return true;
}

bool MaterialCompiler::processFragmentShader(const MaterialLexeme& lexeme,
                                             MaterialBuilder& builder) const noexcept
{
	auto trimmed = lexeme.trimBlockMarkers();
	builder.material(trimmed.getStringValue().c_str(), trimmed.getLine());
	return true;
}

bool MaterialCompiler::processComputeShader(const MaterialLexeme& lexeme,
                                            MaterialBuilder& builder) const noexcept
{
	auto trimmed = lexeme.trimBlockMarkers();
	builder.material(trimmed.getStringValue().c_str(), trimmed.getLine());
	return true;
}

// =============================================================================
// JSON-format processors
// =============================================================================

bool MaterialCompiler::processMaterialJSON(const JsonishValue* value,
                                           MaterialBuilder& builder) const noexcept
{
	if (!value || value->getType() != JsonishValue::OBJECT)
		return false;

	ParametersProcessor processor;
	return processor.process(builder, *value->toJsonObject());
}

bool MaterialCompiler::processVertexShaderJSON(const JsonishValue* value,
                                               MaterialBuilder& builder) const noexcept
{
	if (!value)
		return false;
	auto* str = value->toJsonString();
	if (!str)
		return false;
	builder.materialVertex(str->getString().c_str());
	return true;
}

bool MaterialCompiler::processFragmentShaderJSON(const JsonishValue* value,
                                                 MaterialBuilder& builder) const noexcept
{
	if (!value)
		return false;
	auto* str = value->toJsonString();
	if (!str)
		return false;
	builder.material(str->getString().c_str());
	return true;
}

bool MaterialCompiler::processComputeShaderJSON(const JsonishValue* value,
                                                MaterialBuilder& builder) const noexcept
{
	if (!value)
		return false;
	auto* str = value->toJsonString();
	if (!str)
		return false;
	builder.material(str->getString().c_str());
	return true;
}
