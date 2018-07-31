/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/20 [20:1:2012 - 11:20]
* MODULE : \Code\Project\ProteinBoxDLL\PBUser32dll\PBUser32dll.cpp
* 
* Description:
*   Hook user32.dll 系列函数，处理窗口、Dialog、GlobalHook、etc            
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "../StdAfx.h"
#include "../common.h"
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "PBUser32dllFunctions.h"
#include "PBUser32dllData.h"
#include "PBUser32dll.h"

//////////////////////////////////////////////////////////////////////////

ULONG WINAPI fake_RegisterClassA( WNDCLASSA *lpWndClass );
ULONG WINAPI fake_RegisterClassW( WNDCLASSW *lpWndClass );
ULONG WINAPI fake_RegisterClassExA( WNDCLASSEXA *lpwcx );
ULONG WINAPI fake_RegisterClassExW( WNDCLASSEXW *lpwcx );
ULONG WINAPI fake_UnregisterClassA( LPCSTR lpClassName, HINSTANCE hInstance );
ULONG WINAPI fake_UnregisterClassW( LPWSTR lpClassName, HINSTANCE hInstance );
ULONG WINAPI fake_GetClassInfoA( HINSTANCE hInstance, LPCSTR lpClassName, LPWNDCLASSA lpWndClass );
ULONG WINAPI fake_GetClassInfoW( HINSTANCE hInstance, LPCWSTR lpClassName, LPWNDCLASSW lpWndClass );
ULONG WINAPI fake_GetClassInfoExA( HINSTANCE hInstance, LPCSTR lpszClass, LPWNDCLASSEXA lpwcx );
ULONG WINAPI fake_GetClassInfoExW( HINSTANCE hInstance, LPWSTR lpszClass, LPWNDCLASSEXW lpwcx );
ULONG WINAPI fake_GetClassNameA( HWND hWnd, LPSTR lpClassName, int nMaxCount );
ULONG WINAPI fake_GetClassNameW( HWND hWnd, LPWSTR lpClassName, int nMaxCount );


BOOL g_BlockWinHooks = TRUE ;
/*
BlockWinHooks is a sandbox setting in Proteinbox Ini. It specifies whether Sandboxie will allow sandboxed programs to install system-global hooks. 

Usage: 
[DefaultBox]
BlockWinHooks=n

Specifying BlockWinHooks=n disables this protection, and allows a sandboxed application to install global hooks into all running applications,
both inside and outside the sandbox. 

By default, Sandboxie denies a request to install a global hook, and will instead convert the hook into an application-specific hook, and install
this converted hook only into applications running in the same sandbox as the requesting application.

*/


/*

对应全局变量 g_bFlag_Hook_OpenWinClass_etc

[DefaultBox]
OpenWinClass=ConsoleWindowClass // 表明通过cmd.exe启动的控制台程序(沙箱外)的类名可以被沙箱中的程序操作
OpenWinClass=$:program.exe
OpenWinClass=#
OpenWinClass=*

正常情况下,是不允许沙箱进程操作沙箱外的进程类名的,这里是做例外,即白名单类名.

OpenWinClass=$:program.exe
Permits a program running inside the sandbox to use the PostThreadMessage API to send a message directly to a thread in a target process running outside the sandbox. 
This form of the OpenWinClass setting does not support wildcards,so the process name of the target process must match the name specified in the setting. 

OpenWinClass=#
This setting tells Sandboxie to not alter window class names created by sandboxed programs. Normally, Sandboxie translates class names such as IEFrame to 
Sandbox:DefaultBox::IEFrame in order to better separate windows that belong to sandboxed programs from the rest of the windows in the system. 
However, in some cases, a program outside the sandbox might expect window class names to have a specific name, and therefore might not recognize the windows 
created by a sandboxed program. Specifying OpenWinClass=# resolves this problem, at the cost of a lesser degree of separation. 
Note that OpenWinClass=# does not allow communication with any windows outside the sandbox. 

OpenWinClass=*
This setting tells Sandboxie to not translate window class names as described above, and also makes all windows in the system accessible to sandboxed programs, 
and goes a step further to disable a few other windowing-related Sandboxie functions. This may also cause the Sandboxie indicator [#] to not appear in window titles. 
Note that OpenWinClass=* allows full communication with all windows outside the sandbox. 

*/

typedef enum _OpenWinClass_ 
{
	RegisterClassA_TAG,
	RegisterClassW_TAG,
	RegisterClassExA_TAG,
	RegisterClassExW_TAG,
	UnregisterClassA_TAG,
	UnregisterClassW_TAG,
	GetClassInfoA_TAG,
	GetClassInfoW_TAG,
	GetClassInfoExA_TAG,
	GetClassInfoExW_TAG,
	GetClassNameA_TAG,
	GetClassNameW_TAG
};

// OpenWinClass
static HOOKINFOLittle g_OpenWinClass_Array [] = 
{
	{ Nothing_TAG, RegisterClassA_TAG, "RegisterClassA", 0, NULL, fake_RegisterClassA },
	{ Nothing_TAG, RegisterClassW_TAG, "RegisterClassW", 0, NULL, fake_RegisterClassW },
	{ Nothing_TAG, RegisterClassExA_TAG, "RegisterClassExA", 0, NULL, fake_RegisterClassExA },
	{ Nothing_TAG, RegisterClassExW_TAG, "RegisterClassExW", 0, NULL, fake_RegisterClassExW },
	{ Nothing_TAG, UnregisterClassA_TAG, "UnregisterClassA", 0, NULL, fake_UnregisterClassA },
	{ Nothing_TAG, UnregisterClassW_TAG, "UnregisterClassW", 0, NULL, fake_UnregisterClassW },
	{ Nothing_TAG, GetClassInfoA_TAG, "GetClassInfoA", 0, NULL, fake_GetClassInfoA },
	{ Nothing_TAG, GetClassInfoW_TAG, "GetClassInfoW", 0, NULL, fake_GetClassInfoW },
	{ Nothing_TAG, GetClassInfoExA_TAG, "GetClassInfoExA", 0, NULL, fake_GetClassInfoExA },
	{ Nothing_TAG, GetClassInfoExW_TAG, "GetClassInfoExW", 0, NULL, fake_GetClassInfoExW },
	{ Nothing_TAG, GetClassNameA_TAG, "GetClassNameA", 0, NULL, fake_GetClassNameA },
	{ Nothing_TAG, GetClassNameW_TAG, "GetClassNameW", 0, NULL, fake_GetClassNameW },
};

#define GetOpenWinClassFunc( X )	g_OpenWinClass_Array[ X##_TAG ].OrignalAddress


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+															  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI fake_RegisterClassA( WNDCLASSA *lpWndClass )
{
	WNDCLASSEXA Class ;
	WCHAR Filename[ 0x100 ] = L"" ;
	LPCTSTR lpClassName_old = NULL ;
	_RegisterClassA_ OrignalAddress = (_RegisterClassA_) GetOpenWinClassFunc(RegisterClassA) ;

	memcpy( &Class, lpWndClass, sizeof(WNDCLASSEXA) );
	// 	if ( hModule )
	// 	{
	// 		memset( Filename, 0, 0x100 );
	// 		GetModuleFileNameW( hModule, Filename, 0x7E );
	// 	}
	// 
	// 	lpClassName_old = (int)lpClassName;
	// 	if ( wcsicmp(&Filename, g_offset_comctl32_dll)
	// 		&& wcsicmp(&Filename, g_offset_riched20_dll)
	// 		&& (lpClassName = RedirectClassNameA(lpClassName), !lpClassName) )
	// 	{
	// 		ret = 0;
	// 	}
	// 	else
	// 	{
	// 		Result = g_RegisterClassA_addr(&Class);
	// 		if ( (LPCTSTR)lpClassName_old != lpClassName )
	// 			kfreeExp((PVOID)lpClassName);
	// 		ret = Result;
	// 	}
	// 
	// 	return ret;

	//
	// ? 太奇怪了. hModule & lpClassName 哪儿来的? 凭空蹦出来的吗? 先暂时放着,后续再改
	//

	return OrignalAddress( lpWndClass );
}


ULONG WINAPI fake_RegisterClassW( WNDCLASSW *lpWndClass )
{
	_RegisterClassW_ OrignalAddress = (_RegisterClassW_) GetOpenWinClassFunc(RegisterClassW) ;

	//
	// ? 太奇怪了. hModule & lpClassName 哪儿来的? 凭空蹦出来的吗? 先暂时放着,后续再改
	//

	return OrignalAddress( lpWndClass );
}


ULONG WINAPI fake_RegisterClassExA( WNDCLASSEXA *lpwcx )
{
	_RegisterClassExA_ OrignalAddress = (_RegisterClassExA_) GetOpenWinClassFunc(RegisterClassExA) ;

	//
	// ? 太奇怪了. hModule & lpClassName 哪儿来的? 凭空蹦出来的吗? 先暂时放着,后续再改
	//

	return OrignalAddress( lpwcx );
}


ULONG WINAPI fake_RegisterClassExW( WNDCLASSEXW *lpwcx )
{
	_RegisterClassExW_ OrignalAddress = (_RegisterClassExW_) GetOpenWinClassFunc(RegisterClassExW) ;

	//
	// ? 太奇怪了. hModule & lpClassName 哪儿来的? 凭空蹦出来的吗? 先暂时放着,后续再改
	//

	return OrignalAddress( lpwcx );
}


