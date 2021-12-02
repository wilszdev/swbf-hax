#pragma once

namespace mem
{
	uintptr_t ResolvePtrPath(HANDLE process, uintptr_t ptr, const std::vector<unsigned int>& offsets);
}