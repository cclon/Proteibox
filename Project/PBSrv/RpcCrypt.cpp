/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \SandBox\Code\Project\PBSrv\RpcCrypt.cpp
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
#include "RpcCrypt.h"

#include <WinCrypt.h>
#pragma comment(lib, "crypt32.lib")


//////////////////////////////////////////////////////////////////////////

CRpcCrypt g_CRpcCrypt ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __ProcCrypt( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, int Msg )
{
	return g_CRpcCrypt.ProcCrypt( pInfo, pRpcBuffer, Msg );
}



CRpcCrypt::CRpcCrypt(void)
{
	
}

CRpcCrypt::~CRpcCrypt(void)
{
}



VOID CRpcCrypt::HandlerCrypt( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	_InsertList( pNode, 0x1600, pBuffer, (_PROCAPINUMBERFUNC_)__ProcCrypt, TRUE );

	return;
}


PVOID 
CRpcCrypt::ProcCrypt(
	IN PVOID pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	LPPOSTPROCRPCINFO ret = NULL ;

	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_CryptUnprotectData_ :
		ret = RpcCryptUnprotectData( pRpcBuffer );
		break;

	case _PBSRV_APINUM_CryptProtectData_ :
		ret = RpcCryptProtectData( pRpcBuffer );
		break;

	default:
		break;
	}

	 return (PVOID)ret;
}



LPPOSTPROCRPCINFO CRpcCrypt::RpcCryptUnprotectData( PVOID _pRpcInBuffer)
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_CryptUnprotectData pRpcInBuffer = (LPRPC_IN_CryptUnprotectData) _pRpcInBuffer;

	MYTRACE( "RpcCryptUnprotectData \n" );

	do 
	{
		if ( pRpcInBuffer->TotalLength < sizeof(RPC_IN_CryptUnprotectData)
			|| pRpcInBuffer->pDataIn_cbData > 0xFFFFFF
			|| pRpcInBuffer->pDataIn_cbData + 0x14 > pRpcInBuffer->TotalLength
			|| pRpcInBuffer->pOptionalEntropy_cbData > 0xFFFFFF
			|| pRpcInBuffer->pOptionalEntropy_cbData + pRpcInBuffer->pDataIn_cbData + 0x14 > pRpcInBuffer->TotalLength
			)
		{
			err = ERROR_INVALID_PARAMETER;
			break;
		}

		DATA_BLOB DataIn = {0} ;
		DATA_BLOB DataOut = {0} ;
		DataIn.cbData = pRpcInBuffer->pDataIn_cbData ;
		DataIn.pbData = (BYTE *)pRpcInBuffer->Data ;

		LPWSTR pszDataDescr;
		BOOL bRet = CryptUnprotectData( 
			&DataIn, &pszDataDescr,  
			(DATA_BLOB *)(pRpcInBuffer->pOptionalEntropy_cbData ? &pRpcInBuffer->pOptionalEntropy_cbData : NULL),
			NULL, NULL, pRpcInBuffer->dwFlags, &DataOut
			);

		if ( FALSE == bRet )
		{
			err = GetLastError();
			break;
		}

		DWORD DataDescrLength = wcslen(pszDataDescr);
		DWORD Size = DataOut.cbData + 2 * DataDescrLength + 0x16;
		LPRPC_OUT_CryptUnprotectData pRpcOutBuffer = (LPRPC_OUT_CryptUnprotectData) AllocRpcBuffer( pTotalData, Size);

		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->pDataOut_cbData = DataOut.cbData;
			memcpy( pRpcOutBuffer->pDataOut_pbData, DataOut.pbData, DataOut.cbData );

			pRpcOutBuffer->DataDescrLength = DataDescrLength ;
			memcpy (
				(PVOID)((char *)pRpcOutBuffer->pDataOut_pbData + pRpcOutBuffer->pDataOut_cbData), 
				pszDataDescr,
				2 * DataDescrLength
				);
		}

		if ( pszDataDescr )
			LocalFree(pszDataDescr);
		if ( DataOut.pbData )
			LocalFree(DataOut.pbData);

		return (LPPOSTPROCRPCINFO)pRpcOutBuffer;

	} while (FALSE);
	
	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO CRpcCrypt::RpcCryptProtectData( PVOID _pRpcInBuffer)
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_CryptProtectData pRpcInBuffer = (LPRPC_IN_CryptProtectData) _pRpcInBuffer;

	MYTRACE( "RpcCryptProtectData \n" );

	do 
	{
		if ( pRpcInBuffer->TotalLength < sizeof(RPC_IN_CryptProtectData)
			|| pRpcInBuffer->pDataIn_cbData > 0xFFFFFF
			|| pRpcInBuffer->pDataIn_cbData + 0x18 > pRpcInBuffer->TotalLength
			|| pRpcInBuffer->pOptionalEntropy_cbData > 0xFFFFFF
			|| pRpcInBuffer->DataDescrLength > 0xFFFFFF
			|| pRpcInBuffer->pOptionalEntropy_cbData + pRpcInBuffer->pDataIn_cbData + 0x18 > pRpcInBuffer->TotalLength
			|| pRpcInBuffer->pOptionalEntropy_cbData + pRpcInBuffer->pDataIn_cbData + 0x18 + 2 * pRpcInBuffer->DataDescrLength > pRpcInBuffer->TotalLength
			)
		{
			err = ERROR_INVALID_PARAMETER;
			break;
		}

		DATA_BLOB DataIn = {0} ;
		DATA_BLOB DataOut = {0} ;
		DataIn.cbData = pRpcInBuffer->pDataIn_cbData ;
		DataIn.pbData = (BYTE *)pRpcInBuffer->Data ;

		LPCWSTR szDataDescr = NULL;
		if ( pRpcInBuffer->DataDescrLength )
			szDataDescr = (LPCWSTR)((char *)pRpcInBuffer + pRpcInBuffer->pOptionalEntropy_cbData + pRpcInBuffer->pDataIn_cbData + 0x18);

		BOOL bRet = CryptProtectData( 
			&DataIn, szDataDescr,  
			(DATA_BLOB *)(pRpcInBuffer->pOptionalEntropy_cbData ? &pRpcInBuffer->pOptionalEntropy_cbData : NULL),
			NULL, NULL, pRpcInBuffer->dwFlags, &DataOut
			);

		if ( FALSE == bRet )
		{
			err = GetLastError();
			break;
		}

		DWORD Size = DataOut.cbData + 0x14;
		LPRPC_OUT_CryptProtectData pRpcOutBuffer = (LPRPC_OUT_CryptProtectData) AllocRpcBuffer( pTotalData, Size);

		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->pDataOut_cbData = DataOut.cbData;
			memcpy( pRpcOutBuffer->pDataOut_pbData, DataOut.pbData, DataOut.cbData );
		}

		if ( DataOut.pbData )
			LocalFree(DataOut.pbData);

		return (LPPOSTPROCRPCINFO)pRpcOutBuffer;

	} while (FALSE);
	
	return PostProcRPC( pTotalData, err );
}


///////////////////////////////   END OF FILE   ///////////////////////////////