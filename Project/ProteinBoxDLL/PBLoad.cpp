/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/29 [29:12:2010 - 18:42]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBLoad.cpp
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

#include "stdafx.h"
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBDynamicData.h"
#include "PBUser32dll/PBUser32dll.h"
#include "PBLoadData.h"
#include "PBLoad.h"

//////////////////////////////////////////////////////////////////////////

_QueryActCtxSettingsW_	g_QueryActCtxSettingsW_addr = NULL ;

LPWSTR g_InjectDll_FullPath = NULL ;
/*
InjectDll is a sandbox setting in Sandboxie Ini. It tells Sandboxie to "inject" some DLL into every program in the sandbox. "Inject" means the DLL is 

[DefaultBox]
InjectDll=c:\Program Files\Sandboxie Utilities\Sample.dll

You should specify a full path to the DLL. If the DLL file itself resides within the sandbox, specify the full path inside the sandbox. 

*/

#define GetConsoleFunc( X )		g_HookInfo.CONSOLE.pArray[ X##_TAG ].OrignalAddress


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_LdrLoadDll (
    IN PWSTR SearchPath OPTIONAL,
    IN PULONG LoadFlags OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *BaseAddress
    )
{
	LPWSTR pBuffer = NULL ;
	BOOL bIsLocked = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	_LdrLoadDll_ OrignalAddress = (_LdrLoadDll_) GetLoadFunc(LdrLoadDll) ;

	// 2. 预处理
	pBuffer = (LPWSTR) kmalloc( DllName->Length + 2 );
	memcpy( pBuffer, DllName->Buffer, DllName->Length );
	pBuffer[ DllName->Length / sizeof(WCHAR) ] = 0 ;

	Walk_c_windows_winsxs_Total( pBuffer, SearchPath, 1 );

	kfree( pBuffer );

	// 2. 调用原始函数
	status = OrignalAddress( SearchPath, LoadFlags, DllName, BaseAddress );
	if ( status < 0 ) { return status; }

	// 3.
	if ( Hook_pharase9_TotalDynamic() )
	{
		Handler_GetModuleFileNameW_Total( _GetModuleFileName_Flag_Add_, *BaseAddress, NULL, &bIsLocked );

		if ( bIsLocked ) { LeaveCriticalSection( &g_lock_GetModuleFileName ); }
		return status ;
	}

	LdrUnloadDll( *BaseAddress );
	*BaseAddress = NULL ;

	return STATUS_DLL_NOT_FOUND;
}


ULONG WINAPI
fake_LdrUnloadDll (
	IN PVOID BaseAddress
	)
{
	BOOL bIsLocked = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	_LdrUnloadDll_ OrignalAddress = (_LdrUnloadDll_) GetLoadFunc(LdrUnloadDll) ;

	if ( (ULONG)BaseAddress == (ULONG)__ProcessNode->DllBaseAddr.hModule_Self ) { return STATUS_SUCCESS; }
	
	status = OrignalAddress( BaseAddress );
	if ( ! NT_SUCCESS(status) ) { return status; }

	Hook_pharase9_TotalDynamic();

	Handler_GetModuleFileNameW_Total ( _GetModuleFileName_Flag_Del_, BaseAddress, NULL, &bIsLocked );

	if ( bIsLocked ) { LeaveCriticalSection( &g_lock_GetModuleFileName ); }
	return status;
}


ULONG WINAPI
fake_NtLoadDriver (
    IN PUNICODE_STRING DriverServiceName
    )
{
	_NtLoadDriver_ OrignalAddress = (_NtLoadDriver_) GetLoadFunc(NtLoadDriver) ;

	return OrignalAddress( DriverServiceName );
}


ULONG WINAPI
fake_GetModuleFileNameW (
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    )
{
	BOOL bIsLocked = FALSE ;
	ULONG_PTR retAddr = 0, length = 0, NameLength = 0 ;
	LPWSTR lpModulePath = NULL ;
	
	__asm
	{
		mov eax, dword ptr [ebp+4]
		mov retAddr, eax
	}

	if ( retAddr >= 0x80000000 ) { return g_GetModuleFileNameW_addr( hModule, lpFilename, nSize ) ; }

	// 查找与hModule对应的结点,未找到则新建结点
	lpModulePath = Handler_GetModuleFileNameW_Total( _GetModuleFileName_Flag_Find_, (PVOID)hModule, &NameLength, &bIsLocked );
	if ( NULL == lpModulePath || 0 == NameLength ) 
	{
		// 未找到对应@hModule的结点,新建之
		if ( bIsLocked ) { LeaveCriticalSection( &g_lock_GetModuleFileName ); }
		lpModulePath = Handler_GetModuleFileNameW_Total( _GetModuleFileName_Flag_Add_, (PVOID)hModule, &NameLength, &bIsLocked );
	}

	if ( NULL == lpModulePath || 0 == NameLength ) 
	{
		//  建立结点失败,调用原始函数
		if ( bIsLocked ) { LeaveCriticalSection( &g_lock_GetModuleFileName ); }
		return g_GetModuleFileNameW_addr( hModule, lpFilename, nSize ) ;
	}

	++ NameLength ;
	if ( NameLength > nSize )
	{
		NameLength = nSize ;
		nSize = 0x7a ;
	}
	else
	{
		nSize = 0 ;
	}

	memcpy( lpFilename, lpModulePath, 2 * NameLength );
	if ( 0 == nSize ) { -- NameLength; }

	if ( bIsLocked ) { LeaveCriticalSection( &g_lock_GetModuleFileName ); }
	SetLastError(nSize);
	return NameLength ;
}


