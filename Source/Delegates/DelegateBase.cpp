#include "DelegateBase.h"

void* operator new(size_t size, FDelegateAllocationWrapper& wrapper)
{
	return wrapper.Allocation.ResizeAllocation(size);
}