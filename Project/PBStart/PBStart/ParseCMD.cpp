/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/07 [7:7:2011 - 16:21]
* MODULE : \PBStart\ParseCMD.cpp
* 
* Description:
*
*   该模块负责解析命令行参数; <> 尖括号中的命令表示可有可无
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "ParseCMD.h"

#pragma warning( disable: 4996 )

//////////////////////////////////////////////////////////////////////////

ULONG g_nFreakFileCounts = 0 ;

BOOL g_bNeverDelete = FALSE ;
BOOL g_bFlag_system = FALSE ;
BOOL g_bFlag_Start_RunAsAdmin = FALSE ;

HANDLE g_new_hToken = NULL ;
HANDLE g_new_ImpersonationToken = NULL ;

LPWSTR g_CommandLine_strings = NULL ;

WCHAR g_TmpName[ MAX_PATH ] = L"" ;
WCHAR __szCurrentDirectory[ 0x400 ] = L"" ;
LPWSTR g_lpCurrentDirectory = NULL ;

static const LPWSTR g_FreakFiles_Arrays[ ] = // 畸形目录
{ 
	L"aux",
	L"clock$",
	L"con",
	L"nul",
	L"prn",
	L"com1",
	L"com2",
	L"com3",
	L"com4",
	L"com5",
	L"com6",
	L"com7",
	L"com8",
	L"com9",
	L"lpt1",
	L"lpt2",
	L"lpt3",
	L"lpt4",
	L"lpt5",
	L"lpt6",
	L"lpt7",
	L"lpt8",
	L"lpt9",
} ;


//////////////////////////////////////////////////////////////////////

CParseCMD::CParseCMD()
{
	m_bNoPBCtrl = TRUE ;
	m_bIsSystem = FALSE ;
	m_bIsElevate = FALSE ;
	m_bNeedStartCOM = FALSE ;
	m_bSilent = FALSE ;

	m_hEvent_DeleteSandbox = NULL ;
}

CParseCMD::~CParseCMD()
{

}



BOOL CParseCMD::ParaseCommandLine()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/07 [7:7:2011 - 15:38]

Routine Description:
  处理的命令,除了路径,其他的必须加斜杠 "/"
  "C:\Program Files\ProteinBox\PBStart.exe" c:\windows\system32\calc.exe
  "C:\Program Files\ProteinBox\PBStart.exe" /system c:\windows\system32\calc.exe
  "C:\Program Files\ProteinBox\PBStart.exe" calc.exe
  "C:\Program Files\ProteinBox\PBStart.exe" /internet_explorer
  "C:\Program Files\ProteinBox\PBStart.exe" /internet_explorer "C:\1.url"
  "C:\Program Files\ProteinBox\PBStart.exe" /default_browser [在沙箱中运行默认浏览器]
  "C:\Program Files\ProteinBox\PBStart.exe" /mail_agent
  "C:\Program Files\ProteinBox\PBStart.exe" /silent  no_such_program.exe [即使程序出错,也不提示任何错误]
  "C:\Program Files\ProteinBox\PBStart.exe" /elevate cmd.exe [Vista/WIN7 提权运行程序]
  "C:\Program Files\ProteinBox\PBStart.exe" /terminate [杀掉当前沙箱中的所有进程]
  "C:\Program Files\ProteinBox\PBStart.exe" /box:TestBox /terminate [终止指定沙箱的所有进程]
  "C:\Program Files\ProteinBox\PBStart.exe" /delete_sandbox [删除沙箱文件]
  "C:\Program Files\ProteinBox\PBStart.exe" /delete_sandbox_silent [静默方式删除沙箱文件]
  "C:\Program Files\ProteinBox\PBStart.exe" /reload [重新加载配置文件ProteinBox.ini]
  "C:\Program Files\ProteinBox\PBStart.exe" /force_run [强制运行相关功能]
  "C:\Program Files\ProteinBox\PBStart.exe" /unload [卸载驱动,该接口自用]

