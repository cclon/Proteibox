/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \Code\Project\PBSrv\PBSrv\RpcApiNum0x1B00.cpp
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
#include "RpcCOM.h"


//////////////////////////////////////////////////////////////////////////

CRpcCOM g_CRpcCOM ;
PVOID g_Buffer_COM = NULL;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

static PVOID __ProcCOM( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, int Msg )
{
	return g_CRpcCOM.ProcCOM( pInfo, pRpcBuffer, Msg );
}


CRpcCOM::CRpcCOM(void):
m_PID_COM(0)
{
	
}

CRpcCOM::~CRpcCOM(void)
{
}


PVOID CRpcCOM::HandlerCOM( PVOID _pBuffer, LPTOTOAL_DATA pNode )
{
	LPCOMCONTEXT pBuffer = (LPCOMCONTEXT) _pBuffer;

	if ( NULL == pNode || NULL == pBuffer ) { return NULL; }

	pBuffer->hEvent = CreateEventW(0, 0, 0, 0);
	InitializeCriticalSection(&pBuffer->cs);
	ClearStruct(&pBuffer->ListEntryEx);

	_InsertList( pNode, 0x1B00, pBuffer, (_PROCAPINUMBERFUNC_)__ProcCOM, TRUE );

	m_PID_COM = (int) OpenProcess( 0x100000, TRUE, GetCurrentProcessId() );
	return pBuffer;
}


PVOID 
CRpcCOM::ProcCOM(
	IN PVOID _pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	int Flag = pRpcBuffer->Flag;
	if ( Flag == 0x1BFF )
	{
		//
		//
		//

		return NULL;
	}

	LPPOSTPROCRPCINFO ret = NULL ;
	LPCOMCONTEXT pInfo = (LPCOMCONTEXT) _pInfo;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPPROCCOM_DATA pNode = GetCOMData( (LPCOMCONTEXT)pInfo, *(DWORD *)(Msg + 8), Flag );
	if ( NULL == pNode )
	{
		SetEvent(pInfo->hEvent);
		return PostProcRPC( pTotalData, RPC_S_SERVER_UNAVAILABLE );
	}

	int Reserved = 0;
	switch (Flag)
	{
	case _PBSRV_APINUM_HandlerWhiteCLSID_:
		ret = (LPPOSTPROCRPCINFO)RpcHandlerWhiteCLSID( pInfo, pNode, pRpcBuffer, &Reserved );
		break;

	case _PBSRV_APINUM_CoUnmarshalInterface_:
		ret = (LPPOSTPROCRPCINFO)RpcCoUnmarshalInterface( pInfo, pNode, pRpcBuffer, &Reserved );
		break;

	case _PBSRV_APINUM_CoMarshalInterface_:
		ret = (LPPOSTPROCRPCINFO)RpcCoMarshalInterface( pInfo, pNode, pRpcBuffer, &Reserved );
		break;

	default:
		break;
	}

	return (PVOID)ret;
}


