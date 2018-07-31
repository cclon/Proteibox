/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/20 [20:1:2012 - 14:39]
* MODULE : \Code\Project\ProteinBoxDLL\PBUser32dll\PBUser32dllData.cpp
* 
* Description:
*
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "../StdAfx.h"
#include "../common.h"
#include "../PBDynamicData.h"
#include "../PBComData.h"
#include "../PBFilesData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../MemoryManager.h"
#include "PBUser32dllFunctions.h"
#include "PBUser32dllData.h"

//////////////////////////////////////////////////////////////////////////


ULONG g_offset_jin_Unicode_Length = 0;
ULONG g_offset_jin_Ansi_Length	= 0;

BOOL g_SandboxTitle_NO = FALSE ; // 默认在标题上加沙箱的“[#]”标注
BOOL g_SandboxTitle_NO_Ex = TRUE ; // 默认在标题上加沙箱的“[当前沙箱名]”标注
BOOL g_fEx = FALSE ;

LPWSTR g_pszTileName_new = NULL ;
LPWSTR g_pszTileName_old = NULL ;

ULONG g_WindowClass_new_totalLength = 0 ;
LPWSTR g_WindowClass_new_Array[200] ; // 存放 n < 200 个子控件类的新名字的指针
LPWSTR g_WindowClass_old_Array[200] ; // 存放 n < 200 个子控件类的旧名字的指针

CRITICAL_SECTION g_Lock_Msg ;
LIST_ENTRY_EX g_NodeHead_hhk ;

SANDBOXNAMEINFO g_SandboxNameInfo = {};


static const LPSTR g_ClassNameA_Arrays[ ] =
{ 
	"WorkerA",
	"WorkerW",
	"DDEMLMom",
	"DDEMLEvent",
	"DDEMLAnsiClient",
	"DDEMLAnsiServer",
	"DDEMLUnicodeClient",
	"DDEMLUnicodeServer",
	"#32768",
	"#32770",
	"mdiclient",
	"CHECKLIST_ACLUI",
	"OleDdeWndClass",
	"OleMainThreadWndClass",
	"OLE2UIresimage",
	"OLE2UIiconbox",
	"CicLoaderWndClass",
	"CicMarshalWndClass",
	"CiceroUIWndFrame",
	"PrintTray_Notify_WndClass",
	"MSCTFIME Composition",
	"MSCTFIME UI",
	"IME",
	"AtlAxWin",
	"ComboBoxEx32",
	"SysCredential",
	"SysAnimate32",
	"SysDateTimePick32",
	"SysHeader32",
	"SysIPAddress32",
	"SysLink",
	"SysListView32",
	"SysMonthCal32",
	"SysPager",
	"SysTabControl32",
	"SysTreeView32",
	"ReBarWindow32",
	"ToolbarWindow32",
	"msctls_hotkey",
	"msctls_progress",
	"msctls_trackbar",
	"msctls_statusbar",
	"msctls_updown",
	"tooltips_class",
	"msctls_hotkey32",
	"msctls_progress32",
	"msctls_trackbar32",
	"msctls_statusbar32",
	"msctls_updown32",
	"tooltips_class32",
	"NativeFontCtl",
	"CLIPBRDWNDCLASS",
	"RICHEDIT",
	"RichEdit20A",
	"RichEdit20W",
	"RichEdit20WPT",
	"REComboBox20W",
	"REListBox20W",
	"Button",
	"ComboBox",
	"ComboLBox",
	"Edit",
	"ListBox",
	"ScrollBar",
	"Static",
} ;


// class name
static const LPWSTR g_ClassNameW_Arrays[ ] =
{ 
	L"WorkerA",
	L"WorkerW",
	L"DDEMLMom",
	L"DDEMLEvent",
	L"DDEMLAnsiClient",
	L"DDEMLAnsiServer",
	L"DDEMLUnicodeClient",
	L"DDEMLUnicodeServer",
	L"#32768",
	L"#32770",
	L"mdiclient",
	L"CHECKLIST_ACLUI",
	L"OleDdeWndClass",
	L"OleMainThreadWndClass",
	L"OLE2UIresimage",
	L"OLE2UIiconbox",
	L"CicLoaderWndClass",
	L"CicMarshalWndClass",
	L"CiceroUIWndFrame",
	L"PrintTray_Notify_WndClass",
	L"MSCTFIME Composition",
	L"MSCTFIME UI",
	L"IME",
	L"AtlAxWin",
	L"ComboBoxEx32",
	L"SysCredential",
	L"SysAnimate32",
	L"SysDateTimePick32",
	L"SysHeader32",
	L"SysIPAddress32",
	L"SysLink",
	L"SysListView32",
	L"SysMonthCal32",
	L"SysPager",
	L"SysTabControl32",
	L"SysTreeView32",
	L"ReBarWindow32",
	L"ToolbarWindow32",
	L"msctls_hotkey",
	L"msctls_progress",
	L"msctls_trackbar",
	L"msctls_statusbar",
	L"msctls_updown",
	L"tooltips_class",
	L"msctls_hotkey32",
	L"msctls_progress32",
	L"msctls_trackbar32",
	L"msctls_statusbar32",
	L"msctls_updown32",
	L"tooltips_class32",
	L"NativeFontCtl",
	L"CLIPBRDWNDCLASS",
	L"RICHEDIT",
	L"RichEdit20A",
	L"RichEdit20W",
	L"RichEdit20WPT",
	L"REComboBox20W",
	L"REListBox20W",
	L"Button",
	L"ComboBox",
	L"ComboLBox",
	L"Edit",
	L"ListBox",
	L"ScrollBar",
	L"Static",
} ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



LPSTR RedirectClassNameA( LPSTR szName )
{
	LPSTR ptr = NULL, pBuffer = NULL ;
	BOOL bRedirect = FALSE ;
	CHAR szFormatName[ MAX_PATH ] = "" ;

	if ( (ULONG)szName & 0xFFFF0000 )
	{
		ptr = szName;
	}
	else
	{
		if ( (ULONG)szName <= 0xBFFF || FALSE == g_GetClipboardFormatNameA_addr((unsigned __int16)szName, szFormatName, 255) )
			return szName;

		ptr = szFormatName;
	}

	LPSTR szClassNameFucked = g_SandboxNameInfo.SClasseNameA.szName;
	if ( 0 == strncmp(ptr, szClassNameFucked, g_SandboxNameInfo.SClasseNameA.NameLength) )
		return szName;

	for (int Index = 0; Index < ARRAYSIZEOF(g_ClassNameA_Arrays); Index++ )
	{
		if ( 0 == stricmp(ptr, g_ClassNameA_Arrays[ Index ]) ) { return szName; }
	}

	pBuffer = (LPSTR) kmalloc( strlen(szClassNameFucked) + strlen(ptr) + 1 );
	if ( pBuffer )
	{
		strcpy( pBuffer, szClassNameFucked );
		strcat( pBuffer, ptr );
	}

	return pBuffer;
}


LPWSTR RedirectClassNameW( LPWSTR szName )
{
	LPWSTR ptr = NULL, pBuffer = NULL ;
	BOOL bRedirect = FALSE ;
	WCHAR szFormatName[ MAX_PATH ] = L"" ;

	if ( (ULONG)szName & 0xFFFF0000 )
	{
		ptr = szName;
	}
	else
	{
		if ( (ULONG)szName <= 0xBFFF || FALSE == g_GetClipboardFormatNameW_addr((unsigned __int16)szName, szFormatName, 255) )
			return szName;

		ptr = szFormatName;
	}

	LPWSTR szClassNameFucked = g_SandboxNameInfo.SClasseNameW.szName;
	if ( 0 == wcsncmp(ptr, szClassNameFucked, g_SandboxNameInfo.SClasseNameW.NameLength) )
		return szName;

	for (int Index = 0; Index < ARRAYSIZEOF(g_ClassNameW_Arrays); Index++ )
	{
		if ( 0 == wcsicmp(ptr, g_ClassNameW_Arrays[ Index ]) ) { return szName; }
	}

	pBuffer = (LPWSTR) kmalloc( 2 * (wcslen(szClassNameFucked) + wcslen(ptr)) + 2 );
	if ( pBuffer )
	{
		wcscpy( pBuffer, szClassNameFucked );
		wcscat( pBuffer, ptr );
	}

	return pBuffer;
}


