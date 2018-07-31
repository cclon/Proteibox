// TestIoctlMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestIoctlM.h"
#include "TestIoctlMDlg.h"
#include "Work.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////


BOOL g_Inited = FALSE ;
CString  g_szFileName = "D:\\1.exe" ;

TCHAR m_szRootPath[MAX_PATH];


#define LOADERNAME				_T("PBStart.exe")
#define SUBKEY_PROTEINBOX		_T("SOFTWARE\\Proteinbox\\Config")
#define SUBVALUE_ROOTFOLDER		_T("RootFolder")


/////////////////////////////////////////////////////////////////////////////
// CTestIoctlMDlg dialog

CTestIoctlMDlg::CTestIoctlMDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestIoctlMDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestIoctlMDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestIoctlMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestIoctlMDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestIoctlMDlg, CDialog)
	//{{AFX_MSG_MAP(CTestIoctlMDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDTEST, OnLoad)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EDIT_PATH, &CTestIoctlMDlg::OnEnChangeEditPath)
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CTestIoctlMDlg::OnRunPE)
	ON_BN_CLICKED(IDC_BUTTON1, &CTestIoctlMDlg::OnBrowser)
	ON_BN_CLICKED(IDUNLOAD, &CTestIoctlMDlg::OnUnload)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestIoctlMDlg message handlers

BOOL CTestIoctlMDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_szProcName = (LPSTR) (LPCTSTR)g_szFileName ;
	GetDlgItem(IDC_EDIT_PATH)->SetWindowText( g_szFileName ); // 在EDITSELECT控件中显示文件路径
	
	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow( FALSE );
	GetDlgItem(IDUNLOAD)->EnableWindow( FALSE );
	GetDlgItem(IDTEST)->EnableWindow( TRUE );

	GetModuleFileName( NULL, m_szRootPath, ARRSIZEOF(m_szRootPath) );
	PathRemoveFileSpec( m_szRootPath );

	// 将自身程序的根目录存入注册表
	SHSetValue( HKEY_LOCAL_MACHINE, SUBKEY_PROTEINBOX, SUBVALUE_ROOTFOLDER, REG_SZ, (LPCVOID)m_szRootPath, strlen(m_szRootPath) );
	{
		CHAR showinfo[MAX_PATH] = "";
		sprintf( showinfo, "当前沙箱根目录: %s", m_szRootPath );
		ShowLogInfo( showinfo );
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestIoctlMDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestIoctlMDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTestIoctlMDlg::OnCancel() 
{
	OnUnload();
	CDialog::OnCancel();
}



void CTestIoctlMDlg::OnEnChangeEditPath()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	GetDlgItem(IDC_EDIT_PATH)->GetWindowText( g_szFileName );
	m_szProcName = (LPSTR) (LPCTSTR)g_szFileName ;

	return ;
}

void CTestIoctlMDlg::OnRunPE()
{
	char szLoaderPath[ MAX_PATH + 0x20 ] = "";
	PathCombine(szLoaderPath, m_szRootPath, LOADERNAME);

	RunPE( szLoaderPath, m_szProcName );
	return ;
}

void CTestIoctlMDlg::OnBrowser()
{
	CFileDialog sourceFile(TRUE);

	// 显示选择文件对话框
	if(sourceFile.DoModal() == IDOK)
	{
		g_szFileName = sourceFile.GetPathName() ;
		m_szProcName = (LPSTR) (LPCTSTR)g_szFileName ;

		GetDlgItem(IDC_EDIT_PATH)->SetWindowText(sourceFile.GetPathName()); // 在EDITSELECT控件中显示文件路径
	}

	return ;
}


DWORD WINAPI CTestIoctlMDlg::OnLoadThread(LPVOID lpParameter)
{
	CTestIoctlMDlg* pThis = (CTestIoctlMDlg*)lpParameter;
	if( NULL != pThis )
	{
		pThis->_ThreadProc();
	}

	return 0;
}


BOOL CTestIoctlMDlg::IsDrvLoaded()
{
	HANDLE hFile = CreateFileW (
		g_PBLinkName , 
		FILE_READ_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE , 
		FILE_SHARE_READ | FILE_SHARE_WRITE , 
		NULL , 
		OPEN_EXISTING ,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		0
		);

	if ( INVALID_HANDLE_VALUE != hFile ) 
	{ 
		CloseHandle(hFile);
		return TRUE ; 
	}
	
	return FALSE ; 
}


