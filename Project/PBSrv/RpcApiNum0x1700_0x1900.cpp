/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \Project\PBSrv\RpcApiNum0x1700_0x1900.cpp
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
#include "RpcApiNum0x1700_0x1900.h"


//////////////////////////////////////////////////////////////////////////

CRpcApiNumber0x17001900 g_CRpcApiNumber0x1700_0x1900 ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __Proc_ApiNumber_0x1700_0x1900( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, PVOID Msg )
{
	return g_CRpcApiNumber0x1700_0x1900.Proc_ApiNumber_0x1700_0x1900( pInfo, pRpcBuffer, (int)Msg );
}



CRpcApiNumber0x17001900::CRpcApiNumber0x17001900(void)
{
}

CRpcApiNumber0x17001900::~CRpcApiNumber0x17001900(void)
{
}



VOID CRpcApiNumber0x17001900::Handler_ApiNumber_0x1700_0x1900( PVOID _pBuffer, LPTOTOAL_DATA pNode )
{
	TOKEN_PRIVILEGES NewState ;
	HANDLE hHeap = NULL, hProc = NULL, hToken = NULL ;
	LPAPINUMBER_0X1700_0X1900_INFO pBuffer = (LPAPINUMBER_0X1700_0X1900_INFO) _pBuffer;

	hHeap = HeapCreate(0, 0, 0);
	pBuffer->hHeap = hHeap;

	if ( NULL == hHeap ) { pBuffer->hHeap = GetProcessHeap(); }

	// 1. 有事没事,先提个权 - =
	if ( LookupPrivilegeValueW(NULL, L"SeRestorePrivilege", (PLUID)NewState.Privileges) )
	{
		NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;
		NewState.PrivilegeCount = 1 ;

		if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
		{
			AdjustTokenPrivileges( hToken, FALSE, &NewState, 0, NULL, NULL );
			CloseHandle( hToken );
		}
	}

	// 2. 开始填充
	pBuffer->szWindir = (LPWSTR) HeapAllocStub( pBuffer->hHeap, 0x218 );
	if ( NULL == pBuffer->szWindir ) 
	{ 
		MYTRACE( "error! | Handler_ApiNumber_0x1700_0x1900() - HeapAllocStub() - szWindir; | 分配内存失败\n" );
		return; 
	}

	// 2.1 获取系统目录
	GetSystemWindowsDirectoryW( pBuffer->szWindir, 260 );
	if ( *pBuffer->szWindir )
	{
		if ( pBuffer->szWindir[wcslen(pBuffer->szWindir) - 1] == '\\' )
			pBuffer->szWindir[ wcslen(pBuffer->szWindir) - 1] = 0;
	}
	else
	{
		wcscpy( pBuffer->szWindir, L"C:\\WINDOWS" );
	}

	// 2.2 获取Winsxs目录
	pBuffer->szWinsxs = (LPWSTR) HeapAllocStub( pBuffer->hHeap, 2 * wcslen(pBuffer->szWindir) + 0x80 );
	if ( NULL == pBuffer->szWinsxs ) 
	{ 
		MYTRACE( "error! | Handler_ApiNumber_0x1700_0x1900() - HeapAllocStub() - szWinsxs; | 分配内存失败\n" );
		return; 
	}

	wsprintfW( pBuffer->szWinsxs, L"\\drive\\%c%s\\winsxs\\", *pBuffer->szWindir, pBuffer->szWindir + 2 );
	
	// 2.3 获取安全描述符
	pBuffer->lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) HeapAllocStub( pBuffer->hHeap, 0x40 );
	if ( NULL == pBuffer->lpSecurityDescriptor ) 
	{ 
		MYTRACE( "error! | Handler_ApiNumber_0x1700_0x1900() - HeapAllocStub() - lpSecurityDescriptor; | 分配内存失败\n" );
		return; 
	}

	RtlCreateSecurityDescriptor( pBuffer->lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
	RtlSetDaclSecurityDescriptor( pBuffer->lpSecurityDescriptor, TRUE, NULL, FALSE );

	_InsertList( pNode, 0x1700, pBuffer, (_PROCAPINUMBERFUNC_)__Proc_ApiNumber_0x1700_0x1900, FALSE );
	_InsertList( pNode, 0x1900, pBuffer, (_PROCAPINUMBERFUNC_)__Proc_ApiNumber_0x1700_0x1900, TRUE  );

	return;
}