LPWSTR RedirectWindowNameW( IN LPWSTR lpWindowName )
{
	// 1. 若没有标题名或者设置不显示沙箱标记,直接返回
	if ( NULL == lpWindowName || g_SandboxTitle_NO )
		return lpWindowName;

	ULONG length = 0;
	do 
	{
		// 2.1 若标题名长度小于L"[#]",则直接添加
		length = wcslen(lpWindowName);
		if ( length <= g_offset_jin_Unicode_Length ) { break; }

		// 2.2 检查标题名末尾是否已有" [#]".有则不添加
		LPWSTR ptr = &lpWindowName[ length - g_offset_jin_Unicode_Length ];
		if ( 0 == wcsicmp(ptr, L" [#]") ) { return lpWindowName; }

	} while (FALSE);

	// 3. 开始修改标题名,加入沙箱标记
	ULONG nSize = 2*(length + 13);
	if ( g_SandboxNameInfo.STitleNameW.NameLength )
		nSize += 0x50;

	LPWSTR lpWindowsNameNew = (LPWSTR) kmalloc(nSize);
	if ( g_SandboxNameInfo.STitleNameW.NameLength )
	{
		wsprintf( lpWindowsNameNew, L"[#] %s%s [#]", g_SandboxNameInfo.STitleNameW.szName, lpWindowName );
	}
	else
	{
		wsprintf( lpWindowsNameNew, L"[#] %s [#]", lpWindowName );
	}

	return lpWindowsNameNew;
}


LPSTR RedirectWindowNameA( IN LPSTR lpWindowName )
{
	// 1. 若没有标题名或者设置不显示沙箱标记,直接返回
	if ( NULL == lpWindowName || g_SandboxTitle_NO )
		return lpWindowName;

	ULONG length1 = 0;
	do 
	{
		// 2.1 若标题名长度小于"[#]",则直接添加
		length1 = strlen(lpWindowName);
		if ( length1 <= g_offset_jin_Ansi_Length ) { break; }
		
		// 2.2 检查标题名末尾是否已有" [#]".有则不添加
		LPSTR ptr = &lpWindowName[ length1 - g_offset_jin_Ansi_Length ];
		if ( 0 == stricmp(ptr, " [#]") ) { return lpWindowName; }

	} while (FALSE);

	// 3. 开始修改标题名,加入沙箱标记
	ULONG nSize = length1 + 7;
	if ( g_SandboxNameInfo.STitleNameA.NameLength )
		nSize += 0x28;

	LPSTR lpWindowsNameNew = (LPSTR) kmalloc(nSize);
	if ( g_SandboxNameInfo.STitleNameA.NameLength )
	{
		sprintf( lpWindowsNameNew, "[#] %s%s [#]", g_SandboxNameInfo.STitleNameA.szName, lpWindowName );
	}
	else
	{
		sprintf( lpWindowsNameNew, "[#] %s [#]", lpWindowName );
	}

	return lpWindowsNameNew;
}


int 
GetOrigWindowTitleW (
	HWND hWnd, 
	LPWSTR lpString, 
	int StringLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/04/19 [19:4:2011 - 18:00]

Routine Description:
  因为沙箱中运行的程序标题可能会被改变,比如"[#] [DefaultBox] 测试程序标题 [#]",那么沙箱中的相关程序在调用诸如
  GetWindowTileW()此类函数时,要返回给其原始的标题内容.这里便是去掉"[#] [DefaultBox] [#]"等附加的字符.
    
Arguments:
  hWnd - [IN] 窗口句柄
  lpString - [IN][OUT] 待修改的标题内容
  StringLength - [IN] 标题长度

Return Value:
  修改后的标题长度
    
--*/
{
	ULONG n = 0, Length = 0, RealLength = StringLength ;
	LPWSTR ptr = NULL ;

	if ( StringLength < 2 * g_offset_jin_Unicode_Length || FALSE == IshWndCaptionTitle(hWnd) ) { return StringLength; }

	// 1.1 去掉形如字符串"[#] abcdefg [#]",第一个"[#] "
	if ( 0 == wcsncmp( lpString, L"[#] ", 4 ) )
	{
		Length = 2 * (StringLength - 4);
		memmove( lpString, lpString + 4, Length );
		*(LPWSTR)((char *)lpString + Length) = 0 ;
		RealLength = StringLength - 4 ;
	}

	// 1.2 去掉形如字符串"[#] abcdefg [#]",最后一个" [#]" 
	ptr = &lpString[ RealLength - 4 ];
	if ( 0 == wcsncmp( ptr, L" [#]", 4 ) )
	{
		*ptr = 0 ;
		RealLength -= 4 ;
	}

	// 1.3 若标题中存在"[DefaultBox]",去掉
	ULONG NameLength = g_SandboxNameInfo.STitleNameA.NameLength;
	if ( NameLength && RealLength >= NameLength )
	{
		if ( 0 == wcsncmp( lpString, g_SandboxNameInfo.STitleNameW.szName, NameLength ) )
		{
			n = RealLength - NameLength ;
			memmove( lpString, (char *)lpString + 2 * NameLength, 2 * n );
			lpString[ n ] = 0 ;
			RealLength = n ;
		}
	}

	 return RealLength ;
}


ULONG 
GetOrigWindowTitleA ( 
	IN HWND hWnd,
	OUT LPSTR lpString,
	IN int StringLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/04/19 [19:4:2011 - 18:00]

Routine Description:
  因为沙箱中运行的程序标题可能会被改变,比如"[#] [DefaultBox] 测试程序标题 [#]",那么沙箱中的相关程序在调用诸如
  GetWindowTileW()此类函数时,要返回给其原始的标题内容.这里便是去掉"[#] [DefaultBox] [#]"等附加的字符.
    
Arguments:
  hWnd - 窗口句柄
  lpString - 待修改的标题内容
  StringLength - 标题长度

Return Value:
  修改后的标题长度
    
--*/
{
	LPSTR ptr = NULL ;
	ULONG n = 0, RealLength = StringLength ;

	if ( StringLength < 2 * g_offset_jin_Ansi_Length || FALSE == IshWndCaptionTitle(hWnd) ) { return StringLength; }

	// 1.1 去掉形如字符串"[#] abcdefg [#]",第一个"[#] "
	if ( 0 == strncmp( lpString, "[#] ", 4 ) )
	{
		RealLength = StringLength - 4 ;
		memmove( lpString, lpString + 4, RealLength );
		lpString[ RealLength ] = 0 ;
	}

	// 1.2 去掉形如字符串"[#] abcdefg [#]",最后一个" [#]" 
	ptr = &lpString[ RealLength - 3 ];
	if ( 0 == strncmp( ptr, "[#]", 3 ) ) 
	{
		RealLength -= 4 ;
		lpString[ RealLength ] = 0;
	}

	// 1.3 若标题中存在"[DefaultBox] ",去掉
	ULONG NameLength = g_SandboxNameInfo.STitleNameA.NameLength;
	if ( NameLength && RealLength >= NameLength )
	{
		if ( 0 == strncmp( lpString, g_SandboxNameInfo.STitleNameA.szName, NameLength ) )
		{
			n = RealLength - NameLength ;
			memmove( lpString, &lpString[ NameLength ], n );
			lpString[ n ] = 0 ;
			RealLength = n ;
		}
	}

	 return RealLength ;
}


BOOL 
IshWndCaptionTitle (
	IN HWND hWnd
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/01/07 [7:1:2011 - 14:44]

Routine Description:
  若@hWnd有父窗口,判断父窗口是否是对话框类型,是返回TRUE;
  若无父窗口,对于窗口类型为WS_CAPTION,且非"Edit"的控件返回TRUE
    
Arguments:
  hWnd - 窗口句柄

--*/
{
	int Style ; 
	WCHAR szType[ 0x1E ] = L"" ;

	if ( g_SandboxTitle_NO ) { return FALSE; }

	if ( hWnd == (HWND)_FuckedTag_ ) { return TRUE; }

	if ( g_GetParent_addr(hWnd) )
	{
		if ( g_IsWindowUnicode_addr(hWnd) )
			Style = g_GetWindowLongW_addr( hWnd, DWL_DLGPROC );
		else
			Style = g_GetWindowLongA_addr( hWnd, DWL_DLGPROC );

		if ( Style ) { return TRUE; }
	}
	else
	{
		Style = g_GetWindowLongW_addr(hWnd, GWL_STYLE);
		if ( (Style & WS_CAPTION) == WS_CAPTION )
		{
			if ( g_RealGetWindowClassW_addr( hWnd, szType, 0x1E ) != 4 ) { return TRUE; }
				
			if ( wcsicmp(szType, L"Edit") ) { return TRUE; }
		}
	}

	return FALSE;
}


VOID 
HandlerWindowLong (
	IN HWND hWnd,
	IN BOOL bFlag
	)
{
	ULONG StyleA = 0, StyleW = 0, NewStyleW = 0, NewStyleA = 0 ;

	if ( bFlag || NULL == g_GetParent_addr(hWnd) )
	{
		HandlerAtoms();

		StyleW = g_GetWindowLongW_addr( hWnd, GWL_WNDPROC );
		StyleA = g_GetWindowLongA_addr( hWnd, GWL_WNDPROC );

		if ( (StyleW != (ULONG)AddressBarProcW) && (StyleA != (ULONG)AddressBarProcA)  )
		{
			g_SetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldW, (HANDLE)StyleW );
			g_SetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldA, (HANDLE)StyleA );

			if ( g_IsWindowUnicode_addr(hWnd) )
				g_SetWindowLongW_addr( hWnd, GWL_WNDPROC, (LONG)AddressBarProcW );
			else
				g_SetWindowLongA_addr( hWnd, GWL_WNDPROC, (LONG)AddressBarProcA );

			NewStyleW = g_GetWindowLongW_addr( hWnd, GWL_WNDPROC );
			NewStyleA = g_GetWindowLongA_addr( hWnd, GWL_WNDPROC );

			g_SetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewW, (HANDLE)NewStyleW );
			g_SetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewA, (HANDLE)NewStyleA );
		}
	}

	return;
}


