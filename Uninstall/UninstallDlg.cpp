
// UninstallDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Uninstall.h"
#include "UninstallDlg.h"
#include "afxdialogex.h"
#include <fstream>   // For file operations
#include <iostream>  // For console output
#include <winsvc.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include "ProcessUtil.h"
#include <shobjidl.h>

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


// CUninstallDlg dialog



CUninstallDlg::CUninstallDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UNINSTALL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUninstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUninstallDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CUninstallDlg message handlers

BOOL CUninstallDlg::OnInitDialog()
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

	SS_UnInstall();
	EndDialog(IDCANCEL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUninstallDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CUninstallDlg::OnPaint()
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
HCURSOR CUninstallDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool DeleteRegistryKey(HKEY hKeyRoot, LPCWSTR subKey) {
	// Attempt to delete the specified registry key
	LONG result = RegDeleteKeyExW(hKeyRoot, subKey, KEY_WOW64_64KEY, 0);

	if (result == ERROR_SUCCESS) {
		return true;
	}
	else if (result == ERROR_FILE_NOT_FOUND) {
//		std::wcout << L"Registry key not found: " << subKey << std::endl;
	}
	else {
//		std::wcout << L"Failed to delete the registry key: " << subKey << L". Error code: " << result << std::endl;
	}

	return false;
}


bool DeleteFolder(const std::wstring& folderPath) {
	// Initialize COM library
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize COM library." << std::endl;
		return false;
	}

	IFileOperation* pFileOperation = nullptr;

	// Create the IFileOperation object
	hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_IFileOperation, reinterpret_cast<void**>(&pFileOperation));
	if (FAILED(hr)) {
		std::cerr << "Failed to create IFileOperation instance." << std::endl;
		CoUninitialize();
		return false;
	}

	// Set operation flags - no UI, silent, skip recycle bin
	pFileOperation->SetOperationFlags(FOF_NO_UI);

	// Create an IShellItem from the folder path
	IShellItem* pItem = nullptr;
	hr = SHCreateItemFromParsingName(folderPath.c_str(), NULL, IID_PPV_ARGS(&pItem));
	if (FAILED(hr)) {
		std::cerr << "Failed to create IShellItem from folder path." << std::endl;
		pFileOperation->Release();
		CoUninitialize();
		return false;
	}

	// Add the delete operation to the file operation
	hr = pFileOperation->DeleteItem(pItem, NULL);
	pItem->Release();  // Release the IShellItem as it is no longer needed
	if (FAILED(hr)) {
		std::cerr << "Failed to add delete operation." << std::endl;
		pFileOperation->Release();
		CoUninitialize();
		return false;
	}

	// Perform the operation
	hr = pFileOperation->PerformOperations();
	if (FAILED(hr)) {
		std::cerr << "Failed to perform delete operation." << std::endl;
		pFileOperation->Release();
		CoUninitialize();
		return false;
	}

	// Check if operation was successful
	BOOL aborted;
	hr = pFileOperation->GetAnyOperationsAborted(&aborted);
	if (SUCCEEDED(hr) && !aborted) {
		std::cout << "Folder deleted successfully." << std::endl;
	}
	else {
		std::cerr << "Folder deletion was aborted or failed." << std::endl;
	}

	pFileOperation->Release();
	CoUninitialize();
	return true;
}
// Function to recursively delete files and subdirectories within the given folder
bool DeleteFolderContents(const std::wstring& folderPath) {
	// Construct the search path with a wildcard to find all contents
	std::wstring searchPath = folderPath + L"\\*";
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		std::wcerr << L"Failed to open directory: " << folderPath << std::endl;
		return false;
	}

	do {
		// Skip the current directory (.) and parent directory (..)
		std::wstring itemName = findData.cFileName;
		if (itemName == L"." || itemName == L"..") {
			continue;
		}

		// Construct the full path of the item
		std::wstring itemPath = folderPath + L"\\" + itemName;

		// Check if it's a directory or a file
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Recursively delete the contents of the subdirectory
			if (!DeleteFolderContents(itemPath)) {
				FindClose(hFind);
				return false;
			}
			// Remove the empty directory after its contents are deleted
			if (!RemoveDirectory(itemPath.c_str())) {
				std::wcerr << L"Failed to remove directory: " << itemPath << std::endl;
				FindClose(hFind);
				return false;
			}
		}
		else {
			// It's a file; delete it
			if (!DeleteFile(itemPath.c_str())) {
				std::wcerr << L"Failed to delete file: " << itemPath << std::endl;
				FindClose(hFind);
				return false;
			}
		}

	} while (FindNextFile(hFind, &findData)); // Continue to the next file

	FindClose(hFind);
	return true;
}

void CUninstallDlg::SS_UnInstall()
{
	TCHAR	w_szParam[MAX_PATH] = L"";

#if 1
	swprintf(w_szParam, L"/C sc stop XblGame");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc stop XblGame2");
	_execute(L"cmd.exe", w_szParam, 1);

	swprintf(w_szParam, L"/C sc delete XblGame");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc delete XblGame2");
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

	DeleteFolderContents(L"C:\\Users\\Public\\data");
	DeleteFolder(L"C:\\Users\\Public\\data");
	//. 
	DeleteFolderContents(L"C:\\Users\\Public\\upinfo");
	DeleteFolder(L"C:\\Users\\Public\\upinfo");


#else
	swprintf(w_szParam, L"/C sc stop XblGameW");
	_execute(L"cmd.exe", w_szParam, 1);
	swprintf(w_szParam, L"/C sc delete XblGameW");
	_execute(L"cmd.exe", w_szParam, 1);

	DeleteFolderContents(L"C:\\Users\\Public\\webdata");
	DeleteFolder(L"C:\\Users\\Public\\webdata");

#endif

	//. Delete RegKey
	HKEY hKeyRoot = HKEY_LOCAL_MACHINE;
	LPCWSTR subKey = L"SOFTWARE\\Policies\\Microsoft\\Edge\\ExtensionInstallForcelist";
	DeleteRegistryKey(hKeyRoot, subKey);

	subKey = L"SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist";
	DeleteRegistryKey(hKeyRoot, subKey);


	AfxMessageBox(L"OK ... Uninstall");
}