--*/
{
	ULONG length = 0 ;
	HRESULT hRet = E_OUTOFMEMORY ;
	LPWSTR lpCmdLine  = GetCommandLineW();
	LPWSTR lpCmdLineL = GetCommandLineWEx( lpCmdLine );


	while ( *lpCmdLineL == ' ' ) { ++lpCmdLineL ; }

	if ( *lpCmdLineL == '/' )
	{
		++lpCmdLineL;

		// 1. 处理 /internet_explorer
		length = wcslen( L"internet_explorer" );
		if ( 0 == wcsncmp(lpCmdLineL, L"internet_explorer", length) )
		{
			return ParaseCommandLine_internet_explorer( lpCmdLineL );
		}

		// 2. 处理 /default_browser
		length = wcslen( L"default_browser" );
		if (   0 == wcsncmp(  lpCmdLineL, L"default_browser", length ) 
			|| 0 == wcsnicmp( lpCmdLineL, L"http://", 7 )
			|| 0 == wcsnicmp( lpCmdLineL, L"https://", 8 )
			)
		{
			return ParaseCommandLine_default_browser( lpCmdLineL );
		}

		// 3. 处理 /mail_agent
		length = wcslen( L"mail_agent" );
		if ( 0 == wcsncmp(lpCmdLineL, L"mail_agent", length) )
		{
			return ParaseCommandLine_mail_agent( lpCmdLineL );
		}

		// 4. 处理 /run_dialog; "在沙盘中运行程序"

		// 5. 处理 /start_menu; "Sandboxie 开始菜单"

		// 6. 处理 /delete_sandbox
		length = wcslen( L"delete_sandbox" );
		if ( 0 == wcsncmp(lpCmdLineL, L"delete_sandbox", length) )
		{
			return ParaseCommandLine_delete_sandbox( lpCmdLineL );
		}

		// 7. 处理 /force_run [强制运行相关功能]
		length = wcslen( L"force_run" );
		if ( 0 == wcsncmp(lpCmdLineL, L"force_run", length) )
		{
			DisableForceProcess( lpCmdLineL );
			ExitProcess(0);
		}

		// 8. 处理 /reload  [重新加载配置文件ProteinBox.ini]
		length = wcslen( L"reload" );
		if ( 0 == wcsncmp(lpCmdLineL, L"reload", length) )
		{
			PB_ReloadConf();
			ExitProcess(0);
		}

		// 9. 处理 /terminate [杀掉当前沙箱中的所有进程]
		length = wcslen( L"terminate" );
		if ( 0 == wcsncmp(lpCmdLineL, L"terminate", length) )
		{
			PB_KillAll( 0xFFFFFFFF, L"DefaultBox" );
			ExitProcess(0);
		}

		// 9. 处理 /system 进程路径
		length = wcslen( L"system" );
		if ( 0 == wcsncmp(lpCmdLineL, L"system", length) )
		{
			// 得到"/system"命令后面的进程全路径
			LPWSTR ptr = GetCommandLineWEx( lpCmdLineL );
			if ( NULL == *ptr )
			{
				SetLastError( NO_ERROR );
				MessageBox( NULL, _T("error"), _T("请在/system命令后面指定待运行程序的路径"), MB_OK );
				return FALSE;
			}

			g_CommandLine_strings = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, 0x5000 );
			wcscpy( g_CommandLine_strings, ptr );
			return TRUE;
		}

		// 10. 处理 /dir 目录路径
		length = wcslen( L"dir" );
		if ( 0 == wcsncmp(lpCmdLineL, L"dir", length) )
		{
			LPWSTR ptr1 = lpCmdLineL + 3 ;
			LPWSTR ptr2 = GetCommandLineWEx( ptr1 );

			if ( ptr1 != ptr2 )
			{
				RtlZeroMemory( __szCurrentDirectory, 0x400 );
				wcsncpy( __szCurrentDirectory, ptr1, ((ULONG)((char *)ptr2 - (char *)ptr1) >> 1) - 1 );

				g_lpCurrentDirectory = __szCurrentDirectory ;
				if ( '\"' == *g_lpCurrentDirectory  )
				{
					g_lpCurrentDirectory = __szCurrentDirectory + 1 ;
					LPWSTR ptr3 = wcschr( g_lpCurrentDirectory, '\"' );
					if ( ptr3 ) { *ptr3 = 0; }

					return TRUE;
				}
			}

			return FALSE;
		}

		// 11. 处理 /box: 目录路径
		length = wcslen( L"box:" );
		if ( 0 == wcsncmp(lpCmdLineL, L"box:", length) )
		{
			return FALSE;
		}

		// 12. 处理 /unload
		length = wcslen( L"unload" );
		if ( 0 == wcsncmp(lpCmdLineL, L"unload", length) )
		{
			PB_StopPBDrv();
			ExitProcess(0);
		}
	}
	else
	{
		// 没有指定"/",则只能是 进程名 或者 进程全路径
		g_CommandLine_strings = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, 0x5000 );
		wcscpy( g_CommandLine_strings, lpCmdLineL );
		return TRUE;
	}

	return TRUE;
}



BOOL CParseCMD::StartPBCtrl()
{
	//
	// 启动沙箱的界面管理程序,该功能后期实现
	//

	return TRUE;
}



BOOL CParseCMD::VistaRunAsAdmin( IN BOOL bWork )
{
	WCHAR szCommandLine[ MAX_PATH ] = L"" ;
	SHELLEXECUTEINFOW sei = { 0 };

	if ( FALSE == PB_CanElevateOnVista() ) { return FALSE; }

	if ( g_bFlag_system || (g_bFlag_Start_RunAsAdmin = INIGetConf(L"Start_RunAsAdmin")) )
	{
		if ( FALSE == bWork ) { return TRUE; }

		LPWSTR ptr1 = NULL, ptr2 = NULL ;
		LPWSTR lpCmdLine  = GetCommandLineW();
		wcscpy( szCommandLine, lpCmdLine );

		ptr1 = GetCommandLineWEx( szCommandLine );
		*(ptr1 - 1) = 0 ;

		if ( *szCommandLine == '\"' )
		{
			ptr2 = wcsrchr( szCommandLine, '\"' );
			*ptr2 = 0 ;

			memmove( szCommandLine, szCommandLine + 1, 2 * ((ULONG)((PCHAR)ptr2 - (PCHAR)szCommandLine) / sizeof(WCHAR)) );
		}

		sei.lpParameters = ptr1 ;
		sei.cbSize =sizeof(sei) ;
		sei.fMask = 0 ;
		sei.hwnd = NULL;
		sei.lpVerb = L"runas" ;
		sei.lpFile = szCommandLine ;
		sei.lpDirectory = NULL ;
		sei.nShow = SW_SHOWNORMAL ;
		sei.hInstApp = NULL ;

		ShellExecuteExW( &sei );
		ExitProcess(0);
	}

	return TRUE;
}



BOOL CParseCMD::InitProcess()
{
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bDriverNotLoad = FALSE ;

	for ( int i = 0; ; i = 1 )
	{
		bDriverNotLoad = FALSE;

		// 1. 重要的一步,调用该函数在驱动中建立 PBStart.exe这个总进程节点,以后基于它创建的子进程被认定为沙箱中的进程
		if ( g_bFlag_system )
		{
			status = PB_StartProcess( g_new_hToken, g_new_ImpersonationToken, &bDriverNotLoad );
		}
		else
		{
			status = PB_StartProcess( NULL, NULL, &bDriverNotLoad );
		}

		if ( NT_SUCCESS(status) ) 
		{ 
			// 2. 调用ProteinBoxDll.dll的入口,执行DoWork()函数
			PB_InitProcess();
			return TRUE;
		}

		// 该节点已经在"被沙箱"的链表中了
		if ( STATUS_ALREADY_COMMITTED == status ) { return TRUE; }
		if ( i ) { break; }

		// 若驱动没加载,先加了再说
		if ( bDriverNotLoad ) { PB_StartPBDrv( TRUE ); }
	}
	
	SetLastError(0);
	return FALSE; 
}



LPWSTR CParseCMD::GetCommandLineWEx( IN LPWSTR lpCmdLine )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/07 [7:7:2011 - 16:20]

Routine Description:
  去掉@lpCmdLine中的 "C:\Program Files\ProteinBox\Start.exe"
    
Arguments:
  lpCmdLine - 诸如 "C:\Program Files\ProteinBox\Start.exe"  c:\windows\system32\calc.exe

Return Value:
  去掉"xxx"后的字符串
    
