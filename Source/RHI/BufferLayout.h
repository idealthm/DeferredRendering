#pragma once
#include <string>
#include <vector>

#include "Common/Core.h"
#include "DriverEnums.h"

namespace RHI
{

enum class ShaderDataType
{
	None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool, UShort, UShort2
};

inline uint32_t ShaderDataTypeSize(ShaderDataType type)
{
	switch (type)
	{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		case ShaderDataType::UShort:   return 2;
		case ShaderDataType::UShort2:  return 4;
		default: break;
	}

	ASSERT(false);
	return 0;
}

struct BufferElement
{
	std::string Name;
	ShaderDataType Type;
	uint32_t Size;
	size_t Offset;
	bool Normalized;

	BufferElement() = default;

	BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
		: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
	{
	}

	uint32_t GetComponentCount() const
	{
		switch (Type)
		{
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Mat3:    return 3;
			case ShaderDataType::Mat4:    return 4;
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			case ShaderDataType::UShort:  return 1;
			case ShaderDataType::UShort2: return 2;
			default: break;
		}

		ASSERT(false);
		return 0;
	}
};

class BufferLayout
{
public:
	BufferLayout() {}

	BufferLayout(std::initializer_list<BufferElement> elements)
		: m_Elements(elements)
	{
		CalculateOffsetsAndStride();
	}

	uint32_t GetStride() const { return m_Stride; }
	const std::vector<BufferElement>& GetElements() const { return m_Elements; }

	void AddElement(const BufferElement& element)
	{
		m_Elements.push_back(element); CalculateOffsetsAndStride();
	}

	std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
	std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
	std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
	std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

private:
	void CalculateOffsetsAndStride()
	{
		size_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}

	std::vector<BufferElement> m_Elements;
	uint32_t m_Stride = 0;
};

inline ElementType ShaderDataTypeToElementType(ShaderDataType type)
{
	switch (type)
	{
		case ShaderDataType::Float:   return ElementType::FLOAT;
		case ShaderDataType::Float2:  return ElementType::FLOAT2;
		case ShaderDataType::Float3:  return ElementType::FLOAT3;
		case ShaderDataType::Float4:  return ElementType::FLOAT4;
		case ShaderDataType::Int:     return ElementType::INT;
		case ShaderDataType::UShort:  return ElementType::USHORT;
		case ShaderDataType::UShort2: return ElementType::USHORT2;
		default:
			ASSERT(false);
			return ElementType::FLOAT;
	}
}

inline uint32_t ShaderDataTypeComponentCount(ShaderDataType type)
{
	switch (type)
	{
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::UShort:  return 1;
		case ShaderDataType::UShort2: return 2;
		case ShaderDataType::Mat3:    return 3;
		case ShaderDataType::Mat4:    return 4;
		default:
			ASSERT(false);
			return 1;
	}
}

inline Attribute BufferElementToAttribute(const BufferElement& element, uint8_t bufferSlot, uint8_t stride)
{
	Attribute attr;
	attr.offset = static_cast<uint32_t>(element.Offset);
	attr.stride = stride;
	attr.buffer = bufferSlot;
	attr.type = ShaderDataTypeToElementType(element.Type);
	attr.flags = 0;
	if (element.Normalized)
		attr.flags |= Attribute::FLAG_NORMALIZED;
	return attr;
}

} // namespace RHI
