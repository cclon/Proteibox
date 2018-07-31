/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/01/17 [17:1:2011 - 14:40]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBDynamicData.cpp
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
#include "MemoryManager.h"
#include "PBFilesData.h"
#include "PBLoadData.h"
#include "PBToken.h"
#include "PBServicesData.h"
#include "PBComData.h"
#include "PBServices.h"
#include "PBCreateProcess.h"
#include "PBUser32dll/PBUser32dll.h"
#include "PBAdvapi32dll/PBAdvapi32dll.h"
#include "PBDynamicData.h"

#pragma warning(disable : 4995) 

//////////////////////////////////////////////////////////////////////////

_CoTaskMemAlloc_ g_CoTaskMemAlloc_addr = NULL ;
_LsaConnectUntrusted_ g_LsaConnectUntrusted_addr = NULL ;
_LsaRegisterLogonProcess_ g_LsaRegisterLogonProcess_addr = NULL ;
_VerifyCatalogFile_ g_VerifyCatalogFile_addr = NULL ;
_SxsInstallW_ g_SxsInstallW_addr = NULL ;
_GdiAddFontResourceW_ g_GdiAddFontResourceW_addr = NULL ;
_CreateScalableFontResourceW_ g_CreateScalableFontResourceW_addr = NULL ;
_NtQuerySystemInformation_ g_NtQuerySystemInformation_addr = NULL ;
_CreateActCtxA_ g_CreateActCtxA_addr = NULL ;
_CreateActCtxW_ g_CreateActCtxW_addr = NULL ;

ATOM g_Atom_OleDropTargetInterface = 0 ;
ATOM g_Atom_OleDropTargetInterface_Sandbox = 0 ;
ATOM g_Atom_OleDropTargetMarshalHwnd = 0 ;
ATOM g_Atom_OleDropTargetMarshalHwnd_Sandbox = 0 ;
ATOM g_Atom_OleEndPointID = 0 ;
ATOM g_Atom_OleEndPointID_Sandbox = 0 ;
ATOM g_Atom_SBIE_DropTarget = 0 ;
ATOM g_Atom_SBIE_WindowProcOldW = 0 ;
ATOM g_Atom_SBIE_WindowProcOldA = 0 ;
ATOM g_Atom_SBIE_WindowProcNewW = 0 ;
ATOM g_Atom_SBIE_WindowProcNewA = 0 ;

BOOL g_bFlag_Hook_OpenWinClass_etc = TRUE ; // 默认要过滤创建窗口等系列的函数，除非配置文件明确指定不需要。
/*

OpenWinClass is a sandbox setting in Sandboxie Ini. It specifies the class names for unsandboxed windows that should be accessible by a sandboxed program. 

Examples: 
   [DefaultBox]
   OpenWinClass=ConsoleWindowClass
   OpenWinClass=$:program.exe
   OpenWinClass=#
   OpenWinClass=*

The first example makes console windows created by the cmd.exe process accessible to sandboxed programs. 
Normally, Sandboxie will not permit a sandboxed program to access, communicate, close or destroy a window outside the sandbox. The OpenWinClass settings makes an exception to this rule, and allows specific unsandboxed windows to be accessible. 

Special Forms 
   OpenWinClass=$:program.exe

Permits a program running inside the sandbox to use the PostThreadMessage API to send a message directly to a thread in a target process running outside the sandbox. This form of the OpenWinClass setting does not support wildcards, so the process name of the target process must match the name specified in the setting. 
   OpenWinClass=#

This setting tells Sandboxie to not alter window class names created by sandboxed programs. Normally, Sandboxie translates class names such as IEFrame to Sandbox:DefaultBox::IEFrame in order to better separate windows that belong to sandboxed programs from the rest of the windows in the system. 
However, in some cases, a program outside the sandbox might expect window class names to have a specific name, and therefore might not recognize the windows created by a sandboxed program. Specifying OpenWinClass=# resolves this problem, at the cost of a lesser degree of separation. 
Note that OpenWinClass=# does not allow communication with any windows outside the sandbox. 

   OpenWinClass=*

This setting tells Sandboxie to not translate window class names as described above, and also makes all windows in the system accessible to sandboxed programs, and goes a step further to disable a few other windowing-related Sandboxie functions. This may also cause the Sandboxie indicator [#] to not appear in window titles. 
Note that OpenWinClass=* allows full communication with all windows outside the sandbox. 

*/


