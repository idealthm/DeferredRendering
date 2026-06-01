#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "IncludeExpander.h"
#include "MaterialSpec.h"
#include "ShaderGenerator.h"
#include "Common/Serialization/MaterialChunks.h"
#include "EngineEnum.h"
#include "ShaderInputBuilder.h"

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
        "                        (default: C:\\VulkanSDK\\1.3.216.0\\Bin\\glslc.exe)\n"
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
struct Std140Info { uint32_t size; uint32_t alignment; };

static Std140Info GetStd140Info(UniformType t)
{
	switch (t)
	{
	case UniformType::BOOL:  return {4, 4};
	case UniformType::BOOL2: return {8, 8};
	case UniformType::BOOL3: return {12, 16};
	case UniformType::BOOL4: return {16, 16};
	case UniformType::FLOAT: return {4, 4};
	case UniformType::FLOAT2: return {8, 8};
	case UniformType::FLOAT3: return {12, 16};
	case UniformType::FLOAT4: return {16, 16};
	case UniformType::INT:   return {4, 4};
	case UniformType::INT2:  return {8, 8};
	case UniformType::INT3:  return {12, 16};
	case UniformType::INT4:  return {16, 16};
	case UniformType::UINT:  return {4, 4};
	case UniformType::UINT2: return {8, 8};
	case UniformType::UINT3: return {12, 16};
	case UniformType::UINT4: return {16, 16};
	case UniformType::MAT3:  return {48, 16};
	case UniformType::MAT4:  return {64, 16};
	case UniformType::STRUCT: return {0, 0};
	}
	return {0, 0};
}

static FieldType UniformTypeToFieldType(UniformType t)
{
	switch (t)
	{
	case UniformType::BOOL:  return FieldType::BOOL;
	case UniformType::BOOL2: return FieldType::BOOL2;
	case UniformType::BOOL3: return FieldType::BOOL3;
	case UniformType::BOOL4: return FieldType::BOOL4;
	case UniformType::FLOAT: return FieldType::FLOAT;
	case UniformType::FLOAT2: return FieldType::FLOAT2;
	case UniformType::FLOAT3: return FieldType::FLOAT3;
	case UniformType::FLOAT4: return FieldType::FLOAT4;
	case UniformType::INT:   return FieldType::INT;
	case UniformType::INT2:  return FieldType::INT2;
	case UniformType::INT3:  return FieldType::INT3;
	case UniformType::INT4:  return FieldType::INT4;
	case UniformType::UINT:  return FieldType::UINT;
	case UniformType::UINT2: return FieldType::UINT2;
	case UniformType::UINT3: return FieldType::UINT3;
	case UniformType::UINT4: return FieldType::UINT4;
	case UniformType::MAT3:  return FieldType::MAT3;
	case UniformType::MAT4:  return FieldType::MAT4;
	case UniformType::STRUCT: return FieldType::STRUCT;
	}
	return FieldType::FLOAT;
}

static uint32_t ComputeStd140Size(UniformType t, const std::string& structName)
{
	if (t == UniformType::STRUCT)
		return 0; // struct size unknown at compile time — placeholder
	auto info = GetStd140Info(t);
	return info.size;
}

// ============================================================
// Build structured objects from MaterialSpec
// ============================================================
static void BuildUib(MaterialSpec& spec)
{
	BufferInterfaceBlock::Builder builder;
	builder.name("MaterialParams").alignment(BufferInterfaceBlock::Alignment::std140);

	for (auto& p : spec.properties)
	{
		if (p.kind != PropertyParam::Kind::Uniform) continue;
		Std140Info info = GetStd140Info(p.uniformType);
		builder.add({{ p.name, 0, p.uniformType, std::string(p.structName), static_cast<uint8_t>(info.size), {} }});
	}
	spec.materialUib = builder.build();
}

static void BuildSib(MaterialSpec& spec)
{
	SamplerInterfaceBlock::Builder builder;
	builder.name("materialParams");

	descriptor_binding_t binding = 1;
	for (auto& p : spec.properties)
	{
		if (p.kind != PropertyParam::Kind::Sampler) continue;
		builder.add(p.name, binding++, p.samplerType, RHI::SamplerFormat::FLOAT, false);
	}
	spec.materialSib = builder.build();
}

// ============================================================
// Write .matb binary file
// ============================================================

static const char* ToString(MaterialDomain domain)
{
	switch (domain)
	{
	case MaterialDomain::SURFACE:      return "surface";
	case MaterialDomain::POST_PROCESS: return "postprocess";
	case MaterialDomain::COMPUTE:      return "compute";
	}
	return "unknown";
}

