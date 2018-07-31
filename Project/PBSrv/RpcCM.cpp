/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \SandBox\Code\Project\PBSrv\RpcCM.cpp
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
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include "RpcCM.h"

#pragma comment(lib, "SetupAPI.lib")

//////////////////////////////////////////////////////////////////////////

CRpcCM g_CRpcCM ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __ProcCM( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, int Msg )
{
	return g_CRpcCM.ProcCM( pInfo, pRpcBuffer, Msg );
}


CRpcCM::CRpcCM(void)
{
	
}

CRpcCM::~CRpcCM(void)
{
}


VOID CRpcCM::HandlerCM( PVOID _pBuffer, LPTOTOAL_DATA pNode )
{
	LPHandlerCM_INFO pBuffer = (LPHandlerCM_INFO) _pBuffer;

	if ( NULL == pNode || NULL == pBuffer ) { return; }
	RtlZeroMemory( pBuffer, sizeof(HandlerCM_INFO) );

	HMODULE hModule_setupapi_dll = LoadLibraryW(L"setupapi.dll");
	if ( hModule_setupapi_dll )
	{
		pBuffer->SetupDiGetDevicePropertyW_addr = (ULONG)GetProcAddress( hModule_setupapi_dll, "SetupDiGetDevicePropertyW" );
		pBuffer->SetupDiGetDeviceInterfacePropertyW_addr = (ULONG)GetProcAddress( hModule_setupapi_dll, "SetupDiGetDeviceInterfacePropertyW" );
	}

	HMODULE hModule_cfgmgr32_dll = LoadLibraryW(L"cfgmgr32.dll");
	if ( hModule_cfgmgr32_dll )
	{
		pBuffer->CM_Get_Class_Property_ExW_addr = (ULONG)GetProcAddress( hModule_cfgmgr32_dll, "CM_Get_Class_Property_ExW" );
	}

	HMODULE hModule_winsta_dll = LoadLibraryW(L"winsta.dll");
	if ( hModule_winsta_dll )
	{
		pBuffer->WinStationQueryInformationW_addr = (ULONG)GetProcAddress( hModule_winsta_dll, "WinStationQueryInformationW" );
	}

	_InsertList( pNode, 0x1400, pBuffer, (_PROCAPINUMBERFUNC_)__ProcCM, FALSE );
	return;
}