ULONG WINAPI fake_UnregisterClassA( LPCSTR lpClassName, HINSTANCE hInstance )
{
	BOOL ret = FALSE ;
	LPSTR lpRedirectClassName = NULL ;
	_UnregisterClassA_ OrignalAddress = (_UnregisterClassA_) GetOpenWinClassFunc(UnregisterClassA) ;

	lpRedirectClassName = RedirectClassNameA( (LPSTR)lpClassName );
	ret = OrignalAddress( lpRedirectClassName, hInstance );

	if ( FALSE == ret && ERROR_CLASS_DOES_NOT_EXIST == GetLastError() )
	{
		ret = OrignalAddress( lpClassName, hInstance );
	}

	if ( lpRedirectClassName != lpClassName ) { kfreeExp((PVOID)lpRedirectClassName); }
	return (ULONG)ret;
}


ULONG WINAPI fake_UnregisterClassW( LPWSTR lpClassName, HINSTANCE hInstance )
{
	BOOL ret = FALSE ;
	LPWSTR lpRedirectClassName = NULL ;
	_UnregisterClassW_ OrignalAddress = (_UnregisterClassW_) GetOpenWinClassFunc(UnregisterClassW) ;

	lpRedirectClassName = RedirectClassNameW( lpClassName );
	ret = OrignalAddress( lpRedirectClassName, hInstance );

	if ( FALSE == ret && ERROR_CLASS_DOES_NOT_EXIST == GetLastError() )
	{
		ret = OrignalAddress( lpClassName, hInstance );
	}

	if ( lpRedirectClassName != lpClassName ) { kfreeExp((PVOID)lpRedirectClassName); }
	return (ULONG)ret;
}


ULONG WINAPI fake_GetClassInfoA( HINSTANCE hInstance, LPCSTR lpClassName, LPWNDCLASSA lpWndClass )
{
	BOOL ret = FALSE ;
	LPSTR lpRedirectClassName = NULL ;
	_GetClassInfoA_ OrignalAddress = (_GetClassInfoA_) GetOpenWinClassFunc(GetClassInfoA) ;

	lpRedirectClassName = RedirectClassNameA( (LPSTR)lpClassName );
	ret = OrignalAddress( hInstance, lpRedirectClassName, lpWndClass );


	if ( FALSE == ret && ERROR_CLASS_DOES_NOT_EXIST == GetLastError() )
	{
		ret = OrignalAddress( hInstance, lpClassName, lpWndClass );
	}
	else if ( ret )
	{
		lpWndClass->lpszClassName = lpClassName ;
	} 

	if ( lpRedirectClassName != lpClassName ) { kfreeExp((PVOID)lpRedirectClassName); }
	return (ULONG)ret;
}


ULONG WINAPI fake_GetClassInfoW( HINSTANCE hInstance, LPCWSTR lpClassName, LPWNDCLASSW lpWndClass )
{
	BOOL ret = FALSE ;
	LPWSTR lpRedirectClassName = NULL ;
	_GetClassInfoW_ OrignalAddress = (_GetClassInfoW_) GetOpenWinClassFunc(GetClassInfoW) ;

	lpRedirectClassName = RedirectClassNameW( (LPWSTR)lpClassName );
	ret = OrignalAddress( hInstance, lpRedirectClassName, lpWndClass );


	if ( FALSE == ret && ERROR_CLASS_DOES_NOT_EXIST == GetLastError() )
	{
		ret = OrignalAddress( hInstance, lpClassName, lpWndClass );
	}
	else if ( ret )
	{
		lpWndClass->lpszClassName = lpClassName ;
	} 

	if ( lpRedirectClassName != lpClassName ) { kfreeExp((PVOID)lpRedirectClassName); }
	return (ULONG)ret;
}


ULONG WINAPI fake_GetClassInfoExA( HINSTANCE hInstance, LPCSTR lpszClass, LPWNDCLASSEXA lpwcx )
{
	BOOL ret = FALSE ;
	LPSTR lpRedirectClassName = NULL ;
	_GetClassInfoExA_ OrignalAddress = (_GetClassInfoExA_) GetOpenWinClassFunc(GetClassInfoExA) ;

	lpRedirectClassName = RedirectClassNameA( (LPSTR)lpszClass );
	ret = OrignalAddress( hInstance, lpRedirectClassName, lpwcx );


	if ( FALSE == ret && ERROR_CLASS_DOES_NOT_EXIST == GetLastError() )
	{
		ret = OrignalAddress( hInstance, lpszClass, lpwcx );
	}
	else if ( ret )
	{
		lpwcx->lpszClassName = lpszClass ;
	} 

	if ( lpRedirectClassName != lpszClass ) { kfreeExp((PVOID)lpRedirectClassName); }
	return (ULONG)ret;
}


ULONG WINAPI fake_GetClassInfoExW( HINSTANCE hInstance, LPWSTR lpszClass, LPWNDCLASSEXW lpwcx )
{
	BOOL ret = FALSE ;
	LPWSTR lpRedirectClassName = NULL ;
	_GetClassInfoExW_ OrignalAddress = (_GetClassInfoExW_) GetOpenWinClassFunc(GetClassInfoExW) ;

	lpRedirectClassName = RedirectClassNameW( (LPWSTR)lpszClass );
	ret = OrignalAddress( hInstance, lpRedirectClassName, lpwcx );


	if ( FALSE == ret && ERROR_CLASS_DOES_NOT_EXIST == GetLastError() )
	{
		ret = OrignalAddress( hInstance, lpszClass, lpwcx );
	}
	else if ( ret )
	{
		lpwcx->lpszClassName = lpszClass ;
	} 

	if ( lpRedirectClassName != lpszClass ) { kfreeExp((PVOID)lpRedirectClassName); }
	return (ULONG)ret;
}


ULONG WINAPI fake_GetClassNameA( HWND hWnd, LPSTR lpClassName, int nMaxCount )
{
	ULONG Size = 0, ErrorCode = 0, ret = 0, n = 0 ;
//	LPSTR szClassName = NULL ;
	LPSTR ptr1 = NULL ;
	_GetClassNameA_ OrignalAddress = (_GetClassNameA_) GetOpenWinClassFunc(GetClassNameA) ;

	// 1. 调用原始函数得到句柄对应的类名
//	szClassName = (LPSTR) kmalloc( 0x400 );
	CHAR szClassName[0x400];
	Size = OrignalAddress( hWnd, szClassName, 0x3FF );

	// 2. 若类名不足8字节,或者类名的开头不是"Sandbox:",直接放行之; 否则,去掉"Sandbox:"后返回; 总之,是去掉沙箱的标记,获取原始的窗口类名
	ErrorCode = GetLastError();

	if ( Size > 8 && 0 == strncmp(szClassName, "Sandbox:", 8) && (ptr1 = strchr(szClassName + 8, ':'), ptr1) )
	{
		if ( 0 == nMaxCount ) { goto _end_ ; }

		// 得到去掉"Sandbox:"后的类名,比如在沙箱中一个对话框程序的类名为"Sandbox:#3770",则此时变为"#3770"
		n = szClassName - (ptr1 + 1) + Size ;	
		if ( n > (ULONG)(nMaxCount - 1) ) { n = nMaxCount - 1; }

		memcpy( lpClassName, ptr1 + 1, n );
		lpClassName[ n ] = 0;

		if ( 0xFFFFFFFF != n ) { goto _end_ ; }
	}

	ret = OrignalAddress( hWnd, lpClassName, nMaxCount );

_end_:
//	kfree( szClassName );
	SetLastError( ErrorCode );
	return ret;
}


ULONG WINAPI fake_GetClassNameW( HWND hWnd, LPWSTR lpClassName, int nMaxCount )
{
	ULONG Size = 0, ErrorCode = 0, ret = 0, n = 0 ;
//	LPWSTR szClassName = NULL;
	LPWSTR ptr1 = NULL ;
	_GetClassNameW_ OrignalAddress = (_GetClassNameW_) GetOpenWinClassFunc(GetClassNameW) ;

	// 1. 调用原始函数得到句柄对应的类名
//	szClassName = (LPWSTR) kmalloc( 0x800 );
	WCHAR szClassName[0x400];
	Size = OrignalAddress( hWnd, szClassName, 0x3FF );

	// 2. 若类名不足8字节,或者类名的开头不是"Sandbox:",直接放行之; 否则,去掉"Sandbox:"后返回; 总之,是去掉沙箱的标记,获取原始的窗口类名
	ErrorCode = GetLastError();

	if ( Size > 8 && 0 == wcsncmp(szClassName, L"Sandbox:", 8) && (ptr1 = wcschr(szClassName + 8, ':'), ptr1) )
	{
		if ( 0 == nMaxCount ) { goto _end_ ; }

		// 得到去掉"Sandbox:"后的类名,比如在沙箱中一个对话框程序的类名为"Sandbox:#3770",则此时变为"#3770"
		n = Size - ((ULONG)((char *)(ptr1 + 1) - (char *)szClassName) >> 1);
		if ( n > (ULONG)(nMaxCount - 1) ) { n = nMaxCount - 1; }

		memcpy( lpClassName, ptr1 + 1, n * 2 );
		lpClassName[ n ] = 0;

		if ( 0xFFFFFFFF != n ) { goto _end_ ; }
	}

	ret = OrignalAddress( hWnd, lpClassName, nMaxCount );

_end_:
//	kfree( szClassName );
	SetLastError( ErrorCode );
	return ret;
}