void HandlerAtoms()
{
	WCHAR szPath[ MAX_PATH ] = L"" ;

	if ( 0 == g_Atom_OleDropTargetInterface )
	{
		g_Atom_OleDropTargetInterface = GlobalAddAtomW( L"OleDropTargetInterface" );
		if ( g_Atom_OleDropTargetInterface )
		{
			swprintf( szPath, L"Sandbox:%s:%s", g_BoxInfo.BoxName, L"OleDropTargetInterface");
			g_Atom_OleDropTargetInterface_Sandbox = GlobalAddAtomW( szPath );
		}
	}

	if ( 0 == g_Atom_OleDropTargetMarshalHwnd )
	{
		g_Atom_OleDropTargetMarshalHwnd = GlobalAddAtomW( L"OleDropTargetMarshalHwnd" );
		if ( g_Atom_OleDropTargetMarshalHwnd )
		{
			swprintf( szPath, L"Sandbox:%s:%s", g_BoxInfo.BoxName, L"OleDropTargetMarshalHwnd");
			g_Atom_OleDropTargetMarshalHwnd_Sandbox = GlobalAddAtomW( szPath );
		}
	}

	if ( 0 == g_Atom_OleEndPointID )
	{
		g_Atom_OleEndPointID = GlobalAddAtomW( L"OleEndPointID" );
		if ( g_Atom_OleEndPointID )
		{
			swprintf( szPath, L"Sandbox:%s:%s", g_BoxInfo.BoxName, L"OleEndPointID" );
			g_Atom_OleEndPointID_Sandbox = GlobalAddAtomW( szPath );
		}
	}

	if ( 0 == g_Atom_SBIE_DropTarget )     { g_Atom_SBIE_DropTarget = GlobalAddAtomW(L"SBIE_DropTarget"); }
	if ( 0 == g_Atom_SBIE_WindowProcOldW ) { g_Atom_SBIE_WindowProcOldW = GlobalAddAtomW(L"SBIE_WindowProcOldW"); }
	if ( 0 == g_Atom_SBIE_WindowProcOldA ) { g_Atom_SBIE_WindowProcOldA = GlobalAddAtomW(L"SBIE_WindowProcOldA"); }
	if ( 0 == g_Atom_SBIE_WindowProcNewW ) { g_Atom_SBIE_WindowProcNewW = GlobalAddAtomW(L"SBIE_WindowProcNewW"); }
	if ( 0 == g_Atom_SBIE_WindowProcNewA ) { g_Atom_SBIE_WindowProcNewA = GlobalAddAtomW(L"SBIE_WindowProcNewA"); }
}


VOID WalkMsghWnd ( IN LPARAM lParam )
{
	ULONG pNode = 0 ;

	if ( lParam )
	{
		EnterCriticalSection( &g_Lock_Msg );

		for ( pNode = (ULONG)g_NodeHead_hhk.Flink; pNode; pNode = *(ULONG *)pNode )
		{
			SetWindowsHookExFilterEx( (PVOID)pNode, lParam );
		}

		LeaveCriticalSection( &g_Lock_Msg );
	}
	else
	{
		g_EnumWindows_addr( EnumWindowsProc, GetCurrentThreadId() );
	}

	return;
}


BOOL IsWhitehWnd( HWND hWnd )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/04/12 [12:4:2011 - 18:04]

Routine Description:
  判断当前窗口句柄的黑白性   
    
Arguments:
  hWnd - 待判定的窗口句柄

Return Value:
  TRUE - 白名单窗口句柄; FALSE - 黑名单窗口句柄

--*/
{
	LPWSTR lpRedirectClassName = NULL ;
	WCHAR szClassName[ MAX_PATH ] = L"" ;

	if ( IsWhitehWndEx(hWnd, NULL, NULL) ) { return TRUE; }

	if ( FALSE == g_GetClassNameW_addr(hWnd, szClassName, 255) ) { return FALSE; }

	if ( IsWhiteClassName(szClassName) ) { return TRUE; }

	lpRedirectClassName = RedirectClassNameW( szClassName ) ;
	if ( lpRedirectClassName == szClassName ) { return TRUE; }

	kfreeExp( lpRedirectClassName );
	return FALSE;
}


BOOL 
IsWhitehWndEx (
	IN HWND hWnd,
	OUT ULONG *pdwProcessId,
	OUT ULONG *pdwThreadId
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/01/07 [7:1:2011 - 15:27]

Routine Description:
  判断当前窗口句柄的黑白性
   
Return Value:
  TRUE - 白名单窗口句柄; FALSE - 黑名单窗口句柄

--*/
{
	PB_BOX_INFO BoxInfo ;
	BOOL bRet = FALSE ;
	ULONG PID = 0, TID = 0 ;

	// 1. 通过hWnd得到其窗口对应的PID & TID
	TID = g_GetWindowThreadProcessId_addr( hWnd, &PID );

	if ( pdwProcessId ) { *pdwProcessId = PID; }
	if ( pdwThreadId ) { *pdwThreadId = TID; }

	if ( 0 == TID || 0 == PID ) { return FALSE; }
	
	// 2. 若PID是当前运行于沙箱中的主进程,返回TRUE,表明是操作的窗口是其自己的,当然放行掉...
	if ( PID == g_CurrentPID_runInSandbox )  { return TRUE; }
	
	// 3. 若操作的是沙箱内的其他进程,属于沙箱内进程窗口间的操作,放行掉; 若操作的是沙箱外的窗口,就得拒绝掉啦
	bRet = PB_QueryProcess( PID, &BoxInfo );
	if ( FALSE == bRet )
	{
	//	MYTRACE( L"ko! | IsWhitehWndEx() - PB_QueryProcess(); | 当前操作的PID(%x) 非沙箱进程  \n", PID );
		return FALSE ;
	}

	if ( BoxInfo.SessionId == g_BoxInfo.SessionId && 0 == wcsicmp(BoxInfo.BoxName, g_BoxInfo.BoxName) ) { return TRUE; }
	return FALSE;
}


BOOL IsWhiteClassName( LPWSTR szClassName )
{
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;

	WhiteOrBlack( WhiteOrBlack_Flag_XClassName, szClassName, &bIsWhite, &bIsBlack );
	if ( bIsWhite && FALSE == bIsBlack ) { return TRUE; }

	return FALSE;
}


NTSTATUS FindWindowFilter( PVOID lpClassName, ULONG Flag, HWND hWnd )
{
	NTSTATUS status = STATUS_SUCCESS ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	if ( (ULONG)lpClassName & 0xFFFF0000 )
	{
		// 对于Unicode格式
		RtlInitString( &ansiBuffer, (LPSTR)lpClassName );
		status = RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );
		if ( status >= 0 )
		{
			FindWindowFilterEx( uniBuffer.Buffer, Flag, hWnd );
			RtlFreeUnicodeString(&uniBuffer);
		}
	}
	else
	{
		// 对于Ansi格式
		status = FindWindowFilterEx( (LPCWSTR)lpClassName, Flag, hWnd );
	}

	return status;
}


