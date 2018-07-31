// TestIoctlMDlg.h : header file
//

#if !defined(AFX_TESTIOCTLMDLG_H__732612BD_4AF7_4952_8569_F2805440DAE3__INCLUDED_)
#define AFX_TESTIOCTLMDLG_H__732612BD_4AF7_4952_8569_F2805440DAE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTestIoctlMDlg dialog

class CTestIoctlMDlg : public CDialog
{
// Construction
public:
	CTestIoctlMDlg(CWnd* pParent = NULL);	// standard constructor

	char* m_szProcName ;

// Dialog Data
	//{{AFX_DATA(CTestIoctlMDlg)
	enum { IDD = IDD_TESTIOCTLM_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestIoctlMDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestIoctlMDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoad();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditPath();
	afx_msg void OnRunPE();
	afx_msg void OnBrowser();
	afx_msg void OnUnload();

	void ShowLogInfo( IN LPSTR szInfo );

	static DWORD WINAPI OnLoadThread(LPVOID lpParameter);

protected:
	BOOL IsDrvLoaded();
	void _ThreadProc();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTIOCTLMDLG_H__732612BD_4AF7_4952_8569_F2805440DAE3__INCLUDED_)