ULONG WINAPI fake_GetWindowTextA( HWND hWnd, LPSTR lpString, int nMaxCount )
{
	int Length = 0 ;

	Length = g_GetWindowTextA_addr( hWnd, lpString, nMaxCount );
	return GetOrigWindowTitleA( hWnd, lpString, Length );
}


ULONG WINAPI fake_GetWindowTextW( HWND hWnd, LPWSTR lpString, int nMaxCount )
{
	int Length = 0 ;

	Length = g_GetWindowTextW_addr( hWnd, lpString, nMaxCount );
	return GetOrigWindowTitleW( hWnd, lpString, Length );
}


ULONG WINAPI fake_ExitWindowsEx( UINT uFlags, DWORD dwReserved )
{
	if ( uFlags & EWX_SHUTDOWN_REBOOT_POWEROFF )
	{
		if ( g_ExitWindowsEx_addr( uFlags, dwReserved ) ) { return TRUE; }
	}

	SetLastError( ERROR_PRIVILEGE_NOT_HELD );
	return FALSE;
}


ULONG WINAPI fake_EndTask( HWND hWnd, BOOL fShutdown, BOOL fForce )
{
	DWORD dwResult = 0 ;

	g_SendMessageTimeoutW_addr( hWnd, WM_CLOSE, 0, 0, 1, 1000, &dwResult );
	SetLastError(NO_ERROR);

	return 1;
}


ULONG WINAPI 
fake_CreateWindowExA (
	DWORD dwExStyle, 
	LPCTSTR lpClassName, 
	LPCTSTR lpWindowName, 
	DWORD dwStyle, 
	int X, 
	int Y, 
	int nWidth, 
	int nHeight,
	HWND hWndParent, 
	HMENU hMenu, 
	HINSTANCE hModule, 
	LPVOID lpParam
	)
{
	HWND hWnd = NULL ;
	BOOL bHandler = FALSE ;
	DWORD ErrorCode = ERROR_SUCCESS ;
	LPSTR lpWindowName_new = NULL, lpClassName_new = NULL ;

	if ( g_SandboxTitle_NO
		|| NULL == lpWindowName
		|| (dwStyle & WS_CAPTION) != WS_CAPTION
		|| hWndParent && dwStyle & WS_CHILD
		)
	{
		lpWindowName_new = (LPSTR) lpWindowName ;
	}
	else
	{
		lpWindowName_new = RedirectWindowNameA( (LPSTR)lpWindowName );
	}

	// 操作前lpClasseName为"SJE_FULLSCREEN",操作后为"Sandbox:DefaultBox:SJE_FULLSCREEN"
	lpClassName_new = RedirectClassNameA( (LPSTR)lpClassName );

	hWnd = g_CreateWindowExA_addr (
		dwExStyle,
		lpClassName_new,
		lpWindowName_new,
		dwStyle,
		X,
		Y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hModule,
		lpParam
		);

	if ( hWnd )
	{
		bHandler = TRUE ;
	}
	else 
	{
		ErrorCode = GetLastError();
		if ( ERROR_CANNOT_FIND_WND_CLASS == ErrorCode || ERROR_INVALID_WINDOW_HANDLE  == ErrorCode )
		{
			hWnd = g_CreateWindowExA_addr (
				dwExStyle,
				(LPCSTR)lpClassName,
				lpWindowName_new,
				dwStyle,
				X,
				Y,
				nWidth,
				nHeight,
				hWndParent,
				hMenu,
				hModule,
				lpParam
				);

			if ( hWnd ) { bHandler = TRUE ; }
		}
	}

	if ( bHandler )
	{
		HandlerWindowLong( hWnd, FALSE );

		WalkMsghWnd( NULL );
	}

	// 2. 清理工作
	if ( lpClassName_new != (LPSTR)lpClassName )
	{
		kfree( (PVOID)lpClassName_new );
		SetLastError( GetLastError() );
	}

	if ( lpWindowName_new != (LPSTR)lpWindowName )
	{
		kfree( (PVOID)lpWindowName_new );
		SetLastError( GetLastError() );
	}

	return (ULONG)hWnd ;
}


ULONG WINAPI 
fake_CreateWindowExW (
	DWORD dwExStyle, 
	LPCWSTR lpClassName, 
	LPCWSTR lpWindowName, 
	DWORD dwStyle, 
	int X, 
	int Y, 
	int nWidth, 
	int nHeight, 
	HWND hWndParent, 
	HMENU hMenu, 
	HINSTANCE hModule, 
	LPVOID lpParam
	)
{
	HWND hWnd = NULL ;
	BOOL bHandler = FALSE ;
	DWORD ErrorCode = ERROR_SUCCESS ;
	LPWSTR lpWindowName_new = NULL, lpClassName_new = NULL ;

	if ( g_SandboxTitle_NO
		|| NULL == lpWindowName
		|| (dwStyle & WS_CAPTION) != WS_CAPTION
		|| hWndParent && dwStyle & WS_CHILD
		)
	{
		lpWindowName_new = (LPWSTR) lpWindowName ;
	}
	else
	{
		lpWindowName_new = RedirectWindowNameW( (LPWSTR)lpWindowName );
	}

	lpClassName_new = RedirectClassNameW( (LPWSTR)lpClassName );

	hWnd = g_CreateWindowExW_addr (
		dwExStyle,
		lpClassName_new,
		lpWindowName_new,
		dwStyle,
		X,
		Y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hModule,
		lpParam
		);

	if ( hWnd )
	{
		bHandler = TRUE ;
	}
	else 
	{
		ErrorCode = GetLastError();
		if ( ERROR_CANNOT_FIND_WND_CLASS == ErrorCode || ERROR_INVALID_WINDOW_HANDLE  == ErrorCode )
		{
			hWnd = g_CreateWindowExW_addr (
				dwExStyle,
				lpClassName,
				lpWindowName_new,
				dwStyle,
				X,
				Y,
				nWidth,
				nHeight,
				hWndParent,
				hMenu,
				hModule,
				lpParam
				);

			if ( hWnd ) { bHandler = TRUE ; }
		}
	}

	if ( bHandler )
	{
		HandlerWindowLong( hWnd, FALSE );

		WalkMsghWnd( NULL );
	}

	// 2. 清理工作
	if ( lpClassName_new != lpClassName )
	{
		kfree( (PVOID)lpClassName_new );
		SetLastError( GetLastError() );
	}

	if ( lpWindowName_new != lpWindowName )
	{
		kfree( (PVOID)lpWindowName_new );
		SetLastError( GetLastError() );
	}

	return (ULONG)hWnd ;
}


ULONG WINAPI fake_DefWindowProcA( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	ULONG ret = 0 ;
	LPARAM lParam_new = NULL ;

	// 若是设置文本相关,进行重定向
	lParam_new = lParam ;
	if ( WM_SETTEXT == Msg && IshWndCaptionTitle(hWnd) )
	{
		lParam_new = (LPARAM) RedirectWindowNameA( (LPSTR)lParam );
	}

	ret = g_DefWindowProcA_addr( hWnd, Msg, wParam, lParam_new );

	if ( lParam_new != lParam )
	{
		kfree( (PVOID)lParam_new );
		SetLastError( GetLastError() );
	}

	return ret ;
}


ULONG WINAPI fake_DefWindowProcW( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	ULONG ret = 0 ;
	LPARAM lParam_new = NULL ;

	// 若是设置文本相关,进行重定向
	lParam_new = lParam ;
	if ( WM_SETTEXT == Msg && IshWndCaptionTitle(hWnd) )
	{
		lParam_new = (LPARAM) RedirectWindowNameW( (LPWSTR)lParam );
	}

	ret = g_DefWindowProcW_addr( hWnd, Msg, wParam, lParam_new );

	if ( lParam_new != lParam )
	{
		kfree( (PVOID)lParam_new );
		SetLastError( GetLastError() );
	}

	return ret ;
}


ULONG WINAPI fake_SetParent( HWND hWndChild, HWND hWndNewParent )
{
	ULONG ret = 0; 

	if ( IsWhitehWndEx(hWndChild, NULL, NULL) )
		ret = (ULONG)g_SetParent_addr( hWndChild, hWndNewParent );
	else
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );

	return ret;
}


ULONG WINAPI fake_MoveWindow( HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint )
{
	ULONG ret = 0; 

	if ( IsWhitehWndEx(hWnd, NULL, NULL) )
		ret = g_MoveWindow_addr( hWnd, X, Y, nWidth, nHeight, bRepaint );
	else
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );

	return ret;
}


ULONG WINAPI fake_SetWindowPos( HWND hWnd, int a2, int a3, int a4, int a5, int a6, int a7 )
{
	ULONG ret = 0; 

	if ( IsWhitehWndEx(hWnd, NULL, NULL) )
		ret = g_SetWindowPos_addr( hWnd, (HWND)a2, a3, a4, a5, a6, a7 );
	else
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );

	return ret;
}


ULONG WINAPI fake_RegisterDeviceNotification( int a1, int a2, int a3 )
{
	SetLastError( NO_ERROR );
	return 0x12345678 ;
}


ULONG WINAPI fake_UnregisterDeviceNotification( int a1 )
{
	SetLastError( NO_ERROR );
	return 1;
}


