#include "ShaderLoader.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "TokenList.h"
#include "glad/glad.h"

uint32_t ShaderLoader::CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char *)alloca(sizeof(float) * length);
        GLCall(glGetShaderInfoLog(id, length, &length, message));

        std::cout << "Fail to compile shader!" << (type == GL_VERTEX_SHADER ? "vertex" : "pixel") << std::endl;
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

uint32_t ShaderLoader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));
	
    return program;
}

uint32_t ShaderLoader::CreateShader(const std::string& filePath, DUI* Indui)
{
    static DUI StaticDUI{{"#version 460 core\n"},{},{},{"Shaders/"},{},false, !_DEBUG};

    std::stringstream vertSource, fragSource;
    std::string vertShaderFilePath = filePath + ".vert.glsl";
    std::string fragShaderFilePath = filePath + ".frag.glsl";

    DUI& dui = Indui ? *Indui : StaticDUI;

    PreProcessor(vertSource, vertShaderFilePath, dui);
    PreProcessor(fragSource, fragShaderFilePath, dui);
    return CreateShader(vertSource.str(), fragSource.str());
}

bool ShaderLoader::IsDirectory(const std::string& path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == -1)
        return false;

    return (file_stat.st_mode & S_IFMT) == S_IFDIR;
}

void ShaderLoader::PreProcessor(std::stringstream& ss, const std::string& filename, DUI &dui)
{
    ss << dui.prefix;

	bool inp_missing = false;

    for (const std::string& inc : dui.includes) {
        std::ifstream f(inc);
        if (!f.is_open() || IsDirectory(inc)) {
            inp_missing = true;
            std::cout << "error: could not open include '" << inc << "'" << std::endl;
        }
    }

    for (const std::string& inc : dui.includePaths) {
        if (!IsDirectory(inc)) {
            inp_missing = true;
            std::cout << "error: could not find include path '" << inc << "'" << std::endl;
        }
    }

    std::ifstream f(filename);
    if (!f.is_open() || IsDirectory(filename)) {
        inp_missing = true;
        std::cout << "error: could not open file '" << filename << "'" << std::endl;
    }

    if (inp_missing)
        return ;

    // Perform preprocessing
    OutputList outputList;
    std::vector<std::string> files;
    TokenList outputTokens(files);
    {
        TokenList *rawtokens;
        if (constexpr bool use_istream = true) {
            rawtokens = new TokenList(f, files,filename,&outputList);
        } else {
            f.close();
            rawtokens = new TokenList(filename,files,&outputList);
        }
        rawtokens->removeComments();
        FileDataCache filedata;
        preprocess(outputTokens, *rawtokens, files, filedata, dui, &outputList);
        cleanup(filedata);
        delete rawtokens;
    }

    outputTokens.stringify(ss, false);

    // Output
#ifdef _DEBUG

    for (const Output &output : outputList) {
        std::cerr << outputTokens.file(output.location) << ':' << output.location.line << ": ";
        switch (output.type) {
        case Output::ERROR:
            std::cerr << "#error: ";
            break;
        case Output::WARNING:
            std::cerr << "#warning: ";
            break;
        case Output::MISSING_HEADER:
            std::cerr << "missing header: ";
            break;
        case Output::INCLUDE_NESTED_TOO_DEEPLY:
            std::cerr << "include nested too deeply: ";
            break;
        case Output::SYNTAX_ERROR:
            std::cerr << "syntax error: ";
            break;
        case Output::PORTABILITY_BACKSLASH:
            std::cerr << "portability: ";
            break;
        case Output::UNHANDLED_CHAR_ERROR:
            std::cerr << "unhandled char error: ";
            break;
        case Output::EXPLICIT_INCLUDE_NOT_FOUND:
            std::cerr << "explicit include not found: ";
            break;
        case Output::FILE_NOT_FOUND:
            std::cerr << "file not found: ";
            break;
        case Output::DUI_ERROR:
            std::cerr << "dui error: ";
            break;
        }
        std::cerr << output.msg << std::endl;
    }
#endif
}
