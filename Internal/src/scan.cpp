#include "scan.h"
#include "WindowsLessBloat.h"

uintptr_t scan::PatternScanFirst(char* pattern, char* mask, uintptr_t start, size_t length)
{
	size_t patternLength = strlen(mask);
	for (size_t i = 0; i < length; ++i)
	{
		bool match = true;
		for (size_t j = 0; j < patternLength; ++j)
		{
			bool currentCharMatches = (mask[j] == '?') || (pattern[j] == *(char*)(start + i + j));
			match &= currentCharMatches;
		}
		if (match)
			return start + i;
	}
	return 0;
}

uintptr_t scan::InternalPatternScanFirst(char* pattern, char* mask, uintptr_t start, size_t length)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	for (uintptr_t current = start; current < start + length; current += mbi.RegionSize)
	{
		if (!VirtualQuery((void*)current, &mbi, sizeof(mbi)) ||
			mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		return PatternScanFirst(pattern, mask, current, mbi.RegionSize);
	}

	return 0;
}

int scan::PatternScanAll(std::vector<uintptr_t>* out, char* pattern, char* mask, uintptr_t start, size_t length)
{
	int count = 0;
	size_t patternLength = strlen(mask);
	for (size_t i = 0; i < length; ++i)
	{
		bool match = true;
		for (size_t j = 0; j < patternLength; ++j)
		{
			bool currentCharMatches = (mask[j] == '?') || (pattern[j] == *(char*)(start + i + j));
			match &= currentCharMatches;
		}
		if (match)
		{
			out->emplace_back(start + i);
			++count;
		}
	}
	return count;
}

int scan::InternalPatternScanAll(std::vector<uintptr_t>* out, char* pattern, char* mask, uintptr_t start, size_t length)
{
	int count = 0;
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	for (uintptr_t current = start; current < start + length; current += mbi.RegionSize)
	{
		if (!VirtualQuery((void*)current, &mbi, sizeof(mbi)) ||
			mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		count += PatternScanFirst(pattern, mask, current, mbi.RegionSize);
	}

	return count;
}