--*/
{
	WCHAR Context ;
	BOOL bFlag = FALSE ;
	LPWSTR ptr = lpCmdLine ;

	while ( *ptr == ' ' )
		++ptr ;

	while ( TRUE )
	{
		Context = *ptr ;
		if ( ! Context ) { break; }

		if ( Context == '\"' )
		{
			bFlag = (bFlag == 0) ;
		}
		else
		{
			if ( Context == ' ' && FALSE == bFlag )  { break; }
		}
		
		++ptr ;
	}

	while ( *ptr == ' ' )
		++ptr ;
	
	return ptr;
}



LPWSTR CParseCMD::ParseInternetShortcutURL( IN LPWSTR lpData )
{
	LPWSTR Buffer = NULL, ptr = NULL , ptr1 = NULL ;
	if ( NULL == lpData ) { return NULL; }

	ptr1 = wcsrchr( lpData, '.' );
	if ( NULL == ptr1 || wcsnicmp(ptr1, L".url", 4) ) { return lpData; }

	Buffer = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 0x2000 );	
	wcscpy( Buffer, lpData );
	ptr = Buffer ;

	for ( int i = iswspace(*Buffer); i; i = iswspace(*ptr) ) { ++ptr; }
	
    if ( *ptr == '\"' )
    {
		ptr[wcslen(ptr) - 1] = 0;
		++ptr;
    }

	if ( ! GetFullPathNameW(ptr, 0x7D0, Buffer + 0x800, &lpData) ) { return ptr; }

	if ( GetPrivateProfileStringW(L"InternetShortcut", L"URL", NULL, Buffer + 2, 0x800, Buffer + 0x800) )
	{
		*Buffer = ' ';
		Buffer[1] = '\"';
		wcscat(Buffer, L"\"");
	}

	return Buffer;
}



BOOL CParseCMD::ParaseCommandLine_internet_explorer( IN LPWSTR lpCmdLineL )
{
	ULONG dwLenOut = 0x2800, lpShortcutDataLength = 0 ;
	LPWSTR lpszReturnW = NULL, lpCmdLineEx = NULL, lpShortcutData = NULL ;

	if ( NULL == lpCmdLineL ) { return FALSE; }

	// 1. 得到IE的全路径
	lpszReturnW = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, dwLenOut );			
	if ( AssocQueryStringW( ASSOCF_INIT_BYEXENAME, ASSOCSTR_EXECUTABLE, L"iexplore", NULL, lpszReturnW, &dwLenOut ) )
	{
		SetLastError(NOERROR);
		MessageBoxW( NULL, L"error", L"无法找到Internet explorer主程序", MB_OK );
		return FALSE;
	}
		
	// 2. 若存在快捷方式, 解析快捷方式,得到对应的URL数据
	/*			
	eg:新建1.txt,保存以下内容,改名为1.url后变成快捷方式
	[internetshortcut]
	url=http://www.google.com
	
	"C:\Program Files\Sandboxie\Start.exe" internet_explorer "C:\1.url"		
	*/
	lpCmdLineEx = &lpCmdLineL[ wcslen(L"internet_explorer") ];				
	lpShortcutData = ParseInternetShortcutURL( lpCmdLineEx );

	dwLenOut = (dwLenOut + wcslen(lpCmdLineL) + wcslen(lpShortcutData) + 0x40) * sizeof(WCHAR);
	g_CommandLine_strings = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, dwLenOut );
	wsprintfW( g_CommandLine_strings, L"\"%s\"%s", lpszReturnW, lpShortcutData );

	return TRUE;
}



BOOL CParseCMD::ParaseCommandLine_default_browser( IN LPWSTR lpCmdLineL )
{
	ULONG dwLenOut = 0x2800, length = 0, lpShortcutDataLength = 0 ;
	LPWSTR lpszReturnW = NULL, lpCmdLineEx = NULL, lpShortcutData = NULL ;
	
	if ( NULL == lpCmdLineL ) { return FALSE; }

	// 1. 得到IE的全路径
	lpszReturnW = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, dwLenOut );			
	if ( AssocQueryStringW( 0, ASSOCSTR_EXECUTABLE, L".html", NULL, lpszReturnW, &dwLenOut ) )
	{
		SetLastError(NOERROR);
		MessageBoxW( NULL, L"error", L"无法找到Internet explorer主程序", MB_OK );
		return FALSE;
	}
	
	// 2. 拼接CommandLine
	length = wcslen(L"default_browser") ;
	if ( 0 == wcsncmp(lpCmdLineL, L"default_browser", length) )
	{
		lpCmdLineL += length;
	}

	dwLenOut = (dwLenOut + wcslen(lpCmdLineL) + wcslen(lpszReturnW) + 0x40) * sizeof(WCHAR);
	g_CommandLine_strings = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, dwLenOut );
	wsprintfW( g_CommandLine_strings, L"\"%s\" %s", lpszReturnW, lpCmdLineL );
	
	return TRUE;
}



BOOL CParseCMD::ParaseCommandLine_mail_agent( IN LPWSTR lpCmdLineL )
{
	ULONG dwLenOut = 0x2800, length = 0, lpMailPathLength = 0 ;
	LPWSTR lpszReturnW = NULL, lpMailPath = NULL ;
	
	if ( NULL == lpCmdLineL ) { return FALSE; }

	// 1. 得到IE的全路径
	lpszReturnW = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, dwLenOut );			
	if ( AssocQueryStringW( 0, ASSOCSTR_EXECUTABLE, L"mailto", NULL, lpszReturnW, &dwLenOut ) )
	{
		SetLastError(NOERROR);
		MessageBoxW( NULL, L"error", L"无法找到邮件主程序", MB_OK );
		return FALSE;
	}
	
	// 2. 拼接CommandLine
	lpMailPath = &lpCmdLineL[ wcslen(L"mail_agent") ];				
	lpMailPathLength = wcslen(lpMailPath);
	
	dwLenOut = (dwLenOut + wcslen(lpMailPath) + wcslen(lpszReturnW) + 0x40) * sizeof(WCHAR);
	g_CommandLine_strings = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, dwLenOut );
	wsprintfW( g_CommandLine_strings, L"\"%s\"%s", lpszReturnW, lpMailPath );
	
	return TRUE;
}



