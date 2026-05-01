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
	static constexpr uint32_t GEN_BITS = 8;
	static constexpr uint32_t OFFSET_MASK = (1u << OFFSET_BITS) - 1;
	static constexpr uint32_t GEN_MASK = (1u << GEN_BITS) - 1;

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
	Handle<D> allocate()
	{
		constexpr size_t bucketSize = getBucketSize<D>();
		uint8_t poolAge;
		void* p = mAllocator.alloc(bucketSize, Allocator::MIN_ALIGNMENT, sizeof(typename Allocator::Node), &poolAge);
		ASSERT(p);

		D* obj = ::new (p) D();

		uint32_t offset = uint32_t((char*)p - (char*)mHeapArea.begin());
		ASSERT(offset <= OFFSET_MASK);

		// bump generation stored in the Node before this element
		auto* node = static_cast<typename Allocator::Node*>(p) - 1;
		node->generation = (node->generation + 1) & GEN_MASK;
		if (node->generation == 0) { node->generation = 1; }

		HandleBase::HandleId id = (uint32_t(node->generation) << OFFSET_BITS) | offset;
		return Handle<D>(id);
	}

	template<typename D>
	void deallocate(Handle<D>& handle)
	{
		if (!handle) return;

		void* p = pointerFromHandle(handle);
		auto* node = static_cast<typename Allocator::Node*>(p) - 1;

		D* obj = static_cast<D*>(p);
		obj->~D();

		constexpr size_t bucketSize = getBucketSize<D>();
		mAllocator.free(p, bucketSize, node->age);

		handle = Handle<D>();
	}

	template<typename D>
	D* get(Handle<D> handle) const
	{
		if (!handle) return nullptr;

		HandleBase::HandleId id = handle.GetId();
		uint32_t offset = id & OFFSET_MASK;
		uint32_t gen = (id >> OFFSET_BITS) & GEN_MASK;

		if (offset >= mHeapArea.size()) return nullptr;

		void* p = (char*)mHeapArea.begin() + offset;
		auto* node = static_cast<typename Allocator::Node*>(p) - 1;
		if (node->generation != gen) return nullptr;

		return static_cast<D*>(p);
	}

private:
	template<typename D>
	static constexpr size_t getBucketSize() noexcept {
		if constexpr (sizeof(D) <= P0) { return P0; }
		if constexpr (sizeof(D) <= P1) { return P1; }
		static_assert(sizeof(D) <= P2);
		return P2;
	}

	void* pointerFromHandle(HandleBase handle) const
	{
		HandleBase::HandleId id = handle.GetId();
		uint32_t offset = id & OFFSET_MASK;
		uint32_t gen = (id >> OFFSET_BITS) & GEN_MASK;

		ASSERT(offset < mHeapArea.size());

		void* p = (char*)mHeapArea.begin() + offset;
		auto* node = static_cast<typename Allocator::Node*>(p) - 1;
		ASSERT(node->generation == gen);

		return p;
	}

	class Allocator {
		friend class HandleAllocator;
		static constexpr size_t MIN_ALIGNMENT = alignof(std::max_align_t);
		struct Node {
			uint8_t age;        // 4-bit pool-level double-free detection
			uint8_t generation; // 8-bit handle-level stale access detection
		};
		template<size_t SIZE>
		using Pool = Utils::PoolAllocator<SIZE, MIN_ALIGNMENT, sizeof(Node)>;
		Pool<P0> mPool0;
		Pool<P1> mPool1;
		Pool<P2> mPool2;
		const Utils::AreaPolicy::HeapArea& mArea;
		bool mUseAfterFreeCheckDisabled;
	public:
		explicit Allocator(const Utils::AreaPolicy::HeapArea& area, bool disableUseAfterFreeCheck)
			: mArea(area)
			, mUseAfterFreeCheckDisabled(disableUseAfterFreeCheck)
		{
			const size_t totalSize = area.size();
			const size_t totalWeight = P0 + P1 + P2;
			char* const base = static_cast<char*>(area.begin());

			size_t size0 = totalSize * P0 / totalWeight;
			size_t size1 = totalSize * P1 / totalWeight;
			size_t size2 = totalSize - size0 - size1;

			// Align sub-area sizes to element boundaries
			size0 = (size0 / P0) * P0;
			size1 = (size1 / P1) * P1;

			mPool0 = Pool<P0>(base, size0);
			mPool1 = Pool<P1>(base + size0, size1);
			mPool2 = Pool<P2>(base + size0 + size1, size2);
		}

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
