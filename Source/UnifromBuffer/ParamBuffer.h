#pragma once
#include "RHI/BufferDescriptor.h"
#include "RHI/RHIDriver.h"

namespace RHI { class RHIDriver; }

template<typename T, size_t N = 1>
class TypedBuffer
{
public:
	T& itemAt(size_t i) noexcept
	{
		mDirty = true;
		return mBuffer[i];
	}

	T& edit() noexcept { return itemAt(0); }

	size_t  getSize() const noexcept { return sizeof(T) * N; }
	bool    isDirty() const noexcept { return mDirty; }
	void    clean() const noexcept { mDirty = false; }

	const void* data() const noexcept { return mBuffer; }

	BufferDescriptor toBufferDescriptor(RHI::RHIDriver& driver) const noexcept {
		return toBufferDescriptor(driver, 0, getSize());
	}

	// copy the UBO data and cleans the dirty bits
	BufferDescriptor toBufferDescriptor(RHI::RHIDriver& driver, size_t const offset, size_t const size) const noexcept {
		BufferDescriptor p;
		p.size = size;
		p.buffer = malloc(p.size); // TODO: use out-of-line buffer if too large
		memcpy(p.buffer, reinterpret_cast<const char*>(mBuffer) + offset, p.size); // inlined
		clean();
		p.setCallback([](void* buffer, size_t size, void* user){free(buffer);}, nullptr);
		return p;
	}

private:
	T            mBuffer[N];
	mutable bool mDirty = false;
};


template<typename T, size_t N = 1>
class TypedUniformBuffer
{
public:
	TypedUniformBuffer() = default;

	static size_t GetCapacity() noexcept { return N; }

	void init(RHI::RHIDriver& driver)
	{
		mHandle = driver.CreateBufferObject(sizeof(T) * N, RHI::BufferObjectBinding::UNIFORM, RHI::BufferUsage::DYNAMIC);
	}

	TypedBuffer<T, N>& getTypedBuffer() noexcept { return mTypedBuffer; }
	Handle<RHI::HwBufferObject>   getHandle() const noexcept { return mHandle; }

	T&    itemAt(size_t i) noexcept { return mTypedBuffer.itemAt(i); }
	T&    edit() noexcept           { return mTypedBuffer.edit(); }
	size_t getSize()  const noexcept { return mTypedBuffer.getSize(); }
	bool   isDirty()  const noexcept { return mTypedBuffer.isDirty(); }
	void   clean()    const noexcept { mTypedBuffer.clean(); }

	// --- upload dirty data to GPU ---

	BufferDescriptor toBufferDescriptor(RHI::RHIDriver& driver) const noexcept {
		return mTypedBuffer.toBufferDescriptor(driver);
	}

	// copy the UBO data and cleans the dirty bits
	BufferDescriptor toBufferDescriptor(RHI::RHIDriver& driver, size_t offset, size_t size) const noexcept {
		return mTypedBuffer.toBufferDescriptor(driver, offset, size);
	}

private:
	TypedBuffer<T, N> mTypedBuffer;
	Handle<RHI::HwBufferObject>  mHandle;
};