NTSTATUS FindWindowFilterEx( LPCWSTR lpClassName, ULONG Flag, HWND hWnd )
{
	ULONG FlagDummy = Flag ;
	LPCWSTR lpClassNameOrig = NULL ;
	WCHAR szInfo[ MAX_PATH ] = L"" ;

	if ( (ULONG)lpClassName & 0xFFFF0000 )
	{
		lpClassNameOrig = GetOrigClassName( lpClassName );
		wcsncpy( szInfo, lpClassNameOrig, 0x80 );
		szInfo[ wcslen(szInfo) ] = UNICODE_NULL ;
	}
	else
	{
		swprintf( szInfo, L"#%d", (WORD)lpClassName );
	}

	if ( NULL == hWnd && 0 == (WORD)FlagDummy )
	{
		FlagDummy = 0x2000 ;
	}

	return PB_MonitorPut( FlagDummy | 0x33B, szInfo );
}


LPCWSTR GetOrigClassName( LPCWSTR lpClassName )
{
	LPCWSTR ptr = NULL ; 

	if ( wcsncmp(lpClassName, g_SandboxNameInfo.SClasseNameW.szName, g_SandboxNameInfo.SClasseNameW.NameLength) )
		ptr = lpClassName;
	else
		ptr = &lpClassName[g_SandboxNameInfo.SClasseNameW.NameLength];

	return ptr;
}


PVOID RedirectPropString( ULONG lpString )
{
	WORD lpstring_lowpart = 0 ;
	ULONG lpStringDummy = lpString ;

	HandlerAtoms();

	if ( ((ULONG)lpString & 0xFFFF0000) ) { return (PVOID)lpString; }

	lpstring_lowpart = LOWORD(lpString);
	if ( lpstring_lowpart == g_Atom_OleDropTargetInterface )
	{
		lpString = g_Atom_OleDropTargetInterface_Sandbox;
		lpStringDummy = lpString;
	}

	if ( lpstring_lowpart == g_Atom_OleDropTargetMarshalHwnd )
	{
		lpString = g_Atom_OleDropTargetMarshalHwnd_Sandbox;
		lpStringDummy = lpString;
	}

	if ( lpstring_lowpart == g_Atom_OleEndPointID )
	{
		lpString = g_Atom_OleEndPointID_Sandbox;
		lpStringDummy = lpString;
	}

	return (PVOID)lpStringDummy ;
}


BOOL Handler_SendMessageW_WM_CAP_DRIVER_DISCONNECT( LPWSTR szName, HWND hWnd, int Msg, WPARAM wParam, PULONG pRet )
{
	WCHAR szType[ 40 ] = L"" ;
	PVOID pBuffer = NULL ;
	LPWSTR ptr1 = NULL, ptr2 = NULL ; 
	ULONG Length1 = 0, Length2 = 0, dwErrorCode = 0 ;

	if (   FALSE == g_bIs_Inside_iexplore_exe
		|| g_SandboxTitle_NO
		|| g_RealGetWindowClassW_addr( hWnd, szType, 30 ) != 0x12
		|| wcsicmp( szType, L"msctls_statusbar32" )
		|| wParam != 0xA
		)
	{
		return FALSE ;
	}

	// 只处理一种情况,满足:在IE内,有沙箱标题,窗口类名为....
	ptr1 = wcsstr( szName, L" | " );
	if ( NULL == ptr1 ) { return FALSE; }
	
	ptr2 = ptr1 + 3 ;
	if ( NULL == wcsstr(ptr2, L": ") ) { return FALSE; }

	Length1 = (ULONG)((char *)ptr2 - (char *)szName) ;
	Length2 = ( wcslen(L"Proteinbox") + 1 ) * sizeof(WCHAR) ;

	pBuffer = (PVOID) kmalloc( Length1 + Length2 + 8 );

	memcpy( pBuffer, szName, Length1 );
	memcpy( (char *)pBuffer + Length1, L"Proteinbox", Length2 );
	*pRet = (ULONG) g_SendMessageW_addr( hWnd, Msg, 0xA, (LPARAM)pBuffer );

	kfree( pBuffer );
	dwErrorCode = GetLastError();
	SetLastError( dwErrorCode );

	return TRUE;
}


ULONG HandlerBroadcastMessage( HWND hWnd, PVOID FuncAddr, UINT Msg, UINT wParam, UINT lParam, int uTimeout, int lpdwResult )
{
	BroadcastMessageInfo Buffer ; 

	Buffer.FuncAddr1	= 0 ;
	Buffer.FuncAddr2	= 0 ;
	Buffer.hWnd			= 0 ;
	Buffer.uTimeout		= 0 ;
	Buffer.lpdwResult	= 0 ;
	Buffer.Msg			= Msg ;
	Buffer.wParam		= wParam ;
	Buffer.lParam		= lParam ;

	if ( _FuckedTag_ == (ULONG)hWnd )
	{
		Buffer.FuncAddr1 = (ULONG)FuncAddr ;
	}
	else
	{
		Buffer.hWnd			= hWnd		 ;
		Buffer.uTimeout		= uTimeout	 ;
		Buffer.lpdwResult	= lpdwResult ;
		Buffer.FuncAddr2	= (ULONG)FuncAddr	 ;
	}

	fake_EnumWindows( (WNDENUMPROC)EnumWndProc_broadcastMessage, (LPARAM)&Buffer );
	return 1;
}


BOOL CALLBACK EnumWndProc_broadcastMessage( HWND hWnd, LPBroadcastMessageInfo pBuffer )
{
	typedef void (WINAPI* _fnFuncAddr1_) (HWND, UINT, UINT, UINT);
	typedef void (WINAPI* _fnFuncAddr2_) (HWND, UINT, UINT, UINT, HWND, int, int);
	
	_fnFuncAddr1_ FuncAddr1 = NULL ;
	_fnFuncAddr2_ FuncAddr2 = NULL ;

	FuncAddr1 = (_fnFuncAddr1_) pBuffer->FuncAddr1 ;
	if ( FuncAddr1 )
	{
		FuncAddr1(hWnd, pBuffer->Msg, pBuffer->wParam, pBuffer->lParam);
	}
	else
	{
		FuncAddr2 = (_fnFuncAddr2_) pBuffer->FuncAddr2 ;
		if ( FuncAddr2 )
			FuncAddr2( hWnd,pBuffer->Msg,pBuffer->wParam,pBuffer->lParam,pBuffer->hWnd,pBuffer->uTimeout,pBuffer->lpdwResult );
	}

	return TRUE;
}


static ULONG_PTR g_Current_TickCount = 0 ;

VOID ClearHandlersCache( IN BOOL bFlag )
{
	DWORD dwErrCode = GetLastError();
	DWORD TickCount = GetTickCount();
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( bFlag || TickCount - g_Current_TickCount > 400 )
	{
		g_Current_TickCount = TickCount;

		if ( TryEnterCriticalSection( &g_HandlesLock ) )
		{
			if ( (DWORD)g_pNodeHead_Handles.Flink )
			{
				CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);
				ClearHandlersCacheEx( bFlag, TickCount );
				CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
			}

			LeaveCriticalSection( &g_HandlesLock );
		}
	}

	SetLastError( dwErrCode );
	return;
}


