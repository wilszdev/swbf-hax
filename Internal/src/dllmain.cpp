#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#include "WindowsLessBloat.h"
#include "di.h"
#include "dx.h"
#include "util.h"
#include "drawing.hpp"
#include <cstdio>

#include "swbf-reclass.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_dx9.h"
#include "vendor/imgui/imgui_impl_win32.h"

struct shellcode_info
{
	void* GetModuleHandleA;
	void* FreeLibary;
	void* CloseHandle;
	void* Sleep;
};

DWORD WINAPI uninject_shellcode(shellcode_info* param)
{
	((void(*)(DWORD ms))param->Sleep)(1000);
	HMODULE module = ((HMODULE(*)(LPCSTR moduleName))param->GetModuleHandleA)("Internal.dll");
	if (module && module != INVALID_HANDLE_VALUE)
	{
		((BOOL(*)(HMODULE m))param->FreeLibary)(module);
		((BOOL(*)(HANDLE h))param->CloseHandle)(module);
	}
	return 0;
}

static void Uninject()
{
	puts("uninjecting dll...");
	void* memory = VirtualAlloc(0, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!memory)
	{
		puts(" failed");
		return;
	}

	uint8_t* shellcodeAddress = (uint8_t*)uninject_shellcode;

	// it's a jumptable entry, lets find destination address
	if (*shellcodeAddress == 0xE9)
	{
		int32_t offset = 0;
		memcpy(&offset, shellcodeAddress + 1, 4);
		offset += 5; // length of jmp instruction
		shellcodeAddress += offset;
	}

	printf(" shellcode is located at 0x%zx\n", (uintptr_t)shellcodeAddress);

	memcpy(memory, shellcodeAddress, 0x200); // theres no way the function is all that big

	shellcode_info info = { 0 };
	info.CloseHandle = CloseHandle;
	info.FreeLibary = FreeLibrary;
	info.GetModuleHandleA = GetModuleHandleA;
	info.Sleep = Sleep;

	memcpy((char*)memory + 0x200, &info, sizeof(info));

	printf(" wrote shellcode into memory at 0x%zx\n invoking shellcode...\n", (uintptr_t)memory);

	FreeConsole();

#pragma warning(push)
#pragma warning(disable: 6387)
	CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)memory, (char*)memory + 0x200, 0, 0));
#pragma warning(pop)
}

static void InitImGui(LPDIRECT3DDEVICE9 device, HWND window)
{
	puts("initialising imgui");
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.MouseDrawCursor = true;
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
	puts(" completed init.");
}

static void ShutdownImGui()
{
	puts("shutting down imgui");
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	puts(" completed shutdown.");
}

static LPD3DXFONT font = nullptr;

