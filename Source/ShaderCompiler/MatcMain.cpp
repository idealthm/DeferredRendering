#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include "MaterialCompiler.h"
#include "Common/Material/MaterialBuilder.h"
#include "Common/Material/Package.h"
#include "Common/Serialization/MaterialChunks.h"
#include "EngineEnum.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void PrintUsage()
{
    std::cout <<
        "matc — Material Compiler\n"
        "Generates GLSL shaders from a material JSON specification.\n\n"
        "Usage:\n"
        "  matc.exe <filename> [options]\n\n"
        "Required:\n"
        "  <filename>            Path to the material JSON specification\n\n"
        "Options:\n"
        "  -I codeonly           Output GLSL source only (skip glslc invocation)\n"
        "  -E                    Preprocess only: run glslc -E, write preprocessed GLSL\n"
        "  -o <outputdir>        Output directory (default: CompiledMaterials/)\n"
        "  -w <workdir>          Working directory for relative path resolution\n"
        "  -O <level>            glslc optimization level (0..1, default: 0)\n"
        "  -g                    Generate debug info (glslc -g)\n"
        "  --target <env>        Target environment for glslc\n"
        "                        (e.g. vulkan1.2, opengl4.5; default: vulkan1.2)\n"
        "  --glslc <path>        Path to glslc executable\n"
        "                        (default: $VULKAN_SDK/Bin/glslc.exe or glslc.exe)\n"
        "  --include <path>      Additional include search path (repeatable)\n"
        "  --template-dir <path> Path to template directory (default: Template/)\n"
        "  -h, --help            Print usage and exit\n\n"
        "Output files (per stage):\n"
        "  .vert / .frag         Generated GLSL source\n"
        "  .vert.spv / .frag.spv SPIR-V binary (default path)\n"
        "  .preprocessed.*       Preprocessor output (with -E)\n";
}

static void EnsureDirectory(const std::string& path)
{
    std::string current;
    for (size_t i = 0; i < path.size(); ++i)
    {
        current += path[i];
        if (path[i] == '/' || path[i] == '\\' || i == path.size() - 1)
        {
            if (current.back() == '/' || current.back() == '\\')
                current.pop_back();
            if (!current.empty())
                CreateDirectoryA(current.c_str(), nullptr);
            current += '/';
        }
    }
}

static std::string GetDirectory(const std::string& filePath)
{
    size_t slash = filePath.find_last_of("/\\");
    if (slash == std::string::npos)
        return ".";
    return filePath.substr(0, slash);
}

