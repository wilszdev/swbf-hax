#include "util.h"
namespace util
{
	static BOOL CALLBACK EnumProcFindFirst(HWND window, LPARAM param)
	{
		DWORD pid;
		GetWindowThreadProcessId(window, &pid);
		if (pid == GetCurrentProcessId())
		{
			*reinterpret_cast<HWND*>(param) = window;
			return FALSE;
		}
		return TRUE;
	}


	HWND GetCurrentProcessWindow()
	{
		HWND window = 0;
		EnumWindows(EnumProcFindFirst, (LPARAM)&window);
		return window;
	}
	
	static BOOL CALLBACK EnumProcFindAll(HWND window, LPARAM param)
	{
		DWORD pid;
		GetWindowThreadProcessId(window, &pid);
		if (pid == GetCurrentProcessId())
		{
			reinterpret_cast<std::vector<HWND>*>(param)->push_back(window);
		}
		return TRUE;
	}
	
	std::vector<HWND> GetCurrentProcessWindows()
	{
		std::vector<HWND> windows{};
		EnumWindows(EnumProcFindAll, (LPARAM)&windows);
		return windows;
	}
}