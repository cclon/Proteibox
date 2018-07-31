/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \SandBox\Code\Project\PBSrv\RpcApiNum0x1300.cpp
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
#include <pstore.h>
#include "RpcPStore.h"


//////////////////////////////////////////////////////////////////////////

CRpcPStore g_CRpcPStore ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __ProcPStore( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, int Msg )
{
	return g_CRpcPStore.ProcPStore( pInfo, pRpcBuffer, Msg );
}



CRpcPStore::CRpcPStore(void)
{
	
}

CRpcPStore::~CRpcPStore(void)
{
}



VOID CRpcPStore::HandlerPStore( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	*(DWORD *)pBuffer = 0;
	_InsertList( pNode, 0x1100, pBuffer, (_PROCAPINUMBERFUNC_)__ProcPStore, TRUE );

	QueueUserWorkItem( (LPTHREAD_START_ROUTINE)PStoreThread, pBuffer, WT_EXECUTELONGFUNCTION );
	return;
}


int WINAPI CRpcPStore::PStoreThread( LPVOID param )
{
	typedef HRESULT (WINAPI *_PStoreCreateInstance_)(IPStore** ppProvider, PST_PROVIDERID* pProviderID, void* pReserved, DWORD dwFlags);

	Sleep( 1000 );
	HMODULE hModule = LoadLibraryW( L"pstorec.dll" );
	if ( NULL == hModule ) { return 0; }
	
	_PStoreCreateInstance_ PStoreCreateInstance = (_PStoreCreateInstance_) GetProcAddress(hModule, "PStoreCreateInstance");
	if ( PStoreCreateInstance )
	{
		IPStore* pProvider;

		if ( PStoreCreateInstance(&pProvider, 0, 0, 0) >= 0 )
			InterlockedExchange( (volatile LONG *)param, (ULONG)pProvider );
	}

	return 0;
}