ULONG WINAPI fake_GetShellWindow()
{
	ULONG Flag = 0 ;
	HWND hWnd = NULL ;
	LPWSTR lpClassName = NULL ;
	LPWSTR lpClassNameDummy = L"Progman" ;


	if ( FALSE == g_bFlag_Hook_OpenWinClass_etc ) { return 0; }

	if ( (ULONG)lpClassNameDummy & 0xFFFF0000 && IsWhiteClassName( lpClassNameDummy ) )
	{
		lpClassName = lpClassNameDummy ;
		Flag = 0x1000;
	}
	else
	{
		lpClassName = RedirectClassNameW( lpClassNameDummy );
	}

	hWnd = g_FindWindowW_addr( (LPCWSTR)lpClassName, NULL );
	FindWindowFilterEx( lpClassName, Flag, hWnd );

	if ( lpClassName != lpClassNameDummy ) { kfreeExp( lpClassName ); }
	return (ULONG)hWnd ;	
}


static BOOL CALLBACK EnumChildWindowsFilter( HWND hWnd, LPARAM _lParam )
{
	BOOL bRet = FALSE ;
	PEnumWindows_Argument_Info pBuffer = (PEnumWindows_Argument_Info)_lParam;

	if ( IsWhitehWnd(hWnd) )
		bRet = pBuffer->lpEnumFunc( hWnd, pBuffer->lParam );
	else
		bRet = 1;

	return bRet;
}


ULONG WINAPI fake_EnumWindows( WNDENUMPROC lpEnumFunc, LPARAM lParam )
{
	ULONG bRet = 0 ; 
	EnumWindows_Argument_Info Buffer ;

	if ( g_bFlag_Hook_OpenWinClass_etc )
	{
		Buffer.lpEnumFunc = lpEnumFunc;
		Buffer.lParam = lParam;

		bRet = g_EnumWindows_addr( (WNDENUMPROC)EnumChildWindowsFilter, (LPARAM)&Buffer );
	}
	else
	{
		bRet = g_EnumWindows_addr( lpEnumFunc, lParam );
	}

	return bRet;
}


ULONG WINAPI fake_EnumChildWindows( HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam )
{
	EnumWindows_Argument_Info Buffer ;

	Buffer.lpEnumFunc = lpEnumFunc ;
	Buffer.lParam     = lParam ;

	return (ULONG)g_EnumChildWindows_addr( hWndParent, (WNDENUMPROC)EnumChildWindowsFilter, (LPARAM)&Buffer );
}


ULONG WINAPI fake_EnumThreadWindows( DWORD dwThreadId, WNDENUMPROC lpfn, LPARAM lParam )
{
	EnumWindows_Argument_Info Buffer ;

	Buffer.lpEnumFunc = lpfn ;
	Buffer.lParam     = lParam ;

	return (ULONG)g_EnumThreadWindows_addr( dwThreadId, (WNDENUMPROC)EnumChildWindowsFilter, (LPARAM)&Buffer );
}


ULONG WINAPI fake_EnumDesktopWindows( HDESK hDesktop, WNDENUMPROC lpfn, LPARAM lParam )
{
	EnumWindows_Argument_Info Buffer ; 

	Buffer.lpEnumFunc = lpfn ;
	Buffer.lParam		= lParam ;

	return (ULONG)g_EnumDesktopWindows_addr( hDesktop, (WNDENUMPROC)EnumChildWindowsFilter, (LPARAM)&Buffer );
}


ULONG WINAPI fake_FindWindowA( LPCSTR lpClassName, LPCSTR lpWindowName )
{
	ULONG Flag = 0 ;
	HWND hWnd = NULL ;
	BOOL bRedirect = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPSTR lpClassNameDummy = NULL, lpWindowNameDummy = NULL ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	// 1. 若存在窗口类名,检查黑白性
	if ( !((ULONG)lpClassName & 0xFFFF0000) )
	{
		bRedirect = TRUE ;
	}
	else
	{
		RtlInitString( &ansiBuffer, lpClassName );
		status = RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );
		if ( ! NT_SUCCESS(status) )
		{
			bRedirect = TRUE ;
		}
		else
		{
			if ( IsWhiteClassName(uniBuffer.Buffer) )
			{
				lpClassNameDummy = (LPSTR)lpClassName ;
				Flag = 0x1000 ; // 类名白名单标志位
			}

			RtlFreeUnicodeString( &uniBuffer );

			if ( NULL == lpClassNameDummy ) { bRedirect = TRUE ; }
		}
	}

	if ( bRedirect ) { lpClassNameDummy = RedirectClassNameA( (LPSTR)lpClassName ); }

	// 2. 通过新的窗口类名,调用原始函数得到对应的窗口句柄
	hWnd = g_FindWindowA_addr( lpClassNameDummy, lpWindowName );
	if ( NULL == hWnd && lpWindowName )
	{
		lpWindowNameDummy = RedirectWindowNameA( (LPSTR)lpWindowName );
		if ( lpWindowNameDummy != (LPSTR)lpWindowName )
		{
			hWnd = g_FindWindowA_addr( (LPCSTR)lpClassNameDummy, (LPCSTR)lpWindowNameDummy );
			kfreeExp( lpWindowNameDummy );
		}

	}

	// 3. 处理得到的窗口句柄
	FindWindowFilter( lpClassNameDummy, Flag, hWnd );

	// 4. 收尾工作
	if ( lpClassNameDummy != (LPSTR)lpClassName ) { kfreeExp( lpClassNameDummy ); }

	return (ULONG)hWnd ;
}


ULONG WINAPI fake_FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	ULONG Flag = 0 ;
	HWND hWnd = NULL ;
	BOOL bRedirect = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPWSTR lpClassNameDummy = NULL, lpWindowNameDummy = NULL ;

	// 1. 若存在窗口类名,检查黑白性
	if ( !((ULONG)lpClassName & 0xFFFF0000) )
	{
		bRedirect = TRUE ;
	}
	else
	{	
		if ( IsWhiteClassName( (LPWSTR)lpClassName ) )
		{
			lpClassNameDummy = (LPWSTR)lpClassName ;
			Flag = 0x1000 ; // 类名白名单标志位
		}

		if ( NULL == lpClassNameDummy ) { bRedirect = TRUE ; }
	}

	if ( bRedirect ) { lpClassNameDummy = RedirectClassNameW( (LPWSTR)lpClassName ); }

	// 2. 通过新的窗口类名,调用原始函数得到对应的窗口句柄
	hWnd = g_FindWindowW_addr( lpClassNameDummy, lpWindowName );
	if ( NULL == hWnd && lpWindowName )
	{
		lpWindowNameDummy = RedirectWindowNameW( (LPWSTR)lpWindowName );
		if ( lpWindowNameDummy != (LPWSTR)lpWindowName )
		{
			hWnd = g_FindWindowW_addr( lpClassNameDummy, lpWindowNameDummy );
			kfreeExp( lpWindowNameDummy );
		}

	}

	// 3. 处理得到的窗口句柄
	FindWindowFilterEx( (LPCWSTR)lpClassNameDummy, Flag, hWnd );

	// 4. 收尾工作
	if ( lpClassNameDummy != (LPWSTR)lpClassName ) { kfreeExp( lpClassNameDummy ); }

	return (ULONG)hWnd ;
}


ULONG WINAPI fake_FindWindowExA( HWND hwndParent, HWND hwndChildAfter, LPCSTR lpClassName, LPCSTR lpWindowName )
{
	ULONG Flag = 0 ;
	HWND hWnd = NULL ;
	BOOL bRedirect = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPSTR lpClassNameDummy = NULL, lpWindowNameDummy = NULL ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	// 1. 若存在窗口类名,检查黑白性
	if ( !((ULONG)lpClassName & 0xFFFF0000) )
	{
		bRedirect = TRUE ;
	}
	else
	{
		RtlInitString( &ansiBuffer, lpClassName );
		status = RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );
		if ( ! NT_SUCCESS(status) )
		{
			bRedirect = TRUE ;
		}
		else
		{
			if ( IsWhiteClassName(uniBuffer.Buffer) )
			{
				lpClassNameDummy = (LPSTR)lpClassName ;
				Flag = 0x1000 ; // 类名白名单标志位
			}

			RtlFreeUnicodeString( &uniBuffer );

			if ( NULL == lpClassNameDummy ) { bRedirect = TRUE ; }
		}
	}

	if ( bRedirect ) { lpClassNameDummy = RedirectClassNameA( (LPSTR)lpClassName ); }

	// 2. 通过新的窗口类名,调用原始函数得到对应的窗口句柄
	hWnd = g_FindWindowExA_addr( hwndParent, hwndChildAfter, lpClassNameDummy, lpWindowName );
	if ( NULL == hWnd && lpWindowName )
	{
		lpWindowNameDummy = RedirectWindowNameA( (LPSTR)lpWindowName );
		if ( lpWindowNameDummy != lpWindowName )
		{
			hWnd = g_FindWindowExA_addr( hwndParent, hwndChildAfter, lpClassNameDummy, lpWindowNameDummy );
			kfreeExp( lpWindowNameDummy );
		}

	}

	// 3. 处理得到的窗口句柄
	FindWindowFilter( lpClassNameDummy, Flag, hWnd );

	// 4. 收尾工作
	if ( lpClassNameDummy != lpClassName ) { kfreeExp( lpClassNameDummy ); }

	return (ULONG)hWnd ;
}