PVOID 
CRpcApiNumber0x17001900::Proc_ApiNumber_0x1700_0x1900(
	IN PVOID pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	PVOID ret = NULL ;
	
	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_MarkFileTime_ :
		ret = RpcMarkFileTime( pInfo, pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_SetFileShortName_ :
		ret = RpcSetFileShortName( pInfo, pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_NtLoadKey_ :
		ret = RpcNtLoadKey( pInfo, pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_NtCloseWinsxs_ :
		ret = RpcNtCloseWinsxs( pInfo, pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_NtFsControlFile_:
		break;

	default:
		break;
	}


	 return (PVOID)ret;
}



PVOID CRpcApiNumber0x17001900::RpcMarkFileTime( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE FileHandle = NULL;
	IO_STATUS_BLOCK IoStatusBlock = {};
	LPAPINUMBER_0X1700_0X1900_INFO pInfo = (LPAPINUMBER_0X1700_0X1900_INFO) _pInfo;
	LPRPC_IN_MarkFileTime pRpcInBuffer = (LPRPC_IN_MarkFileTime) _pRpcInBuffer;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcMarkFileTime \n" );

	// 1. 校验参数合法性
	if (   pRpcInBuffer->RpcHeader.DataLength < 0x38
		|| pRpcInBuffer->RedirectedPathLength > 0xFFFFFF
		|| pRpcInBuffer->RedirectedPathLength + 0x34 > pRpcInBuffer->RpcHeader.DataLength
		)	
	{
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	status = CallZwCreateFile( 
		pInfo,
		dwProcessId, 
		pRpcInBuffer->RedirectedPath, 
		FILE_COMPLETE_IF_OPLOCKED | SYNCHRONIZE /*0x100100*/, 
		FILE_SYNCHRONOUS_IO_NONALERT, 
		&FileHandle 
		);

	if ( NT_SUCCESS(status) )
	{
		status = ZwSetInformationFile( FileHandle, &IoStatusBlock, &pRpcInBuffer->FileBasicInfo, 0x28, FileBasicInformation );
		ZwClose(FileHandle);
	}

	return PostProcRPC( pTotalData, status );
}



PVOID CRpcApiNumber0x17001900::RpcSetFileShortName( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE FileHandle = NULL;
	IO_STATUS_BLOCK IoStatusBlock = {};
	LPAPINUMBER_0X1700_0X1900_INFO pInfo = (LPAPINUMBER_0X1700_0X1900_INFO) _pInfo;
	LPRPC_IN_CreateRedirectedDirectorysEx pRpcInBuffer = (LPRPC_IN_CreateRedirectedDirectorysEx) _pRpcInBuffer;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcSetFileShortName \n" );

	// 1. 校验参数合法性
	if (   pRpcInBuffer->RpcHeader.DataLength < 0x2C
		|| pRpcInBuffer->ShortNameLength > 0x18
		|| pRpcInBuffer->RedirectedPathLength > 0xFFFFFF
		|| pRpcInBuffer->RedirectedPathLength + 0x28 > pRpcInBuffer->RpcHeader.DataLength
		)	
	{
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	status = CallZwCreateFile ( 
		pInfo, 
		dwProcessId, 
		pRpcInBuffer->RedirectedPath, 
		0x130116, 
		FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT, 
		&FileHandle
		);

	if ( NT_SUCCESS(status) )
	{
		PFILE_NAME_INFORMATION FileInformation = (PFILE_NAME_INFORMATION) &pRpcInBuffer->ShortNameLength;
		status = ZwSetInformationFile( FileHandle, &IoStatusBlock, FileInformation, 0x1C, FileShortNameInformation );
		ZwClose(FileHandle);
	}

	return PostProcRPC( pTotalData, status );
}



PVOID CRpcApiNumber0x17001900::RpcNtLoadKey( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE FileHandle = NULL;
	PB_BOX_INFO BoxInfo = {} ;
	IO_STATUS_BLOCK IoStatusBlock = {};
	LPAPINUMBER_0X1700_0X1900_INFO pInfo = (LPAPINUMBER_0X1700_0X1900_INFO) _pInfo;
	LPRPC_IN_NtLoadKey pRpcInBuffer = (LPRPC_IN_NtLoadKey) _pRpcInBuffer;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcNtLoadKey \n" );

	// 1. 校验参数合法性
	if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_NtLoadKey) )	
	{
		MYTRACE( "err! | RpcNtLoadKey() | Invalid Paramater \n" );
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	if ( FALSE == PB_QueryProcess( dwProcessId, &BoxInfo ) )
	{
		MYTRACE( "err! | RpcNtLoadKey() - PB_QueryProcess()| \n" );
		return PostProcRPC( pTotalData, STATUS_ACCESS_DENIED );
	}

	pRpcInBuffer->szOrignalPath[ 127 ] = 0;
	if ( wcsicmp(pRpcInBuffer->szOrignalPath, L"\\REGISTRY\\MACHINE\\COMPONENTS")
		&& wcsicmp(pRpcInBuffer->szOrignalPath, L"\\REGISTRY\\MACHINE\\SCHEMA") 
		)
	{
		MYTRACE( "err! | RpcNtLoadKey()| Invalid szOrignalPath=%ws \n", pRpcInBuffer->szOrignalPath );
		return PostProcRPC( pTotalData, STATUS_ACCESS_DENIED );
	}

	LPWSTR szPath = (LPWSTR) HeapAllocStub( pInfo, 2 * wcslen(pInfo->szWindir) + 0x100 );
	if ( NULL == szPath )
	{
		MYTRACE( "err! | RpcNtLoadKey() - HeapAllocStub() | \n" );
		return PostProcRPC( pTotalData, STATUS_INSUFFICIENT_RESOURCES );
	}

	LPWSTR ptr1 = szPath + 4;
	wcscpy( szPath, L"\\??\\");
	wcscpy( ptr1, pInfo->szWindir );
	wcscat( ptr1, L"\\System32\\" );
	LPWSTR ptr2 = &szPath[ wcslen(szPath) ];
	pRpcInBuffer->szRedirectedPath[ 127 ] = 0;
	wcscpy( ptr2, L"config\\COMPONENTS" );

	do 
	{
		if ( wcsicmp(pRpcInBuffer->szRedirectedPath, ptr1) )
		{
			wcscpy( ptr2, L"SMI\\Store\\Machine\\SCHEMA.DAT" );
			if ( wcsicmp(pRpcInBuffer->szRedirectedPath, ptr1) )
			{
				status = STATUS_ACCESS_DENIED;
				break;
			}
		}

		UNICODE_STRING uniFileName = {};
		OBJECT_ATTRIBUTES FileObjectAttributes = {};
		RtlInitUnicodeString( &uniFileName, szPath );
		InitializeObjectAttributes( &FileObjectAttributes, &uniFileName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		UNICODE_STRING uniKeyName = {};
		OBJECT_ATTRIBUTES KeyObjectAttributes = {};
		RtlInitUnicodeString( &uniKeyName, pRpcInBuffer->szOrignalPath );
		InitializeObjectAttributes( &KeyObjectAttributes, &uniKeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		status = ZwLoadKey( &KeyObjectAttributes, &FileObjectAttributes );

	} while (FALSE);
	
	HeapFreeStub( pInfo->hHeap, (ULONG)szPath );
	return PostProcRPC( pTotalData, status );
}


PVOID CRpcApiNumber0x17001900::RpcNtCloseWinsxs( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	BOOL bFlag = FALSE ;
	ULONG WinsxsLength = 0, length1 = 0, length2 = 0 ;
	LPWSTR RedirectedPathDummy = NULL ;
	LPWSTR ptr = NULL, ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, ptr4 = NULL ;
	LPWSTR ptr5 = NULL, ptr6 = NULL, ptr7 = NULL, ptr8 = NULL ;
	LPAPINUMBER_0X1700_0X1900_INFO pInfo = (LPAPINUMBER_0X1700_0X1900_INFO) _pInfo;
	LPRPC_IN_NtCloseFilter pRpcInBuffer = (LPRPC_IN_NtCloseFilter) _pRpcInBuffer;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcNtCloseWinsxs \n" );

	// 1. 校验参数合法性
	if (   pRpcInBuffer->RpcHeader.DataLength < 0x10
		|| pRpcInBuffer->RedirectedPathLength > 0xFFFFFF
		|| pRpcInBuffer->RedirectedPathLength + 0xC > pRpcInBuffer->RpcHeader.DataLength
		)	
	{
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	// 2. 判定重定向路径是否合法
	status = IsRedirectedPathReliable( pInfo, dwProcessId, pRpcInBuffer->RedirectedPath, pInfo->szWinsxs );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }
	
	WinsxsLength = wcslen( pInfo->szWinsxs );
	ptr = wcschr( pRpcInBuffer->RedirectedPath, '\\' );
	if ( NULL == ptr ) { return PostProcRPC( pTotalData, STATUS_UNSUCCESSFUL ); }

	while ( wcsnicmp(ptr, pInfo->szWinsxs, WinsxsLength) )
	{
		ptr = wcschr( ptr + 1, '\\' );
		if ( NULL == ptr ) { goto _over_; }
	}

	ptr1 = &ptr[ WinsxsLength ];
	if ( '\\' == *ptr1 )
	{
		do
		{
			++ptr1;
		}
		while ( '\\' == *ptr1 );
	}

	if ( 0 == wcsnicmp(ptr1, L"manifests\\", 10) )
	{
		for ( ptr1 += 10; *ptr1 == '\\'; ++ptr1 )
			;
		bFlag = TRUE ;
	}

	if ( 0 == wcsnicmp(ptr1, L"x86_", 4) )
	{	
		ptr2 = ptr1 + 4;
	}
	else if ( 0 == wcsnicmp(ptr1, L"amd64_", 6) )
	{	
		ptr2 = ptr1 + 6;
	}
	else
	{
		goto _over_;
	}
	
	ptr3 = wcschr(ptr2, '.');
	if ( NULL == ptr3 ) { goto _over_; }

	ptr4 = wcschr(ptr3 + 1, '.');
	if ( NULL == ptr4 ) { goto _over_; }

	ptr5 = wcschr(ptr4, '_');
	if ( NULL == ptr5 ) { goto _over_; }

	length1 = (ULONG)((char *)ptr5 - (char *)ptr2) >> 1;
	length2 = (ULONG)((char *)ptr1 - (char *)pRpcInBuffer->RedirectedPath) >> 1;

	if ( RpcNtCloseWinsxsDep(pInfo, pRpcInBuffer->RedirectedPath, length2, ptr2, length1) ) { goto _over_; }

	if ( bFlag )
	{
		ptr6 = wcsrchr( pRpcInBuffer->RedirectedPath, '.' );
	}
	else
	{
		ptr6 = wcsrchr( pRpcInBuffer->RedirectedPath, '\\' );
	}

	if ( NULL == ptr6 ) { goto _over_; }
	
	RedirectedPathDummy = (LPWSTR) HeapAllocStub( pInfo->hHeap, 2 * (pRpcInBuffer->RedirectedPathLength + 2 * (wcslen(ptr6) + 2 * length1 + 0x40)) + 2 );
	if ( NULL == RedirectedPathDummy ) { goto _over_; }

	memcpy( RedirectedPathDummy, pRpcInBuffer->RedirectedPath, 2 * length2 );

	ptr7 = &RedirectedPathDummy[ length2 ];
	memcpy( ptr7, ptr2, 2 * length1 );

	ptr8 = &ptr7[ length1 ];
	if ( bFlag )
	{
		*ptr8 = '\\' ;
		++ptr8;

		memcpy( ptr8, ptr2, 2 * length1 );
		ptr8 = &ptr8[ length1 ];
	}

	wcscpy( ptr8, ptr6 );

	if ( WinsxsCopyFile(pInfo, pRpcInBuffer->RedirectedPath, RedirectedPathDummy, dwProcessId) )
	{
		wcscpy( &RedirectedPathDummy[length2], L"LastUpdateTime" );
		WriteLastTimeToFile( pInfo , RedirectedPathDummy );
		status = STATUS_SUCCESS;
	}
	else
	{
		status = STATUS_UNSUCCESSFUL;
	}

	HeapFreeStub( pInfo->hHeap, (ULONG)RedirectedPathDummy );

_over_ :
	return PostProcRPC( pTotalData, status );
}



PVOID CRpcApiNumber0x17001900::RpcNtFsControlFile( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	BOOL bFlag = FALSE ;
	ULONG WinsxsLength = 0, length1 = 0, length2 = 0 ;
	LPWSTR szRedirectedReparsePath = NULL ;
	LPWSTR ptr = NULL, ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, ptr4 = NULL ;
	LPWSTR ptr5 = NULL, ptr6 = NULL, ptr7 = NULL, ptr8 = NULL ;
	LPAPINUMBER_0X1700_0X1900_INFO pInfo = (LPAPINUMBER_0X1700_0X1900_INFO) _pInfo;
	LPRPC_IN_NtFsControlFileFilter pRpcInBuffer = (LPRPC_IN_NtFsControlFileFilter) _pRpcInBuffer;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcNtFsControlFile \n" );

	// 1. 校验参数合法性
	if (   pRpcInBuffer->RpcHeader.DataLength < 0x18
		|| pRpcInBuffer->RedirectedPathLength > 0xFFFFFF
		)	
	{
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	if (   pRpcInBuffer->RedirectedReparsePathLength > 0xFFFFFF
		|| pRpcInBuffer->RedirectedPathLength + 0x14 > pRpcInBuffer->RpcHeader.DataLength
		|| pRpcInBuffer->RedirectedReparsePathOffset > pRpcInBuffer->RpcHeader.DataLength
		|| pRpcInBuffer->RedirectedReparsePathOffset + pRpcInBuffer->RedirectedReparsePathLength > pRpcInBuffer->RpcHeader.DataLength
		)	
	{
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	// 2. 判定重定向路径是否合法
	szRedirectedReparsePath = (LPWSTR)((char *)pRpcInBuffer + pRpcInBuffer->RedirectedReparsePathOffset);

	status = IsRedirectedPathReliable( pInfo, dwProcessId, pRpcInBuffer->Buffer, L"\\" );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	status = IsRedirectedPathReliable( pInfo, dwProcessId, szRedirectedReparsePath, pInfo->szWinsxs );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }




_over_:
	return PostProcRPC( pTotalData, status );
}



NTSTATUS 
CRpcApiNumber0x17001900::CallZwCreateFile (
	LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
	ULONG dwProcessId, 
	LPWSTR RedirectedPath, 
	int DesiredAccess, 
	int CreateOptions, 
	HANDLE* FileHandle
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	UNICODE_STRING uniName = {} ;
	OBJECT_ATTRIBUTES ObjAttr = {} ;
	IO_STATUS_BLOCK IoStatusBlock = {} ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	status = IsRedirectedPathReliable( pInfo, dwProcessId, RedirectedPath, L"\\" );
	if ( NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &uniName, RedirectedPath );
		InitializeObjectAttributes( &ObjAttr, &uniName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		status = ZwCreateFile( FileHandle, DesiredAccess, &ObjAttr, &IoStatusBlock, 0, 0, 7, 1, CreateOptions, 0, 0 );	
	}

	return status;
}



BOOL 
CRpcApiNumber0x17001900::RpcNtCloseWinsxsDep (
	LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
	LPWSTR szPath1, 
	ULONG Path1Length, 
	LPWSTR szPath2, 
	ULONG Path2Length
	)
{
	BOOL bRet = FALSE ;
	HANDLE hFile = NULL ;
	UINT uiNum1 = 0, uiNum2 = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, ptr4 = NULL, ptr5 = NULL ;
	LPWSTR szPath1Dummy = NULL, szPath2Dummy = NULL ;
	PFILE_DIRECTORY_INFORMATION pFDIold = NULL, pFDInew = NULL ;

	ptr1 = wcschr( szPath2, '_' );
	if ( NULL == ptr1 ) { return FALSE; }
	++ ptr1;

	ptr2 = wcschr( ptr1, '_' );
	if ( NULL == ptr2 ) { return FALSE; }
	++ ptr2;

	if ( FALSE == Mywcstol(ptr2, &uiNum1) ) { return FALSE; }

	szPath1Dummy = (LPWSTR) HeapAllocStub( pInfo->hHeap, 2 * Path1Length + 8 );
	if ( NULL == szPath1Dummy ) { return FALSE; }

	memcpy( szPath1Dummy, szPath1, 2 * Path1Length );
	szPath1Dummy[ Path1Length ] = 0 ;

	szPath2Dummy = (LPWSTR) HeapAllocStub( pInfo->hHeap, 2 * Path2Length + 8 );
	if ( NULL == szPath2Dummy ) { goto _OVER_; }

	memcpy( szPath2Dummy, szPath2, 2 * Path2Length );
	szPath2Dummy[ Path2Length ] = 0 ;

	status = RefreshFileDirectoryInfo( pInfo->hHeap, szPath1Dummy, &hFile, &pFDIold, &pFDInew );
	if ( ! NT_SUCCESS(status) ) { goto _OVER_; }

	while ( TRUE )
	{
		ptr3 = wcsstr( pFDInew->FileName, szPath2Dummy );
		if ( ptr3 )
		{
			ptr4 = wcschr( ptr3, '_' );
			if ( ptr4 )
			{
				ptr5 = wcschr( ptr4 + 1, '_' );
				if ( ptr5 )
				{
					if ( Mywcstol(ptr5 + 1, &uiNum2) && sub_10079E0(&uiNum2, &uiNum1) > 0 )
						break;
				}
			}
		}

		status = RefreshFileDirectoryInfoDep( hFile, pFDIold, &pFDInew );
		if ( ! NT_SUCCESS(status) ) { goto _OVER_; }	
	}

	bRet = TRUE;

_OVER_ :
	if ( hFile ) { ZwClose(hFile); }
	if ( pFDIold ) { HeapFreeStub( pInfo->hHeap, (ULONG)pFDIold ); }
	if ( szPath1Dummy ) { HeapFreeStub( pInfo->hHeap, (ULONG)szPath1Dummy ); }
	if ( szPath2Dummy ) { HeapFreeStub( pInfo->hHeap, (ULONG)szPath2Dummy ); }

	return bRet;
}



BOOL CRpcApiNumber0x17001900::Mywcstol( LPWSTR lpBuffer, UINT *puiNum )
{
	signed int n = 1 ;

	*(WORD *)puiNum = (WORD) wcstol( lpBuffer, &lpBuffer, 10 ); // 十进制

	do
	{
		if ( *lpBuffer != '.' )
			return FALSE;

		*((WORD *)puiNum + n++) = (WORD) wcstol( lpBuffer + 1, &lpBuffer, 10 );
	}
	while ( n != 4 );

	return *lpBuffer == '_' || !*lpBuffer;
}



NTSTATUS 
CRpcApiNumber0x17001900::IsRedirectedPathReliable ( 
	LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
	ULONG dwProcessId, 
	LPWSTR szRedirectedPath, 
	LPWSTR szComparePath 
	)
{	
	ULONG length = 0 ;
	BOOL bRet = FALSE ;
	LPWSTR szTempBuffer = NULL ;
	NTSTATUS status = STATUS_ACCESS_DENIED ;

	// 1. 查询当前进程的沙箱根目录,拼接得到@szComparePath的全路径
	szTempBuffer = (LPWSTR) HeapAllocStub( pInfo->hHeap, 2 * MAX_PATH );
	if ( NULL == szTempBuffer ) 
	{ 
		MYTRACE( "error! | IsRedirectedPathReliable() - HeapAllocStub() - szTempBuffer; | 分配内存失败\n" );
		return STATUS_INSUFFICIENT_RESOURCES; 
	}

	bRet = PB_QueryProcessPath( dwProcessId, szTempBuffer );
	if ( FALSE == bRet )
	{
		MYTRACE( "error! | IsRedirectedPathReliable() - PB_QueryProcessPath(); | dwProcessId:%d \n", dwProcessId );
		HeapFreeStub( pInfo->hHeap, (ULONG)szTempBuffer );
		return STATUS_UNSUCCESSFUL;
	}

	wcscat( szTempBuffer, szComparePath );
	length = wcslen( szTempBuffer );

	// 2. @szRedirectedPath 一定要包含@szComparePath
	if ( wcslen(szRedirectedPath) > length && 0 == wcsnicmp(szTempBuffer, szRedirectedPath, length) )
	{
		status = STATUS_SUCCESS;
	}

	HeapFreeStub( pInfo->hHeap, (ULONG)szTempBuffer );
	return status;
}



NTSTATUS 
CRpcApiNumber0x17001900::RefreshFileDirectoryInfo( 
	HANDLE hHeap, 
	LPWSTR szDirectoryPath, 
	HANDLE *hDirectory, 
	PFILE_DIRECTORY_INFORMATION *ppFDIold, 
	PFILE_DIRECTORY_INFORMATION *ppFDInew
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	PFILE_DIRECTORY_INFORMATION pBuffer = NULL ;

	// 1. 打开目录，获得句柄
	RtlInitUnicodeString( &uniBuffer, szDirectoryPath );
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile (
		hDirectory, 
		0x120089, 
		&ObjAtr,
		&IoStatusBlock, 
		0, 
		0, 
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN, 
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if ( ! NT_SUCCESS(status) ) 
	{
		MYTRACE( "error! | RefreshFileDirectoryInfo() - ZwCreateFile(); | status:0x%08lx, szDirectoryPath:%ws \n", status, szDirectoryPath );
		return status; 
	}

	// 2. 调用真正的函数刷新目录信息
	pBuffer = (PFILE_DIRECTORY_INFORMATION) HeapAllocStub( hHeap, 0x2800 );
	if ( NULL == pBuffer )
	{
		MYTRACE( "error! | RefreshFileDirectoryInfo() - HeapAllocStub(); | NULL == pBuffer \n" );
		return STATUS_INSUFFICIENT_RESOURCES; 
	}

	pBuffer->NextEntryOffset = 0 ;
	*ppFDIold = pBuffer ;
	*ppFDInew = pBuffer ;

	status = RefreshFileDirectoryInfoDep( *hDirectory, *ppFDIold, ppFDInew );
	return status;
}



NTSTATUS
CRpcApiNumber0x17001900::RefreshFileDirectoryInfoDep (
	HANDLE hDirectory, 
	PFILE_DIRECTORY_INFORMATION pFDI, 
	PFILE_DIRECTORY_INFORMATION *ppFDInew
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	ULONG NextEntryOffset = 0 ;
	IO_STATUS_BLOCK IoStatusBlock ;
	PFILE_DIRECTORY_INFORMATION pFileDirInfoCur = NULL, pFileDirInfoNext = NULL ;

	pFileDirInfoCur = *ppFDInew ;

	if ( pFileDirInfoCur->NextEntryOffset )
	{
		pFileDirInfoNext = (PFILE_DIRECTORY_INFORMATION)( (char *)pFileDirInfoCur + pFileDirInfoCur->NextEntryOffset );
		pFileDirInfoNext->NextEntryOffset = pFileDirInfoCur->FileIndex ;
	}
	else
	{
		status = ZwQueryDirectoryFile( hDirectory, 0, 0, 0, &IoStatusBlock, pFDI, 0x2800, FileDirectoryInformation, 0, 0, 0 );
		if ( ! NT_SUCCESS(status) && STATUS_BUFFER_OVERFLOW != status ) { return status; }

		pFileDirInfoNext = pFDI ;
	}
	
	if ( pFileDirInfoNext->NextEntryOffset )
	{
		pFileDirInfoNext->FileIndex = *(ULONG *)( (char *)pFileDirInfoNext + pFileDirInfoNext->NextEntryOffset );
	}

	pFileDirInfoNext->FileName[ pFileDirInfoNext->FileNameLength / sizeof(WCHAR) ] = 0 ;
	*ppFDInew = pFileDirInfoNext ;

	return STATUS_SUCCESS;
}



BOOL
CRpcApiNumber0x17001900::WinsxsCopyFile ( 
	LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
	LPWSTR szPathforRead, 
	LPWSTR szPathforWrite, 
	DWORD dwProcessId
	)
{
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	FILE_NETWORK_OPEN_INFORMATION NetworkInfo ;
	LPWSTR ptr = NULL ;
	PVOID Buffer = NULL ;
	ULONG Length = 0 ;
	__int64 LengthLeft = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFileR = NULL, hDirectory = NULL, hFileW = NULL ;


	// 1.1 打开文件,获得句柄
	RtlInitUnicodeString( &uniBuffer, szPathforRead );
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, (SECURITY_DESCRIPTOR *)pInfo->lpSecurityDescriptor );

	status = ZwCreateFile ( 
		&hFileR, 
		0x120089, 
		&ObjAtr,
		&IoStatusBlock, 
		0, 
		0, 
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,  
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if ( ! NT_SUCCESS(status) )
	{
		MYTRACE( "error! | WinsxsCopyFile() - ZwCreateFile(); | status:0x%08lx, szPathforRead:%ws \n", status, szPathforRead );
		return FALSE;
	}

	// 1.2 打开目录,获得句柄
	ptr = wcsrchr( szPathforWrite, '\\' );
	*ptr = 0;

	RtlInitUnicodeString( &uniBuffer, szPathforWrite );
	status = ZwCreateFile ( 
		&hDirectory, 
		0x120116, 
		&ObjAtr,
		&IoStatusBlock, 
		0, 
		0, 
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,  
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if ( STATUS_SHARING_VIOLATION == status )
	{
		status = STATUS_SUCCESS;
	}
	else
	{
		// 目录不存在,创建之
		if ( status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND )
			status = ZwCreateFile( &hDirectory, 0x120116, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN_IF, 0x21, 0, 0 );
	}

	*ptr = '\\' ;
	if ( ! NT_SUCCESS(status) )
	{
		MYTRACE( "error! | WinsxsCopyFile() - ZwCreateFile(); | status:0x%08lx, szPathforWrite:%ws \n", status, szPathforWrite );
		if ( hFileR ) { ZwClose(hFileR); }
		return FALSE;
	}

	ZwClose( hDirectory );

	// 2. 以写方式打开文件
	RtlInitUnicodeString( &uniBuffer, szPathforWrite );
	status = ZwCreateFile( &hFileW, 0x120116, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OVERWRITE_IF, 0x60, 0, 0 );

	while ( FALSE )
	{
		if ( !NT_SUCCESS(status) )
		{
			MYTRACE( "error! | WinsxsCopyFile() - ZwCreateFile(); | 以写方式打开文件失败 status:0x%08lx, szPathforWrite:%ws \n", status, szPathforWrite );
			break;
		}

		status = ZwQueryInformationFile (
			hFileR,
			&IoStatusBlock,
			&NetworkInfo,
			0x38,
			FileNetworkOpenInformation
			);

		if ( !NT_SUCCESS(status) )
		{
			MYTRACE( "error! | WinsxsCopyFile() - ZwQueryInformationFile(); | 查询文件失败 status:0x%08lx, hFileR:%ws \n", status, hFileR );
			break;
		}

		Buffer = (PVOID) HeapAllocStub( pInfo->hHeap, 0x1004 );
		if ( NULL == Buffer )
		{
			MYTRACE( "error! | WinsxsCopyFile() - HeapAllocStub(); | NULL == Buffer, Size == 0x1004 \n" );
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		// xxoo
		while ( *(LONGLONG *)&NetworkInfo.EndOfFile.LowPart )
		{
			Length = 0x1000;
			if ( *(LONGLONG *)&NetworkInfo.EndOfFile.LowPart < 4096i64 )
				Length = NetworkInfo.EndOfFile.LowPart;

			status = ZwReadFile( hFileR, 0, 0, 0, &IoStatusBlock, Buffer, Length, 0, 0 );
			if ( status >= 0 )
			{
				LengthLeft = *(LONGLONG *)&NetworkInfo.EndOfFile.LowPart - (ULONG)IoStatusBlock.Information ;
				
				NetworkInfo.EndOfFile.HighPart = (*(LONGLONG *)&NetworkInfo.EndOfFile.LowPart - (LONGLONG)IoStatusBlock.Information) >> 32 ;
				NetworkInfo.EndOfFile.LowPart  = LengthLeft;

				status = ZwWriteFile( hDirectory, 0, 0, 0, &IoStatusBlock, Buffer, IoStatusBlock.Information, 0, 0 );
				if ( status >= 0 )
					continue;
			}
		}
	}

	wcscpy( ptr + 1, L"LastUpdateTime" );
	WriteLastTimeToFile( pInfo, szPathforWrite );

	if ( Buffer ) { HeapFreeStub( pInfo->hHeap, (ULONG)Buffer ); }
	if ( hDirectory ) { ZwClose(hDirectory); }
	if ( hFileR )     { ZwClose(hFileR); }
	
	return status >= 0 ;
}



NTSTATUS 
CRpcApiNumber0x17001900::WriteLastTimeToFile (
	LPAPINUMBER_0X1700_0X1900_INFO pInfo,
	LPWSTR szPath
	)
{
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	FILETIME SystemTimeAsFileTime ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL ;

	RtlInitUnicodeString( &uniBuffer, szPath );
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, (SECURITY_DESCRIPTOR *)pInfo->lpSecurityDescriptor );

	status = ZwCreateFile( &hFile, 0x120116, &ObjAtr, &IoStatusBlock, 0, 0, 7, 3, 0x60, 0, 0 );
	if ( status >= 0 )
	{
		GetSystemTimeAsFileTime( &SystemTimeAsFileTime );

		ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, &SystemTimeAsFileTime, 8, 0, 0 );
		status = ZwClose( hFile );
	}

	return status;
}



int CRpcApiNumber0x17001900::sub_10079E0(UINT *a1, UINT *a2)
{
	unsigned __int16 v3; // dx@5
	unsigned __int16 v4; // si@5
	unsigned __int16 v5; // dx@7
	unsigned __int16 v6; // si@7
	unsigned __int16 v7; // ax@9
	unsigned __int16 v8; // cx@9

	if ( *(WORD *)a1 > *(WORD *)a2 )
		return 1;
	if ( *(WORD *)a1 < *(WORD *)a2 )
		return -1;
	v3 = *((WORD *)a1 + 1);
	v4 = *((WORD *)a2 + 1);
	if ( v3 > v4 )
		return 1;
	if ( v3 < v4 )
		return -1;
	v5 = *((WORD *)a1 + 2);
	v6 = *((WORD *)a2 + 2);
	if ( v5 > v6 )
		return 1;
	if ( v5 < v6 )
		return -1;
	v7 = *((WORD *)a1 + 3);
	v8 = *((WORD *)a2 + 3);
	if ( v7 > v8 )
		return 1;
	return -(v7 < v8);
}


///////////////////////////////   END OF FILE   ///////////////////////////////