BOOL CParseCMD::ParaseCommandLine_delete_sandbox( IN LPWSTR lpCmdLineL )
{
	BOOL bSilent = FALSE ;
	ULONG length = 0, phrase = 0 ;
	LPWSTR lpContext = NULL ;

	if ( NULL == lpCmdLineL ) { return FALSE; }

	lpContext = &lpCmdLineL[ wcslen(L"delete_sandbox") ];				
	
	length = wcslen( L"_silent" );
	if ( 0 == wcsncmp(lpContext, L"_silent", length) )
	{
		lpContext += length ;
		bSilent = TRUE ;
	}

	length = wcslen( L"_phase" );
	if ( 0 == wcsncmp(lpContext, L"_phase", length) )
	{
		WCHAR step = lpContext[ length ];
		if ( step == '1' )
		{
            phrase = 1;
		}
		else if ( step == '2' )
		{
			phrase = 2;
		}
	}

	DeleteSandbox( bSilent, phrase );
	return TRUE;
}



VOID CParseCMD::DeleteSandbox( BOOL bSilent, ULONG phrase )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/08 [8:7:2011 - 23:36]

Routine Description:
  删除沙箱分为2个阶段:
  1.) 重命名沙箱文件夹; DeleteSandbox_phrase1() / DeleteSandbox_phrase0()
  2.) 删除该文件夹下的所有子文件(递归); DeleteSandbox_phrase2()
    
Arguments:
  bSilent - 是否静默方式删除
  phrase - 删除阶段,可以为0,1,2

--*/
{
	DWORD SID = 0 ;
	WCHAR Name[ MAX_PATH ] = L"" ;
	m_bSilent = bSilent ;

	// 查询配置文件"NeverDelete" --> PB_QueryConf(); 暂时放一放,后期补上
	if ( (phrase > 1) || (FALSE == g_bNeverDelete) )
	{
		if ( FALSE == ProcessIdToSessionId(GetCurrentProcessId(), &SID) ) { SID = 0; }

		wsprintfW( Name, L"Sandboxie_Delete_Sandbox_Session_%d", SID );
		m_hEvent_DeleteSandbox = CreateEventW( NULL, FALSE, FALSE, Name );

		if ( 2 == phrase )
		{
			DeleteSandbox_phrase2();
		} 
		else if ( phrase <= 1 )
		{
			DeleteSandbox_phrase1();
			
			if ( 0 == phrase )
			{
				DeleteSandbox_phrase0();
			}
		}
	}
	else
	{
		SetLastError(NO_ERROR);
	
		if ( FALSE == m_bSilent )
		{
			MessageBox( NULL, _T("Warning"), _T("当前设置不允许删除沙箱的内容"), MB_OK );
		}
	}

	ExitProcess(0);
}



VOID CParseCMD::DeleteSandbox_phrase0()
{
	WCHAR szFileName[ 0x200 ] = L"" ;
	WCHAR szCommandLine[ 0x250 ] = L"" ;
	
	GetModuleFileNameW( NULL, szFileName, 0x1F4 );
	
	wsprintfW( szCommandLine, L"%s%s", szFileName, L"\" delete_sandbox" );
	
	if ( m_bSilent ) { wcscat( szCommandLine, L"_silent" ); }
	
	wcscat( szCommandLine, L"_phase2" ); 
	
	CreateProcess2DeleteSandbox( szCommandLine, FALSE );
	return;
}



VOID CParseCMD::DeleteSandbox_phrase1()
{
	UNICODE_STRING uniBuffer;
	OBJECT_ATTRIBUTES ObjAtr;
	IO_STATUS_BLOCK IoStatusBlock ;
	HANDLE hDir = NULL, hRootDir = NULL ;
	LPWSTR szBoxPath = NULL, ptr = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	FILETIME SystemTimeAsFileTime ;
	FILE_RENAME_INFORMATION FileRenameInfo ;

	// 1. 得到当前待删除沙箱的路径
	szBoxPath = QueryBoxPath( L"DefaultBox" );
	if ( NULL == szBoxPath )
	{
		MessageBox( NULL, _T("error"), _T("无法得到当前沙箱的全路径"), MB_OK );
		ExitProcess(1);
	}

	// 2. 获取对应路径的目录句柄 & 父目录句柄
	memmove( szBoxPath + 4, szBoxPath, (wcslen(szBoxPath) + 1) * sizeof(WCHAR) );
	memcpy( szBoxPath, L"\\??\\", 8 );
	RtlInitUnicodeString( &uniBuffer, szBoxPath ); // "\??\C:\Sandbox\AV\DefaultBox"

	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );
	status = ZwCreateFile( &hDir, 0x110000, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );

	if ( ! NT_SUCCESS(status) )
	{
		if ( status != STATUS_OBJECT_NAME_NOT_FOUND && status != STATUS_OBJECT_PATH_NOT_FOUND )
		{
			MessageBox( NULL, _T("error"), _T("当前沙箱路径不正确"), MB_OK );
			ExitProcess(1);
		}

		return;
	}

	ptr = wcsrchr( szBoxPath, '\\' );
    if ( (((char *)ptr - (char *)szBoxPath) & 0xFFFFFFFE) == 0xC )
		++ptr;

    *ptr = 0;
	
	RtlInitUnicodeString( &uniBuffer, szBoxPath );// "\??\C:\Sandbox\AV"
    status = ZwCreateFile( &hRootDir, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );
	if ( ! NT_SUCCESS(status) ) { ExitProcess(1); }

	// 3. 重命名待删除的沙箱文件夹
	FileRenameInfo.RootDir = hRootDir;
    FileRenameInfo.Replace = FALSE ;

    GetSystemTimeAsFileTime( &SystemTimeAsFileTime );
	
    wsprintfW (
		FileRenameInfo.FileName,                  
		L"__Delete_%ws_%08X%08X",
		L"DefaultBox",
		SystemTimeAsFileTime.dwHighDateTime,
		SystemTimeAsFileTime.dwLowDateTime
		);

    FileRenameInfo.FileNameLength = 2 * wcslen(FileRenameInfo.FileName);
   
	int nIndex = 0;
	ULONG pBuffer = (ULONG) kmalloc( 0x800 );

	while ( TRUE )
	{
		status = ZwSetInformationFile( hDir, &IoStatusBlock, &FileRenameInfo, 0x100, FileRenameInformation );
		if ( status >= 0 ) { break; }

		if ( status == STATUS_ACCESS_DENIED || status == STATUS_SHARING_VIOLATION )
		{
			status = PB_EnumProcessEx( L"DefaultBox", (int*)pBuffer );

			if ( *(DWORD *)pBuffer < 1 ) { break; }
		}

		if ( 30 == nIndex )
		{
			MessageBox( NULL, _T("error"), _T("无法重命名当前沙箱"), MB_OK );
			ExitProcess(1);
		}
		
		++ nIndex;
		Sleep( 200 );
	}

	SetLastError(0);
	kfree( (PVOID)pBuffer );
	ExitProcess(1);
}