ULONG WINAPI fake_FindWindowExW( HWND hwndParent, HWND hwndChildAfter, LPCWSTR lpClassName, LPCWSTR lpWindowName )
{
	ULONG Flag = 0 ;
	HWND hWnd = NULL ;
	BOOL bRedirect = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPWSTR lpClassNameDummy = NULL, lpWindowNameDummy = NULL ;

	// 1. 若存在窗口类名,检查黑白性
	if ( !((ULONG)lpClassName & 0xFFFF0000) )
	{
		bRedirect = TRUE ;
	}
	else
	{	
		if ( IsWhiteClassName( (LPWSTR)lpClassName ) )
		{
			lpClassNameDummy = (LPWSTR)lpClassName ;
			Flag = 0x1000 ; // 类名白名单标志位
		}

		if ( NULL == lpClassNameDummy ) { bRedirect = TRUE ; }
	}

	if ( bRedirect ) { lpClassNameDummy = RedirectClassNameW( (LPWSTR)lpClassName ); }

	// 2. 通过新的窗口类名,调用原始函数得到对应的窗口句柄
	hWnd = g_FindWindowExW_addr( hwndParent, hwndChildAfter, lpClassNameDummy, lpWindowName );
	if ( NULL == hWnd && lpWindowName )
	{
		lpWindowNameDummy = RedirectWindowNameW( (LPWSTR)lpWindowName );
		if ( lpWindowNameDummy != lpWindowName )
		{
			hWnd = g_FindWindowExW_addr( hwndParent, hwndChildAfter, lpClassNameDummy, lpWindowNameDummy );
			kfreeExp( lpWindowNameDummy );
		}

	}

	// 3. 处理得到的窗口句柄
	FindWindowFilterEx( lpClassNameDummy, Flag, hWnd );

	// 4. 收尾工作
	if ( lpClassNameDummy != (LPWSTR)lpClassName ) { kfreeExp( lpClassNameDummy ); }

	return (ULONG)hWnd ;
}


ULONG WINAPI fake_GetPropA( HWND hWnd, LPCSTR lpString )
{
	ULONG ret = 0 ;
	LPCSTR lpStringDummy = NULL ;

	if ( IsWhitehWnd(hWnd) )
	{
		lpStringDummy = (LPSTR) RedirectPropString( (ULONG)lpString );
		ret = (ULONG)g_GetPropA_addr( hWnd, lpStringDummy );
	}
	else
	{
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );
	}

	return ret;
}


ULONG WINAPI fake_GetPropW( HWND hWnd, LPCWSTR lpString )
{
	ULONG ret = 0 ;
	LPWSTR lpStringDummy = NULL ;

	if ( IsWhitehWnd(hWnd) )
	{
		lpStringDummy = (LPWSTR) RedirectPropString( (ULONG)lpString );
		ret = (ULONG)g_GetPropW_addr( hWnd, lpStringDummy );
	}
	else
	{
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );
	}

	return ret;
}


ULONG WINAPI fake_SetPropA( HWND hWnd, LPSTR lpString, HANDLE hData )
{
	ULONG ret = 0 ;
	LPSTR lpStringDummy = NULL ;

	if ( IsWhitehWnd(hWnd) )
	{
		lpStringDummy = (LPSTR) RedirectPropString( (ULONG)lpString );
		ret = g_SetPropA_addr( hWnd, lpStringDummy, hData );
	}
	else
	{
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );
	}

	return ret;
}


ULONG WINAPI fake_SetPropW( HWND hWnd, LPWSTR lpString, HANDLE hData )
{
	ULONG ret = 0 ;
	LPCWSTR lpStringDummy = NULL ;

	if ( IsWhitehWnd(hWnd) )
	{
		lpStringDummy = (LPWSTR) RedirectPropString( (ULONG)lpString );
		ret = g_SetPropW_addr( hWnd, lpStringDummy, hData );
	}
	else
	{
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );
	}

	return ret;
}


ULONG WINAPI fake_RemovePropA( HWND hWnd, LPSTR lpString )
{
	ULONG ret = 0 ;
	LPSTR lpStringDummy = NULL ;

	if ( IsWhitehWnd(hWnd) )
	{
		lpStringDummy = (LPSTR) RedirectPropString( (ULONG)lpString );
		ret = (ULONG)g_RemovePropA_addr( hWnd, lpStringDummy );
	}
	else
	{
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );
	}

	return ret;
}


ULONG WINAPI fake_RemovePropW( HWND hWnd, LPWSTR lpString )
{
	ULONG ret = 0 ;
	LPWSTR lpStringDummy = NULL ;

	if ( IsWhitehWnd(hWnd) )
	{
		lpStringDummy = (LPWSTR) RedirectPropString( (ULONG)lpString );
		ret = (ULONG)g_RemovePropW_addr( hWnd, lpStringDummy );
	}
	else
	{
		SetLastError( ERROR_INVALID_WINDOW_HANDLE );
	}

	return ret;
}


ULONG WINAPI fake_GetWindowLongA( HWND hWnd, int nIndex )
{
	LONG style, lppa, lppw ; 

	HandlerAtoms();

	style = g_GetWindowLongA_addr( hWnd, nIndex );

	if ( GWL_WNDPROC == nIndex )
	{
		lppw = (LONG) g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewW );
		lppa = (LONG) g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewA );

		if ( (lppw && style == lppw) || (lppa && style == lppa) )
		{
			return (LONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldA );
		}
	}
	else if ( (GWL_EXSTYLE == nIndex) && (style & WS_EX_ACCEPTFILES) )
	{
		if ( g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_DropTarget ) )
		{
			//	style &= 0xFFFFFFEF ; // 去掉 WS_EX_ACCEPTFILES 属性
			style |= WS_EX_ACCEPTFILES ;
		}
	}

	return (ULONG)style ;
}


ULONG WINAPI fake_GetWindowLongW( HWND hWnd, int nIndex )
{
	LONG style, lppa, lppw ; 

	HandlerAtoms();

	style = g_GetWindowLongW_addr( hWnd, nIndex );

	if ( GWL_WNDPROC == nIndex )
	{
		lppw = (LONG) g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewW );
		lppa = (LONG) g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewA );

		if ( (lppw && style == lppw) || (lppa && style == lppa) )
		{
			return (LONG) g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldW );
		}
	}
	else if ( (GWL_EXSTYLE == nIndex) && (style & WS_EX_ACCEPTFILES) )
	{
		if ( g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_DropTarget ) )
		{
			//	style &= 0xFFFFFFEF ; // 去掉 WS_EX_ACCEPTFILES 属性
			style |= WS_EX_ACCEPTFILES ;
		}
	}

	return (ULONG)style ;
}


ULONG WINAPI fake_SetWindowLongA( HWND hWnd, int nIndex, LONG dwNewLong )
{
	LONG style, lppa, lppw, ret ; 

	HandlerAtoms();

	style = g_SetWindowLongA_addr( hWnd, nIndex, dwNewLong );

	if ( GWL_WNDPROC == nIndex )
	{
		lppw = (LONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewW );
		lppa = (LONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewA );

		if ( (lppw && style == lppw) || (lppa && style == lppa) )
		{
			ret = (ULONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldA );
			HandlerWindowLong( hWnd, FALSE );
			return ret;
		}
	}
	else if ( (GWL_EXSTYLE == nIndex) && (style & WS_EX_ACCEPTFILES) )
	{
		if ( g_Atom_SBIE_DropTarget )
		{
			g_RemovePropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_DropTarget );
		}
	}

	return (ULONG)style ;
}


ULONG WINAPI fake_SetWindowLongW( HWND hWnd, int nIndex, LONG dwNewLong )
{
	LONG style, lppa, lppw, ret ; 

	HandlerAtoms();

	style = g_SetWindowLongW_addr( hWnd, nIndex, dwNewLong );

	if ( GWL_WNDPROC == nIndex )
	{
		lppw = (LONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewW );
		lppa = (LONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcNewA );

		if ( (lppw && style == lppw) || (lppa && style == lppa) )
		{
			ret = (ULONG)g_GetPropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_WindowProcOldW );
			HandlerWindowLong( hWnd, FALSE );
			return ret;
		}
	}
	else if ( (GWL_EXSTYLE == nIndex) && (style & WS_EX_ACCEPTFILES) )
	{
		if ( g_Atom_SBIE_DropTarget )
		{
			g_RemovePropW_addr( hWnd, (LPCWSTR)g_Atom_SBIE_DropTarget );
		}
	}

	return (ULONG)style ;
}


ULONG WINAPI fake_SendMessageA( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
	{
		ret = HandlerBroadcastMessage( (HWND)_FuckedTag_, g_SendNotifyMessageA_addr, Msg, wParam, lParam, 0, 0 );
	}
	else
	{
		ret = g_SendMessageA_addr( hWnd, Msg, wParam, lParam );

		if ( Msg == WM_GETTEXT )
		{
			ret = GetOrigWindowTitleA( hWnd, (LPSTR)lParam, ret );
		}
	}

	return ret;
}


ULONG WINAPI fake_SendMessageW( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	ULONG ret = 0 ;
	BOOL bRet = FALSE ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
	{
		ret = HandlerBroadcastMessage( (HWND)_FuckedTag_, g_SendNotifyMessageW_addr, Msg, wParam, lParam, 0, 0 );
	}
	else
	{
		if ( WM_CAP_DRIVER_DISCONNECT == Msg )
		{
			bRet = Handler_SendMessageW_WM_CAP_DRIVER_DISCONNECT( (LPWSTR)lParam, hWnd, WM_CAP_DRIVER_DISCONNECT, wParam, &ret );
			if ( bRet ) { return ret; }
		}

		ret = g_SendMessageW_addr( hWnd, Msg, wParam, lParam );
		if ( Msg == WM_GETTEXT )
		{
			ret = GetOrigWindowTitleW( hWnd, (LPWSTR)lParam, ret );
		}
	}

	return ret;
}


