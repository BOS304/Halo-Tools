#include "Addresses.h"

float* Halo::p_fov;
float Halo::fov;
DWORD64 Halo::CameraHookAddress;
Camera* Halo::p_Cam;
Camera Halo::Cam;
HWND Halo::pHwnd;
HMODULE Halo::hMod;

float* Halo::p_viewMatrix;
float* Halo::p_timescale;
float Halo::timescale;

struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};


BOOL is_main_window(HWND handle)
{
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
    handle_data& data = *(handle_data*)lParam;
    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id || !is_main_window(handle))
        return TRUE;
    data.window_handle = handle;
    return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
    handle_data data;
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}

char* Halo::ScanIn(const char* pattern, const char* mask, char* begin, unsigned int size)
{
    unsigned int patternLength = strlen(mask);

    for (unsigned int i = 0; i < size - patternLength; i++)
    {
        bool found = true;
        for (unsigned int j = 0; j < patternLength; j++)
        {
            if (mask[j] != '?' && pattern[j] != *(begin + i + j))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return (begin + i);
        }
    }
    return nullptr;
}

DWORD64 Halo::Scan(LPCWSTR modName, const char* pattern, const char* mask)
{
    HMODULE mod;
    while ((mod = GetModuleHandleW(modName)) == 0);
    MODULEINFO info = MODULEINFO();
    GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(MODULEINFO));
    return (DWORD64)ScanIn(pattern, mask, (char*)info.lpBaseOfDll, info.SizeOfImage);
}

DWORD64 GetBaseAddress(LPCWSTR modName) {
    HMODULE mod = GetModuleHandleW(modName);
    MODULEINFO info = MODULEINFO();
    GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(MODULEINFO));
    return (DWORD64)info.lpBaseOfDll;
}

bool mem::PatchAOB(void* dst, void* src, unsigned int size)
{
    bool _flag = false;
    DWORD oldprotect;
    
    if (VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect))
    {
        _flag = !IsBadWritePtr(dst, size) && !IsBadReadPtr(src, size);
        if (_flag)	memcpy(dst, src, size);
    }
    VirtualProtect(dst, size, oldprotect, &oldprotect);
    return _flag;
}

void Halo::Initialise() {
    Log::Info("---------- Getting HWND -----------");
    pHwnd = find_main_window(GetCurrentProcessId());
    Log::Info("Hwnd: %llx", pHwnd);
}

namespace Halo
{
    namespace Console {
        void SetFov(const char* arg)
        {
            if (arg == NULL)
            {
                Log::Info("Received 0 Argument. Expected 1 Argument.");
                return;
            }

            if (!Hooks::Initialised())
            {
                Log::Info("Not in game!");
                return;
            }

            float num = 0;

            try {
                num = std::stof(arg);
            }
            catch (std::invalid_argument const& e) {
                Log::Error("Console Commands -> Invalid Argument");
            }
            catch (std::out_of_range const& e) {
                Log::Error("Console Commands -> Out of Range");
            }

            *p_fov = num;
        }

        void Init()
        {
            ConsoleCommands::Add("camera_set_fov", SetFov);
        }
    }
}