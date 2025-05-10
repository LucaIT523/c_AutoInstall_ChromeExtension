#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <wtypes.h>
#include <stdio.h>
#include <tchar.h>
#include <sddl.h>
#include <iostream>
#include <string>
#include <codecvt>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include "MyDefine.h"


// Define a struct named 'Person'
struct INSTALLER_INFO {

	//. 1 - Protection , 0 - None
	int		m_nServiceProtection;

	//. 1 - Restart , 0 - None
	int		m_nRestartBrowser;
	//.
	char	m_szAnalyticsServer[MAX_PATH];

	char	m_szAnalyticsToken[MAX_PATH];

	//.
	char	m_szWebServerIPAddr[MAX_PATH];

	//.
	int		m_nPingIntervalTime;

	//. 
	char	m_szWebServiceName[MAX_PATH];
};


// Helper function to kill a process by ID
bool KillProcessById(DWORD processId);


// Helper function to find a process ID by name
DWORD FindProcessId(const std::wstring& processName);

//. 
BOOL TeminateProcess(const std::wstring& processName);

//.
std::wstring charToWstring(const char* pPos);

// Function to get the system directory
std::wstring GetSystemDirectoryPath();

// Function to get the login user name
std::wstring GetLoginUserName();

//.
bool FindFolderInDirectory(const std::wstring& directory, const std::wstring& targetFolderName);
//.
bool FileExists(LPCWSTR filePath);

//.
int _execute(const wchar_t* p_szEXE, const wchar_t* p_pszCommandParam, int	p_bWait /*= 1*/);

//.
size_t getFileSize(const std::wstring& filePath);

//.
int compareVersions(const std::string& version1, const std::string& version2);

//.
std::string readVersionFile(const std::string& filePath);

//.
void cmd_sign(std::string  p_exePath,std::string  p_signToolPath);

//.
std::string RetrieveStringFromRawResource(UINT resourceID);

//.
std::vector<char> RetrieveResourceData(UINT resourceID);

//.
void WriteDataToFile(const std::string& filePath, const std::vector<char>& data);

//.
std::wstring GetExecutableDirectory();

//.
bool UpdateStringResourceInExecutable(const std::wstring& executablePath, const std::string& newStringValue, UINT resourceID);

//.
bool UpdateResourceInExecutable(const std::wstring& executablePath, const std::vector<char>& data, UINT resourceID);

//.
std::string getRandomString(size_t length);
//.
void MyWriteLog(const char* format, ...);

void MyWriteLog_Web(const char* format, ...);
//.
BOOL	IsFileExist(const char* p_FilePaht);

//.
std::string getRandomString(size_t length);

//.
//bool GetCertDateInfo(const std::wstring& filePath, SYSTEMTIME& p_stNotBefore, SYSTEMTIME& p_stNotAfter);

bool Test_GetCertDateInfo(const std::wstring& filePath, SYSTEMTIME& p_stStartTime);

SYSTEMTIME GetCurrentSystemTime();

int CalculateTimeDifference(const SYSTEMTIME& stSysTime, const SYSTEMTIME& stCertTime);

bool AddHeaderToFile(const std::string& inputFilePath, const std::string& outputFilePath);

bool SubHeaderToFile(const std::string& inputFilePath, const std::string& outputFilePath);

bool ModContentToFile(const std::string& inputFilePath, const std::string& outputFilePath);

void AddToStartup(const std::string& appName, const std::string& appPath);

std::string GetProcessName();

void	GetAppNameAndUUIDInfo(std::string p_sFilePath, std::string& p_sAppName, std::string& p_sUUID);


void	SetAppNameAndUUIDInfo(std::string p_sFilePath, std::string& p_sAppName, std::string& p_sUUID);


bool delete_myself();

std::string SpliteServerAddr(const std::string p_sURL);

void GetOpenedHWndFromMyBrowser(HWND& p_hChrome, HWND& p_hEdge);

void TeminateHWndAndProcessName(HWND	p_hOpenedWnd, int  p_nOpt);

bool IsExtensionInstalled(std::wstring& p_sExtensionID, bool p_bChromeOREdge);

bool CheckReadRegistryValues(HKEY hKeyRoot, const std::string subKey, std::string p_ExtensionID);

void TeminateHWndAndProcessName_web(HWND	p_hOpenedWnd, int  p_nOpt);

//bool RunChromeAsLoggedInUser();
//
//
//unsigned long CreateProcessAsTarget(
//	const wchar_t* p_wszTarget,
//	const wchar_t* p_wszPath,
//	const wchar_t* p_wszCmd,
//	const wchar_t* p_wszWorkDir,
//	DWORD p_dwCreateFlag,
//	LPSTARTUPINFOW p_pSI,
//	LPPROCESS_INFORMATION p_pPI
//);
//
//void InitStartupInfo(STARTUPINFOW& si);