VOID CParseCMD::DeleteSandbox_phrase2()
{
	BOOL bRet = FALSE ;
	int SandboxsCounts = 0 ;
	IOCTL_ENUMBOXS_BUFFER Buffer ;
	WCHAR szCurSandboxName[ MAX_PATH + 2 ] = L"" ;

	RtlZeroMemory( &Buffer, sizeof(IOCTL_ENUMBOXS_BUFFER) );

	// 1. 第一回合,得到沙箱数量
	Buffer.Flag = _IOCTL_ENUMBOXS_FLAG_GetSandboxsCounts_ ;
	Buffer.u.SandboxsCounts = (int*) &SandboxsCounts ;

	bRet = PB_EnumBoxes( &Buffer );
	if ( FALSE == bRet || SandboxsCounts > 5 )
	{
		MYTRACE( "error! | DeleteSandbox_phrase2() - PB_EnumBoxes() 1; | 无法获取沙箱的数量 %n ", SandboxsCounts );
		return;
	}

	// 2. 第二回合,删除每个沙箱
	for ( int i=0; i<SandboxsCounts; i++ )
	{
		RtlZeroMemory( szCurSandboxName, MAX_PATH + 2 );
		RtlZeroMemory( &Buffer, sizeof(IOCTL_ENUMBOXS_BUFFER) );

		Buffer.Flag = _IOCTL_ENUMBOXS_FLAG_GetSandboxsCurName_ ;
		Buffer.u.CurrentIndex = i + 1 ;
		Buffer.INDataLength = MAX_PATH + 2 ;
		Buffer.lpData = (PVOID) szCurSandboxName ;

		bRet = PB_EnumBoxes( &Buffer );
		if ( bRet )
		{
			DeleteSandbox_phrase2_dep( szCurSandboxName );
		}
	}

	return;
}



VOID CParseCMD::DeleteSandbox_phrase2_dep( IN LPWSTR szBoxName )
{
	LPWSTR szBoxPath = NULL, ptr = NULL, szBoxRenamedPath = NULL ;
	WCHAR szBoxPathRenamed[ MAX_PATH ] = L"" ;
	WCHAR szDeleteCMD[ MAX_PATH ] = L"%SystemRoot%\\System32\\cmd.exe /c RMDIR /s /q \"%SANDBOX%\"";

	if ( NULL == szBoxName ) { return; }

	// 1. 得到当前待删除沙箱的路径
	szBoxPath = QueryBoxPath( szBoxName );
	if ( NULL == szBoxPath )
	{
		MYTRACE( "error! | DeleteSandbox_phrase2_dep() - QueryBoxPath() | 无法得到当前沙箱(%ws)的全路径", szBoxName );
		return;
	}

	ptr = wcsrchr( szBoxPath, '\\' );
    if ( ptr ) { *ptr = 0; }
	
    wcscat( szBoxPath, L"\\*" );
	wsprintfW( szBoxPathRenamed, L"__Delete_%s_", szBoxPath );

	HANDLE hFind;
	WIN32_FIND_DATAW findData;
	
	// 2. 操作已重命名的待删除沙箱
	szBoxRenamedPath = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, 0x800 );

	hFind = FindFirstFileW( szBoxPathRenamed, &findData );
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MYTRACE( "error! | DeleteSandbox_phrase2_dep() - FindFirstFileW() | (%ws)", szBoxPathRenamed );
		goto _END_ ;
	}

	do
	{
		// 得到已重命名的待删除沙箱的名字,拼接出全路径,进一步处理
		if ( ! (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) { continue; }

		if ( wcslen(findData.cFileName) != (wcslen(szBoxName) + 0x10) ) { continue; }

		if ( wcsnicmp(findData.cFileName, szBoxName, wcslen(szBoxName)) ) { continue; }

		wcscpy( szBoxRenamedPath, szBoxPath );
		wcscpy( &szBoxRenamedPath[wcslen(szBoxRenamedPath) - 1], findData.cFileName );

		WaitforFiletobeFree( szBoxRenamedPath, 20 ); // "C:\Sandbox\AV\__Delete_DefaultBox_01CC405598B511F7"
		
		DeleteSandbox_phrase2_depth( szBoxRenamedPath );
		
		WaitforFiletobeFree( szBoxRenamedPath, 20 );

		
	} while (FindNextFileW(hFind, &findData));

_END_ :
	HeapFree( GetProcessHeap(), 4, szBoxPath );
	return;
}



