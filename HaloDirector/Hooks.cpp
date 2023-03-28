#include "pch.h"
#include "StdInc.h"
#include "UI.h"
#include "MinHook.h"


using namespace Halo;

#define DX11_PRESENT_INDEX 8

MSG msg;
DWORD current_process = 0;

struct Patch {
	void* address;
	const char* bytes[50];
	int length;
};

int patchcount = 0;
Patch* patches[50];
bool Hooked = false;

DWORD64 Hooks::CreateHook(void* toHook, void* hk_func, int len)
{
	if (len < 14) {                                            //if less than 13 bytes
		return 0;                                         //we gtfo
	}

	//allocate 50 byte buffer
	Hooked = true;

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

	//copy the original bytes, for restoring

	patches[patchcount] = new Patch();

	memcpy(patches[patchcount]->bytes, toHook, len);
	patches[patchcount]->length = len;
	patches[patchcount]->address = toHook;
	patchcount++;

	memset(toHook, 0x90, len);

	unsigned char patch[] = {
		0xFF, 0x25,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 //Address goes here
	};
	*(DWORD64*)&patch[6] = (DWORD64)hk_func; //replacing zeros with our function address

	memcpy((void*)toHook, patch, sizeof(patch));
	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);
	return((DWORD64)toHook) + len;
}


#pragma region MouseHook

