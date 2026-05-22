#pragma once

#include "Handle.h"
#include "Utils/PoolAllocator.h"


template<size_t P0, size_t P1, size_t P2>
class HandleAllocator
{
public:
	// HandleId encoding: [ generation:8bit | offset:24bit ]
	// offset is relative to mHeapArea.begin(), so max pool size = 16 MB
	static constexpr uint32_t OFFSET_BITS = 24;
	static constexpr uint32_t AGE_BITS = 8;
	static constexpr uint32_t OFFSET_MASK = (1u << OFFSET_BITS) - 1;
	static constexpr uint32_t AGE_MASK = (1u << AGE_BITS) - 1;

	explicit HandleAllocator(size_t poolSize)
		: mHeapArea(poolSize)
		, mAllocator(mHeapArea, false)
	{}

	~HandleAllocator() = default;
	HandleAllocator(const HandleAllocator&) = delete;
	HandleAllocator& operator=(const HandleAllocator&) = delete;
	HandleAllocator(HandleAllocator&&) = delete;
	HandleAllocator& operator=(HandleAllocator&&) = delete;

	template<typename D>
	HandleBase::HandleId allocateHandle() noexcept
	{
		constexpr size_t BUKET_SIZE = getBucketSize<D>();
		return allocateHandleInPool<BUKET_SIZE>();
	}

	template<typename D>
	void deallocateHandle(HandleBase::HandleId id) noexcept
	{
		constexpr size_t BUKET_SIZE = getBucketSize<D>();
		deallocateHandleFormPool<BUKET_SIZE>(id);
	}

	std::pair<void*, uint32_t> handleToPointer(HandleBase::HandleId id) const noexcept
	{
		if (isPoolHandle(id))
		{
			char* const base = (char*)mHeapArea.begin();
			uint32_t tag = id & (AGE_MASK << OFFSET_BITS);
			size_t const offset = (id & OFFSET_MASK) * Allocator::getAlignment();
			return {static_cast<void*>(base + offset), tag};
		}
		return std::pair<void*, uint32_t>{handleToPointerSlow(id), 0};
	}

	inline HandleBase::HandleId arenaPointerToHandle(void* p, uint32_t tag) const noexcept {
		char* const base = (char*)mHeapArea.begin();
		size_t const offset = (char*)p - base;
		ASSERT((offset % Allocator::getAlignment()) == 0);
		auto id = HandleBase::HandleId(offset / Allocator::getAlignment());
		id |= tag & (AGE_MASK << OFFSET_BITS);
		return id;
	}

	template<size_t SIZE>
	HandleBase::HandleId allocateHandleInPool() noexcept
	{
		uint8_t age;
		void* p = mAllocator.alloc(SIZE, alignof(std::max_align_t), 0, &age);
		if (p)
		{
			uint32_t tag = (uint32_t(age) << OFFSET_BITS);
			return arenaPointerToHandle(p, tag);
		}
		return allocateHandleSlow(SIZE);
	}

	template<size_t SIZE>
	void deallocateHandleFormPool(HandleBase::HandleId id) noexcept
	{
		if (isPoolHandle(id))
		{
			auto [p, tag] = handleToPointer(id);
			const uint8_t age = tag >> OFFSET_BITS;
			mAllocator.free(p, SIZE, age);
		}
		else
		{
			deallocateHandleSlow(id, SIZE);
		}
	}

	bool isPoolHandle(HandleBase::HandleId id) const 
	{
		// TODO: distinguish between pool and non-pool handles
		return true;
	}

	void* handleToPointerSlow(HandleBase::HandleId) const;
	HandleBase::HandleId allocateHandleSlow(uint32_t size);
	void deallocateHandleSlow(HandleBase::HandleId id, size_t size);

	template<typename D, typename... Args>
	Handle<D> allocateAndConstruct(Args&&... args)
	{
		Handle<D> h{allocateHandle<D>()};
		D* addr = handle_cast<D*>(h);
		::new (addr) D(std::forward<Args>(args)...);
		return h;
	}

	template<typename D, typename B, typename ... ARGS>
	std::enable_if_t<std::is_base_of_v<B, D>, D>*
	destroyAndConstruct(Handle<B> const& handle, ARGS&& ... args)
	{
		ASSERT(handle);
		D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
		ASSERT(addr);
		// currently we implement construct<> with dtor+ctor, we could use operator= also
		// but all our dtors are trivial, ~D() is actually a noop.
		addr->~D();
		new(addr) D(std::forward<ARGS>(args)...);
		return addr;
	}

	template <typename B, typename D,
			typename = std::enable_if_t<std::is_base_of_v<B, D>, D>>
	void deallocate(Handle<B>& handle, D const* p) noexcept {
		// allow to destroy the nullptr, similarly to operator delete
		if (p) {
			p->~D();
			deallocateHandle<D>(handle.GetId());
		}
	}

