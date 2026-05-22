#pragma once
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
		mHandle = driver.CreateBufferObject(sizeof(T) * N, BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);
	}

	TypedBuffer<T, N>& getTypedBuffer() noexcept { return mTypedBuffer; }
	Handle<RHI::HwBufferObject>   getHandle() const noexcept { return mHandle; }

	T&    itemAt(size_t i) noexcept { return mTypedBuffer.itemAt(i); }
	T&    edit() noexcept           { return mTypedBuffer.edit(); }
	size_t getSize()  const noexcept { return mTypedBuffer.getSize(); }
	bool   isDirty()  const noexcept { return mTypedBuffer.isDirty(); }
	void   clean()    const noexcept { mTypedBuffer.clean(); }

	// --- upload dirty data to GPU ---

	void commit(RHI::RHIDriver& driver)
	{
		if (mTypedBuffer.isDirty())
		{
			// driver.SetBufferData(mHandle, mTypedBuffer.data(), getSize(), 0);
			mTypedBuffer.clean();
		}
	}

	void commit(RHI::RHIDriver& driver, size_t offset, size_t size)
	{
		// driver.SetBufferData(mHandle, mTypedBuffer.data(), size, offset);
		mTypedBuffer.clean();
	}

private:
	TypedBuffer<T, N> mTypedBuffer;
	Handle<RHI::HwBufferObject>  mHandle;
};
