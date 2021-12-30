#include "scan.h"
#include "WindowsLessBloat.h"

uintptr_t scan::PatternScanFirst(const char* pattern, const char* mask, uintptr_t start, size_t length)
{
	size_t patternLength = strlen(mask);
	for (size_t i = 0; i < length; i += 4)
	{
		bool match = true;
		for (size_t j = 0; j < patternLength; j += 4)
		{
			uint32_t fourByteMask = -1;
			if (mask[j + 0] == '?') fourByteMask &= 0x00FFFFFF;
			if (mask[j + 1] == '?') fourByteMask &= 0xFF00FFFF;
			if (mask[j + 2] == '?') fourByteMask &= 0xFFFF00FF;
			if (mask[j + 3] == '?') fourByteMask &= 0xFFFFFF00;

			bool currentMatch = ((*(uint32_t*)(pattern + j) & fourByteMask) == (*(uint32_t*)(start + i + j) & fourByteMask));
			match &= currentMatch;
		}
		if (match)
			return start + i;
	}
	return 0;
}

uintptr_t scan::InternalPatternScanFirst(const char* pattern, const char* mask, uintptr_t start, size_t length)
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

int scan::PatternScanAll(std::vector<uintptr_t>* out, const char* pattern, const char* mask, uintptr_t start, size_t length)
{
	int count = 0;
	size_t patternLength = strlen(mask);
	for (size_t i = 0; i < length; i += 4)
	{
		bool match = true;
		for (size_t j = 0; j < patternLength; j += 4)
		{
			uint32_t fourByteMask = -1;
			if (mask[j + 0] == '?') fourByteMask &= 0x00FFFFFF;
			if (mask[j + 1] == '?') fourByteMask &= 0xFF00FFFF;
			if (mask[j + 2] == '?') fourByteMask &= 0xFFFF00FF;
			if (mask[j + 3] == '?') fourByteMask &= 0xFFFFFF00;

			bool currentMatch = ((*(uint32_t*)(pattern + j) & fourByteMask) == (*(uint32_t*)(start + i + j) & fourByteMask));
			match &= currentMatch;
		}
		if (match)
		{
			out->emplace_back(start + i);
			++count;
		}
	}
	return count;
}

int scan::InternalPatternScanAll(std::vector<uintptr_t>* out, const char* pattern, const char* mask, uintptr_t start, size_t length)
{
	int count = 0;
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	for (uintptr_t current = start; current < start + length; current += mbi.RegionSize)
	{
		if (!VirtualQuery((void*)current, &mbi, sizeof(mbi)) ||
			mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		count += PatternScanAll(out, pattern, mask, current, 
			mbi.RegionSize > length ? length : mbi.RegionSize);
	}

	return count;
}