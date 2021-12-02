#include "pch.h"
#include "proc.h"

namespace proc
{
	DWORD GetProcIdByName(const char* pName)
	{
		DWORD pid = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 procEntry = {};
			procEntry.dwSize = sizeof(procEntry);

			BOOL ret = Process32First(hSnap, &procEntry);
			while (ret)
			{
				if (!_strcmpi(pName, procEntry.szExeFile))
				{
					pid = procEntry.th32ProcessID;
					break;
				}
				ret = Process32Next(hSnap, &procEntry);
			}
		}
		CloseHandle(hSnap);
		return pid;
	}

	uintptr_t GetModuleBaseAddress(DWORD pid, const char* modName)
	{
		uintptr_t addr = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

		if (hSnap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 modEntry = {};
			modEntry.dwSize = sizeof(modEntry);

			BOOL ret = Module32First(hSnap, &modEntry);
			while (ret)
			{
				if (!_strcmpi(modName, modEntry.szModule))
				{
					addr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
				ret = Module32Next(hSnap, &modEntry);
			}
		}
		CloseHandle(hSnap);
		return addr;
	}
}