/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/02/01 [1:2:2012 - 16:32]
* MODULE : \Code\Project\ProteinBoxDLL\PBShell32dll\PBShell32dll.cpp
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
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../PBServices.h"
#include "../PBServicesData.h"
#include "../PBCreateProcess.h"
#include "../PBUser32dll/PBUser32dll.h"
#include "PBShell32dll.h"

//////////////////////////////////////////////////////////////////////////

typedef BOOL (WINAPI* _SHGetPathFromIDListW_)( LPCITEMIDLIST, LPWSTR );
_SHGetPathFromIDListW_	g_SHGetPathFromIDListW_addr = NULL ;

typedef BOOL (WINAPI* _ShellExecuteExW_)( LPSHELLEXECUTEINFOW );
_ShellExecuteExW_	g_ShellExecuteExW_addr = NULL ;

typedef ULONG (WINAPI* _SHSetInstanceExplorer_lpVtbl_Release_)( IUnknown * );
_SHSetInstanceExplorer_lpVtbl_Release_	g_SHSetInstanceExplorer_lpVtbl_Release_Orignal = NULL ;

typedef /*void*/ULONG (WINAPI* _SHSetInstanceExplorer_)( IUnknown * );
_SHSetInstanceExplorer_	g_SHSetInstanceExplorer_addr = NULL ;

BOOL g_bFlag_SHSetInstanceExplorer = FALSE ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

LPWSTR GetShellexecuteParameter( LPWSTR szPath, LPWSTR *lpParameters )
{
	ULONG Length = 0, TotalLength = 0 ;
	LPWSTR ptr = NULL, ptr1 = NULL, ptr2 = NULL, ptr3 = NULL ;

	if ( NULL == szPath ) { return NULL; }

	if ( *szPath != '\"' )
	{
		ptr = wcschr( szPath, ' ' );
		if ( ptr ) { *lpParameters = ptr + 1; }
		return szPath;
	}

	ptr1 = szPath + 1 ;
	ptr2 = wcschr( ptr1, '\"' );
	if ( NULL == ptr2 ) { return szPath; }

	ptr3 = ptr2 + 1;
	if ( *ptr3 == ' ' ) { *lpParameters = ptr3 + 1; }

	Length = ((ULONG)((char *)ptr3 - (char *)szPath) >> 1) - 2 ;
	TotalLength = 2 * Length + 8;

	ptr = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, TotalLength );
	if ( NULL == ptr ) { return szPath; }

	memcpy( ptr, ptr1, 2 * Length );
	*((WORD *)ptr + Length) = 0 ;
	return ptr;
}


BOOL RunDirectory ( LPWSTR pszPath, BOOLEAN bIsExplore )
{
	BOOL bRet = FALSE ;
	STARTUPINFOW StartupInfo ;
	PROCESS_INFORMATION ProcInfo ;
	WCHAR Windir[ MAX_PATH ] = L"" ;
	WCHAR CommandLine[ MAX_PATH ] = L"" ;
	LPWSTR ptr = NULL ;

	memset( &StartupInfo, 0, sizeof(STARTUPINFOW) );
	StartupInfo.cb      = 0x44 ;
	StartupInfo.dwFlags = 0x80 ;

	GetSystemWindowsDirectoryW( Windir, MAX_PATH );
	wcscat( CommandLine, L"\"" );
	wcscat( CommandLine, Windir );
	wcscat( CommandLine, L"\\explorer.exe\" " );
	if ( bIsExplore ) { wcscat( CommandLine, L"/e,"); }

	wcscat( CommandLine, L"\"" );
	wcscat( CommandLine, pszPath );

	ptr = &CommandLine[ wcslen(CommandLine) ];
	if ( '\\' == *ptr ) { *ptr = 0 ; }

	wcscat( CommandLine, L"\"" );

	//
	// 测试时代码形如ShellExecuteW( NULL, L"explore", L"D:\\WinRAR", NULL, NULL, SW_SHOWNORMAL );
	// 则最终的拼接结果为: ""C:\WINDOWS\explorer.exe" /e,"D:\WinRAR""
	//

	bRet = CreateProcessW( 0, CommandLine, 0, 0, 0, 0, 0, 0, &StartupInfo, &ProcInfo );
	if ( FALSE == bRet ) { return FALSE; }

	CloseHandle( ProcInfo.hThread );
	CloseHandle( ProcInfo.hProcess );

	return TRUE;
}


