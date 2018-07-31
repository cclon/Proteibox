/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/03 [3:12:2010 - 17:45]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDLL\PBAlpcData.cpp
* 
* Description:
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
#include "MemoryManager.h"
#include "ProteinBoxDLL.h"
#include "PBAlpcData.h"

#pragma warning(disable : 4995) 
//////////////////////////////////////////////////////////////////////////

#define _CompletedEvent_	L"PB_ServiceInitComplete_%s"


//////////////////////////////////////////////////////////////////////////

NTSTATUS 
GetAlpcPath (
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT LPWSTR *lppOrignalPath,
	OUT LPWSTR *lppRedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/12/20 [20:12:2010 - 10:21]

Routine Description:
  获取alpc对应的原始 & 重定向 路径;
    
Arguments:
  ObjectAttributes - 
  lppOrignalPath - 保存获取到的原始路径
  lppRedirectedPath - 保存获取到的重定向路径
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ;
	PUNICODE_STRING ObjectName = NULL ;
	HANDLE RootDirectory = NULL ;

	if ( ObjectAttributes )
	{
		ObjectName = ObjectAttributes->ObjectName ;
		RootDirectory = ObjectAttributes->RootDirectory ;
	}
	
	status = GetAlpcPathEx( ObjectName, RootDirectory, lppOrignalPath, lppRedirectedPath, NULL );
	return status;
}


NTSTATUS 
GetAlpcPathEx (
	IN PUNICODE_STRING ObjectName,
	IN HANDLE RootDirectory,
	OUT LPWSTR *lppOrignalPath,
	OUT LPWSTR *lppRedirectedPath,
	OUT BYTE *out_bFlag_Is_Sandbox_Object
	)
{
	ULONG Length = 0;
	PVOID ObjInfo = NULL;
	LPWSTR szObjectName = NULL;
	NTSTATUS status = STATUS_SUCCESS ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( out_bFlag_Is_Sandbox_Object )
		*out_bFlag_Is_Sandbox_Object = 0;

	if ( ObjectName )
	{
		Length = ObjectName->Length & 0xFFFE;
		szObjectName = ObjectName->Buffer;
	}

	// 1. 处理句柄有的情况
	if ( RootDirectory )
	{
		ULONG ObjectInformationLength = 0x100, ReturnLength = 0;
		ObjInfo = CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, Length + 0x100 );
		if ( !Length || !*szObjectName )
		{
			// 2.1 不处理句柄类型为"Directory"
			POBJECT_TYPE_INFORMATION ObjTypeInfo = (POBJECT_TYPE_INFORMATION) ObjInfo;
			status = ZwQueryObject( RootDirectory, ObjectTypeInformation, ObjTypeInfo, ObjectInformationLength, &ReturnLength );
			if ( ! NT_SUCCESS(status) ) { return status; }

			if ( 0 == wcsicmp(ObjTypeInfo->Name.Buffer, L"Directory") )
			{
				*lppOrignalPath = *lppRedirectedPath =  NULL ;
				return status;
			}
		}

		// 2.2 查询句柄对应的对象名
		POBJECT_NAME_INFORMATION ObjNameInfo = (POBJECT_NAME_INFORMATION) ObjInfo;
		status = ZwQueryObject( RootDirectory, ObjectNameInformation, ObjNameInfo, ObjectInformationLength, &ReturnLength );
		if ( status == STATUS_BUFFER_OVERFLOW )
		{
			ObjInfo = CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, Length + ReturnLength );
			ObjNameInfo = (POBJECT_NAME_INFORMATION) ObjInfo;

			status = ZwQueryObject( RootDirectory, ObjectNameInformation, ObjNameInfo, ReturnLength, &ReturnLength );
		}

		if ( ! NT_SUCCESS(status) ) { return status; }
		if ( NULL == ObjNameInfo->Name.Buffer ) 
		{
			*lppRedirectedPath = NULL;
			return status;
		}

		// 2.3 拼接出完整路径
		*lppOrignalPath = ObjNameInfo->Name.Buffer;
		LPWSTR ptr = &ObjNameInfo->Name.Buffer[ ObjNameInfo->Name.Length / sizeof(WCHAR) ];
		if ( Length )
		{
			*ptr = '\\';
			LPWSTR ptr1 = ptr + 1;
			memcpy( ptr1, szObjectName, Length );
			ptr = &ptr1[ Length >> 1 ];
		}

		*ptr = UNICODE_NULL;
	}

	// 2. 处理句柄不存在的情况
	else
	{
		if ( 0 == Length )
		{
			*lppOrignalPath = *lppRedirectedPath =  NULL ;
			return STATUS_SUCCESS;
		}

		LPWSTR lpOrignalPath = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, Length );
		*lppOrignalPath = lpOrignalPath;

		memcpy( lpOrignalPath, szObjectName, Length);
		lpOrignalPath[ Length / sizeof(WCHAR) ] = UNICODE_NULL;
	}

	LPWSTR lpOrignalPathOld = *lppOrignalPath;
	int OrignalPathLength = wcslen(lpOrignalPathOld);
	int OrignalPathLengthDummy = OrignalPathLength ;
	ULONG LpcRootPathLength = g_BoxInfo.LpcRootPathLength / sizeof(WCHAR) - 1 ;

	if ( OrignalPathLength >= (int)LpcRootPathLength )
	{
		do 
		{
			if ( RtlCompareUnicodeStringDummy(*lppOrignalPath, g_BoxInfo.LpcRootPath, (USHORT)LpcRootPathLength) )
			{
				OrignalPathLength = OrignalPathLengthDummy;
				break;
			}
			
			*lppOrignalPath += LpcRootPathLength;
			OrignalPathLength = OrignalPathLengthDummy - LpcRootPathLength;
			OrignalPathLengthDummy -= LpcRootPathLength;

			if ( out_bFlag_Is_Sandbox_Object )
				*out_bFlag_Is_Sandbox_Object = 1;
			
			if ( OrignalPathLength < (int)LpcRootPathLength )
				break;

		} while (TRUE);
	}
	
	LPWSTR lpRedirectedPath = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 1, OrignalPathLength + LpcRootPathLength );
	*lppRedirectedPath = lpRedirectedPath;

	memcpy( lpRedirectedPath, g_BoxInfo.LpcRootPath, 2 * LpcRootPathLength );

	LPWSTR ptr2 = &lpRedirectedPath[ LpcRootPathLength ];
	memcpy( ptr2, *lppOrignalPath, 2 * OrignalPathLengthDummy );
	ptr2[ OrignalPathLengthDummy ] = UNICODE_NULL;

	return STATUS_SUCCESS ;
}