PVOID CRpcCOM::RpcHandlerWhiteCLSID( PVOID _pInfo, PVOID _pNode, PVOID _pRpcInBuffer, int* Flag )
{
	BOOL bRet = FALSE;
	NTSTATUS status = STATUS_SUCCESS ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPCOMCONTEXT pInfo = (LPCOMCONTEXT) _pInfo;
	LPPROCCOM_DATA pNode = (LPPROCCOM_DATA)_pNode;
	LPRPC_IN_HandlerWhiteCLSID pRpcInBuffer = (LPRPC_IN_HandlerWhiteCLSID) _pRpcInBuffer;

	MYTRACE( "RpcHandlerWhiteCLSID \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_HandlerWhiteCLSID) ) 
		return PostProcRPC( pTotalData, E_INVALIDARG );

	LPMappedAddressInfo MappedAddress = pNode->MappedAddress;
	MappedAddress->Length = 0x20;
	memcpy( &MappedAddress->rclsid1, &pRpcInBuffer->rclsidCur, 0x10 );
	memcpy( &MappedAddress->rclsid2, &pRpcInBuffer->rclsid_IClassFactory, 0x10 );

	bRet = PB_IsOpenClsid( &MappedAddress->rclsid1, CLSCTX_LOCAL_SERVER, TRUE );
	if ( FALSE == bRet )
		return PostProcRPC( pTotalData, RPC_E_ACCESS_DENIED );

	int err = WaitForCOMProc( pInfo, pNode, 0x31, Flag );
	if ( err )
		return PostProcRPC( pTotalData, err );

	LPPOSTPROCRPCINFO pRpcOutBuffer = (LPPOSTPROCRPCINFO) AllocRpcBuffer( pTotalData, 0x10);
	if ( pRpcOutBuffer )
	{
		pRpcOutBuffer->Data.AlwaysZero = MappedAddress->PID;
		pRpcOutBuffer->Data.Tag = MappedAddress->CoMarshalInterface_Flag;
	}

	return pRpcOutBuffer;
}


PVOID CRpcCOM::RpcCoUnmarshalInterface( PVOID _pInfo, PVOID _pNode, PVOID _pRpcInBuffer, int* Flag )
{
	BOOL bRet = FALSE;
	NTSTATUS status = STATUS_SUCCESS ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPCOMCONTEXT pInfo = (LPCOMCONTEXT) _pInfo;
	LPPROCCOM_DATA pNode = (LPPROCCOM_DATA)_pNode;
	LPRPC_IN_CoUnmarshalInterface pRpcInBuffer = (LPRPC_IN_CoUnmarshalInterface) _pRpcInBuffer;

	MYTRACE( "RpcCoUnmarshalInterface \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < 0x20 ) 
		return PostProcRPC( pTotalData, E_INVALIDARG );

	if ( pRpcInBuffer->objrefDummy_TotalLength >= 0x400 ) 
		return PostProcRPC( pTotalData, MEM_E_INVALID_SIZE );

	if ( pRpcInBuffer->objrefDummy_TotalLength + 0x1C > pRpcInBuffer->RpcHeader.DataLength )
		return PostProcRPC( pTotalData, E_INVALIDARG );

	LPMappedAddressInfo MappedAddress = pNode->MappedAddress;
	MappedAddress->Length = pRpcInBuffer->objrefDummy_TotalLength + 0x10;
	memcpy( &MappedAddress->rclsid1, &pRpcInBuffer->riid, 0x10 );
	memcpy( &MappedAddress->rclsid2, &pRpcInBuffer->objref, pRpcInBuffer->objrefDummy_TotalLength );

	int err = WaitForCOMProc( pInfo, pNode, 0x36, Flag );
	if ( err )
		return PostProcRPC( pTotalData, err );

	LPPOSTPROCRPCINFO pRpcOutBuffer = (LPPOSTPROCRPCINFO) AllocRpcBuffer( pTotalData, 0x10);
	if ( pRpcOutBuffer )
	{
		pRpcOutBuffer->Data.AlwaysZero = MappedAddress->PID;
		pRpcOutBuffer->Data.Tag = MappedAddress->CoMarshalInterface_Flag;
	}

	return pRpcOutBuffer;
}


PVOID CRpcCOM::RpcCoMarshalInterface( PVOID _pInfo, PVOID _pNode, PVOID _pRpcInBuffer, int* Flag )
{
	BOOL bRet = FALSE;
	NTSTATUS status = STATUS_SUCCESS ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPCOMCONTEXT pInfo = (LPCOMCONTEXT) _pInfo;
	LPPROCCOM_DATA pNode = (LPPROCCOM_DATA)_pNode;
	LPRPC_IN_CoMarshalInterface pRpcInBuffer = (LPRPC_IN_CoMarshalInterface) _pRpcInBuffer;

	MYTRACE( "RpcCoMarshalInterface \n" );

	if ( pRpcInBuffer->RpcHeader.DataLength < 0x24 ) 
		return PostProcRPC( pTotalData, E_INVALIDARG );

	LPMappedAddressInfo MappedAddress = pNode->MappedAddress;
	MappedAddress->CoMarshalInterface_Flag			= pRpcInBuffer->Flag;
	MappedAddress->CoMarshalInterface_dwDestContext = pRpcInBuffer->dwDestContext;
	MappedAddress->CoMarshalInterface_mshlFlags		= pRpcInBuffer->mshlFlags;
	MappedAddress->Length							= 0x10;
	memcpy( &MappedAddress->rclsid1, &pRpcInBuffer->riid, 0x10 );

	int err = WaitForCOMProc( pInfo, pNode, 0x37, Flag );
	if ( err )
		return PostProcRPC( pTotalData, err );

	LPRPC_OUT_CoMarshalInterface pRpcOutBuffer = (LPRPC_OUT_CoMarshalInterface) AllocRpcBuffer( pTotalData, pNode->MappedAddress->Length + 0x14);
	if ( pRpcOutBuffer )
	{
		pRpcOutBuffer->MappedAddress_PID = MappedAddress->PID;
		pRpcOutBuffer->MappedAddress_Length = MappedAddress->Length;
		memcpy(&pRpcOutBuffer->riid, &MappedAddress->rclsid1, MappedAddress->Length);
	}

	return pRpcOutBuffer;
}


int CRpcCOM::WaitForCOMProc(LPCOMCONTEXT pInfo, LPPROCCOM_DATA pNode, int Flag, int *PID)
{
	SetEvent(pNode->hEvent_ComProxy1);

	HANDLE handles[2];
	handles[0] = pNode->hProcess;
	handles[1] = pNode->hEvent_ComProxy2;

	int err = WaitForMultipleObjects( 2, handles, FALSE, 10000 );
	if ( Flag == 0x5F )
		return 0;

	if ( err == 1 )
		return (int)pNode->MappedAddress->Flag;

	EnterCriticalSection(&pInfo->cs);
	RemoveNode_COM( pInfo, pNode );
	LeaveCriticalSection(&pInfo->cs);

	if (PID) { *PID = 1;}
	return RPC_S_SERVER_UNAVAILABLE;
}


BOOL CRpcCOM::RemoveNode_COM(LPCOMCONTEXT pInfo, LPPROCCOM_DATA pNode)
{
	RemoveEntryListEx( &pInfo->ListEntryEx, &pNode->ListEntry );

	if ( pNode->hProcess )
	{
		if ( pNode->hEvent_ComProxy1 && pNode->hEvent_ComProxy2 && pNode->MappedAddress )
		{
			pNode->MappedAddress->Flag = 0xFFFFFFFF;
			WaitForCOMProc( pInfo, pNode, 0x5F, NULL );
		}

		TerminateProcess(pNode->hProcess, 0);
		CloseHandle(pNode->hProcess);
	}

	if ( pNode->MappedAddress )
		UnmapViewOfFile(pNode->MappedAddress);
	if ( pNode->hFileMapping )
		CloseHandle(pNode->hFileMapping);
	if ( pNode->hEvent_ComProxy2 )
		CloseHandle(pNode->hEvent_ComProxy2);
	if ( pNode->hEvent_ComProxy1 )
		CloseHandle(pNode->hEvent_ComProxy1);
	if ( LOBYTE(pNode->Flag) )
		DeleteCriticalSection(&pNode->cs);

	return HeapFree(GetProcessHeap(), 0, pNode);
}


LPPROCCOM_DATA CRpcCOM::GetCOMData(LPCOMCONTEXT pInfo, int dwProcessId, int Flag)
{
	BOOL bNewNode = FALSE;
	PB_BOX_INFO BoxInfo = {} ;

	if ( FALSE == PB_QueryProcess( dwProcessId, &BoxInfo ) )
		return NULL;

	LPPROCCOM_DATA pNodeCur = NULL ;
	UINT TickCount = GetTickCount();
	while (TRUE)
	{
		EnterCriticalSection(&pInfo->cs);

		pNodeCur = (LPPROCCOM_DATA)pInfo->ListEntryEx.Flink;
		if ( NULL == pNodeCur )
			break;

		while ( wcsicmp(pNodeCur->sid, BoxInfo.SID) || wcsicmp(pNodeCur->szBoxName, BoxInfo.BoxName) || pNodeCur->SessionId != BoxInfo.SessionId )
		{
			pNodeCur = (LPPROCCOM_DATA)pNodeCur->ListEntry.Flink;
			if ( NULL == pNodeCur )
			{
				bNewNode = TRUE;
				break;
			}
		}

		if (bNewNode)
			break;

		if ( TryEnterCriticalSection(&pNodeCur->cs) )
		{
			if ( WaitForSingleObject(pNodeCur->hProcess, 0) )
				goto _over_;

			RemoveNode_COM( pInfo, pNodeCur );
			pNodeCur = NULL;
			break;
		}

		LeaveCriticalSection(&pInfo->cs);
		if ( GetTickCount() - TickCount >= 60000 || WaitForSingleObject(pInfo->hEvent, 10000) )
			return NULL;
	}

	if ( (_PBSRV_APINUM_HandlerWhiteCLSID_ != Flag) && (0x1B06 != Flag) )
		goto _over_;

	int err = 0;
	LPWSTR lpReserved = NULL;
	STARTUPINFOW StartupInfo = {} ;
	HANDLE hToken = NULL, hExistingToken = NULL;
	do 
	{
		pNodeCur = (LPPROCCOM_DATA)HeapAlloc( GetProcessHeap(), 8, sizeof(PROCCOM_DATA) );
		if ( NULL == pNodeCur )
		{
			err = ERROR_NOT_SAME_DEVICE;
			break;
		}

		WCHAR Buffer[MAX_PATH] = {};

		wcscpy( pNodeCur->sid, BoxInfo.SID );
		wcscpy( pNodeCur->szBoxName, BoxInfo.BoxName );
		pNodeCur->SessionId = BoxInfo.SessionId;
		InitializeCriticalSection(&pNodeCur->cs);
		pNodeCur->Flag = 1;

		wsprintfW( Buffer, L"Proteinbox_ComProxy_%s_%s_%d_%s", pNodeCur->sid, pNodeCur->szBoxName, pNodeCur->SessionId, L"Event1" );
		pNodeCur->hEvent_ComProxy1 = CreateEventW( NULL, FALSE, FALSE, Buffer );
		if ( NULL == pNodeCur->hEvent_ComProxy1 )
		{
			err = ERROR_WRITE_PROTECT;
			break;
		}

		wsprintfW( Buffer, L"Proteinbox_ComProxy_%s_%s_%d_%s", pNodeCur->sid, pNodeCur->szBoxName, pNodeCur->SessionId, L"Event2" );
		pNodeCur->hEvent_ComProxy2 = CreateEventW( NULL, FALSE, FALSE, Buffer );
		if ( NULL == pNodeCur->hEvent_ComProxy2 )
		{
			err = ERROR_BAD_UNIT;
			break;
		}

		wsprintfW( Buffer, L"Proteinbox_ComProxy_%s_%s_%d_%s", pNodeCur->sid, pNodeCur->szBoxName, pNodeCur->SessionId, L"Map" );
		pNodeCur->hFileMapping = CreateFileMappingW( (HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 0x200000, Buffer );
		if ( NULL == pNodeCur->hFileMapping )
		{
			err = ERROR_NOT_READY;
			break;
		}

		pNodeCur->MappedAddress = (LPMappedAddressInfo) MapViewOfFile( pNodeCur->hFileMapping, 0xF001F, 0, 0, 0x200000 );
		if ( NULL == pNodeCur->MappedAddress )
		{
			err = ERROR_BAD_COMMAND;
			break;
		}

		wsprintfW( Buffer, L"Proteinbox_ComProxy_%s_%s_%d_%s:%d", pNodeCur->sid, pNodeCur->szBoxName, pNodeCur->SessionId, m_PID_COM );
		BOOL bRet = OpenThreadToken( GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, FALSE, &hExistingToken );
		if ( FALSE == bRet )
		{
			err = ERROR_BAD_LENGTH;
			break;
		}

		bRet = DuplicateTokenEx( hExistingToken, 0, 0, SecurityImpersonation, TokenPrimary, &hToken );
		if ( FALSE == bRet )
		{
			err = ERROR_SEEK;
			break;
		}

		memset( &StartupInfo, 0, sizeof(StartupInfo) );
		StartupInfo.lpReserved = 0;
		bRet = PB_RunFromHome( L"PBSvc.exe", Buffer, &StartupInfo, 0 );
		if ( FALSE == bRet )
		{
			err = ERROR_SHARING_VIOLATION;
			break;
		}

		PROCESS_INFORMATION ProcessInformation = {} ;
		lpReserved = StartupInfo.lpReserved;
		memset( &StartupInfo, 0, sizeof(StartupInfo) );
		StartupInfo.dwFlags = 0x80;

		bRet = CreateProcessAsUserW( hToken, 0, lpReserved, 0, 0, 1, 0, 0, 0, &StartupInfo, &ProcessInformation );
		if ( bRet )
		{
			CloseHandle(ProcessInformation.hThread);
			pNodeCur->hProcess = ProcessInformation.hProcess;
		}
		else
		{
			err = ERROR_LOCK_VIOLATION;
		}
	} while (FALSE);
	
	if ( lpReserved )
		HeapFree( GetProcessHeap(), 0, lpReserved );
	if ( hToken )
		CloseHandle(hToken);
	if ( hExistingToken )
		CloseHandle(hExistingToken);
	if ( err )
	{
		RemoveNode_COM(pInfo, pNodeCur);
		pNodeCur = NULL;
	}
	else
	{
		EnterCriticalSection(&pNodeCur->cs);
		InsertList( &pInfo->ListEntryEx, NULL, &pNodeCur->ListEntry );
	}

_over_:
	LeaveCriticalSection(&pInfo->cs);
	if ( pNodeCur )
	{
		RtlZeroMemory( pNodeCur->MappedAddress, sizeof(MappedAddressInfo) );
		pNodeCur->MappedAddress->Flag = Flag;
		pNodeCur->MappedAddress->PID = dwProcessId;
	}

	 return pNodeCur;
}


///////////////////////////////   END OF FILE   ///////////////////////////////