VOID fake_AddressOfEntryPoint()
{
	UNICODE_STRING DllName ;
	PVOID BaseAddress = NULL ;
	ULONG_PTR addr = 0, OldProtect = 0 ;

	// 1. 恢复被patch的字节
	__asm
	{
		lea     eax, [ebp+4]
		mov     eax, [eax]
		sub     eax, 5
		mov		addr, eax
	}

	memcpy( (PVOID)addr, g_AddressOfEntryPoint_OrigData, 5 );

	VirtualProtect( (LPVOID)addr, 5, g_OldProtect, &OldProtect );

	__asm
	{
		lea     edi, [ebp+4]
		mov		eax, addr 
		mov     [edi], eax
	}

	// 2.
	if ( g_pImportTable_mscoree_dll )
	{
		HandlerManagedCode( L"mscoree.dll", g_pImportTable_mscoree_dll );
		g_pImportTable_mscoree_dll = NULL ;
	}

	if ( g_pImportTable_msvcm80_dll )
	{
		HandlerManagedCode( L"msvcm80.dll", g_pImportTable_msvcm80_dll );
		g_pImportTable_msvcm80_dll = NULL ;
	}

	if ( g_pImportTable_msvcm90_dll )
	{
		HandlerManagedCode( L"msvcm90.dll", g_pImportTable_msvcm90_dll );
		g_pImportTable_msvcm90_dll = NULL ;
	}

	if ( GetConsoleWindow() ) { HandlerConsole(); }

	// 加载配置文件中指定的DLL
	if ( g_InjectDll_FullPath )
	{
		_LdrLoadDll_ OrignalAddress = (_LdrLoadDll_) GetLoadFunc(LdrLoadDll) ;

		RtlInitUnicodeString( &DllName, g_InjectDll_FullPath );
		OrignalAddress( NULL, NULL, &DllName, &BaseAddress );
		
		kfree( g_InjectDll_FullPath );
		g_InjectDll_FullPath = NULL ;
	}

	return;
}


ULONG WINAPI
fake_SetConsoleTitleA (
    LPCSTR lpConsoleTitle
    )
{
	_SetConsoleTitleA_ OrignalAddress = (_SetConsoleTitleA_) GetConsoleFunc(SetConsoleTitleA) ;

	LPSTR lpConsoleTitleNew = RedirectWindowNameA( (LPSTR)lpConsoleTitle );
	ULONG ret = (ULONG)OrignalAddress(lpConsoleTitleNew);

	if ( lpConsoleTitleNew != lpConsoleTitle )
		kfreeExp(lpConsoleTitleNew);

	return ret;
}


ULONG WINAPI
fake_SetConsoleTitleW (
    LPCWSTR lpConsoleTitle
    )
{
	_SetConsoleTitleW_ OrignalAddress = (_SetConsoleTitleW_) GetConsoleFunc(SetConsoleTitleW) ;

	LPWSTR lpConsoleTitleNew = RedirectWindowNameW( (LPWSTR)lpConsoleTitle );
	ULONG ret = (ULONG)OrignalAddress(lpConsoleTitleNew);

	if ( lpConsoleTitleNew != lpConsoleTitle )
		kfreeExp(lpConsoleTitleNew);

	return ret;
}


ULONG WINAPI
fake_GetConsoleTitleA (
    LPSTR lpConsoleTitle,
    DWORD nSize
    )
{
	_GetConsoleTitleA_ OrignalAddress = (_GetConsoleTitleA_) GetConsoleFunc(GetConsoleTitleA) ;
	DWORD nWritten = OrignalAddress( lpConsoleTitle, nSize );

	return GetOrigWindowTitleA( (HWND)_FuckedTag_, lpConsoleTitle, nWritten );
}


ULONG WINAPI
fake_GetConsoleTitleW (
    LPWSTR lpConsoleTitle,
    DWORD nSize
    )
{
	_GetConsoleTitleW_ OrignalAddress = (_GetConsoleTitleW_) GetConsoleFunc(GetConsoleTitleW) ;
	DWORD nWritten = OrignalAddress( lpConsoleTitle, nSize );

	return GetOrigWindowTitleW( (HWND)_FuckedTag_, lpConsoleTitle, nWritten );
}


///////////////////////////////   END OF FILE   ///////////////////////////////