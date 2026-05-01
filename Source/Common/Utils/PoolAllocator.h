#pragma once
#include "Common/Core.h"


namespace PointerMath
{
	template<typename A, typename B>
	A* add(A* a, B b)
	{
		return reinterpret_cast<A*>(reinterpret_cast<uintptr_t>(a) + (uintptr_t)(b));
	}

	template<typename A>
	A* align(A* ptr, size_t alignment)
	{
		ASSERT(ptr && !(alignment & (alignment - 1)))
		return reinterpret_cast<A*>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
	}

	template<typename A>
	A* align(A* ptr, size_t alignment, size_t offset)
	{
		return align(add(ptr, offset),  alignment);
	}
}

namespace Utils
{
class FreeList
{
public:
	FreeList() noexcept = default;
	~FreeList() noexcept = default;
	FreeList(void* begin, void* end, size_t elementSize, size_t alignment, size_t offset) noexcept;
	FreeList(const FreeList& rhs) = delete;
	FreeList& operator=(const FreeList& rhs) = delete;
	FreeList(FreeList&& rhs) noexcept = default;
	FreeList& operator=(FreeList&& rhs) noexcept = default;

	void* pop()
	{
		Node* head = mHead;
		mHead = head ? head->next : nullptr;
		ASSERT(!mHead || mHead >= mBegin && mHead < mEnd )
		return head;
	}

	void push(void* ptr)
	{
		ASSERT(ptr);
		ASSERT(ptr >= mBegin && ptr < mEnd);
		Node* node = static_cast<Node*>(ptr);
		node->next = mHead;
		mHead = node;
	}

	struct Node
	{
		Node* next;
	};
private:
	static Node* init(void* begin, void* end,
		size_t elementSize, size_t alignment, size_t offset) noexcept;

	Node* mHead = nullptr;
	void* mBegin = nullptr;
	void* mEnd = nullptr;
};



template<size_t ELEMENT_SIZE,
	size_t ALIGNMENT = alignof(std::max_align_t),
	size_t OFFSET = 0,
	typename FREELIST = FreeList
	>
class PoolAllocator
{
	static_assert(ELEMENT_SIZE >= sizeof(typename FREELIST::Node)
		, "ELEMENT_SIZE must accommodate at least a FreeList::Node");
public:
	PoolAllocator(void* begin, void* end) noexcept
		: mFreeList(begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET)	{
	}

	PoolAllocator(void* begin, size_t size) noexcept
		: PoolAllocator(begin, PointerMath::add(begin, size)){
	}

	template<typename AREA>
	explicit PoolAllocator(const AREA& area) noexcept
		: PoolAllocator(area.begin(), area.end()) {
	}

	// can't be copied
	PoolAllocator(const PoolAllocator& rhs) = delete;
	PoolAllocator& operator=(const PoolAllocator& rhs) = delete;

	// but can be moved
	PoolAllocator(PoolAllocator&& rhs) = default;
	PoolAllocator& operator=(PoolAllocator&& rhs) = default;

	PoolAllocator() noexcept = default;
	~PoolAllocator() noexcept = default;

	static constexpr size_t getSize() noexcept { return ELEMENT_SIZE; }

	void* alloc(size_t size = ELEMENT_SIZE, size_t alignment = ALIGNMENT, size_t offset = OFFSET)
	{
		ASSERT(size == ELEMENT_SIZE);
		ASSERT(alignment == ALIGNMENT);
		ASSERT(offset == OFFSET);
		return mFreeList.pop();
	}
	void free(void* ptr)
	{
		mFreeList.push(ptr);
	}
private:
	FREELIST mFreeList;
};


namespace AreaPolicy
{
class StaticArea
{
public:
	StaticArea(void* begin, void* end) noexcept
		: mBegin(begin), mEnd(end) {}

	StaticArea(const StaticArea& rhs) noexcept = delete;
	StaticArea& operator=(const StaticArea& rhs) noexcept = delete;

	StaticArea(StaticArea&& rhs) noexcept = default;
	StaticArea& operator=(StaticArea&& rhs) noexcept = default;

	~StaticArea() noexcept = default;

	void* data() const noexcept { return mBegin; }
	void* begin() const noexcept { return mBegin; }
	void* end() const noexcept { return mEnd; }
	size_t size() const noexcept { return uintptr_t(mEnd) - uintptr_t(mBegin); }

	friend void swap(StaticArea& lhs, StaticArea& rhs) noexcept
	{
		std::swap(lhs.mBegin, rhs.mBegin);
		std::swap(lhs.mEnd, rhs.mEnd);
	}
private:
	void* mBegin, * mEnd;
};

class HeapArea
{
public:
	explicit HeapArea(size_t size)
	{
		if (size)
		{
			mBegin = ::malloc(size);
			mEnd = PointerMath::add(mBegin, size);
		}
	}

	HeapArea(const HeapArea& rhs) noexcept = delete;
	HeapArea& operator=(const HeapArea& rhs) noexcept = delete;

	HeapArea(HeapArea&& rhs) noexcept = default;
	HeapArea& operator=(HeapArea&& rhs) noexcept = default;

	~HeapArea()
	{
		if (mBegin)
		{
			::free(mBegin);
		}
	}

	void* data() const noexcept { return mBegin; }
	void* begin() const noexcept { return mBegin; }
	void* end() const noexcept { return mEnd; }
	size_t size() const noexcept { return uintptr_t(mEnd) - uintptr_t(mBegin); }

	friend void swap(HeapArea& lhs, HeapArea& rhs) noexcept
	{
		std::swap(lhs.mBegin, rhs.mBegin);
		std::swap(lhs.mEnd, rhs.mEnd);
	}
private:
	void* mBegin, * mEnd;
};
}


template<typename >
class Arena
{
	
};
}
