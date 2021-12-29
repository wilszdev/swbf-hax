#pragma once
#include <cstdint>
#include <vector>

namespace scan
{
	// return first address matching pattern
	uintptr_t PatternScanFirst(const char* pattern, const char* mask, uintptr_t start, size_t length);

	// wrapper for internal, checks that memory region is valid
	uintptr_t InternalPatternScanFirst(const char* pattern, const char* mask, uintptr_t start, size_t length);

	// append vector with addresses, return the number of addresses added
	int PatternScanAll(std::vector<uintptr_t>* out, const char* pattern, const char* mask, uintptr_t start, size_t length);

	// another wrapper for internal
	int InternalPatternScanAll(std::vector<uintptr_t>* out, const char* pattern, const char* mask, uintptr_t start, size_t length);

}