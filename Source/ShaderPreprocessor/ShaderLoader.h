#pragma once
#include <xstring>

#include "ShaderDefines.h"
#include "common/Core.h"


class Shader;

class ShaderLoader
{
public:
	static uint32 CompileShader(unsigned int type, const std::string& source);
	static uint32 CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	static uint32 CreateShader(const std::string& filePath, DUI* Indui);

	static bool IsDirectory(const std::string &path);

	static void PreProcessor(std::stringstream& ss, const std::string& filename, DUI& dui);
};
