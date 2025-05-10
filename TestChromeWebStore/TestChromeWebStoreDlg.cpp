
// TestChromeWebStoreDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TestChromeWebStore.h"
#include "TestChromeWebStoreDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <random>
#include <atlstr.h> 
#include <string>

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


// CTestChromeWebStoreDlg dialog



CTestChromeWebStoreDlg::CTestChromeWebStoreDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTCHROMEWEBSTORE_DIALOG, pParent)
	, m_strChromeEXURL(_T("https://chromewebstore.google.com/detail/"))
	, m_strMSEdgeEXURL(_T("https://microsoftedge.microsoft.com/addons/detail/"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestChromeWebStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CHROME_EX_URL, m_strChromeEXURL);
	DDX_Text(pDX, IDC_EDIT_MSEDGE_EX_URL, m_strMSEdgeEXURL);
}

BEGIN_MESSAGE_MAP(CTestChromeWebStoreDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CTestChromeWebStoreDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTestChromeWebStoreDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CHROME_INSTALL, &CTestChromeWebStoreDlg::OnBnClickedButtonChromeInstall)
	ON_BN_CLICKED(IDC_BUTTON_MSEDGE_INSTALL, &CTestChromeWebStoreDlg::OnBnClickedButtonMsedgeInstall)
END_MESSAGE_MAP()


// CTestChromeWebStoreDlg message handlers

BOOL CTestChromeWebStoreDlg::OnInitDialog()
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

	//SS_Init();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestChromeWebStoreDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestChromeWebStoreDlg::OnPaint()
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
HCURSOR CTestChromeWebStoreDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int		GetRandomSubKey()
{
	std::random_device rd;
	std::mt19937 gen(rd()); // Mersenne Twister engine

	// Define the range [0, 9999]
	std::uniform_int_distribution<> distr(1, 9999);

	// Generate a random number
	int randomNumber = distr(gen);

	return randomNumber;

}
bool CheckRegistryValueExists(HKEY hKeyRoot, const std::string& subKey, const std::string& valueName) {
	HKEY hKey;
	LONG result;

	// Open the specified registry key
	result = RegOpenKeyExA(hKeyRoot, subKey.c_str(), 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		return false; // Could not open the key
	}

	// Query the value to check if it exists
	result = RegQueryValueExA(hKey, valueName.c_str(), NULL, NULL, NULL, NULL);
	RegCloseKey(hKey); // Close the key after querying

	// If the value exists, RegQueryValueEx returns ERROR_SUCCESS
	return (result == ERROR_SUCCESS);
}


void CTestChromeWebStoreDlg::OnBnClickedOk()
{

	int k = GetRandomSubKey();

	k = GetRandomSubKey();

	k = GetRandomSubKey();

	k = GetRandomSubKey();

	HKEY hKeyRoot = HKEY_LOCAL_MACHINE;

	// Example subkey and value name to check
	std::string subKey = "SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist";
	std::string valueName = "22";

	if (CheckRegistryValueExists(hKeyRoot, subKey, valueName)) {
		MessageBoxA(NULL, "The value name exists in the registry!", "Test", MB_OK);
	}
	else {
		MessageBoxA(NULL, "The value name does not exist in the registry", "Test", MB_OK);
	}


}

void  CTestChromeWebStoreDlg::SS_Init()
{
	HKEY	hKey;
	HKEY	hKey_Edge;
	LONG	result;
	TCHAR	valueName[MAX_PATH] = L"";
	TCHAR	valueData[MAX_PATH] = L"";

	// Open the HKEY_LOCAL_MACHINE key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies", 0, KEY_ALL_ACCESS, &hKey);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies", 0, KEY_ALL_ACCESS, &hKey_Edge);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	// Create the Google key
	result = RegCreateKeyEx(hKey, L"Google", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	result = RegCreateKeyEx(hKey, L"Chrome", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	result = RegCreateKeyEx(hKey, L"ExtensionInstallForcelist", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	result = RegCreateKeyEx(hKey_Edge, L"Microsoft", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_Edge, NULL);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	result = RegCreateKeyEx(hKey_Edge, L"Edge", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_Edge, NULL);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}
	result = RegCreateKeyEx(hKey_Edge, L"ExtensionInstallForcelist", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_Edge, NULL);
	if (result != ERROR_SUCCESS) {
		goto L_EXIT;
	}

L_EXIT:
	//RegCloseKey(hKey_Edge);
	RegCloseKey(hKey);
}

void CTestChromeWebStoreDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

void InstallAuthExtension(std::string p_sExtURL, std::string p_sRegMainKey)
{
	HKEY		hKey;
	int			w_nResult = 0;
	int			w_nNameValue = GetRandomSubKey();
	std::string w_strValueName = "";


	// Convert CString to std::string
	std::string w_stdStrChromeURL = p_sExtURL;
	size_t lastSlash = w_stdStrChromeURL.find_last_of('/');
	// Extract the string after the last '/'
	std::string extractedString = w_stdStrChromeURL.substr(lastSlash + 1, 32);

	if (extractedString.size() < 32) {
		goto L_EXIT;
	}

	while (TRUE) {
		std::string subKey = p_sRegMainKey;
		w_strValueName = std::to_string(w_nNameValue);

		if (CheckRegistryValueExists(HKEY_LOCAL_MACHINE, subKey, w_strValueName)) {
			w_nNameValue = GetRandomSubKey();
			continue;
		}
		else {
			break;
		}
	}

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, p_sRegMainKey.c_str(), 0, KEY_ALL_ACCESS, &hKey);
	w_nResult = RegSetValueExA(hKey, w_strValueName.c_str(), 0, REG_SZ, (const BYTE*)extractedString.c_str(), (DWORD)(extractedString.size()));
	RegCloseKey(hKey);
	if (w_nResult != ERROR_SUCCESS) {
		goto L_EXIT;
	}

//	MessageBoxA(NULL, "OK ... please restart chrome.", "Test", MB_OK);

L_EXIT:
	return;
}

void CTestChromeWebStoreDlg::OnBnClickedButtonChromeInstall()
{
	UpdateData(TRUE);

	SS_Init();
	// Convert CString to std::string
	std::string w_stdStrChromeURL = std::string(CStringA(m_strChromeEXURL));
	//InstallAuthExtension(w_stdStrChromeURL, "SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist");
	//InstallAuthExtension(w_stdStrChromeURL, "SOFTWARE\\Policies\\Microsoft\\Edge\\ExtensionInstallForcelist");

	return;
}


void CTestChromeWebStoreDlg::OnBnClickedButtonMsedgeInstall()
{
	UpdateData(TRUE);

	SS_Init();
	// Convert CString to std::string
	std::string w_stdStrChromeURL = std::string(CStringA(m_strMSEdgeEXURL));
	//InstallAuthExtension(w_stdStrChromeURL, "SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist");
	//InstallAuthExtension(w_stdStrChromeURL, "SOFTWARE\\Policies\\Microsoft\\Edge\\ExtensionInstallForcelist");

	return;

}