PVOID 
CRpcPStore::ProcPStore(
	IN PVOID pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	LPPOSTPROCRPCINFO ret = NULL ;

	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_PStoreGetNode_ :
		ret = RpcPStoreGetNode( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_PStoreGetChildNode_ :
		ret = RpcPStoreGetChildNode( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_PStoreGetGrandchildNode_ :
		ret = RpcPStoreGetGrandchildNode( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_PStoreEnumSubtypes_ :
		ret = RpcPStoreEnumSubtypes( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_PStoreEnumItems_ :
		ret = RpcPStoreEnumItems( pInfo, pRpcBuffer );
		break;

	default:
		break;
	}

	 return (PVOID)ret;
}



LPPOSTPROCRPCINFO CRpcPStore::RpcPStoreGetNode( PVOID pInfo, PVOID _pRpcInBuffer)
{
	int err = PST_E_TYPE_NO_EXISTS ;
	IPStore **ppIPStore = (IPStore **)pInfo;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_PBIPStoreGetNode pRpcInBuffer = (LPRPC_IN_PBIPStoreGetNode) _pRpcInBuffer;

	MYTRACE( "RpcPStoreGetNode \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength != sizeof(RPC_IN_PBIPStoreGetNode) )
		{
			err = E_INVALIDARG;
			break;
		}

		IPStore *pIPStore = *ppIPStore ;		
		if ( NULL == pIPStore )
			break;
		
		PPST_TYPEINFO pTemp ;
		err = pIPStore->GetTypeInfo( 0, &pRpcInBuffer->iid_data, &pTemp, 0 );
		if ( err )
		{
			err = pIPStore->GetTypeInfo( 1, &pRpcInBuffer->iid_data, &pTemp, 0 );
			if ( err < 0 ) { break; }
		}

		int length = 2 * wcslen(pTemp->szDisplayName) + 0x16;
		LPRPC_OUT_PBIPStoreGetNode pRpcOutBuffer = (LPRPC_OUT_PBIPStoreGetNode) AllocRpcBuffer( pTotalData, length );
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->Reserved = 0;
			pRpcOutBuffer->NameLength = wcslen(pTemp->szDisplayName);
			wcscpy( pRpcOutBuffer->szDisplayName, pTemp->szDisplayName );
		}

		CoTaskMemFree( pTemp );
		return (LPPOSTPROCRPCINFO)pRpcOutBuffer;

	} while (FALSE);
	
	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO 
CRpcPStore::RpcPStoreGetChildNode( 
	IN PVOID pInfo, 
	IN PVOID _pRpcInBuffer
	)
{
	int err = PST_E_TYPE_NO_EXISTS ;
	IPStore **ppIPStore = (IPStore **)pInfo;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_PBIPStoreGetChildNode pRpcInBuffer = (LPRPC_IN_PBIPStoreGetChildNode) _pRpcInBuffer;

	MYTRACE( "RpcPStoreGetChildNode \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength != sizeof(RPC_IN_PBIPStoreGetChildNode) )
		{
			err = E_INVALIDARG;
			break;
		}

		IPStore *pIPStore = *ppIPStore ;		
		if ( NULL == pIPStore )
			break;

		PPST_TYPEINFO pTemp ;
		err = pIPStore->GetSubtypeInfo( 0, &pRpcInBuffer->iid_data, &pRpcInBuffer->iid_data_sun, &pTemp, 0 );
		if ( err )
		{
			err = pIPStore->GetSubtypeInfo( 1, &pRpcInBuffer->iid_data, &pRpcInBuffer->iid_data_sun, &pTemp, 0 );
			if ( err < 0 ) { break; }
		}

		int length = 2 * wcslen(pTemp->szDisplayName) + 0x16;
		LPRPC_OUT_PBIPStoreGetChildNode pRpcOutBuffer = (LPRPC_OUT_PBIPStoreGetChildNode) AllocRpcBuffer( pTotalData, length );
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->Reserved = 0;
			pRpcOutBuffer->NameLength = wcslen(pTemp->szDisplayName);
			wcscpy( pRpcOutBuffer->szDisplayName, pTemp->szDisplayName );
		}

		CoTaskMemFree( pTemp );
		return (LPPOSTPROCRPCINFO)pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO 
CRpcPStore::RpcPStoreGetGrandchildNode( 
	IN PVOID pInfo, 
	IN PVOID _pRpcInBuffer
	)
{
	BOOL bRet = FALSE ;
	int err = PST_E_TYPE_NO_EXISTS ;
	IPStore **ppIPStore = (IPStore **)pInfo;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_PBIPStoreGetGrandchildNode pRpcInBuffer = (LPRPC_IN_PBIPStoreGetGrandchildNode) _pRpcInBuffer;

	MYTRACE( "RpcPStoreGetGrandchildNode \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_PBIPStoreGetGrandchildNode)
			|| 2 * pRpcInBuffer->Length > 0xFFFFFF
			|| 2 * pRpcInBuffer->Length + 0x2C > pRpcInBuffer->RpcHeader.DataLength
			)
		{
			err = E_INVALIDARG;
			break;
		}

		IPStore *pIPStore = *ppIPStore ;		
		if ( NULL == pIPStore )
			break;

		int Length = 2 * pRpcInBuffer->Length + 2;
		LPWSTR szItemName = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, Length );
		if ( NULL == szItemName ) { return NO_ERROR; }

		RtlZeroMemory( szItemName, Length );
		memcpy( szItemName, pRpcInBuffer->szItemName, Length - 2 );

		PST_PROMPTINFO PromptInfo = { 0 };
		PromptInfo.cbSize = sizeof(PST_PROMPTINFO) ;
		PromptInfo.dwPromptFlags = PST_PF_NEVER_SHOW;
		PromptInfo.hwndApp = 0;
		PromptInfo.szPrompt = 0;

		DWORD cbData = 0;
		BYTE *pbData = NULL;
		err = pIPStore->ReadItem( 0, &pRpcInBuffer->iid_data, &pRpcInBuffer->iid_data_sun, szItemName, &cbData, &pbData, &PromptInfo, 0 );
		if ( err )
		{
			err = pIPStore->ReadItem( 1, &pRpcInBuffer->iid_data, &pRpcInBuffer->iid_data_sun, szItemName, &cbData, &pbData, &PromptInfo, 0 );	
		}

		HeapFree( GetProcessHeap(), 0, szItemName );
		if ( err < 0 ) { break; }

		LPRPC_OUT_PBIPStoreGetGrandchildNode pRpcOutBuffer = (LPRPC_OUT_PBIPStoreGetGrandchildNode) AllocRpcBuffer( pTotalData, cbData + 0x10 );
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->cbData = cbData;
			memcpy( pRpcOutBuffer->Data, pbData, cbData );
		}

		if ( pbData ) { CoTaskMemFree(pbData); }
		return (LPPOSTPROCRPCINFO)pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO 
CRpcPStore::RpcPStoreEnumSubtypes( 
	PVOID pInfo, 
	PVOID _pRpcInBuffer
	)
{
	int err = E_FAIL ;
	IPStore **ppIPStore = (IPStore **)pInfo;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_PBIPStoreEnumSubtypes pRpcInBuffer = (LPRPC_IN_PBIPStoreEnumSubtypes) _pRpcInBuffer;

	MYTRACE( "RpcPStoreEnumSubtypes \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength != sizeof(RPC_IN_PBIPStoreEnumSubtypes) )
		{
			err = E_INVALIDARG;
			break;
		}

		IPStore *pIPStore = *ppIPStore ;		
		if ( NULL == pIPStore )
			break;

		IEnumPStoreTypes *penum ;
		if (pRpcInBuffer->bEnumSubtypes )
			err = pIPStore->EnumSubtypes( pRpcInBuffer->Key, &pRpcInBuffer->ClsidType, 0, &penum );
		else
			err = pIPStore->EnumTypes( pRpcInBuffer->Key, 0, &penum );

		GUID rgelt;
		DWORD celtFetched;
		for ( ; err >= 0; err = penum->Next(1, &rgelt, &celtFetched) )
			;
		if ( penum )
			penum->Release();

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO 
CRpcPStore::RpcPStoreEnumItems( 
	PVOID pInfo, 
	PVOID _pRpcInBuffer
	)
{
	int err = E_FAIL ;
	IPStore **ppIPStore = (IPStore **)pInfo;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_PBIPStoreEnumItems pRpcInBuffer = (LPRPC_IN_PBIPStoreEnumItems) _pRpcInBuffer;

	MYTRACE( "RpcPStoreEnumItems \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength != sizeof(RPC_IN_PBIPStoreEnumItems) )
		{
			err = E_INVALIDARG;
			break;
		}

		IPStore *pIPStore = *ppIPStore ;		
		if ( NULL == pIPStore )
			break;

		IEnumPStoreItems* penum;
		err = pIPStore->EnumItems( pRpcInBuffer->Key, &pRpcInBuffer->ClsidType, &pRpcInBuffer->ClsidStubType, 0, &penum );

		LPWSTR rgelt;
		DWORD celtFetched;
		while ( err >= 0 )
		{
			err = penum->Next( 1, &rgelt, &celtFetched );
			if ( celtFetched )
			{
				wcslen(rgelt);
				CoTaskMemFree(rgelt);
			}
		}

		if ( penum )
			penum->Release();

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


///////////////////////////////   END OF FILE   ///////////////////////////////