PVOID 
CRpcCM::ProcCM(
	IN PVOID pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	PVOID ret = NULL ;

	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_SetupDiGetDevicePropertyW_ :
		ret = RpcSetupDiGetDevicePropertyW( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_CM_Get_Device_Interface_List_filter_ :
		ret = RpcCM_Get_Device_Interface_List_filter( pRpcBuffer );
		break;

	case _PBSRV_APINUM_CM_Get_Device_Interface_Alias_ExW_ :
		ret = RpcCM_Get_Device_Interface_Alias_ExW( pRpcBuffer );
		break;

	case _PBSRV_APINUM_CM_Get_Class_Property_ExW_Win7_ :
		ret = RpcCM_Get_Class_Property_ExW_Win7( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_CM_Get_DevNode_Status_ :
		ret = RpcCM_Get_DevNode_Status( pInfo, pRpcBuffer );
		break;

	case _PBSRV_APINUM_WinStaQueryInformationW_ :
		ret = RpcWinStaQueryInformationW( pInfo, pRpcBuffer );
		break;

	default:
		break;
	}

	 return ret;
}


PVOID CRpcCM::RpcSetupDiGetDevicePropertyW( PVOID _pInfo, PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPHandlerCM_INFO pInfo = (LPHandlerCM_INFO) _pInfo;
	LPRPC_IN_SetupDiGetDevicePropertyW pRpcInBuffer = (LPRPC_IN_SetupDiGetDevicePropertyW) _pRpcInBuffer;

	MYTRACE( "RpcSetupDiGetDevicePropertyW \n" );

	do 
	{
		typedef BOOL (WINAPI *_SetupDiGetDeviceProperty_)(HDEVINFO,PSP_DEVINFO_DATA,DEVPROPKEY*,ULONG *,PBYTE,DWORD,PDWORD,DWORD);
		_SetupDiGetDeviceProperty_ SetupDiGetDevicePropertyW_addr = (_SetupDiGetDeviceProperty_) pInfo->SetupDiGetDevicePropertyW_addr;
		if ( NULL == SetupDiGetDevicePropertyW_addr )
		{
			err = ERROR_NOT_SUPPORTED;
			break;
		}

		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_SetupDiGetDevicePropertyW) )
		{
			err = ERROR_INVALID_PARAMETER;
			break;
		}

		HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList( &pRpcInBuffer->ClassGuid, NULL );
		if ( NULL == DeviceInfoSet )
		{
			err = GetLastError();
			break;
		}

		BOOL bOK = FALSE;
		ULONG PropertyType = 0;
		DWORD RequiredSize = 0;
		SP_DEVINFO_DATA  DeviceInfoData = {};
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);

		BOOL bRet = SetupDiOpenDeviceInfoW( DeviceInfoSet, pRpcInBuffer->DevInst, NULL, 0, &DeviceInfoData );
		if ( bRet )
		{
			bRet = SetupDiGetDevicePropertyW_addr( DeviceInfoSet, &DeviceInfoData, &pRpcInBuffer->PropertyKey, 
				&PropertyType, 0, 0, &RequiredSize, pRpcInBuffer->Flags );	

			if ( bRet || ERROR_INSUFFICIENT_BUFFER == GetLastError() )
			{
				bOK = TRUE;
			}
		}

		if ( FALSE == bOK )
		{
			SetupDiDestroyDeviceInfoList(DeviceInfoSet);
			err = GetLastError();
			break;
		}

		// ok
		LPRPC_OUT_SetupDiGetDevicePropertyW pRpcOutBuffer = (LPRPC_OUT_SetupDiGetDevicePropertyW) AllocRpcBuffer( pTotalData, RequiredSize + 0x14);
		if ( pRpcOutBuffer )
		{
			bRet = SetupDiGetDevicePropertyW_addr( DeviceInfoSet, &DeviceInfoData, &pRpcInBuffer->PropertyKey, 
				&pRpcOutBuffer->PropertyType, (PBYTE)pRpcOutBuffer->PropertyBuffer, RequiredSize, &pRpcOutBuffer->BufferSize, pRpcInBuffer->Flags );	

			if( FALSE == bRet )
				pRpcOutBuffer->RpcHeader.u.Status = GetLastError();
		}

		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


PVOID CRpcCM::RpcCM_Get_Device_Interface_List_filter( PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_CM_Get_Device_Interface_List pRpcInBuffer = (LPRPC_IN_CM_Get_Device_Interface_List) _pRpcInBuffer;

	MYTRACE( "RpcCM_Get_Device_Interface_List_filter \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_CM_Get_Device_Interface_List) )
		{
			err = ERROR_WRITE_PROTECT;
			break;
		}

		ULONG ulLen = 0;
		CONFIGRET ret = 0;
		if ( pRpcInBuffer->bIsUnicode )
			ret = CM_Get_Device_Interface_List_SizeW( &ulLen, &pRpcInBuffer->ClassGuid, 0, pRpcInBuffer->ulFlags );
		else
			ret = CM_Get_Device_Interface_List_SizeA( &ulLen, &pRpcInBuffer->ClassGuid, 0, pRpcInBuffer->ulFlags );

		if ( ret )
		{
			err = ret;
			break;
		}

		ULONG Size = ulLen + 0x18;
		if ( pRpcInBuffer->bIsUnicode )
			Size += ulLen;

		LPRPC_OUT_CM_Get_Device_Interface_List pRpcOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_List) AllocRpcBuffer( pTotalData, Size);
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->Length = ulLen;

			if ( pRpcInBuffer->bIsUnicode )
				pRpcOutBuffer->RpcHeader.u.Status = CM_Get_Device_Interface_ListW( &pRpcInBuffer->ClassGuid, 0, (WCHAR*)pRpcOutBuffer->DeviceInterface, ulLen, pRpcInBuffer->ulFlags );
			else
				pRpcOutBuffer->RpcHeader.u.Status = CM_Get_Device_Interface_ListA( &pRpcInBuffer->ClassGuid, 0, (CHAR*)pRpcOutBuffer->DeviceInterface, ulLen, pRpcInBuffer->ulFlags );
		}
		
		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


