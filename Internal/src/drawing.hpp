#pragma once
#include "WindowsLessBloat.h"
#include <d3d9.h>
#include <d3dx9.h>

namespace drawing
{
	// maybe look at this guys drawing functions for reference
	// https://github.com/TosoxDev/FrmeZ/blob/master/Utils/Drawing/D3DGUI.cpp

	void DrawFilledRect(IDirect3DDevice9* device, int x, int y, int w, int h, D3DCOLOR colour)
	{
		D3DRECT rect{ x, y, x + w, y + h };
		device->Clear(1, &rect, D3DCLEAR_TARGET, colour, 0.0f, 0);
	}

	LPD3DXFONT CreateASingleFont(IDirect3DDevice9* device, const char* fontFaceName)
	{
		LPD3DXFONT font = nullptr;
		D3DXCreateFontA(device, 32, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FW_DONTCARE, fontFaceName, &font);
		return font;
	}

	// render text indside a bounding box
	// format parameter is the DT_<format> macros, such as DT_LEFT, DT_CENTRE, etc.
	void WriteText(LPD3DXFONT font, const char* text, int x, int y, int w, int h, DWORD format, D3DCOLOR colour)
	{
		RECT rect{ x, y, x + w, y + h };
		font->DrawTextA(0, text, -1, &rect, format | DT_NOCLIP, colour);
	}
}