BOOL CALLBACK EnumFunc( HWND hWnd, LPARAM lParam )
{
	DWORD   dwProcessId = 0 ;
	if ( g_IsWindowVisible_addr(hWnd) )
	{
		g_GetWindowThreadProcessId_addr( hWnd, &dwProcessId );
		if ( dwProcessId == GetCurrentProcessId() )
		{ 
			++ *(DWORD *)lParam ; 
		}
	}

	return TRUE;
}


ULONG WINAPI fake_SHSetInstanceExplorer_lpVtbl_Release( IUnknown *punk )
{
	ULONG Ret = 0 ;
	LPARAM lParam = NULL ; 

	if ( (ULONG)punk == (ULONG)_FuckedTag_ )
	{
		Sleep( 6000 );

		fake_EnumWindows( (WNDENUMPROC)EnumFunc, lParam );
		if ( NULL == lParam ) { ExitProcess(0); }

		return 0;
	}
	else
	{
		Ret = g_SHSetInstanceExplorer_lpVtbl_Release_Orignal( punk );
		if ( Ret )
		{
			fake_EnumWindows( (WNDENUMPROC)EnumFunc, (LPARAM)&lParam );
			if ( lParam )
			{
				g_bFlag_SHSetInstanceExplorer = TRUE ;
				return Ret;
			}

			if ( g_bFlag_SHSetInstanceExplorer )
			{
				QueueUserWorkItem( (LPTHREAD_START_ROUTINE)fake_SHSetInstanceExplorer_lpVtbl_Release, (PVOID)_FuckedTag_, 0x10 );
			}
		}
	}

	return Ret ;
}


ULONG WINAPI fake_SHSetInstanceExplorer( /*IUnknown*/int **punk )
{
	ULONG oldProtect = 0 ;
	if ( punk )
	{
		if ( (ULONG)(*punk)[2]/*punk->lpVtbl->Release*/ != (ULONG)fake_SHSetInstanceExplorer_lpVtbl_Release )
		{
			g_SHSetInstanceExplorer_lpVtbl_Release_Orignal = (_SHSetInstanceExplorer_lpVtbl_Release_)(*punk)[2] /*punk->lpVtbl->Release*/ ;

			VirtualProtect( (LPVOID)*punk /*punk->lpVtbl*/, 0xC, PAGE_READWRITE, &oldProtect );

			(*punk)[2] = (ULONG)fake_SHSetInstanceExplorer_lpVtbl_Release ;

			VirtualProtect( (LPVOID)*punk /*punk->lpVtbl*/, 0xC, oldProtect, &oldProtect );
		}
	}

	return g_SHSetInstanceExplorer_addr( (IUnknown*)punk );
}


ULONG WINAPI fake_ShellExecuteExW( LPSHELLEXECUTEINFOW sei )
{
	BOOL bRet = FALSE ;
	ULONG ErrorCode = ERROR_SUCCESS ;
	BOOL bIsExplore = FALSE, bFlag = FALSE, bIsEXE = TRUE ;
	LPCWSTR lpVerb = NULL ;
	LPWSTR lpFile = NULL, lpParameters = NULL ;
	SHELLEXECUTEINFOW SeiDummy ;

	lpVerb = sei->lpVerb ;
	bIsExplore = (lpVerb && 0 == wcsicmp(lpVerb, L"explore")) ;

	if ( NULL == lpVerb || NULL == *lpVerb || bIsExplore || 0 == wcsicmp(lpVerb, L"open") )
	{
		if ( sei->fMask & SEE_MASK_IDLIST )
		{
			lpFile = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, 0x218 );
			if ( FALSE == g_SHGetPathFromIDListW_addr( (LPCITEMIDLIST)sei->lpIDList, lpFile ) ) { *lpFile = 0; }
			bFlag = TRUE ;
		}
		else
		{
			lpFile = GetShellexecuteParameter( (LPWSTR)sei->lpFile, &lpParameters );
			if ( lpFile != sei->lpFile ) { bFlag = TRUE ; }
		}

		if ( PB_IsDirectory( lpFile ) ) { bIsEXE = FALSE; }
	}

	if ( FALSE == bIsEXE )
	{
		bRet = RunDirectory( lpFile, bIsExplore );
		if ( FALSE == bRet ) { ErrorCode = ERROR_FILE_NOT_FOUND; }
	}
	else
	{
		bRet = g_ShellExecuteExW_addr( sei );
		if ( FALSE == bRet  )
		{
			ErrorCode = GetLastError();
			if ( ERROR_FILE_NOT_FOUND == ErrorCode )
			{
				if ( FALSE == bFlag )
				{
					SetLastError(ErrorCode);
					return 0;
				}

				if ( lpParameters && NULL == sei->lpParameters )
				{
					memcpy( &SeiDummy, sei, sizeof(SHELLEXECUTEINFOW) );
					SeiDummy.lpParameters = lpParameters;
					SeiDummy.lpFile = lpFile;

					bRet = g_ShellExecuteExW_addr( &SeiDummy );
					if ( FALSE == bRet ) { ErrorCode = GetLastError(); }

					sei->hInstApp = SeiDummy.hInstApp ;
					sei->hProcess = SeiDummy.hProcess ;	
				}
			}
		}
	}

	if ( bFlag ) { HeapFree( GetProcessHeap(), 0, lpFile ); }
	SetLastError( ErrorCode );
	return bRet;
}


