#pragma once
#include <cstdint>

#include "DriverEnums.h"
#include "Common/Utils/Bitset.h"
#include "glad/glad.h"

class BindingMap
{
	struct CompressedBinding
	{
		uint8_t binding : 7;
		uint8_t sampler : 1; // 1 bit for type
	};

	CompressedBinding (*m_Storage)[MAX_DESCRIPTOR_COUNT];
	Util::bitset64 m_ActiveDescriptors[MAX_DESCRIPTOR_SET_COUNT];
public:
	BindingMap () noexcept
		: m_Storage(new (std::nothrow) CompressedBinding[MAX_DESCRIPTOR_SET_COUNT][MAX_DESCRIPTOR_COUNT])
	{
		memset(m_Storage, 0xFF, sizeof(CompressedBinding[MAX_DESCRIPTOR_SET_COUNT][MAX_DESCRIPTOR_COUNT]));
	}

	~BindingMap() noexcept {
		delete [] m_Storage;
	}

	BindingMap(BindingMap const&) noexcept = delete;
	BindingMap(BindingMap&&) noexcept = delete;
	BindingMap& operator=(BindingMap const&) noexcept = delete;
	BindingMap& operator=(BindingMap&&) noexcept = delete;

	struct Binding {
		GLuint binding;
		RHI::DescriptorType type;
	};

	void insert(descriptor_set_t set, descriptor_binding_t binding, Binding entry) noexcept {
		ASSERT(set < MAX_DESCRIPTOR_SET_COUNT);
		ASSERT(binding < MAX_DESCRIPTOR_COUNT);
		ASSERT(entry.binding < 128); // we reserve 1 bit for the type right now
		m_Storage[set][binding] = { (uint8_t)entry.binding,
								   entry.type == RHI::DescriptorType::SAMPLER ||
								   entry.type == RHI::DescriptorType::SAMPLER_EXTERNAL };
		m_ActiveDescriptors[set].set(binding);
	}

	GLuint get(descriptor_set_t set, descriptor_binding_t binding) const noexcept {
		ASSERT(set < MAX_DESCRIPTOR_SET_COUNT);
		ASSERT(binding < MAX_DESCRIPTOR_COUNT);
		return m_Storage[set][binding].binding;
	}

	Util::bitset64 getActiveDescriptors(descriptor_set_t set) const noexcept {
		return m_ActiveDescriptors[set];
	}
};
