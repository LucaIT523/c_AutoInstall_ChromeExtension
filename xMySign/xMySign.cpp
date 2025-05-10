// xMySign.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include "ProcessUtil.h"
#include "ZipUtil.h"
#include "MyDefine.h"

int main()
{
	std::wstring        exeDir = GetExecutableDirectory();
	std::wstring		w_strSignToolPath = L"";
	std::wstring	    OUT_Installer_File = L"";

	w_strSignToolPath = exeDir + L"\\signtool.exe";

	OUT_Installer_File = exeDir + L"\\Uninstall.exe";
	cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

	OUT_Installer_File = exeDir + L"\\mi_nt_svc.exe";
	cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

	OUT_Installer_File = exeDir + L"\\ntdb.dll";
	cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

	OUT_Installer_File = exeDir + L"\\xagent.exe";
	cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

	OUT_Installer_File = exeDir + L"\\uptimecrx.exe";
	cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

	OUT_Installer_File = exeDir + L"\\websvc.exe";
	cmd_sign(ConvertWStringToString(OUT_Installer_File), ConvertWStringToString(w_strSignToolPath));

	return 0;
}

