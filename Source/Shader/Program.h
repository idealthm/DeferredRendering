#pragma once
#include <array>
#include <string>
#include <vector>

#include "RHI/BindingMap.h"
#include "RHI/DriverEnums.h"

class Program
{
	friend class Material;
public:
	static constexpr uint8_t SHADER_TYPE_COUNT = 3;
	struct Descriptor {
		std::string name;
		RHI::DescriptorType type;
		uint8_t binding;
	};

	using DescriptorBindingsInfo = std::vector<Descriptor>;
	using DescriptorSetInfo = std::array<DescriptorBindingsInfo, MAX_DESCRIPTOR_SET_COUNT>;
	using ShaderBlob = std::vector<uint8_t>;
	using ShaderSource = std::array<ShaderBlob, SHADER_TYPE_COUNT>;

	Program(const ShaderSource& shadersSource, const DescriptorSetInfo& descriptorSetInfo);
	~Program();

	const ShaderSource& GetShadersSource() const {return m_ShadersSource;}
	const DescriptorSetInfo& GetDescriptorBindings() const {return m_DescriptorBindings;}

	std::string getName() const;

private:
	std::string m_Name;
	ShaderSource m_ShadersSource;
	DescriptorSetInfo m_DescriptorBindings;
};