// sechost.dll中待Hook的函数
static const LPSTR g_pharase19_sechostdll_Arrays[ ] =
{
	"ChangeServiceConfigA",
	"ChangeServiceConfigW",
	"ChangeServiceConfig2A",
	"ChangeServiceConfig2W",
	"CloseServiceHandle",
	"ControlService",
	"CreateServiceA",
	"CreateServiceW",
	"DeleteService",
	"OpenSCManagerA",
	"OpenSCManagerW",
	"OpenServiceA",
	"OpenServiceW",
	"QueryServiceConfigA",
	"QueryServiceConfigW",
	"QueryServiceConfig2A",
	"QueryServiceConfig2W",
	"QueryServiceObjectSecurity",
	"QueryServiceStatus",
	"QueryServiceStatusEx",
	"RegisterServiceCtrlHandlerA",
	"RegisterServiceCtrlHandlerW",
	"RegisterServiceCtrlHandlerExA",
	"RegisterServiceCtrlHandlerExW",
	"SetServiceObjectSecurity",
	"NotifyServiceStatusChange",
	"SetServiceStatus",
	"StartServiceA",
	"StartServiceW",
	"StartServiceCtrlDispatcherA",
	"StartServiceCtrlDispatcherW",
};


BOOL WINAPI Hook_pharase7_secur32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_LsaConnectUntrusted_addr     = (_LsaConnectUntrusted_)     GetProcAddress( hModule, "LsaConnectUntrusted" );
	g_LsaRegisterLogonProcess_addr = (_LsaRegisterLogonProcess_) GetProcAddress( hModule, "LsaRegisterLogonProcess" );

	bRet = Mhook_SetHook( (PVOID*)&g_LsaRegisterLogonProcess_addr, fake_LsaRegisterLogonProcess );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase7_secur32dll() - Mhook_SetHook(); | \"LsaRegisterLogonProcess\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase13_sfc_osdll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	DWORD SfcGetNextProtectedFile_addr = 0, SfcIsFileProtected_addr = 0 ;

	SfcIsFileProtected_addr = (DWORD) GetProcAddress( hModule, "SfcIsFileProtected" );
	SfcGetNextProtectedFile_addr = (DWORD) GetProcAddress( hModule, "SfcGetNextProtectedFile" );

	bRet = Mhook_SetHook( (PVOID*)&SfcIsFileProtected_addr, fake_SfcIsFileProtected );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase13_sfc_osdll() - Mhook_SetHook(); | \"SfcIsFileProtected\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&SfcGetNextProtectedFile_addr, fake_SfcGetNextProtectedFile );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase13_sfc_osdll() - Mhook_SetHook(); | \"SfcGetNextProtectedFile\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase16_wevtapidll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	DWORD EvtIntAssertConfig_addr = 0 ;

	EvtIntAssertConfig_addr = (DWORD) GetProcAddress( hModule, "EvtIntAssertConfig" );

	bRet = Mhook_SetHook( (PVOID*)&EvtIntAssertConfig_addr, fake_EvtIntAssertConfig );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase16_wevtapidll() - Mhook_SetHook(); | \"EvtIntAssertConfig\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase17_sxsdll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_SxsInstallW_addr = (_SxsInstallW_) GetProcAddress( hModule, "SxsInstallW" );

	bRet = Mhook_SetHook( (PVOID*)&g_SxsInstallW_addr, fake_SxsInstallW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase17_sxsdll() - Mhook_SetHook(); | \"SxsInstallW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase18_gdi32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_GdiAddFontResourceW_addr = (_GdiAddFontResourceW_) GetProcAddress( hModule, "GdiAddFontResourceW" );
	g_CreateScalableFontResourceW_addr = (_CreateScalableFontResourceW_) GetProcAddress( hModule, "CreateScalableFontResourceW" );

	bRet = Mhook_SetHook( (PVOID*)&g_GdiAddFontResourceW_addr, fake_GdiAddFontResourceW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase18_gdi32dll() - Mhook_SetHook(); | \"GdiAddFontResourceW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateScalableFontResourceW_addr, fake_CreateScalableFontResourceW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase18_gdi32dll() - Mhook_SetHook(); | \"CreateScalableFontResourceW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase19_sechostdll( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	DWORD OrignalAddr = 0, FakeAddr = 0 ;
	HMODULE hModule_advapi32_dll = NULL ;

	hModule_advapi32_dll = Get_advapi32dll_Handle();
	if ( NULL == hModule_advapi32_dll ) { return FALSE; }

	if ( NULL == g_pharase19_sechostdll_Arrays[0] ) { return TRUE; }

	for (int Index = 0; Index < ARRAYSIZEOF(g_pharase19_sechostdll_Arrays); Index++ )
	{
		OrignalAddr = (DWORD) GetProcAddress( hModule, g_pharase19_sechostdll_Arrays[Index] );
		FakeAddr	= (DWORD) GetProcAddress( hModule_advapi32_dll, g_pharase19_sechostdll_Arrays[Index] );
		if ( NULL == OrignalAddr || NULL == FakeAddr ) { break; }

		if ( FALSE == Mhook_SetHook((PVOID*)&OrignalAddr, (PVOID)FakeAddr) )
		{
			MYTRACE( L"error! | Hook_pharase19_sechostdll() - Mhook_SetHook(); | \"%s\" \n", g_pharase19_sechostdll_Arrays[Index] );
			return FALSE ;
		}
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase20_MsgPlusLivedll( IN HMODULE hModule )
{
	int Data = 0 ;
	OBJECT_ATTRIBUTES ObjAtr = {} ;
	UNICODE_STRING KeyName = {} ;
	HANDLE KeyHandle = NULL ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &KeyName, L"\\registry\\user\\current\\software\\Patchou\\Messenger Plus! Live\\GlobalSettings" );

	if ( Stub_NtOpenKey( &ObjAtr, &KeyHandle, KEY_SET_VALUE ) >= 0 )
	{
		RtlInitUnicodeString( &KeyName, L"SafeHook" );
		ZwSetValueKey( KeyHandle, &KeyName, 0, REG_DWORD, &Data, 4 );
		ZwClose( KeyHandle );
	}

	return TRUE;
}


ULONG WINAPI fake_LsaRegisterLogonProcess( PLSA_STRING LsaLogonProcessName, PHANDLE Handle, PLSA_OPERATIONAL_MODE OperationalMode )
{
	NTSTATUS status = STATUS_SUCCESS ;

	status = g_LsaRegisterLogonProcess_addr( LsaLogonProcessName, Handle, OperationalMode );
	if ( ! NT_SUCCESS(status) ) { status = g_LsaConnectUntrusted_addr( Handle ); }

	return status;
}


ULONG WINAPI fake_SfcIsFileProtected(int a1, int a2)
{
	SetLastError( ERROR_FILE_NOT_FOUND );
	return 0;
}

ULONG WINAPI fake_SfcGetNextProtectedFile(int a1, int a2)
{
	SetLastError( ERROR_NO_MORE_FILES );
	return 0;
}


ULONG WINAPI fake_EvtIntAssertConfig(int a1, int a2, int a3)
{
	SetLastError( NO_ERROR );
	return 1;
}


ULONG WINAPI fake_SxsInstallW(int a1)
{
	ULONG ret = 0 ;

	ret = g_SxsInstallW_addr( a1 );
	if ( 0 == ret )
	{
		SetLastError(NO_ERROR);
		ret = 1;
	}

	return ret;
}


LPWSTR GetRedirectedFontPath( IN LPCWSTR lpszFont )
{
	ULONG length = 0, Size = 0 ;
	LPWSTR lpFilePath = NULL ;
	HANDLE hFile = NULL ;
	BOOL bIsHandlerSelfFilePath = FALSE ;

	// 1. 拷贝@lpszFont的内容到新内存中
	lpFilePath = (LPWSTR) kmalloc( 0x2000 );
	length = wcslen(lpszFont);
	if ( length > 0xFFF ) { length = 0xFFF ; }

	Size = 2 * length ;
	memcpy( lpFilePath, lpszFont, Size );
	lpFilePath[ length ] = UNICODE_NULL ;

	// 2. 以只读方式打开该文件,获得句柄
	hFile = CreateFileW( lpFilePath, GENERIC_READ, 7, 0, OPEN_EXISTING, 0, 0 );
	if ( hFile == (HANDLE)INVALID_HANDLE_VALUE ) { return lpFilePath; }

	// 3. 获得也许是重定向后的文件全路径
	if ( PB_GetHandlePath(hFile, lpFilePath, &bIsHandlerSelfFilePath) < 0 || (FALSE == PB_TranslateNtToDosPath(lpFilePath)) )
	{
		memcpy( lpFilePath, lpszFont, Size );
		lpFilePath[ length ] = UNICODE_NULL ;
	}

	CloseHandle( hFile );

	// 4. 返回得到的文件路径(也许经过重定向)
	return lpFilePath;
}


ULONG WINAPI fake_GdiAddFontResourceW( LPCWSTR pwszFileName, int dwFlags, DESIGNVECTOR *pdv )
{
	LPWSTR pwszFileName_new = NULL ;
	ULONG retval = 0 ;
	DWORD dwErrorCode = 0 ;

	pwszFileName_new = GetRedirectedFontPath( pwszFileName );
	retval = g_GdiAddFontResourceW_addr( pwszFileName_new, dwFlags | 0x10, pdv );

	dwErrorCode = GetLastError();
	kfree( pwszFileName_new );
	SetLastError( dwErrorCode );
	return retval;
}


ULONG WINAPI fake_CreateScalableFontResourceW(DWORD fdwHidden, LPCWSTR lpszFontRes, LPCWSTR lpszFontFile, LPCWSTR lpszCurrentPath)
{
	ULONG ret = 0 ;
	DWORD dwErrorCode = 0 ;
	LPCWSTR lpszFontRes_old = NULL, lpszFontFile_new = NULL, lpszFontRes_new = NULL ;

	lpszFontRes_old = lpszFontRes ;
	if ( lpszCurrentPath )
	{
		lpszFontFile_new = lpszFontFile;
		lpszFontRes_new = lpszFontRes;
	}
	else
	{
		lpszFontRes_new  = GetRedirectedFontPath( lpszFontRes  );
		lpszFontFile_new = GetRedirectedFontPath( lpszFontFile );
	}

	ret = g_CreateScalableFontResourceW_addr( 1, lpszFontRes_new, lpszFontFile_new, lpszCurrentPath );
	dwErrorCode = GetLastError();

	if ( lpszFontRes_new != lpszFontRes_old )
		kfree( (PVOID)lpszFontRes_new );

	if ( lpszFontFile_new != lpszFontFile )
		kfree( (PVOID)lpszFontFile_new );

	SetLastError( dwErrorCode );
	return ret;
}


ULONG WINAPI fake_SetLocaleInfo(int a1, int a2, int a3)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return 0;
}


VOID NtQuerySystemInformation_SystemProcessInformation_Filter( PSYSTEM_PROCESS_INFORMATION ProcessInfo )
{
	BOOL bRet = FALSE ;
	PB_BOX_INFO BoxInfo ;
	ULONG NextEntryOffset = 0 ;
	PSYSTEM_PROCESS_INFORMATION pCurrent = NULL, pTemp = ProcessInfo ;

	pCurrent = (PSYSTEM_PROCESS_INFORMATION)((char *)ProcessInfo + ProcessInfo->Next);
	if ( pCurrent == ProcessInfo ) { return; }

	do 
	{
		bRet = PB_QueryProcess( (ULONG)pCurrent->ProcessID, &BoxInfo );
		if ( bRet && 0 == wcsicmp( BoxInfo.BoxName, g_BoxInfo.BoxName ) )
		{
			NextEntryOffset = pCurrent->Next ;

			if ( NextEntryOffset )
				pTemp->Next += NextEntryOffset ;
			else
				pTemp->Next = 0 ;
		} 
		else
		{
			pTemp = pCurrent;
		}

		pCurrent = (PSYSTEM_PROCESS_INFORMATION)((char *)pTemp + pTemp->Next);

	} while ( pCurrent != pTemp );

	return;
}


ULONG WINAPI 
fake_NtQuerySystemInformation (
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation, 
	ULONG SystemInformationLength, 
	PULONG ReturnLength
	)
{
	NTSTATUS status = STATUS_SUCCESS ;

	status = g_NtQuerySystemInformation_addr( SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength );
	if ( ! NT_SUCCESS(status) ) { return status; }
	
	//
	// 查询进程时,跳过沙箱中的进程
	//

	if ( SystemProcessInformation == SystemInformationClass )
	{
		NtQuerySystemInformation_SystemProcessInformation_Filter( (PSYSTEM_PROCESS_INFORMATION)SystemInformation );
	}

	return status;
}


ULONG WINAPI fake_NtCreateTransaction(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
	*(DWORD *)a1 = 0;
	return 0;
}


ULONG WINAPI fake_NtOpenTransaction(int a1, int a2, int a3, int a4, int a5)
{
	*(DWORD *)a1 = 0;
	return 0;
}


void CreateActCtx_Filter_exp( LPWSTR lpFileName, HANDLE hModule )
{
	DWORD flOldProtect = 0 ;
	int FuckedLength = 0, temp = 0 ;
	LPWSTR FuckedBuffer = NULL, Buffer = NULL ;

	sub_7D2523A0( (PIMAGE_DOS_HEADER)hModule, &FuckedBuffer, &FuckedLength, &temp );
	if ( NULL == FuckedBuffer ) { return; }

	Buffer = (LPWSTR) Handler_Read_manifest( lpFileName, FuckedLength );
	if ( NULL == Buffer ) { return; }

	VirtualProtect( (LPVOID)FuckedBuffer, FuckedLength, PAGE_READWRITE, &flOldProtect );

	memcpy( FuckedBuffer, Buffer, FuckedLength );

	VirtualProtect( (LPVOID)FuckedBuffer, FuckedLength, flOldProtect, &flOldProtect );

	kfree( Buffer );
	return;
}


HANDLE CreateActCtx_Filter_ex( PVOID pActCtx, BOOL bIsUnicode )
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL, hModule = NULL ;
	LPWSTR szPath = NULL, lpSourceOld = NULL ;
	PVOID lpFileName = NULL ;
	DWORD dwErrorCode = 0 ;
	ANSI_STRING ansiBuffer;
	BOOL bIsHandlerSelfFilePath = FALSE ;
	PACTCTXW pActCtxW = (PACTCTXW)pActCtx ;
	PACTCTXA pActCtxA = (PACTCTXA)pActCtx ;

	// 1. 通过参数得到待操作的文件路径,若路径为空,放行之; 若文件存在则获取对应的文件句柄(可能被重定向)
	lpFileName = (PVOID)pActCtxW->lpSource ;
	if ( NULL == lpFileName )
	{
		return g_CreateActCtxW_addr( pActCtxW );
	}

	if ( bIsUnicode )
		hFile = CreateFileW( (LPCWSTR)lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
	else
		hFile = CreateFileA( (LPCSTR)lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );

	// 打开文件失败则走原函数流程,放行之
	if ( hFile == (HANDLE)INVALID_HANDLE_VALUE )
	{
		if ( bIsUnicode )
			hModule = (HANDLE)g_CreateActCtxW_addr( pActCtxW );
		else
			hModule = (HANDLE)g_CreateActCtxA_addr( pActCtxA );

		return hModule;
	}

	// 2. 通过文件句柄获取对应的原始路径 & 重定向路径 (不一定能获取到)
	szPath = (LPWSTR) kmalloc( 0x4000 );
	status = PB_GetHandlePath( hFile, szPath, &bIsHandlerSelfFilePath );

	CloseHandle( hFile );
	if ( ! NT_SUCCESS(status) )
	{
		kfree( szPath );
		SetLastError( ERROR_PATH_NOT_FOUND );
		return (HANDLE)INVALID_HANDLE_VALUE;
	}

	// 3. 若未获取到原始路径,放行之
	if ( FALSE == bIsHandlerSelfFilePath )
	{
		kfree( szPath );

		if ( bIsUnicode )
			hModule = (HANDLE)g_CreateActCtxW_addr( pActCtxW );
		else
			hModule = (HANDLE)g_CreateActCtxA_addr( pActCtxA );

		return hModule;
	}

	// 4. 进行关键的一次过滤操作
	if ( g_QueryActCtxSettingsW_addr && LOBYTE(pActCtxW->dwFlags) < 0 )
	{
		CreateActCtx_Filter_exp( szPath, pActCtxW->hModule );
	}

	// 5. 将过滤函数得到的结果(szPath)填充终pActCtx结构中,调用原始函数
	if ( FALSE == PB_TranslateNtToDosPath(szPath) )
	{
		kfree( szPath );
		SetLastError( ERROR_PATH_NOT_FOUND );
		return (HANDLE)INVALID_HANDLE_VALUE;
	}

	lpSourceOld = (LPWSTR)pActCtxW->lpSource ;

	if ( bIsUnicode )
	{
		pActCtxW->lpSource = szPath ;
		hModule = g_CreateActCtxW_addr( pActCtxW );
	}
	else
	{
		UNICODE_STRING uniBuffer ;
		ANSI_STRING ansiBuffer ;

		RtlInitUnicodeString( &uniBuffer, szPath );
		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, TRUE );

		pActCtxA->lpSource = (LPCSTR)ansiBuffer.Buffer;
		hModule = g_CreateActCtxA_addr( pActCtxA );
	}

	dwErrorCode = GetLastError();
	pActCtxW->lpSource = lpSourceOld ;

	if ( FALSE == bIsUnicode ) { RtlFreeAnsiString( &ansiBuffer ); }
	kfree(szPath);
	SetLastError(dwErrorCode);
	return hModule ;
}


