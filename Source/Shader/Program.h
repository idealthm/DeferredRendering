#pragma once
#include <array>
#include <string>
#include <vector>

#include "Common/Material/MaterialTypes.h"
#include "RHI/BindingMap.h"

class Program
{
	friend class Material;
public:
	static constexpr uint8_t SHADER_TYPE_COUNT = 3;
	using ShaderBlob            = std::vector<uint8_t>;
	using ShaderSource          = std::array<ShaderBlob, SHADER_TYPE_COUNT>;

	Program(const ShaderSource& shadersSource, const DescriptorSetInfo& descriptorSetInfo);
	Program(Program&& other) = default;
	~Program();

	ShaderSource& GetShadersSource() {return m_ShadersSource;}
	DescriptorSetInfo& GetDescriptorBindings() {return m_DescriptorBindings;}

	const std::string& getName() const;

private:
	std::string m_Name;
	ShaderSource m_ShadersSource;
	DescriptorSetInfo m_DescriptorBindings;
};
