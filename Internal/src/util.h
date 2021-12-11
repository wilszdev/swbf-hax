#pragma once
#include "WindowsLessBloat.h"
#include <vector>
namespace util
{
	// may not work if process has multiple
	// windows, as it will return the handle
	// to the first it finds.
	HWND GetCurrentProcessWindow();

	// should use this instead, should find all windows
	std::vector<HWND> GetCurrentProcessWindows();
}