PVOID CRpcCM::RpcCM_Get_Device_Interface_Alias_ExW( PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPRPC_IN_CM_Get_Device_Interface_Alias_ExW pRpcInBuffer = (LPRPC_IN_CM_Get_Device_Interface_Alias_ExW) _pRpcInBuffer;

	MYTRACE( "RpcCM_Get_Device_Interface_Alias_ExW \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_CM_Get_Device_Interface_Alias_ExW) )
		{
			err = ERROR_WRITE_PROTECT;
			break;
		}

		if ( pRpcInBuffer->DeviceInterfaceLength + 0x20 > pRpcInBuffer->RpcHeader.DataLength )
		{
			err = ERROR_PATH_NOT_FOUND;
			break;
		}

		ULONG ulLength = 1;
		err = CM_Get_Device_Interface_Alias_ExW (
			pRpcInBuffer->Buffer,
			&pRpcInBuffer->AliasInterfaceGuid,
			(LPWSTR)&pRpcInBuffer->RpcHeader.DataLength + 1,
			&ulLength,
			pRpcInBuffer->ulFlags,
			NULL
			);

		if ( err && err != ERROR_NOT_DOS_DISK )
			break;

		// ok
		ULONG Size = 2 * ulLength + 0x18;
		LPRPC_OUT_CM_Get_Device_Interface_Alias_ExW pRpcOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_Alias_ExW) AllocRpcBuffer( pTotalData, Size);
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->RpcHeader.u.Status = CM_Get_Device_Interface_Alias_ExW (
				pRpcInBuffer->Buffer,
				&pRpcInBuffer->AliasInterfaceGuid,
				pRpcOutBuffer->ReturnBuffer,
				&ulLength,
				pRpcInBuffer->ulFlags,
				NULL 
				);

			pRpcOutBuffer->ReturnLength = ulLength + 1;
		}

		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



PVOID CRpcCM::RpcCM_Get_Device_Interface_Property_ExW( PVOID _pInfo, PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPHandlerCM_INFO pInfo = (LPHandlerCM_INFO) _pInfo;
	LPRPC_IN_CM_Get_Device_Interface_Property_ExW pRpcInBuffer = (LPRPC_IN_CM_Get_Device_Interface_Property_ExW) _pRpcInBuffer;

	MYTRACE( "RpcCM_Get_Device_Interface_Property_ExW \n" );

	do 
	{
		typedef BOOL (WINAPI *_SetupDiGetDeviceInterfacePropertyW_)(HDEVINFO,PSP_DEVICE_INTERFACE_DATA,DEVPROPKEY*,DEVPROPKEY*,PBYTE,DWORD,PDWORD,DWORD);
		_SetupDiGetDeviceInterfacePropertyW_ SetupDiGetDeviceInterfacePropertyW_addr = (_SetupDiGetDeviceInterfacePropertyW_) pInfo->SetupDiGetDeviceInterfacePropertyW_addr;
		if ( NULL == SetupDiGetDeviceInterfacePropertyW_addr || pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_CM_Get_Device_Interface_Property_ExW) )
		{
			err = ERROR_NOT_SUPPORTED;
			break;
		}

		HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList( NULL, NULL );
		if ( NULL == DeviceInfoSet )
		{
			err = GetLastError();
			break;
		}

		BOOL bOK = FALSE;
		DEVPROPKEY PropertyType = {};
		DWORD RequiredSize = 0;
		SP_DEVICE_INTERFACE_DATA  DeviceInterfaceData = {};
		DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

		BOOL bRet = SetupDiOpenDeviceInterfaceW( DeviceInfoSet, pRpcInBuffer->DevInst, 0, &DeviceInterfaceData );
		if ( FALSE == bRet )
		{
			err = ERROR_DEV_NOT_EXIST;
			break;
		}
	
		bRet = SetupDiGetDeviceInterfacePropertyW_addr( DeviceInfoSet, &DeviceInterfaceData, &pRpcInBuffer->PropertyKey, 
			&PropertyType, 0, 0, &RequiredSize, 0 );	

		if ( bRet || ERROR_INSUFFICIENT_BUFFER == GetLastError() )
		{
			bOK = TRUE;
		}

		if ( FALSE == bOK )
		{
			SetupDiDestroyDeviceInfoList(DeviceInfoSet);
			if ( ERROR_INVALID_PARAMETER == err )
				err = ERROR_DEV_NOT_EXIST;
			else
				err = ERROR_WRITE_PROTECT;

			break;
		}

		// ok
		LPRPC_OUT_CM_Get_Device_Interface_Property_ExW pRpcOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_Property_ExW) AllocRpcBuffer( pTotalData, RequiredSize + 0x14);
		if ( pRpcOutBuffer )
		{
			bRet = SetupDiGetDeviceInterfacePropertyW_addr( DeviceInfoSet, &DeviceInterfaceData, &pRpcInBuffer->PropertyKey, 
				(DEVPROPKEY *)&pRpcOutBuffer->Reserved, (PBYTE)pRpcOutBuffer->ReturnBuffer, RequiredSize, &pRpcOutBuffer->ReturnLength, 0 );	

			if( FALSE == bRet )
				pRpcOutBuffer->RpcHeader.u.ErrorCode = ERROR_WRITE_PROTECT;
		}

		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