static void WriteMaterialBinary(const std::string& fullOutputDir,
                                MaterialSpec& spec,
                                const std::string& vertSource,
                                const std::string& fragSource,
                                bool hasSpv)
{
	// ── Build SPIR-V data ────────────────────────────────────────────
	ChunkSpirv::Container spirvData{};
	if (hasSpv)
	{
		auto readSpv = [](const std::string& path, std::vector<uint8_t>& out)
		{
			std::ifstream f(path, std::ios::binary | std::ios::ate);
			if (!f) return;
			out.resize(static_cast<size_t>(f.tellg()));
			f.seekg(0);
			f.read(reinterpret_cast<char*>(out.data()), out.size());
		};
		readSpv(fullOutputDir + "/" + spec.name + ".vert.spv", spirvData.vertexSpirv);
		readSpv(fullOutputDir + "/" + spec.name + ".frag.spv", spirvData.fragmentSpirv);
	}

	// ── Build GLSL data ──────────────────────────────────────────────
	ChunkGlsl::Container glslData{ vertSource, fragSource };

	// ── Domain ───────────────────────────────────────────────────────
	uint8_t domain = 0;
	if (spec.domain == MaterialDomain::SURFACE)      domain = 0;
	else if (spec.domain == MaterialDomain::POST_PROCESS) domain = 1;
	else if (spec.domain == MaterialDomain::COMPUTE)     domain = 2;

	// ── Required attributes mask ─────────────────────────────────────
	uint32_t attrMask = 0;
	for (auto attr : spec.requiredAttributes)
		attrMask |= (1u << static_cast<uint32_t>(attr));

	// ── Properties ───────────────────────────────────────────────────
	std::vector<FlatProperty> properties;
	for (auto& p : spec.properties)
	{
		FlatProperty fp;
		fp.name = p.name;
		fp.uniformType = p.kind == PropertyParam::Kind::Uniform
			? static_cast<uint8_t>(p.uniformType)
			: uint8_t(0);
		properties.push_back(std::move(fp));
	}

	// ── Constants ────────────────────────────────────────────────────
	std::vector<std::string> constants;
	for (auto& k : spec.constants)
		constants.push_back(k.name);

	// ── Build DescriptorSetBindings (per-set binding list) ───────────
	DescriptorSetInfo descBindings{};

	auto& setView = descBindings[static_cast<uint8_t>(DescriptorSetBindingPoints::PER_VIEW)];
	setView.push_back({ "FrameUniforms.frameUniforms", RHI::DescriptorType::UNIFORM_BUFFER,
		static_cast<uint8_t>(PerViewBindingPoints::FRAME_UNIFORM) });

	auto& setRenderable = descBindings[static_cast<uint8_t>(DescriptorSetBindingPoints::PER_RENDERABLE)];
	setRenderable.push_back({ "ObjectUniforms.objectUniforms", RHI::DescriptorType::UNIFORM_BUFFER,
		static_cast<uint8_t>(PerRenderableBindingPoints::OBJECT_UNIFORM) });

	auto& setMat = descBindings[static_cast<uint8_t>(DescriptorSetBindingPoints::PER_MATERIAL)];
	setMat.push_back({ "MaterialParams.materialParams", RHI::DescriptorType::UNIFORM_BUFFER,
		static_cast<uint8_t>(PerMaterialBindingPoint::MATERIAL_UNIFORM) });

	descriptor_binding_t samplerBinding = 1; // just for this set. (0 for ubo)
	for (auto& p : spec.properties)
	{
		if (p.kind != PropertyParam::Kind::Sampler)
			continue;
		setMat.push_back({"materialParams_" + p.name, RHI::DescriptorType::SAMPLER, (uint8_t)setMat.size() });
	}

	if (spec.pipeline == Pipeline::DEFERRED)
	{
		auto& setGBuffer = descBindings[static_cast<uint8_t>(DescriptorSetBindingPoints::G_BUFFER)];
		setGBuffer.push_back({ "gAlbedo",   RHI::DescriptorType::SAMPLER, static_cast<uint8_t>(GBufferBindingPoint::G_BUFFER_ALBEDO) });
		setGBuffer.push_back({ "gNormal",   RHI::DescriptorType::SAMPLER, static_cast<uint8_t>(GBufferBindingPoint::G_BUFFER_NORMAL) });
		setGBuffer.push_back({ "gPosition", RHI::DescriptorType::SAMPLER, static_cast<uint8_t>(GBufferBindingPoint::G_BUFFER_POSITION) });
		setGBuffer.push_back({ "gMaterial", RHI::DescriptorType::SAMPLER, static_cast<uint8_t>(GBufferBindingPoint::G_BUFFER_MATERIAL) });
	}

	// ── Build DescriptorSetLayout (GPU layout objects) ───────────────────
	std::array<RHI::DescriptorSetLayout, 2> descSetLayouts{};

	// Layout[0]: PER_MATERIAL bindings
	{
		auto& layout = descSetLayouts[0].bindings;
		auto& src = descBindings[static_cast<uint8_t>(DescriptorSetBindingPoints::PER_MATERIAL)];
		layout.resize(src.size());
		for (size_t i = 0; i < src.size(); i++)
		{
			layout[i].type       = src[i].type;
			layout[i].binding    = src[i].binding;
			layout[i].stageFlags = RHI::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
			layout[i].count      = 1;
		}
	}

	// Layout[1]: G_BUFFER bindings (deferred only)
	if (spec.pipeline == Pipeline::DEFERRED)
	{
		auto& layout = descSetLayouts[1].bindings;
		auto& src = descBindings[static_cast<uint8_t>(DescriptorSetBindingPoints::G_BUFFER)];
		layout.resize(src.size());
		for (size_t i = 0; i < src.size(); i++)
		{
			layout[i].type       = src[i].type;
			layout[i].binding    = src[i].binding;
			layout[i].stageFlags = RHI::ShaderStageFlags::FRAGMENT;
			layout[i].count      = 1;
		}
	}

	// ── Build AttributeInfo (vertex inputs / fragment outputs) ───────
	ChunkAttributeInfo::Container attrInfo;
	attrInfo.inputs  = BuildVertexInputs(spec);
	attrInfo.outputs = BuildFragmentOutputs(spec);

	// ── Collect chunks ───────────────────────────────────────────────
	ChunkContainer cc;
	uint32_t chunkCount = 0;

	if (hasSpv)                 { cc.Set<ChunkSpirv>(std::move(spirvData)); chunkCount++; }
	                            cc.Set<ChunkGlsl>(std::move(glslData));    chunkCount++;
	                            cc.Set<ChunkName>(std::string(spec.name));  chunkCount++;
	                            cc.Set<ChunkVersion>(1u);                  chunkCount++;
	                            cc.Set<ChunkShading>(std::string(spec.shadingModel)); chunkCount++;
	                            cc.Set<ChunkDomain>(std::move(domain));        chunkCount++;
	                            cc.Set<ChunkUib>(std::move(spec.materialUib));              chunkCount++;
	                            cc.Set<ChunkSib>(std::move(spec.materialSib));              chunkCount++;
	                            cc.Set<ChunkDescriptorSetBindings>(std::move(descBindings)); chunkCount++;
	                            cc.Set<ChunkDescriptorSetLayout>(std::move(descSetLayouts)); chunkCount++;
	                            cc.Set<ChunkAttributeInfo>(std::move(attrInfo));       chunkCount++;
	if (attrMask)               { cc.Set<ChunkRequiredAttrs>(std::move(attrMask)); chunkCount++; }
	if (!properties.empty())    { cc.Set<ChunkProperties>(std::move(properties)); chunkCount++; }
	if (!constants.empty())     { cc.Set<ChunkConstants>(std::move(constants));   chunkCount++; }

	// ── Dry run → real write ─────────────────────────────────────────
	FArchiveWrite dryAr;
	cc.Serialize(dryAr);

	std::vector<uint8_t> buffer(dryAr.Tell());
	FArchiveWrite ar(buffer.data(), buffer.size());
	cc.Serialize(ar);

	// ── Write to disk ────────────────────────────────────────────────
	std::string matbPath = fullOutputDir + "/" + spec.name + ".matb";
	std::ofstream file(matbPath, std::ios::binary);
	if (!file)
	{
		std::cerr << "matc: error: cannot write " << matbPath << '\n';
		return;
	}
	file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	std::cout << "matc: wrote " << matbPath << " (" << chunkCount << " chunks)\n";
}