ULONG WINAPI fake_SendMessageTimeoutA(HWND hWnd, UINT a2, WPARAM a3, LPARAM a4, UINT a5, UINT a6, PDWORD a7)
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
		ret = HandlerBroadcastMessage((HWND)a5, g_SendMessageTimeoutA_addr, a2, a3, a4, a6, (int)a7);
	else
		ret = g_SendMessageTimeoutA_addr(hWnd, a2, a3, a4, a5, a6, a7);

	return ret;
}


ULONG WINAPI fake_SendMessageTimeoutW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, UINT fuFlags, UINT uTimeout, PDWORD lpdwResult)
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
	{
		ret = HandlerBroadcastMessage( (HWND)fuFlags, g_SendMessageTimeoutW_addr, Msg, wParam, lParam, uTimeout, (int)lpdwResult );
	}
	else
	{
		ret = g_SendMessageTimeoutW_addr( hWnd, Msg, wParam, lParam, fuFlags, uTimeout, lpdwResult );
		if ( 0 == ret && g_bIs_Inside_Explorer_exe )
		{
			if ( 0 == Msg && 0 == wParam && 0 == lParam && (fuFlags & SMTO_ABORTIFHUNG) )
			{
				if ( hWnd == g_GetClipboardOwner_addr() )
				{
					*lpdwResult = ret ;
					return 1 ;
				}
			}
		}
	}

	return ret;
}


ULONG WINAPI fake_SendNotifyMessageA(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
		ret = HandlerBroadcastMessage( (HWND)_FuckedTag_, g_SendNotifyMessageA_addr, wMsg, wParam, lParam, 0, 0 );
	else
		ret = g_SendNotifyMessageA_addr( hWnd, wMsg, wParam, lParam );

	return ret;
}


ULONG WINAPI fake_SendNotifyMessageW( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
		ret = HandlerBroadcastMessage( (HWND)_FuckedTag_, g_SendNotifyMessageW_addr, wMsg, wParam, lParam, 0, 0 );
	else
		ret = g_SendNotifyMessageW_addr( hWnd, wMsg, wParam, lParam );

	return ret;
}


ULONG WINAPI fake_PostMessageA(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
	{
		ret = HandlerBroadcastMessage( (HWND)_FuckedTag_, g_PostMessageA_addr, wMsg, wParam, lParam, 0, 0 );
	}
	else
	{
		ret = g_PostMessageA_addr( hWnd, wMsg, wParam, lParam );
		if ( 0 == ret )
		{
			if ( wMsg == WM_DROPFILES || wMsg == WM_DDE_ACK || wMsg == WM_DDE_TERMINATE )
			{
				SetLastError(NO_ERROR);
				ret = 1 ;
			}
		}
	}

	return ret;
}


ULONG WINAPI fake_PostMessageW(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	ULONG ret = 0 ;

	if ( HWND_BROADCAST == hWnd || (HWND)0xFFFFFFFF == hWnd )
	{
		ret = HandlerBroadcastMessage( (HWND)_FuckedTag_, g_PostMessageW_addr, wMsg, wParam, lParam, 0, 0 );
	}
	else
	{
		ret = g_PostMessageW_addr( hWnd, wMsg, wParam, lParam );
		if ( 0 == ret )
		{
			if ( wMsg == WM_DROPFILES || wMsg == WM_DDE_ACK || wMsg == WM_DDE_TERMINATE )
			{
				SetLastError(NO_ERROR);
				ret = 1 ;
			}
		}
	}

	return ret;
}


ULONG WINAPI fake_DispatchMessageA( MSG *lpmsg )
{
	ULONG ret = 0 ;

	if ( lpmsg->message || lpmsg->wParam != _FuckedTag_ )
	{
		ClearHandlersCache( FALSE );
		ret = g_DispatchMessageA_addr( lpmsg );
	}
	else
	{
		WalkMsghWnd( lpmsg->lParam );
	}

	return ret;
}


ULONG WINAPI fake_DispatchMessageW( MSG *lpmsg )
{
	ULONG ret = 0 ;

	if ( lpmsg->message || lpmsg->wParam != _FuckedTag_ )
	{
		ClearHandlersCache( FALSE );
		ret = g_DispatchMessageW_addr( lpmsg );
	}
	else
	{
		WalkMsghWnd( lpmsg->lParam );
	}

	return ret;
}


ULONG WINAPI fake_SetWindowsHookExA( DWORD idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId )
{
	HHOOK ret ;

	if ( dwThreadId || 0 == idHook || idHook == WH_JOURNALPLAYBACK || idHook == WH_KEYBOARD_LL || idHook == WH_MOUSE_LL )
		ret = g_SetWindowsHookExA_addr( idHook, lpfn, hMod, dwThreadId );
	else
		ret = SetWindowsHookExFilter( idHook, lpfn, hMod, FALSE );

	return (ULONG)ret;
}


ULONG WINAPI fake_SetWindowsHookExW( DWORD idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId )
{
	HHOOK ret ; 

	if ( dwThreadId || 0 == idHook || idHook == WH_JOURNALPLAYBACK || idHook == WH_KEYBOARD_LL || idHook == WH_MOUSE_LL )
		ret = g_SetWindowsHookExW_addr(idHook, lpfn, hMod, dwThreadId);
	else
		ret = SetWindowsHookExFilter( idHook, lpfn, hMod, TRUE );

	return (ULONG)ret;
}


ULONG WINAPI fake_UnhookWindowsHookEx( HHOOK _hhk )
{
	LPSetWindowsHookExInfo pNode = NULL ;
	LPSetWindowsHookEx_Little_Info pLittleNode = NULL ;
	LPSetWindowsHookExInfo hhk = (LPSetWindowsHookExInfo) _hhk ;

	if ( ! ((BYTE)hhk & 1) )
	{
		pNode = hhk ;
		if ( hhk->FuckedTag != _FuckedTag_ )
		{
			pNode = NULL ;
		}
	}

	if ( NULL == pNode ) { return g_UnhookWindowsHookEx_addr( (HHOOK)hhk ); ; }

	EnterCriticalSection( &g_Lock_Msg );
	RemoveEntryListEx( &g_NodeHead_hhk, &pNode->ListEntry );
	LeaveCriticalSection( &g_Lock_Msg );

	//
	EnterCriticalSection( &pNode->Lock );

	pLittleNode = (LPSetWindowsHookEx_Little_Info) pNode->LittleNode.Flink ;
	if ( pLittleNode )
	{
		do
		{
			g_UnhookWindowsHookEx_addr( pLittleNode->xx_hook );
			RemoveEntryListEx( &pNode->LittleNode, (PLIST_ENTRY)pLittleNode );
			kfree( pLittleNode );
			pLittleNode = pLittleNode->pFlink ;
		}
		while ( pLittleNode );
	}

	LeaveCriticalSection( &pNode->Lock );
	kfree( pNode );

	return 1 ;
}


ULONG WINAPI 
fake_CreateDialogIndirectParamAorW (
	HINSTANCE hInstance,
	LPCDLGTEMPLATE lpTemplate,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM lParamInit,
	DWORD Flags
	)
{
	HWND hWnd = NULL ;
	LPCDLGTEMPLATE lpDlgTemplateNew = NULL ;
	LPCDLGTEMPLATE lpDlgTemplateDummy = lpTemplate ;

	// 1. 参考ReactOS源码中的DIALOG_ParseTemplate32()
	if ( LOWORD(lpTemplate->style) != 1 || HIWORD(lpTemplate->style) != 0xFFFF )
	{
		lpDlgTemplateNew = (LPCDLGTEMPLATE) DIALOG_ParseTemplate32( (PVOID)lpTemplate );
	}
	else
	{
		lpDlgTemplateNew = (LPCDLGTEMPLATE) DIALOG_ParseTemplate32Ex( (PVOID)lpTemplate );
	}

	// 2. 重解析后,调用原始函数创建窗口,得到对应句柄
	if ( lpDlgTemplateNew ) { lpDlgTemplateDummy = lpDlgTemplateNew; }

	hWnd = g_CreateDialogIndirectParamAorW_addr( hInstance, lpDlgTemplateDummy, hWndParent, lpDialogFunc, lParamInit, Flags );

	// 3. 对创建的窗口句柄进行预处理
	if ( hWnd ) { HandlerWindowLong( hWnd, TRUE ); }

	if ( lpDlgTemplateNew ) { kfreeExp( (PVOID)lpDlgTemplateNew ); }
	return (ULONG)hWnd ;
}


static BOOL CALLBACK MyDialogFunc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	BOOL ret = FALSE ;
	LPVOID lpDialogFunc = NULL ; 
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	HandlerWindowLong( hWnd, TRUE );

	lpDialogFunc = pNode->lpDialogFunc ;
	pNode->lpDialogFunc = NULL ;

	if ( g_IsWindowUnicode_addr(hWnd) ) 
		g_SetWindowLongW_addr( hWnd, DWL_DLGPROC, (LONG)lpDialogFunc );
	else 
		g_SetWindowLongA_addr( hWnd, DWL_DLGPROC, (LONG)lpDialogFunc );

	if ( lpDialogFunc )
	{
		typedef BOOL(CALLBACK *_DLGPROC_)(HWND,UINT,WPARAM,LPARAM);
		_DLGPROC_ pFunc = (_DLGPROC_) lpDialogFunc;
		ret = pFunc( hWnd, msg, wParam, lParam );
	}

	return ret;
}


