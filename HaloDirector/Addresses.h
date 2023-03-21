#pragma once
#include "StdInc.h"

namespace Halo
{
	extern float* p_fov;
	extern float fov;

	extern DWORD64 CameraHookAddress;

	extern Camera* p_Cam;
	extern Camera Cam;

	extern HWND pHwnd;

	extern float* p_viewMatrix;

	extern HMODULE hMod;

	extern float* p_timescale;
	extern float timescale;

	DWORD64 Scan(LPCWSTR modName, const char* pattern, const char* mask);

	void Initialise();

	char* ScanIn(const char* pattern, const char* mask, char* begin, unsigned int size);
};

namespace mem
{
	bool PatchAOB(void* dst, void* src, unsigned int size);
}