VOID CParseCMD::DeleteSandbox_phrase2_depth( IN LPWSTR szBoxPath )
{
	BOOL bIsFreakFile = FALSE, bFlag = FALSE ;
	HANDLE hFindFile = NULL, hHeap = NULL ;
	WIN32_FIND_DATAW wfd ;
	PVOID OldData = NULL ;
	WCHAR Flag = 0, CurData = 0 ;
	LPWSTR szFirstDirectory = NULL, szSecondDirectory = NULL, FileNameRoot = NULL, ptr = NULL, szDirectory = NULL, lpTemp = NULL ;
	DWORD dwFileAttributesSpec = (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
	static const WCHAR dot[] = {'.',0};
    static const WCHAR dotdot[] = {'.','.',0};

	hHeap = HeapCreate( 4, 0, 0 );

	do 
	{
		Flag = 0;
		szFirstDirectory = Split_joint_filePath( hHeap, szBoxPath, L"" );
		if ( NULL == szFirstDirectory ) { break; } // 一个空格+"C:\Sandbox\AV\__Delete_DefaultBox_01CC405598B511F7"

		do 
		{
			FileNameRoot = szFirstDirectory + 2;
			OldData = *(PVOID *)szFirstDirectory;

			szSecondDirectory = Split_joint_filePath( hHeap, FileNameRoot, L"*" ); // 一个空格+"C:\Sandbox\AV\__Delete_DefaultBox_01CC405598B511F7\*"
			hFindFile = FindFirstFileW( szSecondDirectory + 2, &wfd );
			if ( hFindFile == (HANDLE)INVALID_HANDLE_VALUE ) { goto _while_next_; }

			do 
			{
				if ( !lstrcmpW( wfd.cFileName, dot ) || !lstrcmpW( wfd.cFileName, dotdot ) ) { continue; }

				// 去掉文件的 "系统只读隐藏" 属性
				if ( wfd.dwFileAttributes & dwFileAttributesSpec )
				{
					SetFileNormalAttribute( hHeap, FileNameRoot, wfd.cFileName );
				}

				// 处理畸形文件
				bIsFreakFile = FALSE ;
				if ( wcslen(wfd.cFileName) + wcslen(FileNameRoot) > 250 ) 
				{ 
					bIsFreakFile = TRUE; 
				}
				else
				{
					if ( wcslen(wfd.cFileName) < 8 )
					{
						for (int Index = 0; Index < ARRAYSIZEOF(g_FreakFiles_Arrays); Index++ )
						{
							if ( 0 == wcsicmp(wfd.cFileName, g_FreakFiles_Arrays[Index]) )
							{
								bIsFreakFile = TRUE;
								break;
							}
						}
					}
				
					while (FALSE)
					{
						// a. 判断当前文件名是否存在畸形字符串,存在就得处理之
						ptr = wfd.cFileName ;
						CurData = wfd.cFileName[0] ;
						
						while ( CurData )
						{
							if ( CurData >= 0x80 )
							{
								bIsFreakFile = TRUE;
								break;
							}
							
							++ptr ;
							CurData = *ptr ;
						}
						
						if ( bIsFreakFile ) { break; }
						if ( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) { break; }
						
						// b. 处理文件夹
						bFlag = FALSE ;
						if ( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )) 
						{
							bFlag = TRUE; 
						}
						else
						{
							szDirectory = (LPWSTR) HeapAlloc( hHeap, 0, 2 * (wcslen(FileNameRoot) + wcslen(wfd.cFileName) + 4) );
							
							if ( NULL == szDirectory )
							{
								bFlag = TRUE;
							}
							else
							{
								wsprintfW( szDirectory, L"%s\\%s", FileNameRoot, wfd.cFileName );
								if ( FALSE == RemoveDirectoryW(szDirectory) ) { bFlag = TRUE; }
								
								HeapFree( hHeap, 0, szDirectory );
							}
						}
						
						if ( bFlag )
						{
							// 得到子目录,递归枚举
							lpTemp = Split_joint_filePath( hHeap, FileNameRoot, wfd.cFileName );
							*(PVOID *)lpTemp = OldData;
							OldData = (PVOID)lpTemp;
						}
					} // while(FALSE)
				} // 文件名总长度 <= 250

				if ( bIsFreakFile )
				{
					if ( FALSE == HandlerFreakFile(hHeap, FileNameRoot, wfd.cFileName, szBoxPath) )
					{
						if ( m_hEvent_DeleteSandbox )
						{
							CloseHandle(m_hEvent_DeleteSandbox);
							m_hEvent_DeleteSandbox = 0;
						}
						
						ExitProcess(1);
					}

					Flag = 1 ;
				}
			} while (FindNextFileW(hFindFile, &wfd)); // end-of-while FOR enum current directory
	
			FindClose(hFindFile);
_while_next_:
			szFirstDirectory = (LPWSTR)OldData;
		} while ( OldData );

	} while ( Flag != (ULONG)OldData );

	SetFileNormalAttribute( hHeap, szBoxPath, L"" );
	HeapDestroy(hHeap);
	return;
}



LPWSTR CParseCMD::Split_joint_filePath( HANDLE hHeap, LPWSTR FileNameRoot, LPWSTR FileNameShort )
{
	LPWSTR Buffer = NULL ;
	ULONG Size = 0, RootLength = 0 ;
	
	Size = sizeof(WCHAR) * (wcslen(FileNameRoot) + wcslen(FileNameShort)) + 0x10 ;
	Buffer = (LPWSTR) HeapAlloc( hHeap, 0, Size );
	*(DWORD *)Buffer = 0;
	
	RootLength = wcslen(FileNameRoot);
	memcpy( Buffer + 2, FileNameRoot, RootLength * sizeof(WCHAR) );
	
	if ( *FileNameShort )
	{
		Buffer[ RootLength + 2 ] = '\\' ;
		wcscpy( &Buffer[RootLength + 3], FileNameShort );
	}
	else
	{
		Buffer[RootLength + 2] = 0;
	}
	
	return Buffer;
}



BOOL CParseCMD::SetFileNormalAttribute( HANDLE hHeap, LPWSTR FileNameRoot, LPWSTR FileNameShort )
{
	HANDLE hFile = NULL ;
	LPWSTR Buffer = NULL ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	
	if ( NULL == FileNameRoot || NULL == FileNameShort ) { return FALSE; }
	
	Buffer = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 2 * (wcslen(FileNameRoot) + wcslen(FileNameShort)) + 0x10 );
	if ( NULL == Buffer ) { return FALSE; }
	
	wsprintfW( Buffer, L"\\??\\%s\\%s", FileNameRoot, FileNameShort );
    RtlInitUnicodeString( &uniBuffer, Buffer );

	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile( &hFile, 0x100180, &ObjAtr, &IoStatusBlock, 0, 0, 0, FILE_OPEN, 0x20, 0, 0 );
	if ( NT_SUCCESS(status) ) 
	{ 
		status = ZwQueryInformationFile( hFile, &IoStatusBlock, &FileBasicInfo, 0x100, FileBasicInformation );
		if ( NT_SUCCESS(status) ) 
		{
			BOOL bFlag = (FileBasicInfo.FileAttributes & 0xFFFFFFF8) == 0;
			FileBasicInfo.FileAttributes &= 0xFFFFFFF8 ;
			
			if ( bFlag || FileBasicInfo.FileAttributes == FILE_ATTRIBUTE_DIRECTORY )
			{
				__asm
				{
					or      byte ptr [ebp+FileBasicInfo.FileAttributes], 80h
				}
			}
			
			status = ZwSetInformationFile( hFile, &IoStatusBlock, &FileBasicInfo, 0x100, FileBasicInformation );
		}
	}
	
	if ( hFile ) { ZwClose(hFile); }
	HeapFree( hHeap, 0, Buffer );
	return (status == 0);
}



