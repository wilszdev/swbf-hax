#pragma once

namespace proc
{
	DWORD GetProcIdByName(const char* pName);
	uintptr_t GetModuleBaseAddress(DWORD pid, const char* modName);
}