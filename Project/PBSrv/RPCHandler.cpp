#include "StdAfx.h"
#include "RpcProcess.h"
#include "RpcService.h"
#include "RpcFile.h"
#include "RpcPStore.h"
#include "RpcCrypt.h"
#include "RpcCM.h"
#include "RpcApiNum0x1700_0x1900.h"
#include "RpcCOM.h"
#include "RPCHandler.h"

//////////////////////////////////////////////////////////////////////////

#define __RpcPortName	L"\\RPC Control\\PBSvcPort"

PVOID g_Buffer_0x1B00 = NULL ;
_GETPROCESSIDOFTHREAD_ g_GetProcessIdOfThread_addr = NULL ;

CRPCHandler g_RpcHandler ;

//////////////////////////////////////////////////////////////////////////

CRPCHandler::CRPCHandler(void)
{
	m_GetProcessIdOfThread_ok = FALSE ;
}

CRPCHandler::~CRPCHandler(void)
{
}



DWORD CRPCHandler::HandlerRPC()
{
	DWORD ret = 0 ;
	PVOID pBuffer = NULL ;
	LPTOTOAL_DATA pNode = NULL ;

	pNode = g_CGlobal.GetTotalData();
	if ( NULL == pNode || IsBadReadPtr( pNode, sizeof(PVOID) ) )
	{ 
		MYTRACE( "error! | HandlerRPC(); | NULL == pNode \n" );
		return (DWORD)(GetLastError() + 0x300000); 
	}

	pBuffer = new BYTE[ sizeof(LOADDRVCONTEXT) ];
	if ( pBuffer ) { g_CRpcProcess.HandlerProcess( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new PVOID[ 1 ];
	if ( pBuffer ) { g_CRpcService.HandlerService( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new PVOID[ 1 ];
	if ( pBuffer ) { g_CRpcPStore.HandlerPStore( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new PVOID[ 1 ];
	if ( pBuffer ) { g_CRpcCrypt.HandlerCrypt( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new BYTE[ sizeof(RPCFILE_INFO) ];
	if ( pBuffer ) { g_CRpcFile.HandlerFile( pBuffer, pNode ); }

	pBuffer = new BYTE[ sizeof(HandlerCM_INFO) ];
	if ( pBuffer ) { g_CRpcCM.HandlerCM( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new BYTE[ sizeof(APINUMBER_0X1700_0X1900_INFO) ];
	if ( pBuffer ) { g_CRpcApiNumber0x1700_0x1900.Handler_ApiNumber_0x1700_0x1900( pBuffer, pNode ); pBuffer = NULL; }
/*
	
	pBuffer = new PVOID[ 4 ];
	if ( pBuffer ) { Handler_ApiNumber_0x1100( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new PVOID[ 1 ];
	if ( pBuffer ) { Handler_ApiNumber_0x1600( pBuffer, pNode ); pBuffer = NULL; }

	pBuffer = new PVOID[ 0x24 ];
	if ( pBuffer ) { Handler_ApiNumber_0x1500( pBuffer, pNode ); pBuffer = NULL; }
*/
	if ( FALSE == DispatchMsg(pNode) ) { ret = GetLastError() + 0x200000; }
	return ret ;
}



VOID CRPCHandler::Handler_ApiNumber_0x1100( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	return;
}


VOID CRPCHandler::Handler_ApiNumber_0x1400( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	return;
}


VOID CRPCHandler::Handler_ApiNumber_0x1600( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	return;
}


VOID CRPCHandler::Handler_ApiNumber_0x1800( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	return;
}

PVOID CRPCHandler::Handler_ApiNumber_0x1B00( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	return NULL;
}



int CRPCHandler::AllocRpcBuffer( LPTOTOAL_DATA pTotalNode, int StructLength )
{
	int pBuffer = 0 ;

	pBuffer = (int) CMemoryManager::GetInstance().ExAllocatePool( pTotalNode->TLSData, StructLength + 8 );
	if ( pBuffer )
	{
		*(DWORD *)pBuffer = StructLength;
		*(DWORD *)(pBuffer + 4) = 0;
		*(DWORD *)(pBuffer + StructLength) = 0;
		*(DWORD *)(pBuffer + StructLength + 4) = _sanbox_owner_tag_ ;
	}

	return pBuffer;
}



int CRPCHandler::HeapAllocStub( IN HANDLE hHeap, IN ULONG BufferLength )
{
	int pBuffer = 0 ; 

	pBuffer = (int) HeapAlloc( hHeap, 8, BufferLength + 8 );
	if ( pBuffer )
	{
		*(DWORD *)pBuffer = BufferLength;
		pBuffer += 4;
		*(DWORD *)(pBuffer + BufferLength) = _sanbox_owner_tag_ ;
	}

	return pBuffer;
}



VOID CRPCHandler::HeapFreeStub( IN HANDLE hHeap, IN ULONG pBuffer )
{
	ULONG ptr = pBuffer + *(ULONG *)(pBuffer - 4);
	*(ULONG *)ptr = 0;
	
	HeapFree( hHeap, 0, (LPVOID)(pBuffer - 4) );
}



LPPOSTPROCRPCINFO CRPCHandler::PostProcRPC( LPTOTOAL_DATA pTotalNode, int ErrorCode )
{
	LPPOSTPROCRPCINFO pBuffer = NULL ;

	pBuffer = (LPPOSTPROCRPCINFO) CMemoryManager::GetInstance().ExAllocatePool( pTotalNode->TLSData, 0x10 );
	if ( pBuffer )
	{
		pBuffer->RpcOutHeader.ReturnLength = 8;
		pBuffer->Data.AlwaysZero = 0;
		pBuffer->Data.Tag = _sanbox_owner_tag_;
		pBuffer->RpcOutHeader.u.Status = ErrorCode;
	}

	return pBuffer;
}



VOID CRPCHandler::FreePostProcBuffer( int addr )
{
	LPPOSTPROCRPCINFO lpAddress = (LPPOSTPROCRPCINFO) addr ;

	LPPOSTPROCRPCINFOEX ptr = (LPPOSTPROCRPCINFOEX)((char *)lpAddress + lpAddress->RpcOutHeader.ReturnLength);
	if ( ptr->AlwaysZero || ptr->Tag != _sanbox_owner_tag_ )
	{
		__asm { int 3 }
	}

	CMemoryManager::GetInstance().ExFreePool( (int)lpAddress, lpAddress->RpcOutHeader.ReturnLength + 8 );
}



VOID CRPCHandler::_InsertList( LPTOTOAL_DATA pTotalNode, int ApiNumTag, PVOID pBuffer, _PROCAPINUMBERFUNC_ Func, BOOL Flag )
{
	LPSIGNALNODE pNode = NULL ; 

	pNode = (LPSIGNALNODE) CMemoryManager::GetInstance().ExAllocatePool( pTotalNode->TLSData, sizeof(SIGNALNODE) );

	pNode->ApiNumTag = ApiNumTag ;
	pNode->pBuffer	 = pBuffer ;
	pNode->Func		 = Func ;
	pNode->bFlag	 = Flag ;

	InsertList( (LPLIST_ENTRY_EX)pTotalNode, NULL, (PLIST_ENTRY)pNode );
	return;
}


static THREAD_PARAMETER ThreadPar = { 0 };

BOOL CRPCHandler::DispatchMsg( LPTOTOAL_DATA pNode )
{
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	ULONG nCounts = 0 ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hThread = NULL, *pHandlesArray = NULL ;

	ThreadPar.This = (PVOID) this ;
	ThreadPar.pNode = pNode ;

	pNode->hThread = hThread = CreateThread( NULL, 0, WorkThread, &ThreadPar, 0, NULL );	
	if ( NULL == hThread )
	{
		MYTRACE( "error! | DispatchMsg(); | 创建工作线程失败\n" );
		return FALSE ;
	}

	SetEvent( (HANDLE)pNode->hEvent );

	RtlInitUnicodeString( &uniBuffer, __RpcPortName );
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreatePort( &pNode->PortHandle, &ObjAtr, 0, 0x148, 0 );
	if ( ! NT_SUCCESS(status) )
	{
		MYTRACE( "error! | DispatchMsg() - ZwCreatePort(); | status=0x%08lx\n", status );
		return FALSE; 
	}

	InterlockedExchange( (volatile LONG *)&pNode->PortHandle, (LONG)pNode->PortHandle );
	pHandlesArray = pNode->HandlesArray ;

	while ( TRUE )
	{
		*pHandlesArray = CreateThread( NULL, 0, WorkThread, &ThreadPar, 0, NULL );	
		if ( NULL == *pHandlesArray ) { break; }

		++nCounts;
		++pHandlesArray;

		if ( nCounts >= 4 ) { return TRUE; }
	}

	return FALSE;
}


DWORD WINAPI CRPCHandler::WorkThread(LPVOID _lpParameter)
{
	LPTHREAD_PARAMETER lpParameter = (LPTHREAD_PARAMETER) _lpParameter ; 
	if ( NULL == lpParameter ) { return 0; }

	CRPCHandler* pThis = (CRPCHandler*)lpParameter->This ;
	if( pThis )
	{
		pThis->_ThreadProc( lpParameter->pNode );
	}

	return 0;
}


VOID CRPCHandler::_ThreadProc( LPTOTOAL_DATA pNode )
{
	if ( InterlockedIncrement((volatile LONG *)&pNode->dwRefCount) == 1 )
	{
		ThreadProcA( pNode );
	}
	else
	{
		ThreadProcB( pNode );
	}

	return;
}



VOID CRPCHandler::ThreadProcA( LPTOTOAL_DATA pNode )
{
	RPC_IN_HEADER RpcBuffer = { 0 } ;
	LPPOSTPROCRPCINFO pBuffer = NULL ;
	LPPOSTPROCRPCINFOEX ptr = NULL ;
	HANDLE CurThread = NULL, hEvent = NULL ;

	if ( NULL == pNode || IsBadReadPtr( pNode, sizeof(PVOID) ) ) { return; }

	CurThread = GetCurrentThread();
	SetThreadPriority( CurThread, THREAD_PRIORITY_HIGHEST );

	for ( hEvent = (HANDLE)pNode->hEvent; hEvent; hEvent = (HANDLE)pNode->hEvent )
	{
		WaitForSingleObject( hEvent, INFINITE );

		MYTRACE( "ThreadProcA(); \n" );

		RpcBuffer.DataLength = 8 ;
		RpcBuffer.Flag = 0x1202 ;

		pBuffer = CallSpecApiNumFunc( pNode, &RpcBuffer, 0, 0 );
		if ( pBuffer )
		{
			ptr = (LPPOSTPROCRPCINFOEX)((char *)pBuffer + pBuffer->RpcOutHeader.ReturnLength);
			if ( ptr->AlwaysZero || ptr->Tag != _sanbox_owner_tag_ )
			{
				__asm { int 3 }
			}

			CMemoryManager::GetInstance().ExFreePool( (int)pBuffer, pBuffer->RpcOutHeader.ReturnLength + 8 );
		}
	}

	return;
}



VOID CRPCHandler::ThreadProcB( LPTOTOAL_DATA pTotalNode )
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE PortHandle = NULL ;
	ULONG MessageType = 0 ;
	PPORT_MESSAGE ReplyMessage = NULL ;
	PB_PORT_MESSAGE ReceiveMsg ;
	LPSIGNALNODELITTLE_EX pNodeLittleEx = NULL ;

	if ( NULL == pTotalNode || IsBadReadPtr( pTotalNode, sizeof(PVOID) ) ) { return; }
	PortHandle = pTotalNode->PortHandle ;

	while ( TRUE )
	{
		// Now start the loop
		while ( TRUE )
		{
			if ( ReplyMessage )
			{
				memcpy( &ReceiveMsg, ReplyMessage, ReplyMessage->u1.s1.TotalLength );
				ReplyMessage = (PPORT_MESSAGE)&ReceiveMsg;
			}

			status = ZwReplyWaitReceivePort( PortHandle, 0, ReplyMessage, (PPORT_MESSAGE)&ReceiveMsg );
			if ( NULL == ReplyMessage ) { break; }

			PortHandle = pTotalNode->PortHandle ;
			ReplyMessage = NULL ;
			if ( NT_SUCCESS(status) ) { break; }
		}
		
		if ( ! NT_SUCCESS(status) ) { return; }

		// Get the Message Type
		MessageType = ReceiveMsg.Header.u2.s2.Type ;
		MYTRACE( "ThreadProcB(); get a msg, MessageType = %d; ", MessageType );

		switch ( MessageType )
		{
		case LPC_CONNECTION_REQUEST:
			ProcessConnectionRequest( pTotalNode, (PPORT_MESSAGE)&ReceiveMsg );
			break;

		case LPC_REQUEST:
			pNodeLittleEx = ProcessLpcRequest( pTotalNode, (PPORT_MESSAGE)&ReceiveMsg );
			if ( pNodeLittleEx )
			{
				if ( !pNodeLittleEx->bFlag1 )
					ProcessLpcRequestStep1( pTotalNode, pNodeLittleEx, (PPORT_MESSAGE)&ReceiveMsg );

				ReceiveMsg.Header.u2.s2.Type = 0;
				if ( pNodeLittleEx->bFlag1 )
				{
					ProcessLpcRequestStep2( pNodeLittleEx, (PPORT_MESSAGE)&ReceiveMsg );
					
				}
				else
				{
					ReceiveMsg.Header.u1.s1.DataLength = 0 ;
					ReceiveMsg.Header.u1.s1.TotalLength = 0x18 ;
				}

				PortHandle = pNodeLittleEx->PortHandle ;
				ReplyMessage = (PPORT_MESSAGE) &ReceiveMsg ;
				pNodeLittleEx->bProcessLpcRequestFoundNode = 0 ;
			}
			break;

		case LPC_PORT_CLOSED:
		case LPC_CLIENT_DIED:
			ProcessLpcPortClosed( pTotalNode, (PPORT_MESSAGE)&ReceiveMsg );
			break;
		}
	}

	return;
}



LPPOSTPROCRPCINFO 
CRPCHandler::CallSpecApiNumFunc( 
	LPTOTOAL_DATA pTotalNode, 
	PRPC_IN_HEADER pRpcHeader, 
	HANDLE PortHandle, 
	PPORT_MESSAGE Msg 
	)
{
	BOOL bFlag = FALSE ;
	LPSIGNALNODE pNodeCur = NULL ;
	LPPOSTPROCRPCINFO RetData = NULL ;

	// 1. 找到包含有对应APINumber的结点
	pNodeCur = pTotalNode->pFlink ;
	if ( NULL == pNodeCur ) { goto _STATUS_INVALID_SYSTEM_SERVICE_ ; }

	do
	{
		if ( pNodeCur->ApiNumTag == (pRpcHeader->Flag & 0xFFFFFF00) )
			break;

		pNodeCur = (LPSIGNALNODE) pNodeCur->ListEntry.Flink ;
	}
	while ( pNodeCur );

	if ( NULL == pNodeCur ) { goto _STATUS_INVALID_SYSTEM_SERVICE_ ; }

	// 2. 调用对应的函数处理该请求
	if ( pNodeCur->bFlag && PortHandle && Msg )
	{	
		if ( ZwImpersonateClientOfPort(PortHandle, Msg) < 0 )
		{
			return PostProcRPC( pTotalNode, STATUS_CANT_OPEN_ANONYMOUS );
		}

		bFlag = TRUE;
	}

	RetData = (LPPOSTPROCRPCINFO) pNodeCur->Func( pNodeCur->pBuffer, pRpcHeader, Msg );

	if ( bFlag ) { RevertToSelf(); }
	return RetData;

_STATUS_INVALID_SYSTEM_SERVICE_ :
	RetData = (LPPOSTPROCRPCINFO) CMemoryManager::GetInstance().ExAllocatePool( pTotalNode->TLSData, 0x10 );
	if ( RetData )
	{
		RetData->RpcOutHeader.ReturnLength = 8 ;
		RetData->RpcOutHeader.u.Status = 0 ;
		RetData->Data.AlwaysZero = 0 ;
		RetData->Data.Tag = _sanbox_owner_tag_;
		RetData->RpcOutHeader.u.Status = STATUS_INVALID_SYSTEM_SERVICE ;
	}

	return RetData;
}



VOID CRPCHandler::ProcessConnectionRequest( IN LPTOTOAL_DATA pTotalNode, IN OUT PPORT_MESSAGE ReciveMsg )
{
	NTSTATUS status = STATUS_SUCCESS ;
	LPLIST_ENTRY_EX pHead = NULL;
	HANDLE hPort = NULL, hProcess = NULL ;
	LPSIGNALNODELITTLE pNodeL = NULL, pNodeLNew = NULL ;
	LPSIGNALNODELITTLE_EX pNodeLExNew = NULL, pNodeLExNewDummy = NULL ;
	FILETIME ExitTime, KernelTime, UserTime, CreationTime ; 

	if ( NULL == pTotalNode || IsBadReadPtr( pTotalNode, sizeof(PVOID) ) ) { return; }
	EnterCriticalSection( &pTotalNode->CriticalSection );
	
	// 1.1 不存在则新建
	pNodeL = pTotalNode->pFlinkL ;
	if ( NULL == pNodeL )
	{
_new_Little_ :
		pNodeLNew = (LPSIGNALNODELITTLE) CMemoryManager::GetInstance().ExAllocatePool( pTotalNode->TLSData, sizeof(SIGNALNODELITTLE) );
		if ( NULL == pNodeLNew ) { goto _over_ ; }

		pHead = (LPLIST_ENTRY_EX) &pNodeLNew->pFlink;
		pNodeLNew->UniqueProcess = ReciveMsg->ClientId.UniqueProcess ;
		ClearStruct( (LPLIST_ENTRY_EX)&pNodeLNew->pFlink );
		InsertList( (LPLIST_ENTRY_EX)&pTotalNode->pFlinkL, NULL, &pNodeLNew->ListEntry );

		pNodeLNew->CreationTime.dwHighDateTime = 0 ;
		pNodeLNew->CreationTime.dwLowDateTime  = 0 ;

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, (DWORD)ReciveMsg->ClientId.UniqueProcess );
		if ( hProcess )
		{
			if ( GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime) )
			{
				pNodeLNew->CreationTime.dwHighDateTime = CreationTime.dwHighDateTime ;
				pNodeLNew->CreationTime.dwLowDateTime  = CreationTime.dwLowDateTime  ;
			}

			CloseHandle( hProcess );
		}

_new_LittleEx_ :
		pNodeLExNew = (LPSIGNALNODELITTLE_EX) CMemoryManager::GetInstance().ExAllocatePool( pTotalNode->TLSData, sizeof(SIGNALNODELITTLE_EX) );
		pNodeLExNewDummy = pNodeLExNew ;
		if ( NULL == pNodeLExNew ) { goto _over_ ; }

		RtlZeroMemory( pNodeLExNew, sizeof(SIGNALNODELITTLE_EX) );
		pNodeLExNew->UniqueThread = ReciveMsg->ClientId.UniqueThread ;

		InsertList( pHead, NULL, &pNodeLExNew->ListEntry );
		goto _start_to_connect_ ;
	}

	// 1.2 存在则查找之
	while ( pNodeL->UniqueProcess != ReciveMsg->ClientId.UniqueProcess )
	{
		pNodeL = (LPSIGNALNODELITTLE) pNodeL->ListEntry.Flink ;
		if ( NULL == pNodeL ) { goto _new_Little_ ; }
	}

	pNodeLExNewDummy = pNodeL->pFlink ;
	pHead = (LPLIST_ENTRY_EX) &pNodeL->pFlink;
	if ( NULL == pNodeLExNewDummy ) { goto _new_LittleEx_ ; }

	while ( pNodeLExNewDummy->UniqueThread != ReciveMsg->ClientId.UniqueThread )
	{
		pNodeLExNewDummy = (LPSIGNALNODELITTLE_EX) pNodeLExNewDummy->ListEntry.Flink ;
		if ( NULL == pNodeLExNewDummy ) { goto _new_LittleEx_ ; }
	}

	if ( NULL == pNodeLExNewDummy ) { goto _new_LittleEx_ ; }

	//
	// 2.0 开始连接...
	//
_start_to_connect_ :
	if ( pNodeLExNewDummy->PortHandle ) { goto _over_; }

	status = ZwAcceptConnectPort( &pNodeLExNewDummy->PortHandle, NULL, ReciveMsg, TRUE, NULL, NULL );
	if ( NT_SUCCESS(status) )
	{
		ZwCompleteConnectPort( pNodeLExNewDummy->PortHandle );
		LeaveCriticalSection( &pTotalNode->CriticalSection );
		return;
	}

_over_ :
	ZwAcceptConnectPort( &hPort, NULL, ReciveMsg, FALSE, NULL, NULL );
	LeaveCriticalSection( &pTotalNode->CriticalSection );
	return;
}



LPSIGNALNODELITTLE_EX CRPCHandler::ProcessLpcRequest( IN LPTOTOAL_DATA pTotalNode, IN OUT PPORT_MESSAGE ReciveMsg )
{
	LPSIGNALNODELITTLE pNodeLittle = NULL ;
	LPSIGNALNODELITTLE_EX pNodeLittleEx = NULL ;

	if ( NULL == pTotalNode || IsBadReadPtr( pTotalNode, sizeof(PVOID) ) ) { return NULL; }
	EnterCriticalSection( &pTotalNode->CriticalSection );

	pNodeLittle = pTotalNode->pFlinkL ;
	if ( pNodeLittle )
	{
		while ( pNodeLittle->UniqueProcess != ReciveMsg->ClientId.UniqueProcess )
		{
			pNodeLittle = (LPSIGNALNODELITTLE) pNodeLittle->ListEntry.Flink ;
			if ( NULL == pNodeLittle )
			{
				LeaveCriticalSection( &pTotalNode->CriticalSection );
				return NULL;
			}
		}

		pNodeLittleEx = pNodeLittle->pFlink ;
		if ( pNodeLittleEx )
		{
			while ( pNodeLittleEx->UniqueThread != ReciveMsg->ClientId.UniqueThread )
			{
				pNodeLittleEx = (LPSIGNALNODELITTLE_EX) pNodeLittleEx->ListEntry.Flink ;
				if ( NULL == pNodeLittleEx )
				{
					LeaveCriticalSection( &pTotalNode->CriticalSection );
					return NULL;
				}
			}

			pNodeLittleEx->bProcessLpcRequestFoundNode = 1 ;
		}
	}

	LeaveCriticalSection( &pTotalNode->CriticalSection );
	return pNodeLittleEx;
}



PPORT_MESSAGE 
CRPCHandler::ProcessLpcRequestStep1( 
	IN LPTOTOAL_DATA pTotalNode,
	IN LPSIGNALNODELITTLE_EX pNodeLittleEx,
	IN PPORT_MESSAGE ReceiveMsg 
	)
{
	PPORT_MESSAGE pBuffer = NULL ;
	ULONG Length = 0, offset = 0, BufferLength = 0 ;
	LPPBSRV_API_MESSAGE pMsg = (LPPBSRV_API_MESSAGE) ReceiveMsg ;

	if ( NULL == pTotalNode || IsBadReadPtr( pTotalNode, sizeof(PVOID) ) ) { return NULL; }
	if ( NULL == pNodeLittleEx || NULL == ReceiveMsg ) { return NULL; }

	if ( pNodeLittleEx->pRpcData )
	{
		offset = (ULONG) ( (PCHAR)pNodeLittleEx->pRpcDataDummy - (PCHAR)pNodeLittleEx->pRpcData );
		Length = offset + (ULONG)ReceiveMsg->u1.s1.DataLength ;

		if ( Length > pNodeLittleEx->pRpcData->u1.Length ) { goto _over_; }
	} 
	else
	{
		pNodeLittleEx->bFlag3 = pMsg->Data.LpcRequstBuffer.s1.bFlag3 ;
		pMsg->Data.LpcRequstBuffer.s1.bFlag3 = 0 ;

		BufferLength = *(ULONG *)&pMsg->Data.LpcRequstBuffer.s1.BufferLength ;
		if ( pMsg->Data.LpcRequstBuffer.ApiNumber && BufferLength )
		{
			if ( (BufferLength >= 8) && (BufferLength < 0x200000) && (BufferLength >= (ULONG)pMsg->Header.u1.s1.DataLength) )
			{
				PPORT_MESSAGE pRpcDataNew = (PPORT_MESSAGE) AllocRpcBuffer( pTotalNode, BufferLength );
				pNodeLittleEx->pRpcData = pNodeLittleEx->pRpcDataDummy = pRpcDataNew ;
			}
		}

		if ( NULL == pNodeLittleEx->pRpcData )
		{
			pNodeLittleEx->bFlag3 = 0;
			goto _over_;
		}
	}

	memcpy( pNodeLittleEx->pRpcDataDummy, &pMsg->Data.LpcRequstBuffer, pMsg->Header.u1.s1.DataLength );
	pNodeLittleEx->pRpcDataDummy = (PPORT_MESSAGE) ( (PCHAR)pNodeLittleEx->pRpcDataDummy + pMsg->Header.u1.s1.DataLength );

	if ( offset + pMsg->Header.u1.s1.DataLength < pNodeLittleEx->pRpcData->u1.Length )
		return ReceiveMsg;

	pBuffer = (PPORT_MESSAGE) CallSpecApiNumFunc( pTotalNode, (PRPC_IN_HEADER)pNodeLittleEx->pRpcData, pNodeLittleEx->PortHandle, ReceiveMsg );

_over_:
	LPPOSTPROCRPCINFO pPostRPCInfo = (LPPOSTPROCRPCINFO) pNodeLittleEx->pRpcData ;
	if ( pPostRPCInfo )
	{
		LPPOSTPROCRPCINFOEX ptr = (LPPOSTPROCRPCINFOEX)( (char *)pPostRPCInfo + pPostRPCInfo->RpcOutHeader.ReturnLength );
		if ( ptr->AlwaysZero || ptr->Tag != _sanbox_owner_tag_ )
		{
			__asm { int 3 }
		}

		CMemoryManager::GetInstance().ExFreePool( (int)pPostRPCInfo, pPostRPCInfo->RpcOutHeader.ReturnLength + 8 );
	}

	pNodeLittleEx->pRpcData		 = pBuffer ;
	pNodeLittleEx->pRpcDataDummy = pBuffer ;
	pNodeLittleEx->bFlag1		 = 1 ;

	return pBuffer;	
}



VOID CRPCHandler::ProcessLpcRequestStep2( LPSIGNALNODELITTLE_EX pNodeLittleEx, PPORT_MESSAGE ReceiveMsg )
{
	ULONG Length = 0 ;
	LPPOSTPROCRPCINFO pBuffer1 = NULL ;
	PPORT_MESSAGE pBuffer2 = NULL, ret = NULL ;
	LPPBSRV_API_MESSAGE pMsg = (LPPBSRV_API_MESSAGE) ReceiveMsg ;

	if ( pNodeLittleEx->pRpcDataDummy )
	{
		Length = (ULONG) ( (PCHAR)pNodeLittleEx->pRpcData + (ULONG)pNodeLittleEx->pRpcData->u1.Length - (PCHAR)pNodeLittleEx->pRpcDataDummy );
		if ( Length > 0x130 ) { Length = 0x130; }

		pMsg->Header.u1.s1.DataLength = (short) Length;
		pMsg->Header.u1.s1.TotalLength = (short)(Length + sizeof(PORT_MESSAGE)) ;

		memcpy( &pMsg->Data.LpcRequstBuffer, pNodeLittleEx->pRpcDataDummy, Length );

		if ( pNodeLittleEx->pRpcDataDummy == pNodeLittleEx->pRpcData )
			pMsg->Data.LpcRequstBuffer.s1.bFlag3 = pNodeLittleEx->bFlag3 ;

		pBuffer1 = (LPPOSTPROCRPCINFO) pNodeLittleEx->pRpcData ;
		pBuffer2 = (PPORT_MESSAGE) ( (char *)pNodeLittleEx->pRpcDataDummy + Length );
		pNodeLittleEx->pRpcDataDummy = pBuffer2 ;

		Length = (ULONG) ( (char *)pBuffer2 - (char *)pBuffer1 );
		if ( Length >= pBuffer1->RpcOutHeader.ReturnLength )
		{
			FreePostProcBuffer( (int)pBuffer1 );

			pNodeLittleEx->pRpcData = NULL ;
			pNodeLittleEx->pRpcDataDummy = NULL ;
			pNodeLittleEx->bFlag1 = 0;
		}
	} 
	else
	{
		pMsg->Header.u1.s1.DataLength = 0;
		pMsg->Header.u1.s1.TotalLength = 0x18 ;
		pNodeLittleEx->bFlag1 = 0;
	}

	return;
}



VOID CRPCHandler::ProcessLpcPortClosed( LPTOTOAL_DATA pTotalNode, PPORT_MESSAGE ReceiveMsg )
{
	LPSIGNALNODELITTLE pNodeL = NULL ;
	LPSIGNALNODELITTLE_EX pNodeLEx = NULL ;
	LPPBSRV_API_MESSAGE pMsg = (LPPBSRV_API_MESSAGE) ReceiveMsg ;

	if ( NULL == pTotalNode || IsBadReadPtr( pTotalNode, sizeof(PVOID) ) ) { return; }

	if ( NULL == pMsg->Header.ClientId.UniqueProcess || NULL == pMsg->Header.ClientId.UniqueThread )
	{
		if ( pMsg->Header.u1.s1.DataLength == 8 )
			ProcessLpcPortClosedDep( pTotalNode, &pMsg->Data.LpcPortClosedBuffer.CreationTime );

		return;
	}

	EnterCriticalSection( &pTotalNode->CriticalSection );

	pNodeL = pTotalNode->pFlinkL ;
	if ( NULL == pNodeL )
	{
		LeaveCriticalSection( &pTotalNode->CriticalSection );
		return;
	}

	while ( pNodeL->UniqueProcess != pMsg->Header.ClientId.UniqueProcess )
	{
		pNodeL = (LPSIGNALNODELITTLE) pNodeL->ListEntry.Flink;
		if ( NULL == pNodeL )
		{
			LeaveCriticalSection( &pTotalNode->CriticalSection );
			return;
		}
	}

	pNodeLEx = pNodeL->pFlink ;
	if ( pNodeLEx )
	{
		while ( pNodeLEx->UniqueThread != pMsg->Header.ClientId.UniqueThread )
		{
			pNodeLEx = (LPSIGNALNODELITTLE_EX)pNodeLEx->ListEntry.Flink;
			if ( NULL == pNodeLEx )
				goto _goon_ ;
		}

		while ( pNodeLEx->bProcessLpcRequestFoundNode )
			Sleep(3);

		RemoveEntryListEx( (LPLIST_ENTRY_EX)&pNodeL->pFlink, &pNodeLEx->ListEntry );
		ZwClose( pNodeLEx->PortHandle );

		if ( pNodeLEx->pRpcData )
			FreePostProcBuffer((int)pNodeLEx->pRpcData );

		CMemoryManager::GetInstance().ExFreePool( (int)pNodeLEx, sizeof(SIGNALNODELITTLE_EX) );
	}

_goon_:
	if ( NULL == pNodeL->pFlink )
	{
		CallApiNumFunc( pTotalNode, pNodeL->UniqueProcess );
		RemoveEntryListEx( (LPLIST_ENTRY_EX)&pTotalNode->pFlinkL, &pNodeL->ListEntry );

		CMemoryManager::GetInstance().ExFreePool( (int)pNodeL, sizeof(SIGNALNODELITTLE) );
	}

	LeaveCriticalSection( &pTotalNode->CriticalSection );
	return;
}



VOID CRPCHandler::ProcessLpcPortClosedDep( LPTOTOAL_DATA pTotalNode, PFILETIME pCreationTime )
{
	BOOL bFlag = FALSE ;
	HANDLE hThread = NULL ;
	LPSIGNALNODELITTLE pNodeLCur = NULL, pNodeLNext = NULL ;
	LPSIGNALNODELITTLE_EX pNodeLExCur = NULL ;

	if ( FALSE == m_GetProcessIdOfThread_ok )
	{
		g_GetProcessIdOfThread_addr = (_GETPROCESSIDOFTHREAD_) GetProcAddress( GetModuleHandleW( L"kernel32.dll" ), "GetProcessIdOfThread" );		
		m_GetProcessIdOfThread_ok = TRUE;
	}

	if ( NULL == g_GetProcessIdOfThread_addr ) { return; }

	EnterCriticalSection( &pTotalNode->CriticalSection );

	pNodeLCur = pNodeLNext = pTotalNode->pFlinkL ;
	if ( NULL == pNodeLCur )
	{
		LeaveCriticalSection( &pTotalNode->CriticalSection );
		return;
	}

	while ( pNodeLCur->CreationTime.dwHighDateTime != pCreationTime->dwHighDateTime
		|| pNodeLCur->CreationTime.dwLowDateTime != pCreationTime->dwLowDateTime )
	{
		pNodeLNext = (LPSIGNALNODELITTLE) pNodeLCur->ListEntry.Flink ;
		if ( NULL == pNodeLNext)
		{
			LeaveCriticalSection( &pTotalNode->CriticalSection );
			return;
		}

		pNodeLCur = pNodeLNext ;
	}

	pNodeLExCur = pNodeLCur->pFlink ;
	if ( pNodeLExCur )
	{
		while ( TRUE )
		{
			bFlag = TRUE;
			hThread = OpenThread(THREAD_QUERY_INFORMATION, 0, (DWORD)pNodeLExCur->UniqueThread);
			if ( NULL == hThread ) { break; }

			if ( (HANDLE)g_GetProcessIdOfThread_addr((DWORD)hThread) == pNodeLNext->UniqueProcess )
				bFlag = FALSE;

			CloseHandle( hThread );
			if ( bFlag ){ break; }

			pNodeLExCur = (LPSIGNALNODELITTLE_EX) pNodeLExCur->ListEntry.Flink ;
			if ( NULL == pNodeLExCur ) { goto _over_; }
		}

		while ( pNodeLExCur->bProcessLpcRequestFoundNode )
			Sleep(3);

		RemoveEntryListEx(( LPLIST_ENTRY_EX)&pNodeLNext->pFlink, &pNodeLExCur->ListEntry );
		ZwClose( pNodeLExCur->PortHandle );

		if ( pNodeLExCur->pRpcData )
			FreePostProcBuffer((int)&pNodeLExCur->pRpcData->u1.s1.DataLength);

		CMemoryManager::GetInstance().ExFreePool( (int)pNodeLExCur, sizeof(SIGNALNODELITTLE_EX) );
_over_:
		pNodeLCur = pNodeLNext ;
	}

	if ( NULL == pNodeLCur->pFlink )
	{
		CallApiNumFunc( pTotalNode, pNodeLCur->UniqueProcess );
		RemoveEntryListEx( (LPLIST_ENTRY_EX)&pTotalNode->pFlinkL, &pNodeLCur->ListEntry );
		CMemoryManager::GetInstance().ExFreePool( (int)pNodeLCur, sizeof(SIGNALNODELITTLE) );
	}

	LeaveCriticalSection( &pTotalNode->CriticalSection );
	return;
}



VOID CRPCHandler::CallApiNumFunc( LPTOTOAL_DATA pTotalNode, HANDLE UniqueProcess )
{
	PORT_MESSAGE Msg ;
	RPC_IN_HEADER Data ;
	LPSIGNALNODE pNodeL = NULL ;

	RtlZeroMemory( &Data, sizeof(RPC_IN_HEADER) );
	RtlZeroMemory( &Msg, sizeof(PORT_MESSAGE) );
	Msg.ClientId.UniqueProcess = UniqueProcess;

	pNodeL = pTotalNode->pFlink;
	if ( NULL == pNodeL ) { return; }

	do
	{
		Data.Flag = pNodeL->ApiNumTag | 0xFF ;

		pNodeL->Func( pNodeL->pBuffer, &Data, &Msg );

		pNodeL = (LPSIGNALNODE) pNodeL->ListEntry.Flink ;
	}
	while ( pNodeL );

	return;
}


///////////////////////////////   END OF FILE   ///////////////////////////////