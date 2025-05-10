
// SfTHookToolDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "SfTHookTool.h"
#include "SfTHookToolDlg.h"
#include "afxdialogex.h"

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


// CSfTHookToolDlg dialog



CSfTHookToolDlg::CSfTHookToolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SFTHOOKTOOL_DIALOG, pParent)
	, m_strExe(L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe")
	, m_strDll(L"D:\\work\\__chrome_ex_install\\__my_src\\bin\\ntdb.dll")
	, m_strPID(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSfTHookToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strExe);
	DDX_Text(pDX, IDC_EDIT2, m_strDll);
	DDX_Text(pDX, IDC_EDIT3, m_strPID);
}

BEGIN_MESSAGE_MAP(CSfTHookToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSfTHookToolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CSfTHookToolDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSfTHookToolDlg message handlers

BOOL CSfTHookToolDlg::OnInitDialog()
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

	// TODO: Add extra initialization here
	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSfTHookToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSfTHookToolDlg::OnPaint()
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
HCURSOR CSfTHookToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void InjectDll(HANDLE p_hProcess, LPCWSTR p_wszDllPath /*= NULL*/) {
	wchar_t wszDllPath[MAX_PATH]; memset(wszDllPath, 0, sizeof(wszDllPath));
	wcscpy_s(wszDllPath, MAX_PATH, p_wszDllPath);
	unsigned int cchDllPath = sizeof(wszDllPath);

	HMODULE hKernel32 = GetModuleHandleA("Kernelbase.dll");
	FARPROC lb = GetProcAddress(hKernel32, "LoadLibraryW");

	HANDLE hProcess = p_hProcess;

	void* pMem = VirtualAllocEx(hProcess, NULL, cchDllPath, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, pMem, wszDllPath, cchDllPath, NULL);
	HANDLE h = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lb, pMem, 0, NULL);
	WaitForSingleObject(h, 1000);
	VirtualFreeEx(hProcess, pMem, 0, MEM_RELEASE);
	CloseHandle(h);
}
typedef HMODULE(WINAPI* FN_LoadLibraryA)(_In_ LPCSTR lpLibFileName);
typedef HMODULE(WINAPI* FN_LoadLibraryW)(_In_ LPCWSTR lpLibFileName);
typedef HMODULE(WINAPI* FN_LoadLibraryExW)(_In_ LPCWSTR lpLibFileName, _Reserved_ HANDLE hFile, _In_ DWORD dwFlags);
typedef FARPROC(WINAPI* FN_GetProcAddress)(_In_ HMODULE hModule, _In_ LPCSTR lpProcName);
typedef HMODULE(WINAPI* FN_GetModuleHandleA)(_In_opt_ LPCSTR lpModuleName);
typedef HMODULE(WINAPI* FN_GetModuleHandleW)(_In_opt_ LPCWSTR lpModuleName);

typedef BOOL(WINAPI* FN_VirtualProtect)(
	_In_  LPVOID lpAddress,
	_In_  SIZE_T dwSize,
	_In_  DWORD flNewProtect,
	_Out_ PDWORD lpflOldProtect
	);

FN_LoadLibraryA lv_orgLoadLibraryA;
FN_LoadLibraryW lv_orgLoadLibraryW;
FN_LoadLibraryExW lv_orgLoadLibraryExW;
FN_GetProcAddress lv_orgGetProcAddress;
FN_GetModuleHandleA lv_orgGetModuleHandleA;
FN_GetModuleHandleW lv_orgGetModuleHandleW;
FN_VirtualProtect lv_orgVirtualProtect;


void CSfTHookToolDlg::OnBnClickedOk()
{

	HMODULE hModKernel = LoadLibraryW(L"kernelbase.dll");
	if (hModKernel)
	{
		lv_orgLoadLibraryA = (FN_LoadLibraryA)GetProcAddress(hModKernel, "LoadLibraryA");
		lv_orgLoadLibraryW = (FN_LoadLibraryW)GetProcAddress(hModKernel, "LoadLibraryW");
		lv_orgLoadLibraryExW = (FN_LoadLibraryExW)GetProcAddress(hModKernel, "LoadLibraryExW");
		lv_orgGetProcAddress = (FN_GetProcAddress)GetProcAddress(hModKernel, "GetProcAddress");
		lv_orgGetModuleHandleA = (FN_GetModuleHandleA)GetProcAddress(hModKernel, "GetModuleHandleA");
		lv_orgGetModuleHandleW = (FN_GetModuleHandleW)GetProcAddress(hModKernel, "GetModuleHandleW");
		lv_orgVirtualProtect = (FN_VirtualProtect)GetProcAddress(hModKernel, "VirtualProtect");
	}


	UpdateData(TRUE);

	STARTUPINFOW si = { sizeof(si) };
	PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
//	BOOL bSts = CreateProcess(m_strExe, L"", NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, L"C:\\Program Files\\Google\\Chrome\\Application\\126.0.6478.128", &si, &pi);
	BOOL bSts = CreateProcess(m_strExe, L"", NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, L"C:\\Program Files (x86)\\Microsoft\\EdgeCore\\126.0.2592.113", &si, &pi);
	if (bSts) {
		InjectDll(pi.hProcess, m_strDll);
		ResumeThread(pi.hThread);
	}
}


void CSfTHookToolDlg::OnBnClickedButton1()
{
	UpdateData(TRUE);

	int nPID = _wtoi(m_strPID);
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, nPID);
	if (hProc != NULL) 
		InjectDll(hProc, m_strDll);
}
