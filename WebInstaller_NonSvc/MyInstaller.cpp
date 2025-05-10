
// MyInstaller.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "MyInstaller.h"
#include "MyInstallerDlg.h"
#include "ProcessUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyInstallerApp

BEGIN_MESSAGE_MAP(CMyInstallerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMyInstallerApp construction

CMyInstallerApp::CMyInstallerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CMyInstallerApp object

CMyInstallerApp theApp;

HANDLE	checkRun()
{
	HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\__MyUnique_CRX_INSTALLER__");

	// Check if the mutex was created successfully
	if (hMutex == NULL) {
		return NULL;
	}

	// Check if the mutex already exists, which means another instance is running
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(hMutex); // Close the handle before exiting
		return NULL;
	}

	return hMutex;
}

// CMyInstallerApp initialization
BOOL CMyInstallerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	HANDLE w_hMutex = checkRun();
	if (w_hMutex == NULL) {
		return FALSE;
	}

	//. 
	CreateDirectory(L"C:\\Users\\Public\\data", NULL);
	//ProcessCommandLine();

	//.
	CMyInstallerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	CloseHandle(w_hMutex);
	return FALSE;
}

//void CMyInstallerApp::ProcessCommandLine()
//{
//	LPWSTR* argv;
//	int argc;
//	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
//
//	//x MyWriteLog("CMyInstallerApp argc = %d", argc);
//
//	for (int i = 0; i < argc; i++)
//	{
//		if (wcscmp(argv[i], L"--update") == 0)
//		{
//			g_bUpdateParam = TRUE;
//			MyWriteLog("CMyInstallerApp g_bUpdateParam = TRUE");
//		}
//	}
//
//	LocalFree(argv);
//}