ULONG WINAPI fake_RouteTheCall( int a1, int a2, LPSTR szName )
{
	LPWSTR ptr = NULL, lpName = NULL ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	uniBuffer.Buffer = 0 ;
	RtlInitString( &ansiBuffer, szName );
	RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );
	lpName = uniBuffer.Buffer ;

	for ( ptr = uniBuffer.Buffer; *lpName; ++lpName )
	{
		if ( *lpName != '\"' )
		{
			*ptr = *lpName;
			++ ptr ;
		}
	}

	*ptr = 0 ;
	RunDirectory( uniBuffer.Buffer, FALSE );

	ExitProcess( 0 );
	return 0;
}


VOID Hook_pharase10_shell32dll_dep()
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniBuffer ;
	ULONG cbSize = 0 ;
	HANDLE hRootKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	KEY_VALUE_PARTIAL_INFORMATION KeyValueInfo ;

	if ( FALSE == g_bIs_Inside_Explorer_exe ) { return; }

	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &uniBuffer, L"\\registry\\machine\\software\\microsoft\\Windows NT\\CurrentVersion\\Winlogon" );

	status = ZwOpenKey( &hRootKey, KEY_READ, &ObjAtr );
	if ( ! NT_SUCCESS(status) ) { ExitProcess( 0xFFFFFFFF ); }

	RtlInitUnicodeString( &uniBuffer, L"Shell" );
	status = ZwQueryValueKey( hRootKey, &uniBuffer, KeyValuePartialInformation, &KeyValueInfo, 0x100, &cbSize );

	if ( status >= 0 )
	{
		if ( KeyValueInfo.DataLength != 4 || KeyValueInfo.Data[0] != 'x' || KeyValueInfo.Data[1] )
			status = STATUS_BAD_INITIAL_PC;
		else
			status = STATUS_SUCCESS;
	}

	ZwClose( hRootKey );
	if ( ! NT_SUCCESS(status) ) { ExitProcess( 0xFFFFFFFF ); }

	PB_StartCOM();;
}


BOOL WINAPI Hook_pharase10_shell32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_ShellExecuteExW_addr = (_ShellExecuteExW_) GetProcAddress( hModule, "ShellExecuteExW" );
	g_SHSetInstanceExplorer_addr = (_SHSetInstanceExplorer_) GetProcAddress( hModule, "SHSetInstanceExplorer" );

	if ( g_ShellExecuteExW_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_ShellExecuteExW_addr, fake_ShellExecuteExW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase10_shell32dll() - Mhook_SetHook(); | \"ShellExecuteExW\" \n" );
			return FALSE ;
		}
	}

	if ( g_SHSetInstanceExplorer_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_SHSetInstanceExplorer_addr, fake_SHSetInstanceExplorer );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase10_shell32dll() - Mhook_SetHook(); | \"SHSetInstanceExplorer\" \n" );
			return FALSE ;
		}
	}

	g_SHGetPathFromIDListW_addr = (_SHGetPathFromIDListW_) GetProcAddress( hModule, "SHGetPathFromIDListW" );

	Hook_pharase10_shell32dll_dep();
	return TRUE;
}


BOOL WINAPI Hook_pharase11_zipfldrdll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	ULONG RouteTheCall_addr = 0 ;

	if ( wcsicmp(g_BoxInfo.ProcessName, L"rundll32.exe") ) { return TRUE; }

	// 处理 rundll32.exe 进程的 RouteTheCall 函数
	RouteTheCall_addr = (ULONG) GetProcAddress( hModule, "RouteTheCall" );
	if ( 0 == RouteTheCall_addr ) { return TRUE; }

	// Hook
	bRet = Mhook_SetHook( (PVOID*)&RouteTheCall_addr, fake_RouteTheCall );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase11_zipfldrdll() - Mhook_SetHook(); | \"RouteTheCall\" \n" );
		return FALSE ;
	}

	return TRUE;
}

///////////////////////////////   END OF FILE   ///////////////////////////////