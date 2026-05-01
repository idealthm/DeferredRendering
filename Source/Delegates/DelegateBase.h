#pragma once
#include <iostream>
#include <tuple>

#include "Common/Core.h"

template<typename FuncType, typename... VarTypes>
class TCommonDelegateInstance;

template<typename RetType, typename... ArgsTypes, typename... VarTypes>
class TCommonDelegateInstance<RetType(ArgsTypes...), VarTypes...>
{
public:
	virtual ~TCommonDelegateInstance() = default;

	template<typename... InVarTypes>
	explicit TCommonDelegateInstance(InVarTypes&&... Vars)
		: Payload(std::forward<InVarTypes>(Vars)...)
	{
	}   

	virtual RetType Execute(ArgsTypes... Params) = 0;

protected:
	std::tuple<VarTypes...> Payload;
};

template<int32_t Size, uint32_t Alignment>
struct TAlignedBytes
{
	alignas(Alignment) uint8_t Pad[Size];
};


using FAlignedInlineDelegateType = TAlignedBytes<16, 16>; 

class FDelegateAllocation
{
public:
	void* ResizeAllocation(size_t size)
	{
		size_t NewSize = (size + sizeof(FAlignedInlineDelegateType) - 1) / sizeof(FAlignedInlineDelegateType);
		if (NewSize != DelegateSize)
		{
			Data = realloc(Data, NewSize * sizeof(FAlignedInlineDelegateType));
			DelegateSize = NewSize;
		}
		return Data;
	}
protected:
	void* Data = nullptr;
	size_t DelegateSize = 0;
};

void* operator new(size_t size, class FDelegateAllocationWrapper& allocation);

class FDelegateAllocationWrapper
{
	friend void* operator new(size_t size, FDelegateAllocationWrapper& allocation);
public:
	FDelegateAllocationWrapper() = delete;

	FDelegateAllocationWrapper(FDelegateAllocation& Allocation)
		: Allocation(Allocation)
	{}

private:
	FDelegateAllocation& Allocation;
};


template <typename FuncType, typename FunctorType, typename... VarTypes>
class TBaseFunctorDelegateInstance;

template <typename RetValType, typename... ParamTypes, typename FunctorType, typename... VarTypes>
class TBaseFunctorDelegateInstance<RetValType(ParamTypes...), FunctorType, VarTypes...> : public TCommonDelegateInstance<RetValType(ParamTypes...), VarTypes...>
{
	using Super = TCommonDelegateInstance<RetValType(ParamTypes...), VarTypes...>;
public:
	template <typename InFunctorType, typename... InVarTypes>
	explicit TBaseFunctorDelegateInstance(InFunctorType&& InFunctor, InVarTypes&&... Vars)
		: Super	 (std::forward<InVarTypes>(Vars)...)
		, Functor(std::forward<InFunctorType>(InFunctor))
	{
		
	}

	virtual RetValType Execute(ParamTypes... Params) override
	{
		return [&](auto&&... params)
		{
			return std::apply([&](auto&&... Vars)
			{
				return std::forward<FunctorType>(Functor)(std::forward<ParamTypes>(params)..., std::forward<VarTypes>(Vars)...);
			}, this->Payload);
		}(std::forward<ParamTypes>(Params)...);
	}

private:
	mutable std::remove_const_t<FunctorType> Functor;
};

template<typename FuncType>
class TDelegate
{
	
};

template<typename InRetType, typename... ParamTypes>
class TDelegate<InRetType(ParamTypes...)> : public FDelegateAllocation
{
public:
	using RetType	= InRetType;
	using TFuncType = InRetType(*)(ParamTypes...);

	template<typename... VarTypes>						using TFuncPtr			= RetType(*)(ParamTypes..., VarTypes...);
	template<typename UserClass, typename... VarTypes>	using TMethodPtr		= RetType(UserClass::*)(ParamTypes..., VarTypes...);
	template<typename UserClass, typename... VarTypes>	using TConstMethodPtr	= RetType(UserClass::*)(ParamTypes..., VarTypes...) const;

	template<typename FunctorType, typename... VarTypes>
	void BindRaw(FunctorType&& func, VarTypes&&... Vars)
	{
		// DelegateStorge
		new ( FDelegateAllocationWrapper{*this} ) TBaseFunctorDelegateInstance<RetType(ParamTypes...), std::remove_reference_t<FunctorType>, std::decay_t<VarTypes>...>(std::forward<FunctorType>(func), std::forward<VarTypes>(Vars)...);
		// new ( FDelegateAllocationWrapper｛*this｝ ) TBaseFunctorDelegateInstance<RetType(ParamTypes...), std::remove_reference_t<FunctorType>, std::decay_t<VarTypes>...>(std::forward<FunctorType>(func), std::forward<VarTypes>(Vars)...);
	}

	RetType Execute(ParamTypes... Params) const
	{
		TCommonDelegateInstance<RetType(ParamTypes...)>* instance = static_cast<TCommonDelegateInstance<RetType(ParamTypes...)>*>(Data);
		return instance->Execute(std::forward<ParamTypes>(Params)...);
	}
};


namespace Delegate
{
	static void Test()
	{
		auto func = [](int a, int b ) {return a + b;};
		auto funcB = [](int a, int b, int c) {return a + b + c;};
		TDelegate<int(int, int)> d, e;
		d.BindRaw(func);
		e.BindRaw(funcB, 15);

		std::cout << d.Execute(123, 345) << std::endl;
		std::cout << e.Execute(234, 345) << std::endl;
	}
}