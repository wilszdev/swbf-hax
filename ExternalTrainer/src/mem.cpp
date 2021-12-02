#include "pch.h"
#include "mem.h"

namespace mem
{
	uintptr_t ResolvePtrPath(HANDLE process, uintptr_t ptr, const std::vector<unsigned int>& offsets)
	{
		uintptr_t address = ptr;
		for (auto offset : offsets)
		{
			if (!address || !ReadProcessMemory(process, (void*)address, &address, sizeof(address), 0))
				continue;
			address += offset;
		}
		return address;
	}
}