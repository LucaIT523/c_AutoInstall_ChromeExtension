#pragma once


#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <codecvt>


void	MyZipCRXData(std::wstring	p_CRXDataPath, std::wstring	p_CRXZipFilePath);

void	MyUnzipFile(const std::string& zipPath, const std::string& extractPath);

void	Test_MyUnzipFile(const std::string& zipPath, const std::string& extractPath);

std::string ConvertWStringToString(const std::wstring& wstr);