VOID ClearHandlersCacheEx( IN BOOL bFlag, IN int TickCount )
{
	BYTE ObjectTypeIndex = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	ULONG Length = 0, ReturnLength = 0, n = 0 ;
	LPMYSYSTEM_HANDLE_INFORMATION SHI = NULL ;
	LPHANDLE_INFO pNodeCurrent = NULL, pNodeNext = NULL ;
	LPWSTR lpFileName = NULL ;

	while ( TRUE )
	{
		SHI = (LPMYSYSTEM_HANDLE_INFORMATION) kmalloc( Length );

		status = ZwQuerySystemInformation( SystemHandleInformation, SHI, Length, &ReturnLength );
		if ( NT_SUCCESS(status) ) { break; }

		kfree( SHI );
		Length = ReturnLength + 0x40 ;
		if ( status == STATUS_BUFFER_OVERFLOW || status == STATUS_INFO_LENGTH_MISMATCH || status == STATUS_BUFFER_TOO_SMALL )
		{
			if ( n++ < 5 ) { continue; }
		}

		return;
	}

	lpFileName = (LPWSTR) kmalloc( 0x400 );
	pNodeCurrent = (LPHANDLE_INFO) g_pNodeHead_Handles.Flink ;
	if ( NULL == pNodeCurrent ) { goto _end_; }

	do
	{
		pNodeNext = (LPHANDLE_INFO)pNodeCurrent->ListEntry.Flink ;

		if ( bFlag
			|| TickCount - pNodeCurrent->TickCount >= 1000
			&& !GetTargetNameCounts( pNodeCurrent->OrignalPath, lpFileName, SHI, &ObjectTypeIndex ) 
			)
		{
			if ( NULL == wcschr(pNodeCurrent->OrignalPath, ':') )
				PB_Log( pNodeCurrent->OrignalPath );

			RemoveEntryListEx( &g_pNodeHead_Handles, &pNodeCurrent->ListEntry );
		}

		pNodeCurrent = pNodeNext ;
	}
	while ( pNodeCurrent );

_end_ :
	kfree( lpFileName );
	kfree( SHI );
	return ;
}


ULONG 
GetTargetNameCounts (
	IN LPWSTR szTargetName, 
	OUT LPWSTR lpFileName, 
	IN LPMYSYSTEM_HANDLE_INFORMATION HandleInfo, 
	IN PBYTE pObjectTypeIndex
	)
{
	HANDLE HandleValue = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsHandlerSelfFilePath = FALSE ;
	ULONG ReturnLength = 0, i = 0, nIndex = 0 ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	UNICODE_STRING uniBuffer ;
	WCHAR Buffer[ 256 ] = {};
	POBJECT_TYPE_INFORMATION pObjectInfo ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( 0 == HandleInfo->NumberOfHandles ) { return 0; }

	while ( TRUE )
	{
		if ( HandleInfo->Handles[i].ProcessId != g_CurrentPID_runInSandbox ) { goto _While_Next_ ; }

		HandleValue = (HANDLE) HandleInfo->Handles[i].Handle ;
		if ( 0 == *pObjectTypeIndex )
		{
			pObjectInfo = (POBJECT_TYPE_INFORMATION) Buffer;
			RtlZeroMemory( pObjectInfo, 256 );

			status = ZwQueryObject( HandleValue, ObjectTypeInformation, pObjectInfo, 0x80, &ReturnLength );
			if ( ! NT_SUCCESS(status) ) { goto _While_Next_ ; }

			if ( *pObjectInfo->Name.Buffer != 'F' || wcsicmp(pObjectInfo->Name.Buffer, L"File") ) { goto _While_Next_ ; }

			// 只处理文件类型的句柄
			*pObjectTypeIndex = HandleInfo->Handles[i].ObjectTypeNumber ;
		}
		else if ( *pObjectTypeIndex != HandleInfo->Handles[i].ObjectTypeNumber )
		{
			goto _While_Next_ ;
		}

		status = PB_GetFileName( HandleValue, 0x3E8, lpFileName );
		if ( ! NT_SUCCESS(status) ) { goto _While_Next_ ; }
		
		RtlInitUnicodeString( &uniBuffer, lpFileName );
		status = GetFilePath ( &uniBuffer, NULL, &OrignalPath, &RedirectedPath, &bIsHandlerSelfFilePath );
		if ( NT_SUCCESS(status) )
		{
			if ( 0 == wcsicmp( szTargetName, OrignalPath ) )
				++ nIndex ;
		}

_While_Next_ :
		++ i ;
		if ( i >= HandleInfo->NumberOfHandles ) { return nIndex; }
	}

	 return 0 ;
}


PVOID DIALOG_ParseTemplate32 ( IN PVOID lpDlgTemplate )
/*++

Author : sudami [sudami@163.com]
Time   : 2010/03/31 [31:3:2010 - 10:49]

Routine Description:
  创建对话框时,填充处理对话框结构体; 主要是修改标题名 & 窗口类名    
    
Arguments:
  lpDlgTemplate - 待解析的对话框结构体指针 

Return Value:
  重构后的新Buffer
    
--*/
{
	LPBYTE pb;
	UINT cItems; 
	ULONG TotalLength = 0 ;
	PVOID pNode = NULL ;
	LPDLGTEMPLATE pdt = (LPDLGTEMPLATE) lpDlgTemplate ;

	// 1. 跳过普通对话框结构体的头部,之后是各种控件信息. 主要是重构标题名 & 窗口类名
	cItems = pdt->cdit;
	pb = CollectDialogHeader( lpDlgTemplate );

	// 2. 处理子控件信息. 主要是重构其中的窗口类名 
	pb = CollectDialogChildrenControls( pb, cItems );

	// 3. 重构一个新的对话框结构体
	TotalLength = ((ULONG)pb - (ULONG)lpDlgTemplate) + g_WindowClass_new_totalLength 
		+ 2 * (wcslen( g_pszTileName_new ) + 8 * cItems) + 8 ;
	
	pNode = kmalloc( TotalLength );
	if ( NULL == pNode ){ return NULL ; }

	BuildDialogTemplate( pNode, lpDlgTemplate );
	return pNode ;
}


PVOID DIALOG_ParseTemplate32Ex( IN PVOID lpDlgTemplate )
/*++

Author : sudami [sudami@163.com]
Time   : 2010/03/31 [31:3:2010 - 10:49]

Routine Description:
  创建对话框时,填充处理对话框结构体; 主要是修改标题名 & 窗口类名    
    
Arguments:
  lpDlgTemplate - [IN] 要解析的对话框结构体指针 

Return Value:
  重构后的新Buffer
    
--*/
{
	LPBYTE pb;
	UINT cItems; 
	ULONG TotalLength = 0 ;
	PVOID pNode = NULL ;
	LPDLGTEMPLATEEX pdtex = (LPDLGTEMPLATEEX) lpDlgTemplate ;

	// 1. 跳过普通对话框结构体的头部,之后便是各种控件信息. 主要是重构标题名 & 窗口类名
	cItems = pdtex->cDlgItems ;
	pb = CollectDialogHeader( lpDlgTemplate );

	// 2. 处理子控件信息. 主要是重构其中的窗口类名 
	pb = CollectDialogChildrenControls( pb, cItems );

	// 3. 重构一个新的对话框结构体
	TotalLength = ((ULONG)pb - (ULONG)lpDlgTemplate) + g_WindowClass_new_totalLength 
		+ 2 * (wcslen( g_pszTileName_new ) + 8 * cItems) + 8 ;
	
//	pNode = GlobalAlloc( 0, TotalLength );
	pNode = kmalloc( TotalLength );
	if ( NULL == pNode ){ return NULL ; }

	BuildDialogTemplate( pNode, lpDlgTemplate );
	return pNode ;
}


PBYTE SkipIDorString( IN LPBYTE pb )
{
	LPWORD pw = (LPWORD)pb;

	if ( 0xFFFF == *pw )
	{
		return (LPBYTE)(pw + 2) ;
	}

	while (*pw++ != 0)
	{
		continue ;
	}

	return (LPBYTE)pw ;
}


