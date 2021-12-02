#pragma once
#include "WindowsLessBloat.h"

namespace util
{
	// may not work if process has multiple
	// windows, as it will return the handle
	// to the first it finds.
	HWND GetCurrentProcessWindow();
}