// ============================================================
// Run glslc and capture output
// ============================================================
static bool RunGlslc(const std::string& glslcPath,
                     const std::string& inputFile,
                     const std::string& outputFile,
                     const std::string& stage,
                     const std::vector<std::string>& glslcArgs)
{
    std::string cmdLine = "\"" + glslcPath + "\"";
    cmdLine += " -fshader-stage=" + stage;
    cmdLine += " \"" + inputFile + "\"";
    if (!outputFile.empty())
        cmdLine += " -o \"" + outputFile + "\"";
    for (const auto& a : glslcArgs)
        cmdLine += " " + a;

    std::cout << "matc: glslc " << stage << ": " << inputFile;
    if (!outputFile.empty())
        std::cout << " -> " << outputFile;
    std::cout << '\n';

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hOutRead = nullptr, hOutWrite = nullptr;
    CreatePipe(&hOutRead, &hOutWrite, &sa, 0);
    SetHandleInformation(hOutRead, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION pi = {};
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.hStdOutput = hOutWrite;
    si.hStdError = hOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    std::vector<char> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back('\0');

    BOOL ok = CreateProcessA(
        nullptr, cmdBuf.data(),
        nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW,
        nullptr, nullptr,
        &si, &pi);

    CloseHandle(hOutWrite);

    std::string pipeOutput;
    if (ok)
    {
        CloseHandle(pi.hThread);
        char buf[4096];
        DWORD read;
        while (ReadFile(hOutRead, buf, sizeof(buf) - 1, &read, nullptr) && read > 0)
        {
            buf[read] = '\0';
            pipeOutput += buf;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(hOutRead);

        if (!pipeOutput.empty())
            std::cerr << pipeOutput;

        return exitCode == 0;
    }
    else
    {
        CloseHandle(hOutRead);
        std::cerr << "matc: error: failed to launch glslc: " << GetLastError() << '\n';
        return false;
    }
}

// ============================================================
// std140 layout helpers
// ============================================================
static void DumpMatb(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "matc: cannot open " << path << '\n';
		return;
	}
	const size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0);
	std::vector<uint8_t> buffer(size);
	file.read(reinterpret_cast<char*>(buffer.data()), size);

	FArchiveRead ar(buffer.data(), buffer.size());
	ChunkContainer cc;
	cc.Deserialize(ar);

	auto sep = [] { std::cout << "----------------------------------------\n"; };

	std::cout << "=== " << path << " (" << size << " bytes) ===\n\n";

	std::string name;
	if (cc.Get<ChunkName>(name) && !name.empty())
		std::cout << "Name:           " << name << '\n';

	uint32_t version = 0;
	if (cc.Get<ChunkVersion>(version))
		std::cout << "Version:        " << version << '\n';

	std::string shading;
	if (cc.Get<ChunkShading>(shading) && !shading.empty())
		std::cout << "Shading:        " << shading << '\n';

	uint8_t domain = 0;
	if (cc.Get<ChunkDomain>(domain))
		std::cout << "Domain:         " << (int)domain << '\n';

	uint32_t attrMask = 0;
	if (cc.Get<ChunkRequiredAttrs>(attrMask) && attrMask)
		std::cout << "ReqAttrs mask:  0x" << std::hex << attrMask << std::dec << '\n';

	// SPIR-V
	ChunkSpirv::Container spv;
	if (cc.Get<ChunkSpirv>(spv)) {
		std::cout << "[ChunkSpirv]  " << spv.size() << " entries\n";
		for (auto& e : spv)
			std::cout << "  " << (int)e.pass << ": vert=" << e.vertexSpirv.size() << "B  frag=" << e.fragmentSpirv.size() << "B\n";
	}

	// Raw hex of first 64 bytes of ChunkSpirv and ChunkGlsl payloads
	{
		sep();
		std::cout << "[Raw payload hex dump]\n";
		auto dumpHex = [&](const char* label) {
			// Re-deserialize to get raw data
			FArchiveRead rawAr(buffer.data(), buffer.size());
			ChunkContainer rawCc;
			rawCc.Deserialize(rawAr);
			std::cout << "  " << label << ":\n  ";
			int col = 0;
			for (size_t i = 0; i < min(buffer.size(), (size_t)64); ++i) {
				printf("%02X ", buffer[i]);
				if (++col == 16) { std::cout << "\n  "; col = 0; }
			}
			std::cout << std::endl;
		};
		dumpHex("file header");
	}

	// UIB
	BufferInterfaceBlock uib;
	if (cc.Get<ChunkUib>(uib))
	{
		sep();
		std::cout << "[ChunkUib]  " << uib.getName()
		          << "  size=" << uib.getSize() << '\n';
		for (auto& f : uib.getFieldInfoList())
			std::cout << "  " << f.name << "  off=" << f.offset << "  type=" << (int)f.type
			          << "  stride=" << (int)f.stride << "  struct=" << f.structName << '\n';
	}

	// SIB
	SamplerInterfaceBlock sib;
	if (cc.Get<ChunkSib>(sib))
	{
		sep();
		std::cout << "[ChunkSib]  " << sib.getName() << '\n';
		for (auto& s : sib.getSamplerInfoList())
			if (!s.name.empty())
				std::cout << "  " << s.name << "  binding=" << (int)s.binding
				          << "  type=" << (int)s.type << '\n';
	}

	// DescriptorSetBindings
	DescriptorSetInfo dsl;
	if (cc.Get<ChunkDescriptorSetBindings>(dsl))
	{
		sep();
		std::cout << "[ChunkDescriptorSetBindings]\n";
		for (size_t set = 0; set < dsl.size(); set++)
		{
			if (dsl[set].empty()) continue;
			const char* setName = "?";
			switch (set) {
			case static_cast<size_t>(DescriptorSetBindingPoints::PER_VIEW):       setName = "PER_VIEW"; break;
			case static_cast<size_t>(DescriptorSetBindingPoints::PER_RENDERABLE): setName = "PER_RENDERABLE"; break;
			case static_cast<size_t>(DescriptorSetBindingPoints::PER_MATERIAL):   setName = "PER_MATERIAL"; break;
			case static_cast<size_t>(DescriptorSetBindingPoints::G_BUFFER):       setName = "G_BUFFER"; break;
			}
			std::cout << "  set=" << set << " (" << setName << ")\n";
			for (auto& d : dsl[set])
				std::cout << "    " << d.name << "  binding=" << (int)d.binding
				          << "  type=" << (int)d.type << '\n';
		}
	}

	// DescriptorSetLayout
	std::array<RHI::DescriptorSetLayout, MAX_DESCRIPTOR_SET_COUNT> descLayouts;
	if (cc.Get<ChunkDescriptorSetLayout>(descLayouts))
	{
		sep();
		std::cout << "[ChunkDescriptorSetLayout]\n";
		for (size_t li = 0; li < descLayouts.size(); li++)
		{
			if (descLayouts[li].bindings.empty()) continue;
			const char* setName = "?";
			switch (li) {
			case static_cast<size_t>(DescriptorSetBindingPoints::PER_VIEW):       setName = "PER_VIEW"; break;
			case static_cast<size_t>(DescriptorSetBindingPoints::PER_RENDERABLE): setName = "PER_RENDERABLE"; break;
			case static_cast<size_t>(DescriptorSetBindingPoints::PER_MATERIAL):   setName = "PER_MATERIAL"; break;
			case static_cast<size_t>(DescriptorSetBindingPoints::G_BUFFER):       setName = "G_BUFFER"; break;
			}
			std::cout << "  layout[" << li << "] (" << setName << ")\n";
			for (auto& b : descLayouts[li].bindings)
			{
				if (b.count == 0) continue;
				std::cout << "    binding=" << (int)b.binding
				          << "  type=" << (int)b.type
				          << "  stage=" << (int)b.stageFlags
				          << "  count=" << b.count << '\n';
			}
		}
	}

	// MaterialAttributesInfo — mesh vertex attribute mapping
	ChunkMaterialAttributesInfo::Container attrInfo;
	if (cc.Get<ChunkMaterialAttributesInfo>(attrInfo))
	{
		sep();
		std::cout << "[MaterialAttributesInfo]  " << attrInfo.size() << " entries\n";
		for (auto& [name, loc] : attrInfo)
			std::cout << "  " << name << "  loc=" << (int)loc << '\n';
	}

	// AttributeInputOutput — vertex inputs + fragment outputs
	ChunkAttributeInputOutput::Container attrIO;
	if (cc.Get<ChunkAttributeInputOutput>(attrIO))
	{
		sep();
		std::cout << "[AttributeInputOutput]\n";
		std::cout << "  Vertex inputs:\n";
		for (auto& v : attrIO.inputs)
			std::cout << "    " << v.name << "  loc=" << (int)v.location << "  type=" << (int)v.type << '\n';
		std::cout << "  Fragment outputs:\n";
		for (auto& v : attrIO.outputs)
			std::cout << "    " << v.name << "  loc=" << (int)v.location << "  type=" << (int)v.type << '\n';
	}

	// Properties / Constants
	std::vector<FlatProperty> props;
	if (cc.Get<ChunkProperties>(props))
	{
		sep();
		std::cout << "[ChunkProperties]  " << props.size() << " entries\n";
		for (auto& p : props)
			std::cout << "  " << p.name << "  uniformType=" << (int)p.uniformType << '\n';
	}
	std::vector<std::string> consts;
	if (cc.Get<ChunkConstants>(consts))
	{
		sep();
		std::cout << "[ChunkConstants]  " << consts.size() << " entries\n";
		for (auto& c : consts)
			std::cout << "  " << c << '\n';
	}

	sep();
}