PBYTE CollectDialogHeader( IN PVOID _pdt )
/*++

Reserver: sudami [sudami@163.com]
Time    : 2010/03/31 [31:3:2010 - 11:31]

Routine Description:
  跳过普通对话框结构体的头部,之后便是各种控件信息.
  整个对话框结构一般包括
   ________________________
  | 0x12字节的头部         |
  |________________________|
  | 菜单名(menu)           |
  |________________________|
  | 窗口类名(window class) |
  |________________________|
  | 标题名(window Text)    |
  |________________________|
  | 字体信息(font)         |
  |________________________|

Return Value:
  越过结构体头后的新指针
    
--*/
{
	LPBYTE pb = NULL ;
	LPWSTR pszWindowClass = NULL ;
	LPWSTR pszTileName = NULL ;
	LPDLGTEMPLATE pdt = (LPDLGTEMPLATE) _pdt ;
	LPDLGTEMPLATEEX pdtex = NULL ;

	if ( LOWORD(pdt->style) != 1 || HIWORD(pdt->style) != 0xFFFF )
	{
		pdtex = NULL;
		g_fEx = FALSE;
	}
	else
	{
		pdtex = (LPDLGTEMPLATEEX)pdt;
		g_fEx = TRUE;
	}

	// 1. 跳过结构体头部 0x12 / 0x1A 字节
	pb = (LPBYTE)((int)pdt + (g_fEx ? /*sizeof(DLGTEMPLATEEX)*/ 0x1A : sizeof(DLGTEMPLATE)));

	// 2. 跳过菜单(menu)相关信息; If there is a menu ordinal, add 4 bytes skip it. Otherwise it is a string or just a 0.
	pb = SkipIDorString(pb);

	// 3. 跳过window class相关信息
	g_WindowClass_old_Array[ 0 ] = NULL;
	g_WindowClass_new_Array[ 0 ] = NULL;

	switch( GET_WORD( pb ) )
	{
	case 0x0000:
		pb += 2;
		break;

	case 0xffff:
		pb += 4;
		break;

	default:
		g_WindowClass_old_Array[ 0 ] = pszWindowClass = (LPWSTR) pb ;
		pb += (wcslen( pszWindowClass ) + 1) * sizeof(WCHAR);

		// 若有窗口类名 --> 重定向(加上沙箱标志)[申请了内存]
		g_WindowClass_new_Array[ 0 ] = RedirectClassNameW( pszWindowClass );
		g_WindowClass_new_totalLength = 2 * wcslen( g_WindowClass_new_Array[ 0 ]  );
		break;
	}

	// 4.此时指针指向标题名(window Text); --> 重定向标题名(加上沙箱标志)[申请了内存]
	g_pszTileName_old = pszTileName = (LPWSTR) pb ;
	pb += (wcslen( pszTileName ) + 1) * sizeof(WCHAR); // 跳过标题名,将指针移动到结构体中下个地址

	g_pszTileName_new = RedirectWindowNameW( pszTileName );// eg:原来的标题为: "DialogBoxParam",此时变为:"[#] DialogBoxParam [#]"
	
	// 5.. 跳过字体(font)相关信息; Skip font type, size and name, adjust to next dword boundary.
	if ((g_fEx ? pdtex->dwStyle : pdt->style) & 0x48 /*(DS_SETFONT | DS_FIXEDSYS)*/)
	{
		pb += g_fEx ? ( sizeof(DWORD) + sizeof(WORD) ) : sizeof(WORD) ; // 越过 2 / 6 字节的字体大小(pointSize)
		pb = SkipIDorString(pb);	// 越过字体类型说明的字符串
	}

	return pb;
}


PBYTE 
CollectDialogChildrenControls(
	IN LPBYTE pb,
	IN ULONG nCounts
	)
/*++

Reserver: sudami [sudami@163.com]
Time    : 2010/03/31 [31:3:2010 - 11:31]

Routine Description:
  处理子控件信息,子控件结构一般包括
   ____________________________
  | 0x12字节的头部             |
  |____________________________|
  | 窗口类名(window class)     |
  |____________________________|
  | 控件本身内容(control Text) |
  |____________________________|
  | 其他内容                   |
   ----------------------------

Return Value:
  越过结构体头后的新指针
    
--*/
{
	DWORD nIndex = 1; // 必须是1，因为0的位置已经被Dialog header占据
	PWSTR pszWindowClass = NULL ;
	PWSTR pszTileName = NULL ;
	PWSTR pszWindowClass_control_new = NULL ;
	PWSTR pszName_control_new = NULL ;

	while ( nCounts-- )
	{
		// Point at the next dialog item. (DWORD aligned)
		pb = (LPBYTE)(((ULONG_PTR)pb + 3) & ~3);

		// 1. 跳过当前子控件的结构体头部 0x12 / 0x18 字节
		pb += ( g_fEx ? 0x18 : sizeof(DLGTEMPLATE) ) ; // IDB中加的0x18,好奇怪,为什么不是结构体大小0x1A呢? [待解决 2010-3-31 18:57:40]

		// 2. 跳过当前子控件的window class相关信息
		g_WindowClass_old_Array[ nIndex ] = NULL;
		g_WindowClass_new_Array[ nIndex ] = NULL;

		switch( GET_WORD( pb ) )
		{
		case 0x0000:
			pb += 2;
			break;

		case 0xffff:
			pb += 4;
			break;

		default:
			g_WindowClass_old_Array[ nIndex ] = pszWindowClass = (LPWSTR) pb ; // 保存旧的类名指针
			pb += 2 * (wcslen( pszWindowClass ) + 1);

			// 若有窗口类名 --> 重定向(加上沙箱标志)[申请了内存]
			g_WindowClass_new_Array[ nIndex ] = RedirectClassNameW( pszWindowClass );
			g_WindowClass_new_totalLength += 2 * wcslen( g_WindowClass_new_Array[ nIndex ] );
			break;
		}

		nIndex++ ;

		// 3.跳过当前子控件的控件名(control Text);
		pb = SkipIDorString( pb );
		if ( GET_WORD( pb ) )
		{
			pb += GET_WORD( pb );
		}

		pb += 2 ;
	} // end-of-while

	return pb;
}