	template<typename Dp, typename B>
	std::enable_if_t<std::is_pointer_v<Dp> && std::is_base_of_v<B, std::remove_pointer_t<Dp>>, Dp>
	handle_cast(const Handle<B>& handle) const
	{
		auto [p, tag] = handleToPointer(handle.GetId());
		auto* node = static_cast<typename Allocator::Node*>(p) - 1;
		auto age = tag >> OFFSET_BITS;
		ASSERT(node->age == age);

		return static_cast<Dp>(p);
	}

	template<typename B>
	bool is_valid(Handle<B>& handle) {
		if (!handle) {
			// null handles are invalid
			return false;
		}
		auto [p, tag] = handleToPointer(handle.getId());
		if (isPoolHandle(handle.getId())) {
			uint8_t const age = (tag >> OFFSET_BITS) & (( 1 << AGE_BITS) - 1);
			auto const pNode = static_cast<typename Allocator::Node*>(p);
			uint8_t const expectedAge = pNode[-1].age;
			return expectedAge == age;
		}
		return p != nullptr;
	}

private:
	template<typename D>
	static constexpr size_t getBucketSize() noexcept {
		if constexpr (sizeof(D) <= P0) { return P0; }
		if constexpr (sizeof(D) <= P1) { return P1; }
		static_assert(sizeof(D) <= P2);
		return P2;
	}

	class Allocator {
		friend class HandleAllocator;
		static constexpr size_t MIN_ALIGNMENT = alignof(std::max_align_t);
		struct Node {
			uint8_t age;
		};
		template<size_t SIZE>
		using Pool = Utils::PoolAllocator<SIZE, MIN_ALIGNMENT, sizeof(Node)>;
		Pool<P0> mPool0;
		Pool<P1> mPool1;
		Pool<P2> mPool2;
		const Utils::AreaPolicy::HeapArea& mArea;
		bool mUseAfterFreeCheckDisabled;
	public:
		explicit Allocator(const Utils::AreaPolicy::HeapArea& area, bool disableUseAfterFreeCheck);

		static constexpr size_t getAlignment() noexcept { return MIN_ALIGNMENT; }

		[[nodiscard]] inline void* alloc(size_t size, size_t, size_t, uint8_t* outAge) noexcept {
			void* p = nullptr;
			if      (size <= mPool0.getSize()) p = mPool0.alloc(size);
			else if (size <= mPool1.getSize()) p = mPool1.alloc(size);
			else if (size <= mPool2.getSize()) p = mPool2.alloc(size);
			if (p) {
				Node const* const pNode = static_cast<Node const*>(p);
				*outAge = pNode[-1].age;
			}
			return p;
		}

		inline void free(void* p, size_t size, uint8_t age) noexcept {
			ASSERT(p >= mArea.begin() && (char*)p + size <= (char*)mArea.end());

			Node* const pNode = static_cast<Node*>(p);
			uint8_t& expectedAge = pNode[-1].age;
			if (!mUseAfterFreeCheckDisabled) {
				ASSERT(expectedAge == age);
			}
			expectedAge = (expectedAge + 1) & 0xF;

			if (size <= mPool0.getSize()) { mPool0.free(p); return; }
			if (size <= mPool1.getSize()) { mPool1.free(p); return; }
			if (size <= mPool2.getSize()) { mPool2.free(p); return; }
		}
	};

	Utils::AreaPolicy::HeapArea mHeapArea; // declared before mAllocator (init order)
	Allocator mAllocator;
};

template <size_t P0, size_t P1, size_t P2>
void* HandleAllocator<P0, P1, P2>::handleToPointerSlow(HandleBase::HandleId) const
{
	// TODO: find id in heap memory map.
	return nullptr;
}

template <size_t P0, size_t P1, size_t P2>
HandleBase::HandleId HandleAllocator<P0, P1, P2>::allocateHandleSlow(uint32_t size)
{
	// TODO: Allocate Form malloc.
	ASSERT(0);
	return 0;
}

template <size_t P0, size_t P1, size_t P2>
void HandleAllocator<P0, P1, P2>::deallocateHandleSlow(HandleBase::HandleId id, size_t size)
{
	// TODO: Deallocate by free.
	ASSERT(0);
}

template <size_t P0, size_t P1, size_t P2>
HandleAllocator<P0, P1, P2>::Allocator::Allocator(const Utils::AreaPolicy::HeapArea& area,
	bool disableUseAfterFreeCheck): mArea(area)
									, mUseAfterFreeCheckDisabled(disableUseAfterFreeCheck)
{
	memset(area.data(), 0, area.size());

	const size_t totalSize = area.size();
	const size_t totalWeight = P0 + P1 + P2;
	size_t const count = totalSize / totalWeight;
	char* const p0 = static_cast<char*>(area.begin());
	char* const p1 = p0 + count * P0;
	char* const p2 = p1 + count * P1;

	mPool0 = Pool<P0>(p0, count * P0);
	mPool1 = Pool<P1>(p1, count * P1);
	mPool2 = Pool<P2>(p2, count * P2);
}

using HandleAllocatorGL = HandleAllocator<32, 96, 136>;