PVOID CRpcCM::RpcCM_Get_Class_Property_ExW_Win7( PVOID _pInfo, PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPHandlerCM_INFO pInfo = (LPHandlerCM_INFO) _pInfo;
	LPRPC_IN_CM_Get_Class_Property_ExW_Win7 pRpcInBuffer = (LPRPC_IN_CM_Get_Class_Property_ExW_Win7) _pRpcInBuffer;

	MYTRACE( "RpcCM_Get_Class_Property_ExW_Win7 \n" );

	do 
	{
		typedef ULONG (WINAPI *_CM_Get_Class_Property_ExW_Win7_)(LPGUID,DEVPROPKEY*,ULONG*,PVOID,PULONG,ULONG,int);
		_CM_Get_Class_Property_ExW_Win7_ CM_Get_Class_Property_ExW_Win7_addr = (_CM_Get_Class_Property_ExW_Win7_) pInfo->CM_Get_Class_Property_ExW_addr;
		if ( NULL == CM_Get_Class_Property_ExW_Win7_addr || pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_CM_Get_Class_Property_ExW_Win7) )
		{
			err = ERROR_WRITE_PROTECT;
			break;
		}

		ULONG unknow1 = 0, RequiredSize = 0;		
		err = CM_Get_Class_Property_ExW_Win7_addr( &pRpcInBuffer->ClassGuid1, &pRpcInBuffer->PropertyKey, &unknow1, 0, &RequiredSize, pRpcInBuffer->Reserved, 0 );	

		if ( err && ERROR_NOT_DOS_DISK != GetLastError() )
			break;

		// ok
		LPRPC_OUT_CM_Get_Class_Property_ExW_Win7 pRpcOutBuffer = (LPRPC_OUT_CM_Get_Class_Property_ExW_Win7) AllocRpcBuffer( pTotalData, RequiredSize + 0x14);
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->ReturnLength = RequiredSize;
			pRpcOutBuffer->RpcHeader.u.ErrorCode = CM_Get_Class_Property_ExW_Win7_addr( &pRpcInBuffer->ClassGuid1, &pRpcInBuffer->PropertyKey,
						&pRpcOutBuffer->Reserved, pRpcOutBuffer->ReturnBuffer, &pRpcOutBuffer->ReturnLength, pRpcInBuffer->Reserved, 0 );	
		}

		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


