#pragma once
#include <cstdint>
#include <limits.h>

#include "Core.h"


class HandleBase
{
public:
	using HandleId = uint32_t;
	static constexpr const HandleId NullId = HandleId{UINT_MAX};

	constexpr HandleBase() : mObjectId(NullId){}

	explicit operator bool() const { return mObjectId != NullId; }

	HandleId GetId() const { return mObjectId; }

	explicit HandleBase(HandleId id) noexcept : mObjectId(id) {
		ASSERT(mObjectId != NullId);
	}
protected:
	HandleBase(HandleBase const& rhs) noexcept = default;
	HandleBase& operator=(HandleBase const& rhs) noexcept = default;

	HandleBase(HandleBase&& rhs) noexcept
			: mObjectId(rhs.mObjectId) {
		rhs.mObjectId = NullId;
	}

	HandleBase& operator=(HandleBase&& rhs) noexcept {
		if (this != &rhs) {
			mObjectId = rhs.mObjectId;
			rhs.mObjectId = NullId;
		}
		return *this;
	}
private:
	HandleId mObjectId;
};


template<typename T>
class Handle : public HandleBase
{
public:
	Handle() noexcept = default;

	Handle(Handle const& rhs) noexcept = default;
	Handle(Handle&& rhs) noexcept = default;

	using HandleBase::HandleBase;

	// compare handles of the same type
	bool operator==(const Handle& rhs) const noexcept { return GetId() == rhs.GetId(); }
	bool operator!=(const Handle& rhs) const noexcept { return GetId() != rhs.GetId(); }
	bool operator<(const Handle& rhs) const noexcept { return GetId() < rhs.GetId(); }
	bool operator<=(const Handle& rhs) const noexcept { return GetId() <= rhs.GetId(); }
	bool operator>(const Handle& rhs) const noexcept { return GetId() > rhs.GetId(); }
	bool operator>=(const Handle& rhs) const noexcept { return GetId() >= rhs.GetId(); }

	template<typename B, typename = std::enable_if_t<std::is_base_of_v<T, B>> >
	Handle(Handle<B> const& base) noexcept : HandleBase(base) { }
};