ULONG CreateActCtx_Filter( PVOID pActCtx, BOOL bIsUnicode )
{
	DWORD dwErrorCode;
	LPWSTR lpName = NULL ;
	HANDLE hModule = NULL ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;
	PACTCTXW pActCtxW = (PACTCTXW)pActCtx ;
	PACTCTXA pActCtxA = (PACTCTXA)pActCtx ;

	if ( pActCtxW->dwFlags & 4 )
	{
		return (ULONG)CreateActCtx_Filter_ex( pActCtx, bIsUnicode );
	}
	else
	{
		lpName = Handler_winsxs_lastupdatetime( TRUE );
		if ( NULL == lpName ) { return (ULONG)CreateActCtx_Filter_ex( pActCtx, bIsUnicode ); }

		pActCtxW->dwFlags |= 4 ;

		if ( bIsUnicode )
		{
			pActCtxW->lpAssemblyDirectory = lpName ;

			hModule = CreateActCtx_Filter_ex( pActCtx, TRUE );
			dwErrorCode = GetLastError();
		}
		else
		{
			RtlInitUnicodeString( &uniBuffer, lpName );
			RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, TRUE );

			pActCtxA->lpAssemblyDirectory = (LPCSTR)ansiBuffer.Buffer ;

			hModule = CreateActCtx_Filter_ex( pActCtx, FALSE );
			dwErrorCode = GetLastError();
			RtlFreeAnsiString( &ansiBuffer );
		}

		pActCtxW->dwFlags &= 0xFFFFFFFB ;
		pActCtxW->lpAssemblyDirectory = NULL ;

		SetLastError( dwErrorCode );
	}

	return (ULONG)hModule ;
}


