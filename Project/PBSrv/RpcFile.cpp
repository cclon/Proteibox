/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \SandBox\Code\Project\PBSrv\RpcApiNum0x1500.cpp
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
#include "RpcFile.h"


//////////////////////////////////////////////////////////////////////////

CRpcFile g_CRpcFile ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __ProcFile( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, PVOID Msg )
{
	return g_CRpcFile.ProcFile( pInfo, pRpcBuffer, (int)Msg );
}


CRpcFile::CRpcFile(void)
{
}

CRpcFile::~CRpcFile(void)
{
}


VOID CRpcFile::HandlerFile( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	LPRPCFILE_INFO pTemp = (LPRPCFILE_INFO) pBuffer;

	if ( NULL == pNode || NULL == pBuffer ) { return; }

	InitializeCriticalSectionAndSpinCount( &pTemp->cs, 0x3E8 );
	ClearStruct( &pTemp->ListEntryEx );

	_InsertList( pNode, 0x1500, pBuffer, (_PROCAPINUMBERFUNC_)__ProcFile, TRUE );
	return;
}



PVOID 
CRpcFile::ProcFile(
	IN PVOID _pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	PVOID ret = NULL ;
	LPRPCFILE_INFO pInfo = (LPRPCFILE_INFO) _pInfo ;

	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_PipeCommunication_ :
		ret = RpcPipeCommunication( pInfo, pRpcBuffer, *(int *)(Msg + 8));
		break;

	case _PBSRV_APINUM_CloseLittleHandle_ :
		ret = RpcCloseLittleHandle( pInfo, pRpcBuffer, *(int *)(Msg + 8));
		break;

	case _PBSRV_APINUM_NtSetInformationFile_ :
		ret = RpcNtSetInformationFile( pInfo, pRpcBuffer, *(int *)(Msg + 8));
		break;

	case _PBSRV_APINUM_NtReadFile_ :
		ret = RpcNtReadFile( pInfo, pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_NtWriteFile_ :
		ret = RpcNtWriteFile( pInfo, pRpcBuffer, *(int*)(Msg + 8) );
		break;

	case _PBSRV_APINUM_ClearUpFile_ :
		RpcClearUpFile( pInfo, *(int*)(Msg + 8) );
		break;

	default:
		break;
	}

	 return (PVOID)ret;
}