bool get_state() {
	if (GetMessage(&msg, GetActiveWindow(), 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

bool is_foreground() {
	HWND foreground = GetForegroundWindow();
	DWORD foregroundID = 0;
	GetWindowThreadProcessId(foreground, &foregroundID);

	return (foregroundID == current_process);
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		if (!Halo::p_Cam || !Halo::p_fov || !Hooks::Initialised() || IsBadWritePtr(Halo::p_Cam, sizeof(Camera)) || IsBadWritePtr(Halo::p_fov, sizeof(float)))
			return CallNextHookEx(0, nCode, wParam, lParam);

		if (wParam == WM_MOUSEWHEEL)
		{
			MSLLHOOKSTRUCT* pMhs = (MSLLHOOKSTRUCT*)lParam;
			short zDelta = HIWORD(pMhs->mouseData);
			HWND foreground = GetForegroundWindow();
			DWORD foregroundID = 0;
			GetWindowThreadProcessId(foreground, &foregroundID);

			if (foregroundID == current_process)
			{
				if (zDelta < 0)
				{
					//Down
					if (GetKeyState(VK_SHIFT) & 0x8000)
					{
						if (*Halo::p_fov + 5.0f < 150.0f)
						{
							*Halo::p_fov += 5.0f;
						}
						else {
							*Halo::p_fov = 150.0f;
						}
					}
					else {
						Halo::p_Cam->rotation.z += Math::radians(5);
					}

				}
				else {
					//Up
					if (GetKeyState(VK_SHIFT) & 0x8000)
					{
						if (*Halo::p_fov - 5.0f > 1.0f) {
							*Halo::p_fov -= 5.0f;
						}
						else {
							*Halo::p_fov = 1.0f;
						}
					}
					else {
						Halo::p_Cam->rotation.z -= Math::radians(5);
					}
				}
			}
		}
		else if (wParam == WM_MBUTTONDOWN)
		{
			HWND foreground = GetForegroundWindow();
			DWORD foregroundID = 0;
			GetWindowThreadProcessId(foreground, &foregroundID);

			Log::Debug("Resetting to default");

			if (foregroundID == current_process)
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					*p_fov = 80.0;
				}
				else {
					p_Cam->rotation.z = 0;
				}
			}
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	static bool bKeySubtractPressed = false;
	static bool bKeyAddPressed = false;
	BOOL fEatKeystroke = FALSE;

	if (nCode == HC_ACTION && is_foreground())
	{
		PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			
			switch (p->vkCode)
			{
			case VK_ADD:
				if (!bKeyAddPressed)
				{
					DollyCam::AddDollyTick(1);
					Log::Info("Add 1 Tick(Dolly)");
					bKeyAddPressed = true;
				}
				break;
			case VK_SUBTRACT:
				if (!bKeySubtractPressed)
				{
					DollyCam::AddDollyTick(-1);
					Log::Info("Add -1 Tick(Dolly)");
					bKeySubtractPressed = true;
				}
				break;
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			switch (p->vkCode)
			{
				case VK_F2:
					Settings::draw_camera_path = !Settings::draw_camera_path;
					break;
				case 'Q':
					Log::Info("Executing: %s", UI::GetCurrentName());
					UI::Do();
					break;
				case '1':
					UI::Left();
					Log::Info("Index: %d  ->  %s", UI::GetIndex(), UI::GetCurrentName());
					break;
				case '2':
					UI::Right();
					Log::Info("Index: %d  ->  %s", UI::GetIndex(), UI::GetCurrentName());
					break;
				case VK_INSERT:
					DollyCam::AddMarkerDollyTick();
					Log::Info("Inserting Camera Marker(Dolly)");
					break;
				case VK_NEXT:
					if (*Halo::p_timescale - 0.1 >= 0)
						*Halo::p_timescale -= 0.1;
					else 
						*Halo::p_timescale = 0.0;
					break;
				case VK_PRIOR:
					if (*Halo::p_timescale < 10.0f)
						*Halo::p_timescale += 0.1;
					break;
				case VK_ADD:
					bKeyAddPressed = false;
					break;
				case VK_SUBTRACT:
					bKeySubtractPressed = false;
					break;
				case VK_DELETE:
					DollyCam::RemoveClosestNode();
					Log::Info("RemoveClosestNode");
					break;
			}
			break;
		}
	}

	if (bKeyAddPressed || bKeySubtractPressed) {
		DollyCam::AddDollyTick(bKeyAddPressed ? 1 : -1);
	}

	return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

DWORD WINAPI MouseHook(LPVOID param)
{
	HHOOK mousehook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
	HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

	while (true)
	{
		get_state();
		Sleep(20);
		UnhookWindowsHookEx(mousehook);
		return 0;
	}
}

#pragma endregion

#pragma region Keyboard Hook

DWORD WINAPI KeyboardHook(LPVOID param) {
	

	//while (true) {
	//
	//}

}

#pragma endregion

static uintptr_t hModule;
static uintptr_t TEBAddress;

static uintptr_t pTarget_0;
static uintptr_t pTarget_1;
static uintptr_t pTarget_2;
uintptr_t ppOriginal_0;
uintptr_t ppOriginal_1;
uintptr_t ppOriginal_2;
bool bInit_0 = false;
DWORD64 DollyHook_Return;

extern "C" __int64 GetTeb(void);
extern "C" __int32 GetTick(void);

void Dolly_Hook() {
	if (!bInit_0)
	{
		TEBAddress = GetTeb();
		DollyCam::Init(hModule, TEBAddress);
		bInit_0 = true;
	}
	if (DollyCam::IsSync())
		for (int i = 0; i < GetTick(); i++)
			DollyCam::MainFunction();
	else
		DollyCam::MainFunction();
}

void Uninit_Hook() {
	bInit_0 = false;
	DollyCam::Uninit();
}

extern "C" void SetDraw(void* bdraw);
extern "C" void SetDolly(void* f, void* ori);
extern "C" void SetUninit(void* f, void* ori);
extern "C" void SetCam(void* cam, void* ori);
extern "C" void HookDolly(void);
extern "C" void HookCamera(void);
extern "C" void HookUninit(void);


bool bdraw = false;

// 1,2835,0,0
#define OFFSET_pTarget_0 0xB2808
#define OFFSET_pTarget_1 0x208270
#define OFFSET_pTarget_2 0xB2ED6
#define OFFSET_p_fov 0x29D2BB0
#define OFFSET_p_timescale 0x1E9B69C

DWORD WINAPI HookThread(LPVOID lpReserved)
{
	SetDolly(Dolly_Hook, &ppOriginal_0);
	SetUninit(Uninit_Hook, &ppOriginal_2);
	MH_Initialize();
	while (true)
	{
		hModule = (uintptr_t)GetModuleHandleW(L"halo3.dll");
		if (!hModule) continue;
		pTarget_0 = hModule + OFFSET_pTarget_0;
		pTarget_2 = hModule + OFFSET_pTarget_2;
		Halo::p_fov = (float*)(hModule + OFFSET_p_fov + 0x8 + 0x6C);
		Halo::p_timescale = (float*)(hModule + OFFSET_p_timescale);

		if (*(BYTE*)pTarget_0 != 0xE9)
		{
			bInit_0 = false;
			MH_DisableHook((LPVOID)pTarget_0);
			MH_CreateHook((LPVOID)pTarget_0, HookDolly, (LPVOID*)&ppOriginal_0);
			MH_EnableHook((LPVOID)pTarget_0);
			MH_DisableHook((LPVOID)pTarget_2);
			MH_CreateHook((LPVOID)pTarget_2, HookUninit, (LPVOID*)&ppOriginal_2);
			MH_EnableHook((LPVOID)pTarget_2);
		}
	}
	MH_Uninitialize();
}

void Hooks::Initialise()
{
	CreateThread(NULL, 0, &MouseHook, NULL, 0, NULL);
	CreateThread(NULL, 0, &HookThread, NULL, 0, NULL);
	//CreateThread(NULL, 0, &KeyboardHook, NULL, 0, NULL);
	current_process = GetCurrentProcessId();

    Log::Info("Hooks Initialised");
} 

bool Hooks::Initialised()
{
	hModule = (uintptr_t)GetModuleHandleW(L"halo3.dll");
	if (!hModule) return false;
	pTarget_0 = hModule + OFFSET_pTarget_0;
	return (*(BYTE*)pTarget_0 == 0xE9) && bInit_0;
}