NTSTATUS 
CreateDirOrLinks (
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	)
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPWSTR ptr1 = NULL, ptr2 = NULL ; 
	HANDLE hDirectory = NULL ;

	// 打开原始的父目录
	ptr1 = wcsrchr( OrignalPath, '\\' );
	if ( NULL == ptr1 ) { return STATUS_OBJECT_PATH_NOT_FOUND; }
	*ptr1 = 0;

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &uniObjectName, OrignalPath );

	status = ZwOpenDirectoryObject( &hDirectory, DIRECTORY_QUERY, &ObjAtr );
	*ptr1 = '\\';
	
	if ( ! NT_SUCCESS(status) ) { return status; }
	ZwClose( hDirectory );

	// 操作重定向目录
	ptr2 = wcschr( RedirectedPath + 1, '\\' );
	if ( NULL == ptr2 ) { return STATUS_SUCCESS; }

	ULONG CurrentLength = 0 ;
	ULONG LpcRootPathLength = g_BoxInfo.LpcRootPathLength / sizeof(WCHAR) - 1 ;

	while ( TRUE )
	{
		CurrentLength = (ULONG)((PCHAR)ptr2 - (PCHAR)RedirectedPath) / sizeof(WCHAR) ;
		if ( CurrentLength > LpcRootPathLength )
		{
			*ptr2 = 0 ;
			status = PB_CreateDirOrLink( RedirectedPath, NULL );
			*ptr2 = '\\' ;

			if ( ! NT_SUCCESS(status) ) { break; }
		}

		ptr2 = wcschr( ptr2 + 1, '\\' );
		if ( NULL == ptr2 ) { return STATUS_SUCCESS; }
	}

	return status;
}


