#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "GLSLGenerator.h"
#include "IncludeExpander.h"
#include "MaterialSpec.h"

#define WIN32_LEAN_AND_MEAN
#include <sstream>
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
              << spec.domain << ", shading: " << spec.shadingModel << ")\n";

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

    // --- Step 3: Generate GLSL ---
    SetTemplateDirectory(templateDir);
    SetTargetVulkan(targetEnv.find("vulkan") != std::string::npos);
    std::cout << "matc: using template directory: " << templateDir << '\n';

    std::ostringstream vertOs;
    GenerateVertexShader(vertOs, spec, expandedVert);
    std::string vertSource = vertOs.str();

    std::ostringstream fragOs;
    GenerateFragmentShader(fragOs, spec, expandedFrag);
    std::string fragSource = fragOs.str();

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
    if (!spec.vertexCode.empty() || spec.domain == "surface")
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
    if (!spec.fragmentCode.empty() || spec.domain == "surface")
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

    std::cout << "matc: done.\n";
    return 0;
}
