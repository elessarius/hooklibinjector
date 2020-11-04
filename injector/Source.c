#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>

BOOL setPrivileges(LPCTSTR szPrivName)
{
	TOKEN_PRIVILEGES tp = { 0 };
	HANDLE hToken = 0;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		printf("OpenProcessToken failed\n");

	if (!LookupPrivilegeValue(NULL, szPrivName, &tp.Privileges[0].Luid))
		printf("LookupPrivilegeValue failed\n");

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
	{
		printf("AdjustTokenPrivileges failed\n");
		CloseHandle(hToken);
		return TRUE;
	}

	return FALSE;
}

BOOL IsWow64(HANDLE process)
{
	BOOL bIsWow64 = FALSE;

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(process, &bIsWow64))
		{
			//handle error
		}
	}
	return bIsWow64;
}

BOOL IsX86Process(HANDLE process)
{
	SYSTEM_INFO systemInfo = { 0 };
	GetNativeSystemInfo(&systemInfo);

	// x86 environment
	if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
		return TRUE;

	// Check if the process is an x86 process that is running on x64 environment.
	// IsWow64 returns true if the process is an x86 process
	return IsWow64(process);
}

DWORD getPIDproc(char *procname)
{
	DWORD pid = 0;
	PROCESSENTRY32 peProcessEntry;

	HANDLE CONST hSnapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return pid;
	}

	peProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &peProcessEntry);
	do {
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, peProcessEntry.th32ProcessID);
		if (IsX86Process(hProcess))
			printf("%d:%s (x86)\n", peProcessEntry.th32ProcessID, peProcessEntry.szExeFile);
		else
			printf("%d:%s (x64)\n", peProcessEntry.th32ProcessID, peProcessEntry.szExeFile);

		if (!strcmp(peProcessEntry.szExeFile, procname))
		{
			printf("%d:%s\n", peProcessEntry.th32ProcessID, peProcessEntry.szExeFile);
			DWORD pid = peProcessEntry.th32ProcessID;
			CloseHandle(hSnapshot);

			return pid;
		}

	} while (Process32Next(hSnapshot, &peProcessEntry));

	CloseHandle(hSnapshot);
	return pid;
}

BOOL inject(DWORD dwPID, char *szDllPath)
{
	DWORD dwErr = 0;
	SIZE_T dllPathSize = strlen(szDllPath) + 1;
	HANDLE hRemoteThread;
	LPVOID lpLoadDllPath;
	LPVOID lpLoadLibraryAFunction;
	HANDLE hProcess;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (!hProcess)
	{
		dwErr = GetLastError();
		OutputDebugString("OpenProcessError fail");
		return FALSE;
	}

	lpLoadLibraryAFunction = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (!lpLoadLibraryAFunction)
	{
		dwErr = GetLastError();
		OutputDebugString("GetProcAddress fail");
		CloseHandle(hProcess);
		return FALSE;
	}

	lpLoadDllPath = VirtualAllocEx(hProcess, NULL, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!lpLoadDllPath)
	{
		dwErr = GetLastError();
		OutputDebugString("VirtualAllocEx fail");
		CloseHandle(hProcess);
		return FALSE;
	}

	if (!WriteProcessMemory(hProcess, lpLoadDllPath, szDllPath, dllPathSize, NULL))
	{
		dwErr = GetLastError();
		OutputDebugString("WriteProcessMemory fail");
		CloseHandle(hProcess);
		return FALSE;
	}

	hRemoteThread = CreateRemoteThread(hProcess, NULL, 4096 * 16, (LPTHREAD_START_ROUTINE)lpLoadLibraryAFunction, lpLoadDllPath, NULL, NULL);
	if (!hRemoteThread)
	{
		dwErr = GetLastError();
		OutputDebugString("CreateRemoteThread fail");
		CloseHandle(hProcess);
		return FALSE;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);
	if (!VirtualFreeEx(hProcess, hRemoteThread, dllPathSize, MEM_RELEASE))
	{
		dwErr = GetLastError();
		OutputDebugString("VirtualFreeEx fail");
		CloseHandle(hRemoteThread);
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hRemoteThread);
	CloseHandle(hProcess);
	return TRUE;
}

int main()
{
	char proc_name[] = "test.exe";
	char path_dll[] = "D:\\Projects\\hooklibinjector\\Debug\\hook.dll";

	setPrivileges(SE_DEBUG_NAME);
	inject(getPIDproc(proc_name), path_dll);
	Sleep(50000);
}