PVOID 
CRpcFile::RpcPipeCommunication( 
	IN PVOID _pNode,
	IN PVOID _pRpcInBuffer, 
	IN int Flag 
	)
{
	int err = NO_ERROR;
	UNICODE_STRING uniName;
	OBJECT_ATTRIBUTES ObjAtr ;
	WCHAR wszName[ MAX_PATH ] = L"";
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPCFILE_INFO pNode = (LPRPCFILE_INFO) _pNode ;
	LPRPC_IN_Imitate_Pipe_Communication pRpcInBuffer = (LPRPC_IN_Imitate_Pipe_Communication) _pRpcInBuffer ;

	MYTRACE( "RpcPipeCommunication \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_Imitate_Pipe_Communication) ) 
	{
		return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER );
	}

	LPRPC_OUT_COMMONINFO pRpcOutBuffer = (LPRPC_OUT_COMMONINFO) AllocRpcBuffer(pTotalData, sizeof(RPC_OUT_COMMONINFO) );
	if ( NULL == pRpcOutBuffer ) { return NULL; }

	if ( pRpcInBuffer->PipeName[0] )
	{
		wsprintfW( wszName, L"\\device\\mup\\%ws\\PIPE\\", pRpcInBuffer->PipeName  );
	}
	else
	{
		wcscpy( wszName, L"\\device\\namedpipe\\" );
	}

	wcscat( wszName, pRpcInBuffer->szName );
	RtlInitUnicodeString( &uniName, wszName );
	InitializeObjectAttributes( &ObjAtr, &uniName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	RtlImpersonateSelf();

	HANDLE hFileRedirected = NULL ;
	status = ZwCreateFile(
		&hFileRedirected,
		0xC0100080,
		&ObjAtr,
		&IoStatusBlock,
		0,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		pRpcInBuffer->CreateOptions & (FILE_OPEN_NO_RECALL | FILE_NON_DIRECTORY_FILE),
		NULL,
		0);

	pRpcOutBuffer->IoStatusBlock.Status = IoStatusBlock.Status;
	pRpcOutBuffer->IoStatusBlock.Information = IoStatusBlock.Information;
	pRpcOutBuffer->status = status;
	pRpcOutBuffer->err = IoStatusBlock.Status;
	pRpcOutBuffer->Reserved2 = 0;

	if ( ! NT_SUCCESS(status) ) { pRpcOutBuffer; }
	
	LPRPCNTREADFILE_LITTLE_INFO pNodeL = (LPRPCNTREADFILE_LITTLE_INFO) HeapAlloc(GetProcessHeap(), 0, sizeof(RPCNTREADFILE_LITTLE_INFO));
	if ( NULL == pNodeL )
	{
		ZwClose( hFileRedirected );
		pRpcOutBuffer->status = STATUS_INSUFFICIENT_RESOURCES;
		return pRpcOutBuffer;
	}

	EnterCriticalSection( &pNode->cs );

	pNodeL->Flag = Flag;
	pNodeL->hFileOrignal = (HANDLE)GetTickCount();
	pNodeL->hFileRedirected = hFileRedirected;

	HANDLE hEvent = CreateEventW(0, 0, 0, 0);
	pNodeL->hEvent = hEvent;
	if ( hEvent )
	{
		InsertList( &pNode->ListEntryEx, NULL, (PLIST_ENTRY)pNodeL );
	}
	else
	{
		HeapFree(GetProcessHeap(), 0, pNodeL);
		ZwClose(hFileRedirected);
		pRpcOutBuffer->status = STATUS_INSUFFICIENT_RESOURCES;
	}

	LeaveCriticalSection( &pNode->cs );
	pRpcOutBuffer->RedirectedFileHandle = pNodeL->hFileOrignal ;

	return pRpcOutBuffer;
}



PVOID 
CRpcFile::RpcCloseLittleHandle( 
	PVOID _pNode, 
	PVOID _pRpcInBuffer, 
	int Flag 
	)
{
	int err = NO_ERROR;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL, hEvent = NULL ;
	LPRPC_OUT_NtReadFile pRpcOutBuffer = NULL ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPCFILE_INFO pNode = (LPRPCFILE_INFO) _pNode ;
	LPRPC_IN_HANDLE pRpcInBuffer = (LPRPC_IN_HANDLE) _pRpcInBuffer ;

	MYTRACE( "RpcCloseLittleHandle \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_HANDLE) ) 
		{ 
			err = STATUS_INVALID_PARAMETER;
			break;
		}

		HANDLE hEvent = NULL;
		HANDLE hFileRedirected = GetRedirectedFile( pNode, pRpcInBuffer->hFile, Flag, &hEvent, TRUE );
		if ( NULL == hFileRedirected ) 
		{ 
			err = STATUS_INVALID_HANDLE;
			break;
		}

		LPRPC_OUT_HEADER pOutBuffer = (LPRPC_OUT_HEADER) AllocRpcBuffer( pTotalData, 8 );
		if ( pOutBuffer )
		{
			pOutBuffer->u.Status = ZwClose(hFileRedirected);
			CloseHandle( hEvent );
		}

		return pOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



PVOID 
CRpcFile::RpcNtSetInformationFile( 
	PVOID _pNode, 
	PVOID _pRpcInBuffer, 
	int Flag 
	)
{
	int err = NO_ERROR;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL, hEvent = NULL ;
	LPRPC_OUT_NtReadFile pRpcOutBuffer = NULL ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPCFILE_INFO pNode = (LPRPCFILE_INFO) _pNode ;
	LPRPC_IN_FilePipeInformation pRpcInBuffer = (LPRPC_IN_FilePipeInformation) _pRpcInBuffer ;

	MYTRACE( "RpcNtSetInformationFile \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_FilePipeInformation)
			|| pRpcInBuffer->FileInfoLength > 0xFFFFFF
			|| pRpcInBuffer->FileInfoLength + 0x10 > pRpcInBuffer->RpcHeader.DataLength
			) 
		{ 
			err = STATUS_INVALID_PARAMETER;
			break;
		}

		// 查找对应的节点
		EnterCriticalSection( &pNode->cs );

		HANDLE hFileRedirected = NULL;
		LPRPCNTREADFILE_LITTLE_INFO pNodeLCur = (LPRPCNTREADFILE_LITTLE_INFO) pNode->ListEntryEx.Flink;

		do 
		{
			if ( pNodeLCur )
			{
				while ( pNodeLCur->hFileOrignal != pRpcInBuffer->hFile || pNodeLCur->Flag != Flag )
				{
					pNodeLCur = pNodeLCur->Flink;
					if ( NULL == pNodeLCur )
						break;
				}

				hFileRedirected = pNodeLCur->hFileRedirected;
			}
		} while (FALSE);
		
		LeaveCriticalSection( &pNode->cs );
	
		if ( NULL == hFileRedirected ) 
		{ 
			err = STATUS_INVALID_HANDLE;
			break;
		}

		LPRPC_OUT_COMMONINFO pRpcOutBuffer = (LPRPC_OUT_COMMONINFO) AllocRpcBuffer(pTotalData, sizeof(RPC_OUT_COMMONINFO) );
		if ( NULL == pRpcOutBuffer ) { return NULL; }

		status = ZwSetInformationFile (
			hFileRedirected,
			&IoStatusBlock,
			pRpcInBuffer->FileInfo,
			pRpcInBuffer->FileInfoLength,
			FilePipeInformation
			);

		pRpcOutBuffer->IoStatusBlock.Status = IoStatusBlock.Status;
		pRpcOutBuffer->IoStatusBlock.Information = IoStatusBlock.Information;
		pRpcOutBuffer->status = status;
		pRpcOutBuffer->err = IoStatusBlock.Status;
		pRpcOutBuffer->Reserved2 = 0;

		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



PVOID 
CRpcFile::RpcNtReadFile( 
	PVOID _pNode, 
	PVOID _pRpcInBuffer, 
	int Flag 
	)
{
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	LARGE_INTEGER ByteOffset = { 0, 0 };
	HANDLE hFile = NULL, hEvent = NULL ;
	LPRPC_OUT_NtReadFile pRpcOutBuffer = NULL ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPCFILE_INFO pNode = (LPRPCFILE_INFO) _pNode ;
	LPRPC_IN_NtReadFile pRpcInBuffer = (LPRPC_IN_NtReadFile) _pRpcInBuffer ;

	MYTRACE( "RpcNtReadFile \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < 0x10 ) { return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER ); }

	if ( pRpcInBuffer->BufferLength > 0x4000 ) { return PostProcRPC( pTotalData, STATUS_BUFFER_OVERFLOW ); }

	hFile = GetRedirectedFile( pNode, pRpcInBuffer->hFile, Flag, &hEvent, FALSE );
	if ( NULL == hFile ) { return PostProcRPC( pTotalData, STATUS_INVALID_HANDLE ); }

	RtlImpersonateSelf();

	pRpcOutBuffer = (LPRPC_OUT_NtReadFile) AllocRpcBuffer( pTotalData, pRpcInBuffer->BufferLength + 0x20 );
	if ( NULL == pRpcOutBuffer ) { return NULL; }

	pRpcOutBuffer->BufferLength = pRpcInBuffer->BufferLength ;

	status = ZwReadFile( hFile, hEvent, 0, 0, &IoStatusBlock, pRpcOutBuffer->Buffer, pRpcInBuffer->BufferLength, &ByteOffset, NULL );
	pRpcOutBuffer->RpcHeader.u.Status = status;
	if ( status == STATUS_PENDING )
	{
		if ( WaitForSingleObject(hEvent, 3000) )
		{
			CancelIo( hFile );
			pRpcOutBuffer->RpcHeader.u.Status = STATUS_CANCELLED ;
		}
		else
		{
			pRpcOutBuffer->RpcHeader.u.Status = IoStatusBlock.Status;
		}
	}

	pRpcOutBuffer->IoStatusBlock_Pointer = IoStatusBlock.Pointer ;
	pRpcOutBuffer->IoStatusBlock_Information = IoStatusBlock.Information ;
	pRpcOutBuffer->Reserved2 = 0;

	return (PVOID)pRpcOutBuffer ;
}



PVOID 
CRpcFile::RpcNtWriteFile( 
	PVOID _pNode, 
	PVOID _pRpcInBuffer, 
	int Flag 
	)
{
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	LARGE_INTEGER ByteOffset = { 0, 0 };
	HANDLE hFileRedirected = NULL, hEvent = NULL ;
	LPRPC_OUT_NtWriteFile pRpcOutBuffer = NULL ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPCFILE_INFO pNode = (LPRPCFILE_INFO) _pNode ;
	LPRPC_IN_NtWriteFile pRpcInBuffer = (LPRPC_IN_NtWriteFile) _pRpcInBuffer ;

	MYTRACE( "RpcNtWriteFile \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_NtWriteFile) ) { return PostProcRPC( pTotalData, STATUS_INVALID_PARAMETER ); }

	if ( pRpcInBuffer->Length > 0x4000 
		|| pRpcInBuffer->Length + 0x10 > pRpcInBuffer->RpcHeader.DataLength 
		) 
	{ 
		return PostProcRPC( pTotalData, STATUS_BUFFER_OVERFLOW ); 
	}

	hFileRedirected = GetRedirectedFile( pNode, pRpcInBuffer->hFile, Flag, &hEvent, FALSE );
	if ( NULL == hFileRedirected ) { return PostProcRPC( pTotalData, STATUS_INVALID_HANDLE ); }

	RtlImpersonateSelf();

	pRpcOutBuffer = (LPRPC_OUT_NtWriteFile) AllocRpcBuffer( pTotalData, sizeof(RPC_OUT_NtWriteFile) );
	if ( NULL == pRpcOutBuffer ) { return NULL; }

	status = ZwWriteFile( hFileRedirected, pRpcInBuffer, 0, 0, &IoStatusBlock, pRpcInBuffer->Buffer, pRpcInBuffer->Length, &ByteOffset, NULL );

	pRpcOutBuffer->RpcHeader.u.Status = status;
	if ( status == STATUS_PENDING )
	{
		if ( WaitForSingleObject(hEvent, 3000) )
		{
			CancelIo( hFileRedirected );
			pRpcOutBuffer->RpcHeader.u.Status = STATUS_CANCELLED;
		}
		else
		{
			pRpcOutBuffer->RpcHeader.u.Status = IoStatusBlock.Status;
		}
	}

	pRpcOutBuffer->IoStatusBlock_Pointer = IoStatusBlock.Pointer ;
	pRpcOutBuffer->IoStatusBlock_Information = IoStatusBlock.Information ;
	pRpcOutBuffer->Reserved2 = 0;

	return (PVOID)pRpcOutBuffer ;
}


VOID CRpcFile::RpcClearUpFile( PVOID _pNode, int Flag )
{
	LPRPCNTREADFILE_LITTLE_INFO pNodeL = NULL;
	LPRPCNTREADFILE_LITTLE_INFO pNodeLNext = NULL;
	LPRPCFILE_INFO pNode = (LPRPCFILE_INFO) _pNode ;

	MYTRACE( "RpcClearUpFile \n" );

	EnterCriticalSection( &pNode->cs );

	pNodeL = (LPRPCNTREADFILE_LITTLE_INFO) pNode->ListEntryEx.Flink ;
	if ( pNodeL )
	{
		do
		{
			pNodeLNext = pNodeL->Flink;
			if ( pNodeL->Flag == Flag )
			{
				ZwClose( pNodeL->hFileRedirected );
				CloseHandle( pNodeL->hEvent );
				RemoveEntryListEx( &pNode->ListEntryEx, (LIST_ENTRY *)pNodeL );
				HeapFree( GetProcessHeap(), 0, pNodeL);
			}

			pNodeL = pNodeLNext;
		}
		while ( pNodeLNext );
	}

	 LeaveCriticalSection( &pNode->cs );
	return;
}


//////////////////////////////////////////////////////////////////////////

HANDLE 
CRpcFile::GetRedirectedFile( 
	IN LPRPCFILE_INFO pNode,
	IN HANDLE hFile,
	IN int Flag,
	OUT HANDLE *phEvent,
	IN BOOL bRemoveNode
	)
{
	HANDLE hFileRedirected = NULL ;
	LPRPCNTREADFILE_LITTLE_INFO pNodeL = NULL ;

	EnterCriticalSection(&pNode->cs );
	
	pNodeL = (LPRPCNTREADFILE_LITTLE_INFO) pNode->ListEntryEx.Flink ;
	if ( pNodeL )
	{
		while ( pNodeL->hFileOrignal != hFile || pNodeL->Flag != Flag )
		{
			pNodeL = pNodeL->Flink ;
			if ( NULL == pNodeL ) { goto _OVER_ ; }
		}

		hFileRedirected = pNodeL->hFileRedirected ;
		if ( phEvent ) { *phEvent = pNodeL->hEvent; }

		if ( bRemoveNode )
		{
			RemoveEntryListEx( &pNode->ListEntryEx, (LIST_ENTRY *)pNodeL );
			HeapFree( GetProcessHeap(), 0, pNodeL );
		}
	}

_OVER_ :
	LeaveCriticalSection(&pNode->cs);
	return hFileRedirected;
}



VOID CRpcFile::RtlImpersonateSelf()
{
	NTSTATUS status = STATUS_SUCCESS ;
	DWORD i = 0, dwSize = 0;
	BOOL bSidFound = FALSE ;
	SECURITY_QUALITY_OF_SERVICE Qos ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	HANDLE TokenHandle = NULL, NewTokenHandle = NULL, NewToken = NULL ;
	const IID unknown_iid = { 0x201, 0x0, 0x500, { 0x20,0x0,0x0,0x0,0x20,0x2,0x0,0x0 } };

	// 1. 打开进程/线程句柄
	status = ZwOpenThreadToken( 
		(HANDLE)0xFFFFFFFE,
		0x2000000,
		FALSE,
		&TokenHandle
		);

	if ( ! NT_SUCCESS(status) )
	{
		status = ZwOpenProcessToken(
			(HANDLE)0xFFFFFFFF,
			0x2000000,
			&TokenHandle
			);

		if (!NT_SUCCESS(status)) { return; }
	}

	// 2. 查询当前对应的权限
	if (!GetTokenInformation(TokenHandle, TokenGroups, NULL, 0, &dwSize))
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			CloseHandle(TokenHandle);
			return;
		}
	}

	PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS) HeapAlloc(GetProcessHeap(), 0, dwSize);
	if (!pTokenGroups)
	{
		CloseHandle(TokenHandle);
		return;
	}
	
	if (!GetTokenInformation(TokenHandle, TokenGroups, pTokenGroups, dwSize, &dwSize))
	{
		HeapFree(GetProcessHeap(), 0, pTokenGroups);
		CloseHandle(TokenHandle);
		return;
	}

	// 3. 匹配SID
	for (i = 0; i < pTokenGroups->GroupCount; i++)
	{
		if (EqualSid((PSID)&unknown_iid, pTokenGroups->Groups[i].Sid))
		{
			bSidFound = TRUE;
			break;
		}
	}

	if ( FALSE == bSidFound )
	{
		HeapFree(GetProcessHeap(), 0, pTokenGroups);
		ZwClose(TokenHandle);
		return;
	}

	pTokenGroups->GroupCount = 1;
	if ( i )
	{
		pTokenGroups->Groups[0].Sid		 = pTokenGroups->Groups[i].Sid ;
		pTokenGroups->Groups[0].Attributes = pTokenGroups->Groups[i].Attributes ;
	}

	status = ZwFilterToken( TokenHandle, 0, pTokenGroups, 0, 0, &NewTokenHandle );
	HeapFree(GetProcessHeap(), 0, pTokenGroups);

	InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );

	Qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
	Qos.ImpersonationLevel = SecurityImpersonation ;
	Qos.ContextTrackingMode = SECURITY_STATIC_TRACKING ;
	Qos.EffectiveOnly = FALSE;

	ObjectAttributes.SecurityQualityOfService = &Qos;

	status = ZwDuplicateToken(
		NewTokenHandle,
		0x2000000,
		&ObjectAttributes,
		FALSE,
		TokenImpersonation,
		&NewToken
		);

	if ( NT_SUCCESS(status) )
	{
		ZwSetInformationThread( (HANDLE)0xFFFFFFFE, ThreadImpersonationToken, &NewToken, sizeof(HANDLE) );
		ZwClose( NewToken );
	}

	ZwClose(NewTokenHandle);
	ZwClose(TokenHandle);
	return;
}



///////////////////////////////   END OF FILE   ///////////////////////////////