BOOL CParseCMD::HandlerFreakFile( HANDLE hHeap, LPWSTR FileNameRoot, LPWSTR FileNameShort, LPWSTR szBoxPath )
{
	HANDLE hFile = NULL, hRootDir = NULL ;
	LPWSTR Buffer = NULL, NewName = NULL, ptr1 = NULL, ptr2 = NULL ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	FILETIME SystemTimeAsFileTime ;
	FILE_RENAME_INFORMATION FileRenameInfo ;
	
	if ( NULL == FileNameRoot || NULL == FileNameShort || NULL == szBoxPath ) { return FALSE; }
	
	Buffer = (LPWSTR) HeapAlloc( hHeap, 0, 2 * (wcslen(FileNameRoot) + wcslen(FileNameShort)) + 0x10 );
	if ( NULL == Buffer ) { return FALSE; }
	
	++ g_nFreakFileCounts ;
	GetSystemTimeAsFileTime( &SystemTimeAsFileTime );
	wsprintfW( Buffer, L"\\??\\%s\\%s", FileNameRoot, FileNameShort );

	NewName = (LPWSTR) HeapAlloc( hHeap, 0, 2 * wcslen(szBoxPath) + 0x80 );
	wsprintfW(
		NewName,
		L"\\??\\%s\\%08X-%08X-%08X",
		szBoxPath,
		SystemTimeAsFileTime.dwHighDateTime,
		SystemTimeAsFileTime.dwLowDateTime,
		g_nFreakFileCounts
		);

	ptr1 = wcsrchr( NewName, '\\' );
	*ptr1 = 0 ;
	ptr2 = ptr1 + 1 ;
    
	RtlInitUnicodeString( &uniBuffer, Buffer );	
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile( &hFile, 0x110000, &ObjAtr, &IoStatusBlock, 0, 0, 0, 1, 0x20, 0, 0 );
	if ( NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &uniBuffer, NewName );
		status = ZwCreateFile(
			&hRootDir,
			0x120089,
			&ObjAtr,
			&IoStatusBlock,
			0,
			FILE_ATTRIBUTE_NORMAL,
			7,
			FILE_OPEN,
			0x21,
			0,
			0
			);

		if ( NT_SUCCESS(status) )
		{
			FileRenameInfo.Replace = 0;
			FileRenameInfo.RootDir = hRootDir;
			FileRenameInfo.FileNameLength = 2 * wcslen(ptr2);
			
			wcscpy( FileRenameInfo.FileName, ptr2 );
			
			int nIndex = 0;
			do
			{
				status = ZwSetInformationFile( hFile, &IoStatusBlock, &FileRenameInfo, 0x100, FileRenameInformation );
				if ( status != STATUS_SHARING_VIOLATION )
					break;
				
				Sleep(300);
				++nIndex;
			}
			while ( nIndex < 20 );
		}
	}
	
	if ( hRootDir ) { ZwClose(hRootDir); }
	if ( hFile ) { ZwClose(hFile); }

	HeapFree(hHeap, 0, NewName);
	HeapFree(hHeap, 0, Buffer);
	return (status == 0);
}



VOID CParseCMD::WaitforFiletobeFree( IN LPWSTR szPath, IN int nCounts )
{
	int n = 0 ;
	HANDLE hFile = NULL ;
	LPWSTR Buffer = NULL ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;

	if ( nCounts <= 0 ) { return; }

	Buffer = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, wcslen(szPath) * sizeof(WCHAR) + 0x10 );
	if ( NULL == Buffer ) { return; }

	wsprintfW( Buffer, L"\\??\\%s", szPath );
    RtlInitUnicodeString( &uniBuffer, Buffer );

	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	while ( TRUE )
	{
		status = ZwCreateFile (
			&hFile,
			0x110000,
			&ObjAtr,
			&IoStatusBlock,
			0,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
			0,
			0
			);

		if ( NT_SUCCESS(status) )
		{
			ZwClose( hFile );
			break;
		}

		Sleep( 500 );
        ++n;
		if ( n > nCounts ) { break; }
	}

	HeapFree( GetProcessHeap(), 0, Buffer );
	return;
}



LPWSTR CParseCMD::QueryBoxPath( IN LPWSTR szBoxName )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/09 [9:7:2011 - 0:16]

Routine Description:
  查询@szBoxName对应的沙箱全路径  
    
Arguments:
  szBoxName - 待查询的沙箱名
  DefaultLength - 默认沙箱名长度

Return Value:
  @szBoxName对应的沙箱全路径
    
--*/
{
	BOOL bRet = FALSE ; 
	ULONG ReturnLength = 0 ;
	LPWSTR szBoxPath = NULL ;

	szBoxPath = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, MAX_PATH );
	if ( NULL == szBoxPath ) { return NULL; }

	bRet = PB_QueryBoxPath( szBoxName, szBoxPath );
	if ( FALSE == bRet ) { return NULL; }

	if ( FALSE == PB_TranslateNtToDosPath(szBoxPath) )
	{
		HeapFree( GetProcessHeap(), 0, szBoxPath );
		szBoxPath = NULL;
	}

	return szBoxPath;
}



BOOL CParseCMD::CreateProcess2DeleteSandbox( IN LPWSTR lpData, IN BOOL bWait )
{
	BOOL bRet = FALSE ;
	DWORD dwExitCode = 0 ;
	STARTUPINFOW StartupInfo ;
	PROCESS_INFORMATION ProcessInfo ;
	WCHAR szCommandLine[ 0x300 ] = L"" ;

	RtlZeroMemory( &StartupInfo, sizeof(STARTUPINFOW) );
	RtlZeroMemory( &ProcessInfo, sizeof(PROCESS_INFORMATION) );

	ExpandEnvironmentStringsW( lpData, szCommandLine, 0x2F8 );

	StartupInfo.cb = 0x44 ;
	StartupInfo.dwFlags = 0x81 ;
	StartupInfo.wShowWindow = 0;

	bRet = CreateProcessW( NULL, szCommandLine, 0, 0, 0, 0, 0, 0, &StartupInfo, &ProcessInfo );
	if ( FALSE == bRet )
	{
		ExitProcess(0);
	}
	
	if ( FALSE == bWait ) { return TRUE; }

	WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

	if ( GetExitCodeProcess(ProcessInfo.hProcess, (LPDWORD)&dwExitCode) && dwExitCode )
    {
		CloseHandle( ProcessInfo.hProcess );
		SetLastError( dwExitCode );
		ExitProcess(0);
    }
	
    bRet = CloseHandle( ProcessInfo.hProcess );
	return bRet;
}



