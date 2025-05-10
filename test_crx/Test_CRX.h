
// Test_CRX.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTestCRXApp:
// See Test_CRX.cpp for the implementation of this class
//

class CTestCRXApp : public CWinApp
{
public:
	CTestCRXApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTestCRXApp theApp;