VOID
Copy_and_move (
	IN OUT LPBYTE *ptrDest,
	IN OUT LPBYTE *ptrSrc,
	IN ULONG size
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/03/31 [31:3:2010 - 15:24]

Routine Description:
  拷贝指定大小的内容,同时移动指针    
    
Arguments:
  ptrDest - [IN] 目的地
  ptrSrc - [IN]  源数据
  size - [IN] 要拷贝的数据大小

--*/
{
	memcpy( *ptrDest, *ptrSrc, size );

	*ptrSrc += size ;
	*ptrDest += size ;

	return ;
}


VOID
BuildDialogTemplate (
	OUT PVOID lpDlgTemplate_new,
	IN PVOID lpDlgTemplate_old
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/03/31 [31:3:2010 - 14:23]

Routine Description:
  构造新的对话框结构体  
    
Arguments:
  lpDlgTemplate_new - [OUT] 新结构体的内存地址
  lpDlgTemplate_old - [IN] 老结构体地址

--*/
{
	LPDLGTEMPLATE pdt = (LPDLGTEMPLATE) lpDlgTemplate_old ;
	LPBYTE pb_new = (LPBYTE) lpDlgTemplate_new ;
	LPBYTE pb_old = (LPBYTE) lpDlgTemplate_old ;
	
	// 1. 拷贝结构体头
	Copy_and_move( &pb_new, &pb_old, ( g_fEx ? 0x1A /*sizeof(DLGTEMPLATEEX)*/ : sizeof(DLGTEMPLATE) ) );

	// 2. 拷贝菜单(menu)相关信息
	if ( 0xFFFF == GET_WORD( pb_old ) )
	{
		*(WORD*) pb_new = 0xFFFF ;
		*(WORD *)(pb_new + 0x2) = *(WORD *)(pb_old + 0x2);

		pb_new += 0x4 ;
		pb_old += 0x4 ;
	}
	else
	{
		wcscpy( (LPWSTR)pb_new, (LPWSTR)pb_old ) ;
		pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
		pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;
	}

	// 3. 拷贝window class相关信息
	if ( g_WindowClass_new_Array[0] )
	{
		wcscpy( (LPWSTR)pb_new, g_WindowClass_new_Array[0] ) ;
		pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
		pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;
	}
	else
	{
		*(WORD *) pb_new = *(WORD *) pb_old ;
		pb_new += 0x2 ;

		if ( 0xFFFF == GET_WORD( pb_old ) )
		{
			pb_old += 0x2 ;
			*(WORD *) pb_new = *(WORD *) pb_old ;

			pb_new += 0x2 ;
		}

		pb_old += 0x2 ;
	}

	// 4. 拷贝标题名相关信息
	wcscpy( (LPWSTR)pb_new, g_pszTileName_new ) ;
	pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
	pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;

	// 释放申请的Buffer
	if ( g_pszTileName_new != g_pszTileName_old )
		kfreeExp( g_pszTileName_new );

	// 5. 拷贝字体(font)相关信息
	ULONG nCounts = 0 ;
	LPDLGTEMPLATEEX pdtex = (LPDLGTEMPLATEEX) pdt; ;

	if (g_fEx)
	{
		nCounts = (ULONG)pdtex->cDlgItems ; // 子控件的数量
		if ( pdtex->dwStyle & 0x48 )
		{
			*(DWORD *) pb_new = *(DWORD *) pb_old ;
			pb_new += 0x4 ;
			pb_old += 0x4 ;

			*(WORD *) pb_new = *(WORD *) pb_old ;
			pb_new += 0x2 ;
			pb_old += 0x2 ;

			wcscpy( (LPWSTR)pb_new, (LPWSTR)pb_old ) ;
			pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
			pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;
		}
	} 
	else
	{
		nCounts = (ULONG)pdt->cdit ;  // 子控件的数量
		if ( pdt->style & 0x48 )
		{
			*(WORD *) pb_new = *(WORD *) pb_old ;
			pb_new += 0x2 ;
			pb_old += 0x2 ;

			wcscpy( (LPWSTR)pb_new, (LPWSTR)pb_old ) ;
			pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
			pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;
		}
	}

	// 6.填充子控件信息
	DWORD nIndex = 1 ; // 必须为1，因为0的位置是供Dialog header使用的
	LPWSTR pCurrentWindowClass = NULL ;

	if ( 0 == nCounts ) { return; }
	while ( nCounts-- )
	{
		*(WORD *) pb_new = 0 ;

		// Point at the next dialog item. (DWORD aligned)
		pb_new = (LPBYTE)(((ULONG_PTR)pb_new + 3) & ~3);
		pb_old = (LPBYTE)(((ULONG_PTR)pb_old + 3) & ~3);

		// 1. 拷贝当前子控件的结构体头部 0x12 字节
		Copy_and_move( &pb_new, &pb_old, ( g_fEx ? 0x18 : sizeof(DLGTEMPLATE) ) );

		// 2. 拷贝当前子控件的window class相关信息
		pCurrentWindowClass = g_WindowClass_new_Array[ nIndex ] ;
		if ( pCurrentWindowClass )
		{
			wcscpy( (LPWSTR)pb_new, pCurrentWindowClass ) ;
			pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
			pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;
		}
		else
		{
			*(WORD *) pb_new = *(WORD *) pb_old ;
			pb_new += 0x2 ;

			if ( 0xFFFF == GET_WORD( pb_old ) )
			{
				pb_old += 0x2 ;
				*(WORD *) pb_new = *(WORD *) pb_old ;

				pb_new += 0x2 ;
			}

			pb_old += 0x2 ;
		}

		// 3. 拷贝当前子控件的控件名(control Text);
		if ( 0xFFFF == GET_WORD( pb_old ) )
		{
			*(DWORD *)pb_new =  *(DWORD *) pb_old ;
			pb_new += 0x4 ;
			pb_old += 0x4 ;
		}
		else
		{
			wcscpy( (LPWSTR)pb_new, (LPWSTR)pb_old ) ;
			pb_new += 2 * (wcslen((LPWSTR)pb_new) + 1) ;
			pb_old += 2 * (wcslen((LPWSTR)pb_old) + 1) ;
		}

		// 4. 其他内容
		if ( GET_WORD( pb_old ) )
		{
			ULONG length = GET_WORD(pb_old) + 2 ;
			memcpy( pb_new, pb_old, length );
			pb_new += length ;
			pb_old += length ;
		}
		else
		{
			*(WORD *)pb_new = 0 ;
			pb_new += 0x2 ;
			pb_old += 0x2 ;
		}

		// 5. 释放内存
		if ( g_WindowClass_new_Array[ nIndex ] && g_WindowClass_new_Array[ nIndex ] != g_WindowClass_old_Array[ nIndex ] )
			kfreeExp( g_WindowClass_new_Array[ nIndex ] );

		nIndex ++ ;
	} // end-of-while

	return ;
}


HHOOK SetWindowsHookExFilter( DWORD idHook, HOOKPROC lpfn, HINSTANCE hMod, BOOL bIsUnicode )
{
	BOOL bFlag = FALSE ;
	LPSetWindowsHookExInfo pNode = NULL ;
	int pBuffer = 0, nIndex = 1, ErrorCode = 0 ;

	pNode = (LPSetWindowsHookExInfo) kmalloc( sizeof(SetWindowsHookExInfo) );
	pNode->lpfn			= lpfn ;
	pNode->FuckedTag	= (ULONG)_FuckedTag_ ;
	pNode->idHook		= idHook ;
	pNode->hMod			= hMod ;
	pNode->bIsUnicode	= bIsUnicode ;

	ClearStruct( &pNode->LittleNode );
	InitializeCriticalSection( &pNode->Lock );

	pBuffer = (int) kmalloc( 4016 );
	*(DWORD *) pBuffer = 0 ;

	g_EnumWindows_addr( (WNDENUMPROC)EnumWndProc_InsertTID, (LPARAM)pBuffer );

	for ( ErrorCode = 0; nIndex <= *(int *)pBuffer; ++nIndex )
	{
		if ( SetWindowsHookExFilterEx( (PVOID)pNode, *(DWORD *)(pBuffer + 4 * nIndex)) )
		{
			bFlag = TRUE ;
		}
		else
		{
			if ( NO_ERROR == ErrorCode || ERROR_ACCESS_DENIED == ErrorCode )
				ErrorCode = GetLastError();
		}
	}

	kfree( (PVOID)pBuffer );
	if ( bFlag )
	{
		EnterCriticalSection( &g_Lock_Msg );
		InsertListA( &g_NodeHead_hhk, NULL, &pNode->ListEntry );
		LeaveCriticalSection(&g_Lock_Msg);

		SetLastError( NO_ERROR );
	}
	else
	{
		kfree( pNode );
		SetLastError( ErrorCode );
		pNode = NULL;
	}

	return (HHOOK)pNode ;
}


BOOL 
SetWindowsHookExFilterEx (
	IN PVOID Data,
	IN DWORD dwThreadId
	)
{
	ULONG dwErrorCode = ERROR_SUCCESS ;
	BOOL bRet = FALSE, bAddNode = FALSE ;
	HANDLE hThread = NULL ;
	HHOOK xx_hook = NULL ;
	FILETIME ExitTime, KernelTime, UserTime, CreationTime ; 
	LPSetWindowsHookEx_Little_Info OldLittleNode = NULL, NewLittleNode = NULL ;
	LPSetWindowsHookExInfo pNode = (LPSetWindowsHookExInfo) Data ;

	// 1.1 检测参数合法性
	if ( NULL == Data ) { return FALSE; }

	// 1.2 查询线程的时间信息
	hThread = (HANDLE) OpenThread( THREAD_QUERY_INFORMATION, 0, dwThreadId );
	if ( NULL == hThread ) { return FALSE; }

	bRet = GetThreadTimes( hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime );
	CloseHandle( hThread );
	if ( FALSE == bRet ) { return FALSE; }

	// 2. 遍历链表
	EnterCriticalSection( &pNode->Lock );

	OldLittleNode = (LPSetWindowsHookEx_Little_Info) pNode->LittleNode.Flink ;
	if ( OldLittleNode )
	{
		while ( TRUE )
		{
			if (   OldLittleNode->dwThreadId == dwThreadId
				&& OldLittleNode->CreationTime.dwLowDateTime == CreationTime.dwLowDateTime
				&& OldLittleNode->CreationTime.dwHighDateTime == CreationTime.dwHighDateTime
				)
			{
				bAddNode = FALSE ;
				break;
			}

			OldLittleNode = OldLittleNode->pFlink ;
			if ( NULL == OldLittleNode )
			{
				bAddNode = TRUE;
				break;
			}
		}
	}

	if ( FALSE == bAddNode ) { goto _END_ ; }

	// 3. 链表中不存在当前节点,新建之
	NewLittleNode = (LPSetWindowsHookEx_Little_Info) kmalloc( sizeof(SetWindowsHookEx_Little_Info) );

	if ( pNode->bIsUnicode )
		xx_hook = g_SetWindowsHookExW_addr( pNode->idHook, pNode->lpfn, pNode->hMod, dwThreadId );
	else
		xx_hook = g_SetWindowsHookExA_addr( pNode->idHook, pNode->lpfn, pNode->hMod, dwThreadId );

	NewLittleNode->xx_hook = xx_hook ;

	if ( xx_hook )
	{
		NewLittleNode->dwThreadId = dwThreadId ;
		NewLittleNode->CreationTime.dwLowDateTime = CreationTime.dwLowDateTime ;
		NewLittleNode->CreationTime.dwHighDateTime = CreationTime.dwHighDateTime ;

		InsertListA( &pNode->LittleNode, NULL, (PLIST_ENTRY)NewLittleNode );
		goto _END_ ;
	}

	dwErrorCode = GetLastError();
	LeaveCriticalSection( &pNode->Lock );
	kfree( NewLittleNode );
	if ( !dwErrorCode ) { dwErrorCode = ERROR_ACCESS_DENIED; }
	SetLastError(dwErrorCode);
	return FALSE;

_END_ :
	LeaveCriticalSection( &pNode->Lock );
	return TRUE;
}


BOOL CALLBACK EnumWndProc_InsertTID( HWND hWnd, LPARAM lParam )
{
	ULONG dwProcessId = 0, dwThreadId = 0, nIndex = 1 ;
	ULONG pBuffer = (ULONG) lParam ;

	if ( *(DWORD *)lParam < 1000 && IsWhitehWndEx(hWnd, &dwProcessId, &dwThreadId) )
	{
		if ( *(DWORD *)pBuffer < 1 )
		{
_ok_:
			*(DWORD *)pBuffer = nIndex ;
			*(DWORD *)(pBuffer + 4 * nIndex) = dwThreadId ;
		}
		else
		{
			while ( *(DWORD *)(pBuffer + 4 * nIndex) != (DWORD) lParam )
			{
				++ nIndex ;
				if ( nIndex > *(DWORD *)pBuffer ) { goto _ok_ ; }
			}
		}

	}
	
	return TRUE;
}


HMODULE Get_hModule_user32DLL()
{
	HMODULE ret = NULL ;

	ret = GetModuleHandleW( L"user32.dll" );
	if ( NULL == ret ) { ret = LoadLibraryW( L"user32.dll" ); }

	return ret;
}


LRESULT CALLBACK AddressBarProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ret = 0 ;
	WNDPROC pfn = NULL ;
	LPARAM lParamNew = NULL ;
	
	if ( FALSE == AddressBarProcFilter(uMsg, lParam, wParam, hWnd) ) { return 0; }

	if ( uMsg == WM_SETTEXT && IshWndCaptionTitle(hWnd) )
		lParamNew = (LPARAM) RedirectWindowNameW((PWSTR)lParam);
	else
		lParamNew = lParam ;

	pfn = (WNDPROC)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldW );
	ret = g_CallWindowProcW_addr( (FARPROC)pfn, hWnd, uMsg, wParam, lParamNew );

	if ( lParamNew != lParam )
	{
		kfree( (PVOID)lParamNew );
		SetLastError( GetLastError() );
	}

	return ret;
}


