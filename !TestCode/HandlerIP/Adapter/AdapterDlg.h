
// AdapterDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CAdapterDlg dialog
class CAdapterDlg : public CDialog
{
// Construction
public:
	CAdapterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ADAPTER_DIALOG };

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
	afx_msg void OnSelchangeCOMBOUserType();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedApply();
    CEdit m_CtrlEdtIP;
    CEdit m_CtrlEdtMAC;
	CEdit m_CtrlEdtDescription;

	CComboBox	m_ComboBox_Description ;
	int m_type ;
public:
	afx_msg void OnCbnSelchangeComboDescription();
};
