#pragma once

#include <windows.h>
#include <iostream>


bool StopService(const std::wstring& serviceName);

unsigned long checkServiceStatus(const wchar_t* p_wszName);

DWORD GetExplorerPID(std::string	p_sProcessName);


bool UninjectDLL(DWORD processID, const char* dllName);

bool InjectDll_SVC(HANDLE p_hProcess, LPCWSTR p_wszDllPath /*= NULL*/);

void restart_browser(TCHAR* p_sProcessName, int p_nOpt);

bool FindHookDllINExplor(DWORD processID, const char* dllName);

std::wstring stringToWstring(const std::string& str);

void	RunAgentExe(char* p_TempPath, std::wstring& p_Opt);
