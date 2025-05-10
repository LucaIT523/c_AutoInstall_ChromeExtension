
// Test_CRXDlg.h : header file
//

#pragma once


// CTestCRXDlg dialog
class CTestCRXDlg : public CDialogEx
{
// Construction
public:
	CTestCRXDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_CRX_DIALOG };
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
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CComboBox m_ctrlComboCRX;

	void	SS_Init();
	afx_msg void OnBnClickedButtonUnInstall();
};