LRESULT CALLBACK AddressBarProcA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ret = 0 ;
	WNDPROC pfn = NULL ;
	LPARAM lParamNew = NULL ;

	if ( FALSE == AddressBarProcFilter(uMsg, lParam, wParam, hWnd) ) { return 0; }

	if ( uMsg == WM_SETTEXT && IshWndCaptionTitle(hWnd) )
		lParamNew = (LPARAM) RedirectWindowNameA((LPSTR)lParam);
	else
		lParamNew = lParam ;

	pfn = (WNDPROC)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldA );
	ret = g_CallWindowProcA_addr( (FARPROC)pfn, hWnd, uMsg, wParam, lParamNew );

	if ( lParamNew != lParam )
	{
		kfree( (PVOID)lParamNew );
		SetLastError( GetLastError() );
	}

	return ret;
}


BOOL 
AddressBarProcFilter (
	IN UINT wMsg,
	IN LPARAM lParam,
	IN WPARAM wParam,
	IN HWND hWnd
	)
{
	if ( wMsg )
	{
		if ( WM_DROPFILES == wMsg )
		{
			if ( HandlerDropfiles( hWnd, wParam ) ) { return FALSE; }
		}
		else if ( WM_DEVICECHANGE == wMsg ) 
		{
			PB_DeviceChange( wParam, lParam ); 
		}
	}
	else
	{
		if ( wParam == _FuckedTag_ )
		{
			WalkMsghWnd( lParam );
			return FALSE;
		}
	}

	return TRUE ;
}


BOOL CALLBACK
EnumWindowsProc (
	IN HWND hWnd,
	IN LPARAM lParam
	)
{
	ULONG PID = 0, TID = 0 ;

	if ( IsWhitehWndEx( hWnd, &PID, &TID) && (TID != (ULONG)lParam) )
	{
		g_PostMessageW_addr( hWnd, WM_NULL, _FuckedTag_, lParam );
	}

	return TRUE;
}


BOOL 
HandlerDropfiles (
	IN HWND hWnd,
	IN WPARAM hdrop
	)
{
	POINT pt ;
	HRESULT hr = 0 ;
	DWORD dwEffect = 1 ;
	BOOL bIsWMP = FALSE ;
	LPOLEClipbrdEx Buffer = NULL ;
	LPSIDropTargetImpl Info = NULL ;
	HMODULE hModule_shell32_dll = NULL ;

	if ( FALSE == g_bFlag_Hook_OpenWinClass_etc ) { return FALSE; }

	if ( NULL == hdrop ) { return FALSE; }

	Info = (LPSIDropTargetImpl) g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_DropTarget );
	if ( NULL == Info ) { return FALSE; }

	Buffer = (LPOLEClipbrdEx) kmalloc( 0x20 );
	if ( Buffer )
	{
		Buffer->lpVtbl1 = &g_OLEClipbrd_IDataObject_VTable ;
		Buffer->lpVtbl2 = &g_OLEClipbrd_IEnumFORMATETC_VTable ;

		Buffer->hModule_shell32_dll = hModule_shell32_dll = LoadLibraryW( L"shell32.dll" );
		if ( hModule_shell32_dll ) { Buffer->DragQueryFileW_Addr = (ULONG) GetProcAddress(hModule_shell32_dll, "DragQueryFileW"); }

		Buffer->pIDataObjectSrc = NULL ;
		Buffer->hdrop = hdrop ;
		Buffer->RefCounts = 1 ;
	}

	g_GetCursorPos_addr( &pt );

	hr = Info->lpVtbl->DragEnter( (IDropTarget *)Info, (IDataObject *)Buffer, 0, pt.x, pt.y, &dwEffect );
	if ( hr >= 0 )
	{
		hr = Info->lpVtbl->DragOver( (IDropTarget *)Info, 0, pt.x, pt.y, &dwEffect );
		if ( !dwEffect && 0 == wcsicmp( g_BoxInfo.ProcessName, L"wmplayer.exe" ) )
		{
			bIsWMP = TRUE ;
		}

		if ( hr >= 0 && (dwEffect || bIsWMP) )
		{
			Info->lpVtbl->Drop( (IDropTarget *)Info, (IDataObject *)Buffer, 0, pt.x, pt.y, &dwEffect );
		}
	}

	Buffer->lpVtbl1->Release( (IDataObject*)Buffer );

	g_DragFinish_addr( (HDROP)hdrop );
	return TRUE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////