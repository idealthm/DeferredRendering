#include "Program.h"

Program::Program(const ShaderSource& shadersSource, const DescriptorSetInfo& descriptorSetInfo)
	: m_ShadersSource(shadersSource), m_DescriptorBindings(descriptorSetInfo)
{
}

Program::~Program()
{
}

const std::string& Program::getName() const
{
	return m_Name;
}