BOOL
PB_StartCOM_Ex (
	IN LPWSTR szName
	)
{
	int n = 0 ;
	HANDLE hEvent = NULL ;
	LPWSTR Buffer = NULL ;
	BOOL bFlagCreateEvent = FALSE ;
	WCHAR Path[ MAX_PATH ] = L"" ;
	WCHAR Name[ MAX_PATH ] = L"" ;
	STARTUPINFOW StartupInfo ;
	PROCESS_INFORMATION ProcessInfo ;

	if ( wcsicmp( g_BoxInfo.ProcessName, g_PBRpcSs_exe ) )
	{
		if (   0 == wcsicmp( g_BoxInfo.ProcessName, g_PBDcomLaunch_exe )
			|| wcsicmp( szName, L"\\RPC Control\\epmapper" ) 
			)
		{ 
			return FALSE;
		}

		Buffer = L"RpcSs";
	}
	else
	{
		if ( wcsicmp( szName, L"\\RPC Control\\actkernel" ) ) { return FALSE; }

		Buffer = L"DcomLaunch" ;
	}

	hEvent = GetEventHandleSBIE( Buffer, &bFlagCreateEvent );
	if ( NULL == hEvent ) { return FALSE; }

	if ( FALSE == bFlagCreateEvent ) 
	{ 
		WaitForSingleObject( hEvent, INFINITE );
		CloseHandle( hEvent );
		return TRUE;
	}

	wcscpy( Path, L"PB" );
	wcscat( Path, Buffer );
	wcscat( Path, L".exe");

	memset( &StartupInfo, 0, sizeof(StartupInfo) );
	StartupInfo.cb = sizeof(StartupInfo); ;
	StartupInfo.dwFlags = STARTF_FORCEOFFFEEDBACK ;

	if ( FALSE == PB_RunFromHome( Path, NULL, &StartupInfo, &ProcessInfo ) )
	{
		return FALSE;
	}

	CloseHandle( ProcessInfo.hProcess );
	CloseHandle( ProcessInfo.hThread );
	WaitForSingleObject( hEvent, INFINITE );
	CloseHandle( hEvent );

	if ( PB_IsOpenCOM() ) { return TRUE; }

	if ( wcsicmp( Buffer, L"RpcSs" ) ) { return TRUE; }

	swprintf( Name, _CompletedEvent_, L"DcomLaunch" );

	while ( n < 10 )
	{
		++n ;
		Sleep( 50 );
		
		hEvent = OpenEventW( EVENT_ALL_ACCESS, FALSE, Name );
		if ( hEvent )
		{
			WaitForSingleObject( hEvent, INFINITE );
			CloseHandle( hEvent );
			break;
		}

		if ( wcsicmp( Buffer, L"RpcSs" ) ) { return TRUE; }
	}

	return TRUE;
}


HANDLE 
GetEventHandleSBIE (
	IN LPWSTR szName,
	OUT BOOL* bFlagCreateEvent
	)
{
	HANDLE hEvent = NULL ;
	WCHAR Buffer[ MAX_PATH ] = L"" ;

	swprintf( Buffer, _CompletedEvent_, szName );
	if ( bFlagCreateEvent )
	{
		*bFlagCreateEvent = FALSE ;
		hEvent = CreateEventW( NULL, TRUE, FALSE, Buffer );
		
		if ( hEvent && NO_ERROR == GetLastError() ) 
		{ 
			*bFlagCreateEvent = TRUE; 
		}
	}
	else
	{
		hEvent = OpenEventW( EVENT_ALL_ACCESS, FALSE, Buffer );
	}

	return hEvent ;
}


HMODULE GetPBDllHandle()
{
	HMODULE hModule = NULL ;

	hModule = GetModuleHandleW( L"ProteinBoxDLL.dll" );
	if ( NULL == hModule )
		hModule = GetModuleHandleW( L"ProteinBoxDLLX.dll" );
	
	return hModule;
}


VOID HandlerAlpcList()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/01 [1:11:2010 - 17:03]

Routine Description:
  初始化alpc的黑白名单,其实驱动中已经初始化好了.此函数可有可无      