ULONG WINAPI fake_DialogBoxIndirectParamAorW(HANDLE hModule, LPCDLGTEMPLATE lpDlgTemplate, HANDLE hwndOwner, PVOID lpDialogFunc, LPARAM dwInitParam, int fAnsiFlags)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/05/05 [5:5:2011 - 16:13]

Routine Description:
  重点关注 和对话框创建相关的5个函数中都会用到的的重要过滤函数 DIALOG_ParseTemplate32(Ex)
  DialogBoxIndirectParamA / DialogBoxIndirectParamW / DialogBoxIndirectParamAorW / DialogBoxParamA / DialogBoxParamW
    
--*/
{
	HWND hWnd = NULL ;
	LPCDLGTEMPLATE lpDlgTemplateNew = NULL ;
	LPCDLGTEMPLATE lpDlgTemplateDummy = lpDlgTemplate ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 参考ReactOS源码中的DIALOG_ParseTemplate32()
	if ( LOWORD(lpDlgTemplate->style) != 1 || HIWORD(lpDlgTemplate->style) != 0xFFFF )
	{
		lpDlgTemplateNew = (LPCDLGTEMPLATE) DIALOG_ParseTemplate32( (PVOID)lpDlgTemplate );
	}
	else
	{
		lpDlgTemplateNew = (LPCDLGTEMPLATE) DIALOG_ParseTemplate32Ex( (PVOID)lpDlgTemplate );
	}

	// 2. 重解析后,调用原始函数创建窗口,得到对应句柄
	if ( lpDlgTemplateNew ) { lpDlgTemplateDummy = lpDlgTemplateNew; }

	pNode->lpDialogFunc = lpDialogFunc ;

	hWnd = (HWND)g_DialogBoxIndirectParamAorW_addr( (HINSTANCE)hModule, lpDlgTemplateDummy, (HWND)hwndOwner, (DLGPROC)MyDialogFunc, dwInitParam, fAnsiFlags );

	pNode->lpDialogFunc = NULL ;

	// 3. 清理工作
	if ( lpDlgTemplateNew ) { kfreeExp( (PVOID)lpDlgTemplateNew ); }
	return (ULONG)hWnd ;
}


ULONG WINAPI fake_CreateDialogParamA(HMODULE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	HRSRC hRes ;
	HGLOBAL hDialogRes ;
	PVOID lpDlgTemplate ;
	ULONG ret = 0 ;

	hRes = FindResourceA( hInstance, lpTemplateName, (LPCSTR)RT_DIALOG );

	if ( hRes && (hDialogRes = LoadResource(hInstance, hRes)) != 0 && (lpDlgTemplate = LockResource(hDialogRes)) != 0 )
		ret = fake_CreateDialogIndirectParamAorW( hInstance, (LPCDLGTEMPLATE)lpDlgTemplate, hWndParent, lpDialogFunc, dwInitParam, 2 );

	return ret;
}


ULONG WINAPI fake_CreateDialogParamW(HMODULE hModule, LPCWSTR lpName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	HRSRC hRes ;
	HGLOBAL hDialogRes ;
	PVOID lpDlgTemplate ;
	ULONG ret = 0 ;

	hRes = FindResourceW( hModule, lpName, (LPCWSTR)RT_DIALOG );

	if ( hRes && (hDialogRes = LoadResource(hModule, hRes)) != 0 && (lpDlgTemplate = LockResource(hDialogRes)) != 0 )
		ret = fake_CreateDialogIndirectParamAorW(hModule, (LPCDLGTEMPLATE)lpDlgTemplate, hWndParent, lpDialogFunc, lParamInit, 0);

	return ret;
}


ULONG WINAPI fake_CreateDialogIndirectParamA(HANDLE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	return fake_CreateDialogIndirectParamAorW( (HINSTANCE)hInstance, (LPCDLGTEMPLATE)lpTemplate, hWndParent, lpDialogFunc, lParamInit, 2 );
}


ULONG WINAPI fake_CreateDialogIndirectParamW(HANDLE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	return fake_CreateDialogIndirectParamAorW( (HINSTANCE)hInstance, (LPCDLGTEMPLATE)lpTemplate, hWndParent, lpDialogFunc, lParamInit, 0 );
}


ULONG WINAPI fake_DialogBoxParamA( HMODULE hModule, LPCSTR lpName, HWND hwndOwner, PVOID lpDialogFunc, LPARAM dwInitParam )
{
	HRSRC hRes ;
	HGLOBAL hDialogRes ;
	PVOID lpDlgTemplate ;
	ULONG ret = 0 ;

	hRes = FindResourceA( hModule, lpName, (LPCSTR)RT_DIALOG );

	if ( hRes && (hDialogRes = LoadResource(hModule, hRes)) != 0 && (lpDlgTemplate = LockResource(hDialogRes)) != 0 )
		ret = fake_DialogBoxIndirectParamAorW( hModule, (LPCDLGTEMPLATE)lpDlgTemplate, hwndOwner, lpDialogFunc, dwInitParam, 2 );
	
	return ret;
}


ULONG WINAPI fake_DialogBoxParamW( HMODULE hModule, LPCWSTR lpName, HWND hwndOwner, PVOID lpDialogFunc, LPARAM dwInitParam )
{
	HRSRC hRes ;
	HGLOBAL hDialogRes ;
	PVOID lpDlgTemplate ;
	ULONG ret = 0 ;

	hRes = FindResourceW( hModule, lpName, (LPCWSTR)RT_DIALOG );

	if ( hRes && (hDialogRes = LoadResource(hModule, hRes)) != 0 && (lpDlgTemplate = LockResource(hDialogRes)) != 0 )
		ret = fake_DialogBoxIndirectParamAorW( hModule, (LPCDLGTEMPLATE)lpDlgTemplate, hwndOwner, lpDialogFunc, dwInitParam, 0 );
	
	return ret;
}


ULONG WINAPI fake_DialogBoxIndirectParamA(HANDLE hModule, PVOID lpDlgTemplate, HWND hwndowner, PVOID lpDialogFunc, LPARAM dwInitParam)
{
	return fake_DialogBoxIndirectParamAorW( hModule, (LPCDLGTEMPLATE)lpDlgTemplate, hwndowner, lpDialogFunc, dwInitParam, 2 );
}


ULONG WINAPI fake_DialogBoxIndirectParamW(HANDLE hModule, PVOID lpDlgTemplate, HWND hwndowner, PVOID lpDialogFunc, LPARAM dwInitParam)
{
	return (ULONG)fake_DialogBoxIndirectParamAorW( hModule, (LPCDLGTEMPLATE)lpDlgTemplate, hwndowner, lpDialogFunc, dwInitParam, 0 );
}