void CTestIoctlMDlg::_ThreadProc()
{

	BOOL bRet = FALSE ;

	if ( g_Inited ) { return; }

	// 操作配置文件相关
	ShowLogInfo( "初始化配置文件 \n" );
	bRet = HandlerConf();
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! | HandlerConf() | 操作配置文件失败 \n" );
		return;
	}

	// 加载ProteinBox.sys
	ShowLogInfo( "加载ProteinBox.sys \n" );
	CHAR buffer[MAX_PATH];

	GetCurrentDirectory( MAX_PATH, buffer) ;
	sprintf( buffer + strlen(buffer), "\\ProteinBoxDrv.sys" );

	bRet = LoadDriver( buffer, ProteinBoxDrv_LinkName, &g_drv_ProteinBoxDrv );
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! | LoadDriver() | 加载ProteinBox.sys失败 \n" );
		goto _error_ ;
	}

	// 激活R0的等待线程,R3负责将配置文件数据传给驱动
	bRet = g_Conf->GetDrvPointer( g_drv_ProteinBoxDrv );
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! g_Conf->GetDrvPointer() \n" );
		goto _error_ ;
	}

	bRet = g_Conf->Wakeup_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! g_Conf->Wakeup_R0_InitConfigData() \n" );
		goto _error_ ;
	}

	//
	// 这里要无限等待R0，因为驱动顶多一分钟后会激活我们，所以无限等待，反正一分钟后会有结果
	// 如果等待时间过短，会出问题。
	//
	ShowLogInfo( "无限等待R0... \n" );
	bRet = g_Conf->Waitfor_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! g_Conf->Waitfor_R0_InitConfigData() \n" );
		goto _error_ ;
	}

	ShowLogInfo( "等待成功，go on \n" );
	// 告诉驱动开始Hook
	Ioctl_HookObject( g_drv_ProteinBoxDrv, TRUE ) ;
	Ioctl_HookShadow( g_drv_ProteinBoxDrv, TRUE ) ;

	// 建立沙箱母体
	//	ShowLogInfo("1. Call Ioctl_StartProcess(); 创建沙箱母体 \n");
	//	Ioctl_StartProcess( g_drv_ProteinBoxDrv ) ;

	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow( TRUE );
	GetDlgItem(IDUNLOAD)->EnableWindow( TRUE );

	ShowLogInfo( "初始化完毕 \n" );
	g_Inited = TRUE ;
	if ( g_Conf )
	{
		delete g_Conf ;
		g_Conf = NULL;
	}
	return;

_error_ :
	if ( g_Conf )
	{
		delete g_Conf ;
		g_Conf = NULL;
	}
	g_Inited = FALSE ;
	return;
}


void CTestIoctlMDlg::OnLoad() 
{
	if ( IsDrvLoaded() )
	{
		ShowLogInfo("驱动未被卸载,需重启系统 \n") ;
		return ; 
	}

	GetDlgItem(IDTEST)->EnableWindow( FALSE );

	HANDLE hThread = CreateThread( NULL, 0, OnLoadThread, this, 0, NULL );	
	if ( hThread )
		CloseHandle(hThread);
	
	return;
}


void CTestIoctlMDlg::OnUnload()
{
	if ( FALSE == g_Inited )
	{ 
		ShowLogInfo("驱动未加载,无需卸载 \n") ;
		return ; 
	}

	if ( g_drv_ProteinBoxDrv )
	{
		Ioctl_HookShadow( g_drv_ProteinBoxDrv, FALSE ) ; // 卸载Shadow ssdt的Inline Hook
		UnloadDriver( g_drv_ProteinBoxDrv );
		g_drv_ProteinBoxDrv = NULL;
		ShowLogInfo("驱动已卸载 \n") ;
	}

	GetDlgItem(IDTEST)->EnableWindow( TRUE );
	GetDlgItem(IDUNLOAD)->EnableWindow( FALSE );
	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow( FALSE );

	g_Inited = FALSE ;
	return ;
}


void CTestIoctlMDlg::ShowLogInfo( IN LPSTR szInfo )
{
	CString Temp;
	GetDlgItem(IDC_EDIT_SHOWINFO)->GetWindowText(Temp);

	CString LogInfo = szInfo ;
	Temp.Append( "\r\n" );
	Temp += LogInfo;
	
	GetDlgItem(IDC_EDIT_SHOWINFO)->SetWindowText( Temp );
	return ;
}