ULONG WINAPI fake_CreateActCtxA( PCACTCTXA pActCtx )
{
	return CreateActCtx_Filter( (PVOID)pActCtx, FALSE );
}


ULONG WINAPI fake_CreateActCtxW( PCACTCTXW pActCtx )
{
	return CreateActCtx_Filter( (PVOID)pActCtx, TRUE );
}


NTSTATUS Stub_NtOpenKey( POBJECT_ATTRIBUTES ObjAtr, PHANDLE pKeyHandle, ULONG DesireAccess )
{
	ULONG ResultLength = 0x1000 ;
	LPWSTR Name = NULL, ptr = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	PKEY_NAME_INFORMATION pBuffer = NULL ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;

	// 1. 若存在父键句柄,则拼接出全路径KeyPath,否则直接用
	if ( ObjAtr->RootDirectory )
	{

		pBuffer = (PKEY_NAME_INFORMATION) kmalloc( 0x1000 );
		status = ZwQueryKey( ObjAtr->RootDirectory, KeyNameInformation, pBuffer, 0x1000, &ResultLength );
		if ( ! NT_SUCCESS(status) )
		{
			kfree( pBuffer );
			return status;
		}

		ptr = &pBuffer->Name[ pBuffer->NameLength / sizeof(WCHAR) ];
		*ptr = '\\';
		wcscpy( ptr + 1, ObjAtr->ObjectName->Buffer );

		Name = pBuffer->Name;
	}
	else
	{
		Name = ObjAtr->ObjectName->Buffer ;
	}

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, Name, &bIsWhite, &bIsBlack );

	if ( bIsBlack )
		status = STATUS_BAD_INITIAL_PC;
	else
		status = ZwOpenKey( pKeyHandle, DesireAccess, ObjAtr );

	return status;
}



/*

ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 



ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 


ULONG WINAPI 
*/

///////////////////////////////   END OF FILE   ///////////////////////////////