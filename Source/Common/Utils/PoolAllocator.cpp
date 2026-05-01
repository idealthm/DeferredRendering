#include "PoolAllocator.h"

namespace Utils
{
	FreeList::FreeList(void* begin, void* end, size_t elementSize, size_t alignment, size_t offset) noexcept
	: mHead(init(begin, end, elementSize, alignment, offset)), mBegin(begin), mEnd(end)
	{
	}

	FreeList::Node* FreeList::init(void* begin, void* end, size_t elementSize, size_t alignment, size_t offset) noexcept
	{
		void* const b = PointerMath::align(begin, alignment, offset);
		void* const e = PointerMath::align(PointerMath::add(b, elementSize), alignment, offset);

		ASSERT(b >= begin && b < end);
		ASSERT(e > b && e >= begin && e < end);

		const size_t d = reinterpret_cast<uintptr_t>(e) - reinterpret_cast<uintptr_t>(b);
		const size_t n = (reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(b)) / d;

		Node* head = static_cast<Node*>(b);

		Node* cur = head;
		for (int i = 0; i < n - 1; ++i)
		{
			cur->next = PointerMath::add(cur, d);
			cur = cur->next;
		}

		cur->next = nullptr;

		ASSERT(cur < end && PointerMath::add(cur, e) <= end);

		return head;
	}
}