void hkDirectX_Reset()
{
	if (font)
	{
		font->Release();
		font = nullptr;
	}
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

static void ShutdownHax()
{
	if (font)
	{
		font->Release();
		font = nullptr;
	}
	ShutdownImGui();
	dx::RemoveHooks();
	di::RemoveHooks();
	Uninject();
}

static bool showImGuiMenu = false;
static uintptr_t exeBase = 0;
void hkDirectX_EndScene(LPDIRECT3DDEVICE9 device)
{
	static bool initialised = false;
	static HWND windowHandle = util::GetCurrentProcessWindow();
	if (!exeBase) exeBase = (uintptr_t)GetModuleHandleA(NULL);
	if (!initialised)
	{
		InitImGui(device, windowHandle);
		initialised = true;
	}

	static bool toggleKeyDownLastFrame = false;
	bool toggleKeyDownThisFrame = (GetAsyncKeyState(VK_DELETE));
	if (toggleKeyDownThisFrame && !toggleKeyDownLastFrame) showImGuiMenu = !showImGuiMenu;

	if (!font) font = drawing::CreateASingleFont(device, "Arial");
	drawing::WriteText(font, "press DELETE for hax", 10, 10, 100, 100, DT_LEFT, D3DCOLOR_ARGB(0xFF, 0xFF, 0x00, 0x00));

	static bool lockHealth = false;
	static float healthValue = 300.0f;
	static bool infiniteAmmo = false;

	// really weird macro, but i guess it works most of the time
#define PTR_IS_VALID(ptr) ((ptr) && (((uintptr_t)(ptr) & 0xFF000000) != 0x3F000000) && (((uintptr_t)(ptr)) != 0x2580E1C) && (((uintptr_t)(ptr)) != 0xF334F334) && (((uintptr_t)(ptr)) >= 0x1000))

#pragma region macros for accessing data
#define profileName ((wchar_t*)(exeBase + 0x5AB0D2))
#define spawnManagerPtr (((SpawnManager**)(exeBase + 0x62EA50)))
#define spawnManager (*spawnManagerPtr)
#define helpfulDataPtr (((TheStartOfSomeRandomDataThing**)(exeBase + 0x01D95D24)))
#define helpfulData (*helpfulDataPtr)
#define playerDataYay (helpfulData->localPlayerAimer.classWithPlayerDataYay)
#define entitySoldier (helpfulData->localPlayerAimer.classWithPlayerDataYay->currentEntitySoldierInstance)
#pragma endregion

#pragma region infinite ammo
	if (infiniteAmmo &&
		PTR_IS_VALID(spawnManagerPtr) &&
		PTR_IS_VALID(spawnManager) &&
		PTR_IS_VALID(spawnManager->playerCharacter) &&
		!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

		PTR_IS_VALID(helpfulDataPtr) &&
		PTR_IS_VALID(helpfulData) &&
		PTR_IS_VALID(helpfulData->localPlayerAimer.currentGun))
	{
		helpfulData->localPlayerAimer.currentGun->proportionOfAmmunitionLeftInMag = 1.0;
		helpfulData->localPlayerAimer.currentGun->weaponHeat = 0.0;
	}
#pragma endregion

#pragma region health lock
	if (PTR_IS_VALID(spawnManagerPtr) &&
		PTR_IS_VALID(spawnManager) &&
		PTR_IS_VALID(spawnManager->playerCharacter) &&
		!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

		PTR_IS_VALID(helpfulDataPtr) &&
		PTR_IS_VALID(helpfulData) &&
		PTR_IS_VALID(playerDataYay) &&
		PTR_IS_VALID(entitySoldier))
	{
		if (lockHealth)
		{
			entitySoldier->curHealth = healthValue;
		}
		else
		{
			healthValue = entitySoldier->curHealth;
		}
	}
#pragma endregion

	bool shouldShutdownHax = false;

	if (showImGuiMenu)
	{
#pragma region cursor
		auto& io = ImGui::GetIO();

		// use the position of the in game cursor, bc it locks to the window for us and stuff like that.
		RECT clientRect = { 0 };
		GetClientRect(windowHandle, &clientRect);

		// for some odd reason, (1024, 624) is the bottom left position for the in-game cursor.
		// it is independent of resolution or window size.
		io.MousePos.x = ((float)(*(int*)(exeBase + 0x7027C4))) / 1024.0f * (float)clientRect.right;
		io.MousePos.y = ((float)(*(int*)(exeBase + 0x7027C8))) / 624.0f * (float)clientRect.bottom;
		io.WantSetMousePos = true;
#pragma endregion

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

#pragma region imgui::begin with profile name
		if (profileName && *profileName)
		{
			char tmpProfileName[32] = { 0 };
			wcstombs_s(0, tmpProfileName, profileName, 32);
			char title[64] = { 0 };
			sprintf_s(title, 64, "Hello there, %s!", tmpProfileName);
			ImGui::Begin(title);
		}
		else
		{
			ImGui::Begin("Hello there!");
		}
#pragma endregion

#pragma region team and class names
		if (PTR_IS_VALID(spawnManagerPtr) && PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentClass) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentClass->className) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentTeam) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentTeam->teamName))
		{
			char tmpTeamName[32] = { 0 };
			char tmpClassName[32] = { 0 };

			wcstombs_s(0, tmpTeamName, spawnManager->playerCharacter->currentTeam->teamName, 32);
			wcstombs_s(0, tmpClassName, spawnManager->playerCharacter->currentClass->className, 32);
			ImGui::Text("team:%s\nclass:%s\n", tmpTeamName, tmpClassName);
		}
#pragma endregion