// ============================================================
// Dump .matb contents for debugging
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
	if (cc.Get<ChunkSpirv>(spv))
		std::cout << "SPIR-V:        vert=" << spv.vertexSpirv.size()
		          << "B  frag=" << spv.fragmentSpirv.size() << "B\n";

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
			std::cout << "  set=" << set << '\n';
			for (auto& d : dsl[set])
				std::cout << "    " << d.name << "  binding=" << (int)d.binding
				          << "  type=" << (int)d.type << '\n';
		}
	}

	// DescriptorSetLayout
	std::array<RHI::DescriptorSetLayout, 2> descLayouts;
	if (cc.Get<ChunkDescriptorSetLayout>(descLayouts))
	{
		sep();
		std::cout << "[ChunkDescriptorSetLayout]\n";
		for (size_t li = 0; li < descLayouts.size(); li++)
		{
			std::cout << "  layout[" << li << "]\n";
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

	// AttributeInfo
	ChunkAttributeInfo::Container attr;
	if (cc.Get<ChunkAttributeInfo>(attr))
	{
		sep();
		std::cout << "[ChunkAttributeInfo]\n";
		std::cout << "  Inputs:\n";
		for (auto& v : attr.inputs)
			std::cout << "    " << v.name << "  loc=" << (int)v.location << "  type=" << (int)v.type << '\n';
		std::cout << "  Outputs:\n";
		for (auto& v : attr.outputs)
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

// ============================================================

int main(int Argc, char* Argv[])
{
    std::string inputFile;
    std::string outputDir = "CompiledMaterials";
    std::string workDir;
    std::string glslcPath = "C:\\VulkanSDK\\1.3.216.0\\Bin\\glslc.exe";
    std::string templateDir = "Template";
    std::string targetEnv = "vulkan1.2";
    std::vector<std::string> includePaths;
    std::vector<std::string> glslcExtraArgs;
    bool codeOnly = false;
    bool preprocessOnly = false;
    bool debugInfo = false;
    std::string optLevel;
    std::string dumpPath;

    // --- Parse arguments ---
    for (int i = 1; i < Argc; ++i)
    {
        std::string arg = Argv[i];

        if (arg == "-I" && i + 1 < Argc)
        {
            std::string mode = Argv[++i];
            if (mode == "codeonly") codeOnly = true;
            else { std::cerr << "matc: unknown -I mode '" << mode << "'. Valid: codeonly\n"; return 1; }
        }
        else if (arg == "-E")
            preprocessOnly = true;
        else if (arg == "-g")
            debugInfo = true;
        else if (arg == "-O" && i + 1 < Argc)
            optLevel = Argv[++i];
        else if (arg == "-o" && i + 1 < Argc)
            outputDir = Argv[++i];
        else if (arg == "-w" && i + 1 < Argc)
            workDir = Argv[++i];
        else if (arg == "--target" && i + 1 < Argc)
            targetEnv = Argv[++i];
        else if (arg == "--glslc" && i + 1 < Argc)
            glslcPath = Argv[++i];
        else if (arg == "--include" && i + 1 < Argc)
            includePaths.push_back(Argv[++i]);
        else if (arg == "--dump" && i + 1 < Argc)
            dumpPath = Argv[++i];
        else if (arg == "--template-dir" && i + 1 < Argc)
            templateDir = Argv[++i];
        else if (arg == "-h" || arg == "--help")
        {
            PrintUsage();
            return 0;
        }
        else if (!arg.empty() && arg[0] != '-')
            inputFile = arg;
        else
        {
            std::cerr << "matc: unknown argument: " << arg << '\n';
            PrintUsage();
            return 1;
        }
    }

    if (!dumpPath.empty())
    {
        DumpMatb(dumpPath);
        return 0;
    }

    if (inputFile.empty())
    {
        std::cerr << "matc: missing input filename\n\n";
        PrintUsage();
        return 1;
    }

    // --- Resolve working directory ---
    if (workDir.empty())
        workDir = GetDirectory(inputFile);
    while (!workDir.empty() && (workDir.back() == '/' || workDir.back() == '\\'))
        workDir.pop_back();

    // Default include search paths
    if (includePaths.empty())
    {
        includePaths.push_back(workDir + "/Shaders");
        includePaths.push_back(workDir);
    }

    // --- Step 1: Read and parse JSON ---
    std::cout << "matc: reading " << inputFile << '\n';

    MaterialSpec spec;
    try
    {
        std::ifstream f(inputFile);
        if (!f.is_open())
        {
            std::cerr << "matc: error: cannot open input file: " << inputFile << '\n';
            return 1;
        }

        std::string jsonStr((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        spec = ParseMaterialSpec(jsonStr);
    }
    catch (const std::exception& e)
    {
        std::cerr << "matc: error: " << e.what() << '\n';
        return 1;
    }

    std::cout << "matc: parsed material '" << spec.name << "' ("
              << ToString(spec.domain) << ", shading: " << spec.shadingModel << ")\n";

    // default Code.
    if (spec.vertexCode.empty())
    {
        spec.vertexCode = "void materialVertex(out MaterialVertexInputs inputs)\n{\n}";
    }
    if (spec.fragmentCode.empty())
    {
        spec.fragmentCode = "void material(out MaterialInputs inputs)\n{\n}";
    }

    // --- Step 2: Expand includes ---
    std::string inputDir = GetDirectory(inputFile);
    std::string expandedVert, expandedFrag;

    try
    {
        if (!spec.vertexCode.empty())
        {
            expandedVert = ExpandIncludes(spec.vertexCode, inputDir, includePaths);
            std::cout << "matc: expanded vertex includes\n";
        }
        if (!spec.fragmentCode.empty())
        {
            expandedFrag = ExpandIncludes(spec.fragmentCode, inputDir, includePaths);
            std::cout << "matc: expanded fragment includes\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "matc: error expanding includes: " << e.what() << '\n';
        return 1;
    }

    // --- Step 3: Build structured objects --------------------------
    BuildUib(spec);
    BuildSib(spec);

    // --- Step 4: Generate GLSL --------------------------------------
    ShaderGenerator shaderGen;
    shaderGen.SetTemplateDirectory(templateDir);
    shaderGen.SetTargetVulkan(targetEnv.rfind("vulkan", 0) == 0);
    std::cout << "matc: using template directory: " << templateDir << std::endl;

    std::string vertSource = shaderGen.GenerateVertexShader(spec, expandedVert);
    std::string fragSource = shaderGen.GenerateFragmentShader(spec, expandedFrag);

    // --- Step 4: Write GLSL output ---
    std::string fullOutputDir = workDir + "/" + outputDir + "/" + spec.name;
    EnsureDirectory(fullOutputDir);

    std::string vertFile = fullOutputDir + "/" + spec.name + ".vert";
    std::string fragFile = fullOutputDir + "/" + spec.name + ".frag";

    {
        std::ofstream out(vertFile);
        if (!out) { std::cerr << "matc: error: cannot write " << vertFile << '\n'; return 1; }
        out << vertSource;
        std::cout << "matc: wrote " << vertFile << " (" << vertSource.size() << " bytes)\n";
    }
    {
        std::ofstream out(fragFile);
        if (!out) { std::cerr << "matc: error: cannot write " << fragFile << '\n'; return 1; }
        out << fragSource;
        std::cout << "matc: wrote " << fragFile << " (" << fragSource.size() << " bytes)\n";
    }

    // --- Step 5: Compile / preprocess (unless code-only) ---
    if (codeOnly)
    {
        std::cout << "matc: done (code-only).\n";
        return 0;
    }

    // Build glslc argument list from options
    // -E and SPIR-V compilation are mutually exclusive
    if (preprocessOnly)
    {
        glslcExtraArgs.push_back("-E");
    }
    else
    {
        glslcExtraArgs.push_back("--target-env=" + targetEnv);
        if (!optLevel.empty())
            glslcExtraArgs.push_back("-O" + optLevel);
        if (debugInfo)
            glslcExtraArgs.push_back("-g");
    }

    // Compile vertex stage
    if (!spec.vertexCode.empty() || spec.domain == MaterialDomain::SURFACE)
    {
        std::string outFile = preprocessOnly
            ? fullOutputDir + "/" + spec.name + ".preprocessed.vert"
            : fullOutputDir + "/" + spec.name + ".vert.spv";

        if (!RunGlslc(glslcPath, vertFile, outFile, "vert", glslcExtraArgs))
        {
            std::cerr << "matc: error: vertex shader compilation failed\n";
            return 1;
        }
    }

    // Compile fragment stage
    if (!spec.fragmentCode.empty() || spec.domain == MaterialDomain::SURFACE)
    {
        std::string outFile = preprocessOnly
            ? fullOutputDir + "/" + spec.name + ".preprocessed.frag"
            : fullOutputDir + "/" + spec.name + ".frag.spv";

        if (!RunGlslc(glslcPath, fragFile, outFile, "frag", glslcExtraArgs))
        {
            std::cerr << "matc: error: fragment shader compilation failed\n";
            return 1;
        }
    }

    // --- Step 6: Write .matb binary ---
    WriteMaterialBinary(fullOutputDir, spec,
                        vertSource, fragSource,
                        !preprocessOnly && !codeOnly);

    std::cout << "matc: done.\n";
    return 0;
}