PVOID CRpcCM::RpcCM_Get_DevNode_Status( PVOID _pInfo, PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPHandlerCM_INFO pInfo = (LPHandlerCM_INFO) _pInfo;
	LPRPC_IN_CM_Get_DevNode_Status pRpcInBuffer = (LPRPC_IN_CM_Get_DevNode_Status) _pRpcInBuffer;

	MYTRACE( "RpcCM_Get_DevNode_Status \n" );

	do 
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_CM_Get_DevNode_Status) )
		{
			err = ERROR_WRITE_PROTECT;
			break;
		}

		// ok
		LPRPC_OUT_CM_Get_DevNode_Status pRpcOutBuffer = (LPRPC_OUT_CM_Get_DevNode_Status) AllocRpcBuffer( pTotalData, sizeof(RPC_OUT_CM_Get_DevNode_Status));
		if ( pRpcOutBuffer )
		{
			DEVINST dnDevInst;
			pRpcOutBuffer->ulStatus = 0;
			pRpcOutBuffer->ulProblemNumber = 0;

			err = CM_Locate_DevNodeW( &dnDevInst, pRpcInBuffer->DevInst, 0 );
			if ( !err )
				err = CM_Get_DevNode_Status(&pRpcOutBuffer->ulStatus, &pRpcOutBuffer->ulProblemNumber, dnDevInst, 0);
			pRpcOutBuffer->RpcHeader.u.ErrorCode = err;
		}

		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


PVOID CRpcCM::RpcWinStaQueryInformationW( PVOID _pInfo, PVOID _pRpcInBuffer )
{
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();
	LPHandlerCM_INFO pInfo = (LPHandlerCM_INFO) _pInfo;
	LPRPC_IN_WINSTAQUERYINFORMATIONW pRpcInBuffer = (LPRPC_IN_WINSTAQUERYINFORMATIONW) _pRpcInBuffer;

	MYTRACE( "RpcWinStaQueryInformationW \n" );

	do 
	{
		typedef BOOLEAN (WINAPI *PWINSTATIONQUERYINFORMATIONW)(HANDLE,ULONG,int,PVOID,ULONG,PULONG);
		PWINSTATIONQUERYINFORMATIONW pfnWinStationQueryInformationW = (PWINSTATIONQUERYINFORMATIONW)pInfo->WinStationQueryInformationW_addr ;
		if ( NULL == pfnWinStationQueryInformationW )
		{
			err = ERROR_SERVICE_DOES_NOT_EXIST;
			break;
		}

		if ( pRpcInBuffer->RpcHeader.DataLength != sizeof(RPC_IN_WINSTAQUERYINFORMATIONW)
			|| pRpcInBuffer->ulWinStationInformationLength > 0xFFFFFF
			)
		{
			err = ERROR_INVALID_PARAMETER;
			break;
		}

		PVOID pWinStationInformation = (PVOID) HeapAlloc( GetProcessHeap(), 0, pRpcInBuffer->ulWinStationInformationLength );	
		if ( NULL == pWinStationInformation )
		{
			err = ERROR_OUTOFMEMORY;
			break;
		}

		err = NO_ERROR;
		ULONG ulReturnLength = 0;
		BOOLEAN fRet = pfnWinStationQueryInformationW( NULL, pRpcInBuffer->ulLogonId, pRpcInBuffer->WinStationInformationClass,
			pWinStationInformation, pRpcInBuffer->ulWinStationInformationLength, &ulReturnLength ) ;

		if ( !fRet )
			err = GetLastError();

		ULONG Size = 0x10;
		if ( fRet )
			Size = ulReturnLength + 0x10;

		LPRPC_OUT_WINSTAQUERYINFORMATIONW pRpcOutBuffer = (LPRPC_OUT_WINSTAQUERYINFORMATIONW) AllocRpcBuffer( pTotalData, Size );
		if ( pRpcOutBuffer )
		{
			pRpcOutBuffer->RpcHeader.u.ErrorCode = err;
			pRpcOutBuffer->ulReturnLength = ulReturnLength;
			if ( fRet )
				memcpy( pRpcOutBuffer->WinStationInformation, pWinStationInformation, ulReturnLength );
		}

		HeapFree( GetProcessHeap(), 0, pWinStationInformation );
		return pRpcOutBuffer;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}


///////////////////////////////   END OF FILE   ///////////////////////////////