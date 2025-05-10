
// TestChromeWebStoreDlg.h : header file
//

#pragma once


// CTestChromeWebStoreDlg dialog
class CTestChromeWebStoreDlg : public CDialogEx
{
// Construction
public:
	CTestChromeWebStoreDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTCHROMEWEBSTORE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString m_strChromeEXURL;
	CString m_strMSEdgeEXURL;
	afx_msg void OnBnClickedButtonChromeInstall();
	afx_msg void OnBnClickedButtonMsedgeInstall();

	void  SS_Init();
};