static const char* PassName(MaterialPass p)
{
	switch (p) {
	case MaterialPass::Depth:       return "Depth";
	case MaterialPass::Surface:     return "Surface";
	case MaterialPass::Lighting:    return "Lighting";
	case MaterialPass::PostProcess: return "PostProcess";
	case MaterialPass::Compute:     return "Compute";
	default: return "Unknown";
	}
}

static void DumpGlsl(const std::string& path, const std::string& outDir)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "matc: cannot open " << path << '\n';
		return;
	}
	const size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0);
	std::vector<uint8_t> buffer(size);
	file.read(reinterpret_cast<char*>(buffer.data()), size);

	FArchiveRead ar(buffer.data(), buffer.size());
	ChunkContainer cc;
	cc.Deserialize(ar);

	std::string name;
	cc.Get<ChunkName>(name);

	ChunkGlsl::Container glslEntries;
	if (!cc.Get<ChunkGlsl>(glslEntries) || glslEntries.empty())
	{
		std::cerr << "matc: no GLSL data found in " << path << '\n';
		return;
	}

	std::string baseName = name.empty() ? "material" : name;
	std::string dir = outDir.empty() ? "." : outDir;

	for (auto& e : glslEntries)
	{
		const char* pn = PassName(e.pass);
		std::string vertFile = dir + "/" + baseName + "_" + pn + ".vert";
		std::string fragFile = dir + "/" + baseName + "_" + pn + ".frag";

		{
			std::ofstream f(vertFile);
			if (f) { f << e.vertexGlsl; std::cout << "matc: wrote " << vertFile << " (" << e.vertexGlsl.size() << " bytes)\n"; }
		}
		{
			std::ofstream f(fragFile);
			if (f) { f << e.fragmentGlsl; std::cout << "matc: wrote " << fragFile << " (" << e.fragmentGlsl.size() << " bytes)\n"; }
		}
	}
}

