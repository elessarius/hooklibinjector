#include <Windows.h>
#include "MinHook.h"

//typedef HANDLE(WINAPI *MESSAGEBOXW)(HWND, LPCWSTR, LPCWSTR, UINT);
// Pointer for calling original MessageBoxW.
static int (WINAPI *fpOriginalFunc)(
	LPCTSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDistribution,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	);

// Detour function which overrides MessageBoxW.
//__declspec(dllexport) 
HANDLE  WINAPI DetourHookFunc
(
	LPCTSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDistribution,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
	return (HANDLE)MessageBoxW(NULL, L"hooked...", L"MinHook Sample", MB_OK);
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "Attached!", MB_OK);
		// Initialize MinHook.
		if (MH_Initialize() != MH_OK)
		{
			return TRUE;
		}

		// Create a hook for MessageBoxW, in disabled state.
		if (MH_CreateHook(&CreateFileA, &DetourHookFunc,
			reinterpret_cast<LPVOID*>(&fpOriginalFunc)) != MH_OK)
		//if (MH_CreateHookApi(L"kernel32", "CreateFileA", &DetourHookFunc,
		//	reinterpret_cast<LPVOID*>(&fpOriginalFunc)) != MH_OK)
		{
			MessageBoxA(NULL, "Fail Create Hook", "Fail!", MB_OK);
			return TRUE;
		}

		// Enable the hook for MessageBoxW.
		if (MH_EnableHook(&CreateFileA) != MH_OK)
		{
			return TRUE;
		}
		break;
	case DLL_THREAD_ATTACH:
		MessageBoxA(NULL, "DLL_THREAD_ATTACH!", "Attached", 0);
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		// Uninitialize MinHook.
		MessageBoxA(NULL, "DLL_PROCESS_DETACH!", "Detached", 0);
		if (MH_Uninitialize() != MH_OK)
		{
			return TRUE;
		}
		break;
	}
	return TRUE;
}