--*/
{
	HANDLE Section = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING FileName ;

	RtlInitUnicodeString( &FileName, L"\\KnownDlls\\kernel32.dll" );
	InitializeObjectAttributes( &ObjAtr, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	ZwOpenSection(
		&Section,
		SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
		&ObjAtr
		);

	if ( Section ) { ZwClose( Section ); }
	return ;
}


VOID CreateObjectLink()
{
	HANDLE hEvent = NULL ;
	WCHAR Name[ MAX_PATH ] = L"" ;
	NTSTATUS status = STATUS_SUCCESS ;
	ULONG_PTR CurrentPID = 0, length = 0 ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL, RedirectedPathDummy = NULL, ptr1 = NULL ;

	// 1. 创建一个"自动设置的未受信"事件
	CurrentPID = GetCurrentProcessId();
	swprintf( Name, L"PB_DummyEvent_%d", CurrentPID ); 

	hEvent = CreateEventW( NULL, FALSE, FALSE, Name );
	if ( NULL == hEvent ) { return; }

	// 2. 获取该创建对象的名字 
	status = GetAlpcPathEx( NULL, hEvent, &OrignalPath, &RedirectedPath, NULL );
	if ( ! NT_SUCCESS(status) ) { return; }

	if ( NULL == RedirectedPath || (ptr1 = wcsrchr(RedirectedPath, '\\'), NULL == ptr1) ) { goto _over_ ; }

	*ptr1 = 0 ;
	ZwClose( hEvent );

	//  
	// 3. 尝试打开以下对象名,查看是否能够成功
	// "\Sandbox\AV\DefaultBox\Session_0\BaseNamedObjects"
	// "\Sandbox\AV\DefaultBox\Session_0\BaseNamedObjects\Global"
	// "\Sandbox\AV\DefaultBox\Session_0\BaseNamedObjects\Local"
	// "\Sandbox\AV\DefaultBox\Session_0\BaseNamedObjects\Session"
	//  

	status = PB_CreateDirOrLink( RedirectedPath, NULL );
	if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
	{
		status = CreateDirOrLinks( OrignalPath, RedirectedPath );
		if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

		status = PB_CreateDirOrLink( RedirectedPath, NULL );
	}

	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	//  3.1
	length = 2 * wcslen(RedirectedPath) + 0x40 ;
	RedirectedPathDummy = (LPWSTR) kmalloc( length );
	
	wcscpy( RedirectedPathDummy, RedirectedPath );
	wcscat( RedirectedPathDummy, L"\\Global" );
	
	status= PB_CreateDirOrLink( RedirectedPathDummy, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	// 3.2
	RtlZeroMemory( RedirectedPathDummy, length );
	wcscpy( RedirectedPathDummy, RedirectedPath );
	wcscat( RedirectedPathDummy, L"\\Local" );

	status= PB_CreateDirOrLink( RedirectedPathDummy, RedirectedPath );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	// 3.3
	RtlZeroMemory( RedirectedPathDummy, length );
	wcscpy( RedirectedPathDummy, RedirectedPath );
	wcscat( RedirectedPathDummy, L"\\Session" );

	status= PB_CreateDirOrLink( RedirectedPathDummy, RedirectedPath );

_over_ :
	kfree( RedirectedPathDummy );
	return;
}


NTSTATUS MyRpcImpersonateClient()
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE TokenHandle = NULL;
	SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = SecurityAnonymous;
	ULONG TokenInformationSize = sizeof(SECURITY_IMPERSONATION_LEVEL);

	do 
	{
		status = ZwOpenThreadToken( (HANDLE)0xFFFFFFFE, TOKEN_QUERY, 0, &TokenHandle );
		if (! NT_SUCCESS(status))
			break;

		status = ZwQueryInformationToken(
			TokenHandle,
			TokenImpersonationLevel,
			(PVOID) &ImpersonationLevel,
			TokenInformationSize,
			&TokenInformationSize
			);

		if (! NT_SUCCESS(status))
			ImpersonationLevel = SecurityAnonymous;

		ZwClose(TokenHandle);
	} while (FALSE);

	if ( ImpersonationLevel >= SecurityImpersonation )
		return STATUS_SUCCESS;

	HANDLE ImpersonationToken = NULL;
	ZwSetInformationThread(
		(HANDLE)0xFFFFFFFE,
		ThreadImpersonationToken,
		(PVOID)&ImpersonationToken,
		(ULONG)sizeof( HANDLE )
		);

	HANDLE ProcessToken = NULL;
	status = ZwOpenProcessToken( (HANDLE)0xFFFFFFFF, 0x2000000, &ProcessToken );
	if (! NT_SUCCESS(status))
		return status;

	OBJECT_ATTRIBUTES ObjectAttributes = {};
	SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService = {};

	InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
	ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;
	SecurityQualityOfService.Length = 0xC;
	SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
	SecurityQualityOfService.ContextTrackingMode = 0;
	SecurityQualityOfService.EffectiveOnly = 0;

	status = ZwDuplicateToken( ProcessToken, 0x2000000, &ObjectAttributes, 0, TokenImpersonation, &ImpersonationToken);
	if ( status >= 0 )
	{
		status = ZwSetInformationThread( (HANDLE)0xFFFFFFFE, ThreadImpersonationToken, &ImpersonationToken, 4 );
		ZwClose(ImpersonationToken);
	}

	ZwClose(ProcessToken);
	return status;
}


///////////////////////////////   END OF FILE   ///////////////////////////////