static int BuildLightingMaterial(const CompilerConfig& config)
{
    std::cout << "matc: building Lighting material" << std::endl;
    MaterialCompiler compiler;
    return compiler.CompileLighting(config) ? 0 : 1;
}
// ============================================================

int main(int Argc, char* Argv[])
{
    CompilerConfig config;
    std::string workDir;
    std::string dumpPath;
    std::string dumpGlslPath;
    bool codeOnly = false;

    { char* vk = nullptr; _dupenv_s(&vk, nullptr, "VULKAN_SDK"); config.glslcPath = vk ? std::string(vk) + "/Bin/glslc.exe" : "glslc.exe"; free(vk); }

    for (int i = 1; i < Argc; ++i) {
        std::string arg = Argv[i];
        if (arg == "-I" && i + 1 < Argc) { std::string m = Argv[++i]; if (m == "codeonly") codeOnly = true; else { std::cerr << "matc: unknown -I mode\n"; return 1; } }
        else if (arg == "-o" && i + 1 < Argc) config.outputDir = Argv[++i];
        else if (arg == "-w" && i + 1 < Argc) workDir = Argv[++i];
        else if (arg == "--target" && i + 1 < Argc) config.targetEnv = Argv[++i];
        else if (arg == "--glslc" && i + 1 < Argc) config.glslcPath = Argv[++i];
        else if (arg == "--include" && i + 1 < Argc) config.includePaths.push_back(Argv[++i]);
        else if (arg == "--dump" && i + 1 < Argc) dumpPath = Argv[++i];
        else if (arg == "--dump-glsl" && i + 1 < Argc) { dumpGlslPath = Argv[++i]; if (i + 1 < Argc && Argv[i + 1][0] != '-') config.outputDir = Argv[++i]; }
        else if (arg == "--template-dir" && i + 1 < Argc) config.templateDir = Argv[++i];
        else if (arg == "-h" || arg == "--help") { PrintUsage(); return 0; }
        else if (!arg.empty() && arg[0] != '-') config.inputFile = arg;
        else { std::cerr << "matc: unknown argument: " << arg << '\n'; PrintUsage(); return 1; }
    }

    if (!dumpPath.empty()) { DumpMatb(dumpPath); return 0; }
    if (!dumpGlslPath.empty()) { DumpGlsl(dumpGlslPath, config.outputDir); return 0; }
    if (config.inputFile.empty()) { std::cerr << "matc: missing input filename\n"; PrintUsage(); return 1; }

    config.compileSpirv = !codeOnly;

    if (workDir.empty()) workDir = GetDirectory(config.inputFile);
    while (!workDir.empty() && (workDir.back() == '/' || workDir.back() == '\\')) workDir.pop_back();
    if (config.includePaths.empty()) { config.includePaths.push_back(workDir + "/Shaders"); config.includePaths.push_back(workDir); }

    if (config.inputFile == "__lighting__") { return BuildLightingMaterial(config); }

    MaterialCompiler compiler;
    return compiler.Run(config) ? 0 : 1;
}
