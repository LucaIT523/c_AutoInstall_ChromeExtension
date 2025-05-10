
// MyInstaller.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMyInstallerApp:
// See MyInstaller.cpp for the implementation of this class
//

class CMyInstallerApp : public CWinApp
{
public:
	CMyInstallerApp();

// Overrides
public:
	virtual BOOL InitInstance();

	//void ProcessCommandLine();
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMyInstallerApp theApp;