#pragma region health lock toggle
		if (PTR_IS_VALID(spawnManagerPtr) &&
			PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

			PTR_IS_VALID(helpfulDataPtr) &&
			PTR_IS_VALID(helpfulData) &&
			PTR_IS_VALID(playerDataYay) &&
			PTR_IS_VALID(entitySoldier))
		{
			ImGui::Checkbox("lock health", &lockHealth);
			if (lockHealth)
			{
				ImGui::SliderFloat("health", &healthValue, 0.0f, 5000.0f);
			}
			else
			{
				ImGui::SliderFloat("health", &(entitySoldier->curHealth), 0.0f, 5000.0f);
				healthValue = entitySoldier->curHealth;
			}
		}
#pragma endregion

#pragma region infinite ammo toggle
		if (PTR_IS_VALID(spawnManagerPtr) &&
			PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			!PTR_IS_VALID(spawnManager->playerCharacter->ptrToControllingThing) &&

			PTR_IS_VALID(helpfulDataPtr) &&
			PTR_IS_VALID(helpfulData) &&
			PTR_IS_VALID(helpfulData->localPlayerAimer.currentGun))
		{
			ImGui::Checkbox("infinite ammo", &infiniteAmmo);
		}
#pragma endregion

#pragma region instant win

		// instant win by setting remaining units of other team to zero
		if (PTR_IS_VALID(spawnManagerPtr) &&
			PTR_IS_VALID(spawnManager) &&
			PTR_IS_VALID(spawnManager->playerCharacter) &&
			PTR_IS_VALID(spawnManager->playerCharacter->currentTeam) &&
			ImGui::Button("instant win"))
		{
			int index = spawnManager->playerCharacter->currentTeam->indexInTeamsArray;

			int targetIndex = -1;
			if (index == 1) targetIndex = 2;
			else if (index == 2) targetIndex = 1;

			if (targetIndex != -1)
			{
				spawnManager->Teams[targetIndex]->remainingUnits = 0;
			}
		}
#pragma endregion

		if (ImGui::Button("Uninject hax dll"))
		{
			shouldShutdownHax = true;
		}

		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	toggleKeyDownLastFrame = toggleKeyDownThisFrame;
	if (shouldShutdownHax || GetAsyncKeyState(VK_INSERT))
	{
		ShutdownHax();
	}
}

void hkDirectInput_GetDeviceState(DIMOUSESTATE2* mouseState)
{
	if (showImGuiMenu)
	{
		if (!exeBase) exeBase = (uintptr_t)GetModuleHandleA(NULL);

		// pass mouse clicks to imgui
		auto& io = ImGui::GetIO();
		for (int i = 0; i < 5; ++i)
			io.MouseDown[i] = mouseState->rgbButtons[i];

		// update in-game mouse cursor ourselves
		int* cursorX = (int*)(exeBase + 0x7027C4);
		int* cursorY = (int*)(exeBase + 0x7027C8);

		*cursorX += mouseState->lX;
		*cursorY += mouseState->lY;

		if (*cursorX < 0) *cursorX = 0;
		if (*cursorY < 0) *cursorY = 0;
		if (*cursorX > 1024) *cursorX = 1024;
		if (*cursorY > 624) *cursorX = 624;

		// block mouse inputs for game
		for (int i = 0; i < 8; ++i)
			mouseState->rgbButtons[i] = 0;
		mouseState->lX = 0;
		mouseState->lY = 0;
		mouseState->lZ = 0;
	}
}

static void WINAPI InjectedThread(HMODULE module)
{
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	printf(
		"===========================\n"
		"injected dll at 0x%zx\n"
		"===========================\n", (uintptr_t)module);

	HWND window = util::GetCurrentProcessWindow();

	if (IsWindowUnicode(window))
	{
		wchar_t buffer[128] = { 0 };
		GetWindowTextW(window, buffer, sizeof(buffer) / sizeof(*buffer));
		wprintf(L"window text: %s\n", buffer);
	}
	else
	{
		char buffer[128] = { 0 };
		GetWindowTextA(window, buffer, sizeof(buffer) / sizeof(*buffer));
		printf("window text: %s\n", buffer);
	}

	bool hooksSuccess = dx::CreateHooks(window) && di::CreateHooks();

	if (!hooksSuccess)
	{
		puts("failed to create hooks. try injecting again.");
		FreeConsole();
		FreeLibraryAndExitThread(module, 0);
	}
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
#pragma warning(push)
#pragma warning(disable : 6387)
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InjectedThread, module, 0, 0));
#pragma warning(pop)
	}
	return TRUE;
}