BOOL CParseCMD::DisableForceProcess( IN LPWSTR lpCmdLineL )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/14 [14:7:2011 - 11:34]

Routine Description:
  "C:\Program Files\ProteinBox\Start.exe" /force_run enable/yes/1 [开启所有的强制功能]
  "C:\Program Files\ProteinBox\Start.exe" /force_run disable/no/0 [取消所有的强制功能]
  "C:\Program Files\ProteinBox\Start.exe" /force_run enable/yes/1 c:\path\to\program.exe [允许强制程序运行进程[one]]
  "C:\Program Files\ProteinBox\Start.exe" /force_run disable/no/0 c:\path\to\program.exe [取消强制程序运行进程[one]]

--*/
{
	int Flag = 0 ;
	BOOL bRet = FALSE ;
	LPWSTR lpContext = NULL, ptr = NULL, szForcePath = NULL ;
	
	if ( NULL == lpCmdLineL ) { return FALSE; }
	
	lpContext = &lpCmdLineL[ wcslen(L"force_run") ];
	if ( NULL == lpContext )
	{
		MYTRACE( "error! | DisableForceProcess(); | 未指定参数 " );
		return FALSE;
	}
	
	// 忽略空格
	while ( *lpContext == ' ' ) { ++lpContext ; }
	if ( ! *lpContext ) { return FALSE;; }
	
	do 
	{
		// 是否是启用状态
		if ( 0 == wcsncmp(lpContext, L"enable", 6) ) { ptr = lpContext + 6; }
		if ( 0 == wcsncmp(lpContext, L"yes", 3) ) { ptr = lpContext + 3; }
		if ( 0 == wcsncmp(lpContext, L"1", 1) ) { ptr = lpContext + 1; }

		if ( ptr )
		{
			Flag = _FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForce_ ;
			if ( ! *ptr ) 
			{ 
				// 允许所有
				Flag = _FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForceALL_ ;
			}
			else
			{
				// 允许单个, 处理有进程路径的情况
				bRet = GetForcePath( ptr, &szForcePath );
				if ( FALSE == bRet || NULL == szForcePath )
				{
					MYTRACE( "error! | DisableForceProcess() - GetForcePath(); | 获取进程路径失败 " );
					return FALSE;
				}
			}

			break;
		}

		// 是否是禁用状态
		if ( 0 == wcsncmp(lpContext, L"disable", 7) ) { ptr = lpContext + 7; }
		if ( 0 == wcsncmp(lpContext, L"no", 2) ) { ptr = lpContext + 2; }
		if ( 0 == wcsncmp(lpContext, L"0", 1) ) { ptr = lpContext + 1; }

		if ( ptr )
		{
			Flag = _FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForce_ ;
			if ( ! *ptr ) 
			{ 
				// 禁止所有
				Flag = _FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForceALL_ ;
			}
			else
			{
				// 禁止单个, 处理有进程路径的情况
				bRet = GetForcePath( ptr, &szForcePath );
				if ( FALSE == bRet || NULL == szForcePath )
				{
					MYTRACE( "error! | DisableForceProcess() - GetForcePath(); | 获取进程路径失败 " );
					return FALSE;
				}
			}

			break;
		}

		if ( NULL == ptr )
		{
			MYTRACE( "error! | DisableForceProcess(); | 命令不合法,请输入有效命令行 " );
			return FALSE;
		}

	} while (FALSE);

	return PB_DisableForceProcess( Flag, szForcePath );
}



BOOL CParseCMD::GetForcePath( IN LPWSTR lpCmdLineL, OUT LPWSTR* szForcePath )
{
	LPWSTR ptr = NULL ;

	ptr = lpCmdLineL;
	if ( NULL == ptr ) { return FALSE; }
	
	for ( int i = iswspace(*ptr); i; i = iswspace(*ptr) ) { ++ptr; }
	
	if ( *ptr == '\"' )
	{
		ptr[wcslen(ptr) - 1] = 0;
		++ptr;
	}
	
	if ( wcslen(ptr) > 0x2000 )
	{
		MYTRACE( "error! | GetForcePath(); | 字符串路径超长,不予处理 %ws", ptr );
		return FALSE;
	}
	
	*szForcePath = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 0x2000 );
	wcscpy( *szForcePath, ptr );

	return TRUE;
}



BOOL CParseCMD::INIGetConf( IN LPWSTR Key )
{
	ULONG nIndex = 0 ;
	BOOL bRet = FALSE ;
	WCHAR Context[ 0x10 ] = L"" ;
	RPC_IN_HEADER pRpcInBuffer ;
	LPRPC_OUT_HEADER pRpcOutBuffer = NULL ;

	//  
	// RPC通信 -
	// 根据@TokenHandle的对当前用户的SID,转换成UserName,
	// 而后经过一个算法,将用户名转换成数字,拼接为形如[UserSettings_11EE0284]
	// 的格式,这个对应配置文件Sandboxie.ini的[Section]字段,将该字段传回来.
	// 若没有则用默认的
	// 

	pRpcInBuffer.Flag = _PBSRV_APINUM_INIGetCurUserSection_ ;
	pRpcInBuffer.DataLength = 8;
	
	pRpcOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pRpcInBuffer );
	if ( NULL == pRpcOutBuffer )
	{
		PB_StartPBSvc( FALSE ) ;
		PB_StartPBDrv( TRUE ) ;

		do
		{
			Sleep( 500 );

			pRpcInBuffer.Flag = _PBSRV_APINUM_INIGetCurUserSection_ ;
			pRpcInBuffer.DataLength = 8;

			pRpcOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pRpcInBuffer );
			if ( pRpcOutBuffer ) { break; }

			++ nIndex;
		}
		while ( nIndex < 8 );
	}

	if ( pRpcOutBuffer )
	{
		// 这里不用里面的内容,直接查询 [GlobalSetting]
		PB_FreeReply( pRpcOutBuffer );
	}

	bRet = PB_QueryConf( L"GlobalSetting", Key, 0x10, (PVOID)Context );
	if ( bRet )
	{
		if ( 0 == wcsicmp(Context, L"y") || 0 == wcsicmp(Context, L"1") ) { return TRUE; }
	}

	return FALSE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////