BOOL Hook_pharase12_user32dll_Windows_dep1( IN HMODULE hModule )
{
	// 1. 保存字符串"Sandbox:DefaultBox:"的Unicode & Ansi格式到全局变量中
	g_SandboxNameInfo.SClasseNameW.szName = (LPWSTR) kmalloc( 2 * (wcslen(g_BoxInfo.BoxName) + wcslen(L"Sandbox")) + 6 );
	swprintf( g_SandboxNameInfo.SClasseNameW.szName, L"%s:%s:", L"Sandbox", g_BoxInfo.BoxName); // "Sandbox:DefaultBox:"

	g_SandboxNameInfo.SClasseNameW.NameLength = (ULONG)wcslen( g_SandboxNameInfo.SClasseNameW.szName );
	ULONG length = g_SandboxNameInfo.SClasseNameW.NameLength + 1;
	g_SandboxNameInfo.SClasseNameA.szName = (LPSTR) kmalloc( length );

	USES_CONVERSION;
	strcpy( g_SandboxNameInfo.SClasseNameA.szName, W2A(g_SandboxNameInfo.SClasseNameW.szName) );
	// 	WideCharToMultiByte( 
	// 		0,0, 
	// 		g_SandboxNameInfo.SClasseNameW.szName, g_SandboxNameInfo.SClasseNameW.NameLength,
	// 		g_SandboxNameInfo.SClasseNameA.szName, length,
	// 		0,0 
	// 		);

	g_SandboxNameInfo.SClasseNameA.szName[ length -1 ] = 0 ;

	//
	// 2. 查询配置文件中的"OpenWinClass",决定是否HOOK 以下函数
	// TODO:{sudami}需调用PB_QueryConf函数; 我们先简化一下儿,后期调试时再修补  - -|
	//

	if ( FALSE == g_bFlag_Hook_OpenWinClass_etc ) { return TRUE; } // 至TRUE表明沙箱中可以直接访问系统的全局设置

	BOOL bRet = FALSE;
	int i = 0, TotalCounts = 0 ;

	// 获取函数原始地址
	LPHOOKINFOLittle pArray = (LPHOOKINFOLittle)g_OpenWinClass_Array;
	TotalCounts = ARRAYSIZEOF( g_OpenWinClass_Array );
	for( i=0; i<TotalCounts; i++ )
	{
		pArray[i].OrignalAddress = (PVOID) GetProcAddress( hModule, pArray[i].FunctionName );
	}

	// 进行Hook
	for( i=0; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[i] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep1() - HookOne(); | \"%s\" \n", pArray[i].FunctionName );
			return FALSE ;
		}
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep2()
{
	BOOL bRet = FALSE ;
	//
	// 1. 查询配置文件"BoxNameTitle",是否需要在运行与沙箱中的标题上加上Sandbox的名字以提示
	// TODO:{sudami} 需调用PB_QueryConf函数; 我们先简化一下儿,后期调试时再修补  - -|
	//

	if ( FALSE == g_SandboxTitle_NO && FALSE == g_SandboxTitle_NO_Ex )
	{
		// 拼接成诸如 "[DefaultBox] " 的格式
		g_SandboxNameInfo.STitleNameA.NameLength = wcslen( g_BoxInfo.BoxName ) + 3;
		g_SandboxNameInfo.STitleNameW.NameLength = 2 * (wcslen( g_BoxInfo.BoxName ) + 3);

		g_SandboxNameInfo.STitleNameW.szName = (LPWSTR) kmalloc( g_SandboxNameInfo.STitleNameW.NameLength );
		swprintf( g_SandboxNameInfo.STitleNameW.szName, L"[%s]  ", g_BoxInfo.BoxName); 

		USES_CONVERSION;
		g_SandboxNameInfo.STitleNameA.szName = (LPSTR) kmalloc( g_SandboxNameInfo.STitleNameW.NameLength );
		strcpy( g_SandboxNameInfo.STitleNameA.szName, W2A(g_SandboxNameInfo.STitleNameW.szName) );
	}

	// 2. 开始HOOK标题相关的函数
	g_offset_jin_Unicode_Length = wcslen(L" [#]");
	g_offset_jin_Ansi_Length = strlen(" [#]");

	bRet = Mhook_SetHook( (PVOID*)&g_GetWindowTextA_addr, fake_GetWindowTextA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep2() - Mhook_SetHook(); | \"GetWindowTextA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_GetWindowTextW_addr, fake_GetWindowTextW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep2() - Mhook_SetHook(); | \"GetWindowTextW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep3()
{
	BOOL bRet = FALSE ;

	bRet = Mhook_SetHook( (PVOID*)&g_ExitWindowsEx_addr, fake_ExitWindowsEx );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"ExitWindowsEx\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_EndTask_addr, fake_EndTask );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"EndTask\" \n" );
		return FALSE ;
	}

	if ( g_bFlag_Hook_OpenWinClass_etc )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_CreateWindowExA_addr, fake_CreateWindowExA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"CreateWindowExA\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_CreateWindowExW_addr, fake_CreateWindowExW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"CreateWindowExW\" \n" );
			return FALSE ;
		}
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DefWindowProcA_addr, fake_DefWindowProcA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"DefWindowProcA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DefWindowProcW_addr, fake_DefWindowProcW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"DefWindowProcW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SetParent_addr, fake_SetParent );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"SetParent\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_MoveWindow_addr, fake_MoveWindow );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"MoveWindow\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SetWindowPos_addr, fake_SetWindowPos );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"SetWindowPos\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_RegisterDeviceNotificationA_addr, fake_RegisterDeviceNotification );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"RegisterDeviceNotificationA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_RegisterDeviceNotificationW_addr, fake_RegisterDeviceNotification );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"RegisterDeviceNotificationW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_UnregisterDeviceNotification_addr, fake_UnregisterDeviceNotification );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep3() - Mhook_SetHook(); | \"UnregisterDeviceNotification\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep4()
{
	BOOL bRet = FALSE ;

	bRet = Mhook_SetHook( (PVOID*)&g_GetShellWindow_addr, fake_GetShellWindow );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"GetShellWindow\" \n" );
		return FALSE ;
	}

	if ( FALSE == g_bFlag_Hook_OpenWinClass_etc ) { return TRUE; }

	bRet = Mhook_SetHook( (PVOID*)&g_EnumWindows_addr, fake_EnumWindows );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"EnumWindows\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_EnumChildWindows_addr, fake_EnumChildWindows );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"EnumChildWindows\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_EnumThreadWindows_addr, fake_EnumThreadWindows );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"EnumThreadWindows\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_EnumDesktopWindows_addr, fake_EnumDesktopWindows );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"EnumDesktopWindows\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_FindWindowA_addr, fake_FindWindowA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"FindWindowA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_FindWindowW_addr, fake_FindWindowW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"FindWindowW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_FindWindowExA_addr, fake_FindWindowExA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"FindWindowExA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_FindWindowExW_addr, fake_FindWindowExW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep4() - Mhook_SetHook(); | \"FindWindowExW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep5()
{
	BOOL bRet = FALSE ;

	HandlerAtoms();

	if ( PB_IsOpenCOM() )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_GetPropA_addr, fake_GetPropA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"GetPropA\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_GetPropW_addr, fake_GetPropW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"GetPropW\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_SetPropA_addr, fake_SetPropA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"SetPropA\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_SetPropW_addr, fake_SetPropW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"SetPropW\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_RemovePropA_addr, fake_RemovePropA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"RemovePropA\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_RemovePropW_addr, fake_RemovePropW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"RemovePropW\" \n" );
			return FALSE ;
		}
	}
	else
	{
		if ( FALSE == g_bFlag_Hook_OpenWinClass_etc ) { return TRUE; }

		bRet = Mhook_SetHook( (PVOID*)&g_GetWindowLongA_addr, fake_GetWindowLongA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"GetWindowLongA\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_GetWindowLongW_addr, fake_GetWindowLongW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"GetWindowLongW\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_SetWindowLongA_addr, fake_SetWindowLongA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"SetWindowLongA\" \n" );
			return FALSE ;
		}

		bRet = Mhook_SetHook( (PVOID*)&g_SetWindowLongW_addr, fake_SetWindowLongW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep5() - Mhook_SetHook(); | \"SetWindowLongW\" \n" );
			return FALSE ;
		}
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep6()
{
	BOOL bRet = FALSE ;

	bRet = Mhook_SetHook( (PVOID*)&g_SendMessageA_addr, fake_SendMessageA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"SendMessageA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SendMessageW_addr, fake_SendMessageW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"SendMessageW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SendMessageTimeoutA_addr, fake_SendMessageTimeoutA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"SendMessageTimeoutA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SendMessageTimeoutW_addr, fake_SendMessageTimeoutW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"SendMessageTimeoutW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SendNotifyMessageA_addr, fake_SendNotifyMessageA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"SendNotifyMessageA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SendNotifyMessageW_addr, fake_SendNotifyMessageW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"SendNotifyMessageW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_PostMessageA_addr, fake_PostMessageA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"PostMessageA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_PostMessageW_addr, fake_PostMessageW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"PostMessageW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DispatchMessageA_addr, fake_DispatchMessageA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"DispatchMessageA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DispatchMessageW_addr, fake_DispatchMessageW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep6() - Mhook_SetHook(); | \"DispatchMessageW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep7()
{
	BOOL bRet = FALSE ;

	InitializeCriticalSection( &g_Lock_Msg );
	ClearStruct( &g_NodeHead_hhk );

	// 1. 先查询配置文件,是否需要做 GlobalHook 的防护

	// TODO:{sudami} 需调用PB_QueryConf函数; 我们先简化一下儿,后期调试时再修补  - -|
	if ( FALSE == g_BlockWinHooks ) { return TRUE; } 

	// 2. 需要限制全局Hook在当前沙箱内
	bRet = Mhook_SetHook( (PVOID*)&g_SetWindowsHookExA_addr, fake_SetWindowsHookExA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep7() - Mhook_SetHook(); | \"SetWindowsHookExA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_SetWindowsHookExW_addr, fake_SetWindowsHookExW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep7() - Mhook_SetHook(); | \"SetWindowsHookExW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_UnhookWindowsHookEx_addr, fake_UnhookWindowsHookEx );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep7() - Mhook_SetHook(); | \"UnhookWindowsHookEx\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase12_user32dll_Windows_dep8()
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_bFlag_Hook_OpenWinClass_etc ) { return TRUE; }

	bRet = Mhook_SetHook( (PVOID*)&g_CreateDialogParamA_addr, fake_CreateDialogParamA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"CreateDialogParamA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateDialogParamW_addr, fake_CreateDialogParamW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"CreateDialogParamW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateDialogIndirectParamA_addr, fake_CreateDialogIndirectParamA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"CreateDialogIndirectParamA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateDialogIndirectParamW_addr, fake_CreateDialogIndirectParamW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"CreateDialogIndirectParamW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateDialogIndirectParamAorW_addr, fake_CreateDialogIndirectParamAorW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"CreateDialogIndirectParamAorW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DialogBoxParamA_addr, fake_DialogBoxParamA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"DialogBoxParamA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DialogBoxParamW_addr, fake_DialogBoxParamW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"DialogBoxParamW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DialogBoxIndirectParamA_addr, fake_DialogBoxIndirectParamA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"DialogBoxIndirectParamA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DialogBoxIndirectParamW_addr, fake_DialogBoxIndirectParamW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"DialogBoxIndirectParamW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_DialogBoxIndirectParamAorW_addr, fake_DialogBoxIndirectParamAorW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase12_user32dll_Windows_dep8() - Mhook_SetHook(); | \"DialogBoxIndirectParamAorW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase12_user32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	bRet = GetUser32dllFuncAddrs(hModule);
	if ( FALSE == bRet ) 
	{ 
		MYTRACE( L"err! | Hook_pharase12_user32dll() - GetUser32dllFuncAddrs(); | " );
		return FALSE; 
	}

	bRet = Hook_pharase12_user32dll_Windows_dep1(hModule);
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep2();
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep3();
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep4();
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep5();
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep6();
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep7();
	if ( FALSE == bRet ) { return FALSE; }

	bRet = Hook_pharase12_user32dll_Windows_dep8();
	if ( FALSE == bRet ) { return FALSE; }

	return TRUE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////