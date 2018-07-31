
// AdapterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Adapter.h"
#include "AdapterDlg.h"
#include "NetworkAdapter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////


CNetworkAdapter g_adapter;



//////////////////////////////////////////////////////////////////////////

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAdapterDlg dialog




CAdapterDlg::CAdapterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdapterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAdapterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_IP, m_CtrlEdtIP);
    DDX_Control(pDX, IDC_EDIT_MAC, m_CtrlEdtMAC); 
	DDX_Control(pDX, IDC_COMBO_DESCRIPTION, m_ComboBox_Description);
}

BEGIN_MESSAGE_MAP(CAdapterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDCANCEL, &CAdapterDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &CAdapterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CAdapterDlg::OnBnClickedApply)
	ON_CBN_SELCHANGE(IDC_COMBO_DESCRIPTION, OnSelchangeCOMBOUserType)
END_MESSAGE_MAP()


// CAdapterDlg message handlers

BOOL CAdapterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	//
	//
	//

	m_type = 0 ;

	CString strText ;

	ULONG nCounts = g_adapter.GetDescriptionCounts(); 

	m_ComboBox_Description.InsertString( nCounts, TEXT("sudami") );

	for ( int i=0; i< nCounts; i++ )
	{
		strText = g_adapter.GetDescription( i ) ;
		m_ComboBox_Description.AddString( strText );
	}

	m_ComboBox_Description.SetCurSel(0);

    strText = g_adapter.GetIPAddress( 0 );
    m_CtrlEdtIP.SetWindowText( strText );

    strText = g_adapter.GetMACAddress( 0 );
    m_CtrlEdtMAC.SetWindowText( strText );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAdapterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAdapterDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
void CAdapterDlg::OnSelchangeCOMBOUserType() 
{
//	MessageBox(_T("aaa"));
	CString strText ;

	m_type = m_ComboBox_Description.GetCurSel();


	strText = g_adapter.GetIPAddress( m_type );
	m_CtrlEdtIP.SetWindowText( strText );

	strText = g_adapter.GetMACAddress( m_type );
	m_CtrlEdtMAC.SetWindowText( strText );
}



HCURSOR CAdapterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAdapterDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    OnCancel();
	
}

void CAdapterDlg::OnBnClickedOk()
{  
	g_adapter.InitLocalAdapterInfo() ;
//    OnOK();
}


VOID CAdapterDlg::OnBnClickedApply()
{
	CString strText;

	int n = m_type ;

	m_CtrlEdtIP.GetWindowText( strText );
	g_adapter.SetIPAddress( strText, n );

	m_CtrlEdtMAC.GetWindowText( strText );
	g_adapter.SetMACAddress( strText, n );

	if ( !g_adapter.MakeChange( n ) )
	{
		AfxMessageBox( _T("Error") );
	}

	g_adapter.InitLocalAdapterInfo() ;
}
