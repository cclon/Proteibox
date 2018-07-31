/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/02/01 [1:2:2012 - 15:36]
* MODULE : \Code\Project\ProteinBoxDLL\PBSetupapidll\PBSetupapidll.cpp
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

#include "../StdAfx.h"
#include "../common.h"
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../PBServices.h"
#include "../PBServicesData.h"
#include "../PBCreateProcess.h"
#include "PBSetupapidllData.h"
#include "PBSetupapidll.h"

//////////////////////////////////////////////////////////////////////////

BOOL g_bIsWhilte_rpc_control_ntsvcs = FALSE ;
ULONG_PTR g_CM_Get_Class_Property_ExW_addr = 0 ;

typedef BOOL (WINAPI* _SetupDiGetDevicePropertyW_) ( /*HDEVINFO*/ HANDLE,PSP_DEVINFO_DATA,const DEVPROPKEY *,DEVPROPTYPE *,PBYTE,DWORD,PDWORD,DWORD );
_SetupDiGetDevicePropertyW_ g_SetupDiGetDevicePropertyW_addr = NULL ;

typedef DWORD (WINAPI* _CM_Is_Version_Available_Ex_) ( int, int );
_CM_Is_Version_Available_Ex_ g_CM_Is_Version_Available_Ex_addr = NULL ;

static const LPWSTR g_Hook_pharase8_setupapidll_Arrays[ ] =
{ 
	L"\\RPC Control\\ntsvcs",
	L"\\RPC Control\\plugplay",
} ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

ULONG WINAPI fake_CM_Is_Version_Available_Ex(int a1, int a2)
{
	return 1;
}


ULONG WINAPI 
fake_CM_Enumerate_Classes( 
	ULONG ulClassIndex, 
	LPGUID ClassGuid, 
	ULONG ulFlags 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/04/06 [6:4:2011 - 17:58]

Routine Description:
  该函数作用即是根据@ulClassIndex,从g_pMappedAddr_DeviceSetupClasses_array中取得相应的GUID,填充至@ClassGuid

  g_pMappedAddr_DeviceSetupClasses_array保存的指针指向以0x50大小为单位的系列数组. 形如
  0:000> dd 3b0004
  003b0004  25dbce51 4a726c8f 4cb56d8a 35c84f2b // 前0x10是Index对应的 GUID
  003b0014  00430057 00550045 00420053 00000053 // 这里开始是其对应的名字,eg:"WCEUSBS"
  003b0024  00000000 00000000 00000000 00000000
  003b0034  00000000 00000000 00000000 00000000
  003b0044  00000000 00000000 00000000 00000000
  003b0054  36fc9e60 11cfc465 45445680 00005453 // GUID
  003b0064  00530055 00000042 00000000 00000000 // ...
    
--*/
{
	LPGUID ClassGuid_new = NULL ;

	if ( ulFlags ) { return CR_INVALID_FLAG; }

	if ( ulClassIndex >= (ULONG)g_pMappedAddr_DeviceSetupClasses_ClassIndex ) { return CR_NO_SUCH_VALUE; }

	ClassGuid_new = (LPGUID)(g_pMappedAddr_DeviceSetupClasses_array + 0x50 * ulClassIndex);
	
	memcpy( ClassGuid, ClassGuid_new, sizeof(GUID) );
	return CR_SUCCESS;
}


ULONG WINAPI 
fake_CM_Enumerate_Classes_Ex (
	ULONG ulClassIndex, 
	LPGUID ClassGuid, 
	ULONG ulFlags, 
	HANDLE hMachine
	)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = fake_CM_Enumerate_Classes(ulClassIndex, ClassGuid, ulFlags);

	return ret;
}


ULONG WINAPI fake_CM_Get_Class_NameA(LPGUID ClassGuid, PCHAR Buffer, PULONG pulLength, ULONG ulFlags)
{
	return CM_Get_Class_Name_filter( ulFlags, ClassGuid, Buffer, pulLength, FALSE );
}


ULONG WINAPI fake_CM_Get_Class_NameW(LPGUID ClassGuid, PWCHAR Buffer, PULONG pulLength, ULONG ulFlags)
{
	return CM_Get_Class_Name_filter( ulFlags, ClassGuid, Buffer, pulLength, TRUE );
}


ULONG WINAPI fake_CM_Get_Class_Name_ExA(LPGUID ClassGuid, PCHAR Buffer, PULONG pulLength, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ; 

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Class_Name_filter( ulFlags, ClassGuid, Buffer, pulLength, FALSE );

	return ret;
}


ULONG WINAPI fake_CM_Get_Class_Name_ExW(LPGUID ClassGuid, PWCHAR Buffer, PULONG pulLength, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ; 

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Class_Name_filter( ulFlags, ClassGuid, Buffer, pulLength, TRUE );

	return ret;
}


ULONG WINAPI 
fake_CM_Get_Device_ID_ExA (
	DWORD dnDevInst, 
	PCHAR Buffer, 
	ULONG BufferLen, 
	ULONG ulFlags, 
	HANDLE hMachine
	)
{
	UNICODE_STRING uniBuffer ;
	ANSI_STRING ansiBuffer ;

	if ( hMachine ) { return CR_MACHINE_UNAVAILABLE; }
	
	if ( BufferLen < wcslen((LPWSTR)dnDevInst) ) { return CR_BUFFER_SMALL; }
	
	RtlInitUnicodeString( &uniBuffer, (PCWSTR)dnDevInst );

	ansiBuffer.Length = 0;
	ansiBuffer.MaximumLength = (USHORT)BufferLen;
	ansiBuffer.Buffer = Buffer ;

	RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	return CR_SUCCESS;
}


ULONG WINAPI 
fake_CM_Get_Device_ID_ExW (
	DWORD dnDevInst, 
	PWCHAR Buffer, 
	ULONG BufferLen, 
	ULONG ulFlags, 
	HANDLE hMachine
	)
{
	DWORD length = 0 ;

	if ( hMachine ) { return CR_MACHINE_UNAVAILABLE; }

	length = wcslen( (LPWSTR)dnDevInst );
	if ( BufferLen < length ) { return CR_BUFFER_SMALL; }

	if ( BufferLen >= length + 1 ) { ++ length; }
	memcpy( Buffer, (PVOID)dnDevInst, 2 * length );

	return CR_SUCCESS;
}


ULONG WINAPI fake_CM_Get_Device_ID_Size_Ex(PULONG pulLen, DWORD dnDevInst, ULONG ulFlags, HANDLE hMachine)
{
	if ( hMachine ) { return CR_MACHINE_UNAVAILABLE; }

	*pulLen = wcslen( (LPWSTR)dnDevInst );
	return CR_SUCCESS;
}


ULONG WINAPI fake_CM_Get_Device_ID_ListA(PCSTR pszFilter, PCHAR Buffer, ULONG BufferLen, ULONG ulFlags)
{
	return CM_Get_Device_ID_List_filter( FALSE, ulFlags, pszFilter, Buffer, BufferLen );
}


ULONG WINAPI fake_CM_Get_Device_ID_ListW(PCSTR pszFilter, PVOID Buffer, ULONG BufferLen, ULONG ulFlags)
{
	return CM_Get_Device_ID_List_filter( TRUE, ulFlags, pszFilter, Buffer, BufferLen );
}


ULONG WINAPI fake_CM_Get_Device_ID_List_ExA(PCSTR pszFilter, PVOID Buffer, ULONG BufferLen, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Device_ID_List_filter( FALSE, ulFlags, pszFilter, Buffer, BufferLen );

	return ret;
}


ULONG WINAPI fake_CM_Get_Device_ID_List_ExW(PCSTR pszFilter, PVOID Buffer, ULONG BufferLen, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Device_ID_List_filter( TRUE, ulFlags, pszFilter, Buffer, BufferLen );

	return ret;
}


ULONG WINAPI fake_CM_Get_Device_ID_List_SizeW(PULONG pulLen, PCSTR pszFilter, ULONG ulFlags)
{
	if ( pszFilter || ulFlags )
	{
		if ( pszFilter ) { return CR_NO_SUCH_VALUE; }
		if ( ulFlags ) { return CR_INVALID_FLAG; }
	}

	if ( g_pMappedAddr_DeviceIdList_ClassIndex > 1 )
		*pulLen = g_pMappedAddr_DeviceIdList_ClassIndex;
	else
		*pulLen = 1;

	return CR_SUCCESS;
}


ULONG WINAPI fake_CM_Get_Device_ID_List_SizeA(PULONG pulLen, PCSTR pszFilter, ULONG ulFlags)
{
	return fake_CM_Get_Device_ID_List_SizeW( pulLen, pszFilter, ulFlags );
}


ULONG WINAPI fake_CM_Get_Device_ID_List_Size_Ex(PULONG pulLen, PCSTR pszFilter, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = fake_CM_Get_Device_ID_List_SizeW( pulLen, pszFilter, ulFlags );

	return ret;
}


ULONG WINAPI 
fake_CM_Get_Device_Interface_ListA (
	LPGUID ClassGuid, 
	PVOID DeviceID, 
	PVOID DeviceInterface, 
	ULONG ulSize, 
	int ulFlags
	)
{
	DWORD ErrorCode = 0 ;
	RPC_IN_CM_Get_Device_Interface_List pInBuffer ;
	LPRPC_OUT_CM_Get_Device_Interface_List pOutBuffer = NULL ;

	pInBuffer.RpcHeader.DataLength = 0x20 ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_CM_Get_Device_Interface_List_filter_ ;

	memcpy( &pInBuffer.ClassGuid, ClassGuid, sizeof(GUID) );
	pInBuffer.ulFlags = ulFlags ;
	pInBuffer.bIsUnicode = FALSE ;

	pOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_List) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return CR_FAILURE; }

	ErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS == ErrorCode )
	{
		if ( ulSize < pOutBuffer->Length )
		{
			kfree( pOutBuffer );
			return CR_BUFFER_SMALL;
		}

		memcpy( DeviceInterface, pOutBuffer->DeviceInterface, pOutBuffer->Length );
	}

	PB_FreeReply( pOutBuffer );
	return ErrorCode;
}


ULONG WINAPI fake_CM_Get_Device_Interface_ListW(LPGUID ClassGuid, PVOID DeviceID, PVOID DeviceInterface, ULONG ulSize, ULONG ulFlags)
{
	return CM_Get_Device_Interface_List_filter( ClassGuid, NULL, TRUE, DeviceInterface, ulSize, ulFlags );
}


ULONG WINAPI fake_CM_Get_Device_Interface_List_ExA(LPGUID ClassGuid, PVOID DeviceID, PVOID DeviceInterface, ULONG ulSize, int a5, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = fake_CM_Get_Device_Interface_ListA(ClassGuid, DeviceID, DeviceInterface, ulSize, a5);

	return ret;
}


ULONG WINAPI fake_CM_Get_Device_Interface_List_ExW(LPGUID ClassGuid, PVOID DeviceID, PVOID DeviceInterface, ULONG ulSize, int ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Device_Interface_List_filter( ClassGuid, NULL, TRUE, DeviceInterface, ulSize, ulFlags );

	return ret;
}


ULONG WINAPI fake_CM_Get_Device_Interface_List_SizeA(PULONG ulSize, LPGUID ClassGuid, PVOID DeviceID, int ulFlags)
{
	return CM_Get_Device_Interface_List_filter( ClassGuid, ulSize, FALSE, 0, 0, ulFlags );
}


ULONG WINAPI fake_CM_Get_Device_Interface_List_SizeW(PULONG ulSize, LPGUID ClassGuid, PVOID DeviceID, int ulFlags)
{
	return CM_Get_Device_Interface_List_filter( ClassGuid, ulSize, TRUE, 0, 0, ulFlags );
}


ULONG WINAPI fake_CM_Get_Device_Interface_List_Size_ExA(PULONG ulSize, LPGUID ClassGuid, PVOID DeviceID, int ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Device_Interface_List_filter( ClassGuid, ulSize, FALSE, 0, 0, ulFlags );

	return ret;
}


ULONG WINAPI fake_CM_Get_Device_Interface_List_Size_ExW(PULONG ulSize, LPGUID ClassGuid, PVOID DeviceID, int ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_Device_Interface_List_filter(ClassGuid, ulSize, TRUE, 0, 0, ulFlags);

	return ret;
}


ULONG WINAPI fake_CM_Locate_DevNodeW(DWORD *pdnDevInst, PWCHAR pDeviceID, ULONG ulFlags)
{
	PWCHAR pszName = NULL ; 

	if ( NULL == pDeviceID || 0 == g_pMappedAddr_DeviceIdList_array || 0 == *(WORD *)g_pMappedAddr_DeviceIdList_array ) 
	{ 
		*pdnDevInst = 0;
		return CR_NO_SUCH_DEVNODE ;
	}

	pszName = (PWCHAR)g_pMappedAddr_DeviceIdList_array ;

	while ( wcsicmp(pszName, pDeviceID) )
	{
		pszName += wcslen(pszName) + 1;
		if ( NULL == *pszName )
		{ 
			*pdnDevInst = 0;
			return CR_NO_SUCH_DEVNODE ;
		}
	}

	*pdnDevInst = (DWORD)pszName ;
	return CR_SUCCESS;
}


ULONG WINAPI fake_CM_Locate_DevNodeA(DWORD *pdnDevInst, PCHAR pDeviceID, ULONG ulFlags)
{
	ULONG Ret = 0 ;
	ANSI_STRING AnsiBuffer ;
	UNICODE_STRING UniBuffer ;

	if ( NULL == pDeviceID )
	{
		*pdnDevInst = 0;
		return CR_NO_SUCH_DEVNODE;
	}
	
	RtlInitString(&AnsiBuffer, pDeviceID);
	RtlAnsiStringToUnicodeString( &UniBuffer, &AnsiBuffer, TRUE );

	if ( NULL == UniBuffer.Buffer ) { return CR_OUT_OF_MEMORY; }
		
	Ret = fake_CM_Locate_DevNodeW( pdnDevInst, UniBuffer.Buffer, ulFlags );
	RtlFreeUnicodeString( &UniBuffer );

	return Ret;
}


ULONG WINAPI fake_CM_Locate_DevNode_ExA(DWORD *pdnDevInst, CHAR *pDeviceID, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = fake_CM_Locate_DevNodeA(pdnDevInst, pDeviceID, ulFlags);

	return ret;
}


ULONG WINAPI fake_CM_Locate_DevNode_ExW(DWORD *pdnDevInst, wchar_t *pDeviceID, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = fake_CM_Locate_DevNodeW(pdnDevInst, pDeviceID, ulFlags);

	return ret;
}


ULONG WINAPI fake_CM_Get_DevNode_Status(PULONG pulStatus, PULONG pulProblemNumber, DWORD dnDevInst, ULONG ulFlags)
{
	DWORD ErrorCode = ERROR_SUCCESS ;
	RPC_IN_CM_Get_DevNode_Status pInBuffer ;
	LPRPC_OUT_CM_Get_DevNode_Status pOutBuffer = NULL ;

	pInBuffer.RpcHeader.DataLength = 0x208 ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_CM_Get_DevNode_Status_ ;
	wcscpy( pInBuffer.DevInst, (const wchar_t *)dnDevInst );

	pOutBuffer = (LPRPC_OUT_CM_Get_DevNode_Status) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return CR_FAILURE; }
	
	ErrorCode = pOutBuffer->RpcHeader.u.Status;
	if ( ErrorCode )
	{
		*pulStatus = 0;
		*pulProblemNumber = 0;
	}
	else
	{
		*pulStatus = pOutBuffer->ulStatus ;
		*pulProblemNumber = pOutBuffer->ulProblemNumber ;
	}

	PB_FreeReply( pOutBuffer );
	return ErrorCode;
}


ULONG WINAPI fake_CM_Get_DevNode_Status_Ex(PULONG pulStatus, PULONG pulProblemNumber, DWORD dnDevInst, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = fake_CM_Get_DevNode_Status(pulStatus, pulProblemNumber, dnDevInst, ulFlags);

	return ret;
}


ULONG WINAPI fake_CM_Get_DevNode_Registry_PropertyA(DWORD dnDevInst, ULONG ulProperty, PULONG pulRegDataType, PVOID Buffer, PULONG pulLength, ULONG ulFlags)
{
	return CM_Get_DevNode_Registry_Property_filter( ulProperty, dnDevInst, ulFlags, pulRegDataType, Buffer, pulLength, FALSE );
}


ULONG WINAPI fake_CM_Get_DevNode_Registry_PropertyW(DWORD dnDevInst, ULONG ulProperty, PULONG pulRegDataType, PVOID Buffer, PULONG pulLength, ULONG ulFlags)
{
	return CM_Get_DevNode_Registry_Property_filter( ulProperty, dnDevInst, ulFlags, pulRegDataType, Buffer, pulLength, TRUE );
}


ULONG WINAPI fake_CM_Get_DevNode_Registry_Property_ExA(DWORD dnDevInst, ULONG ulProperty, PULONG pulRegDataType, PVOID Buffer, PULONG pulLength, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0 ;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_DevNode_Registry_Property_filter( ulProperty, dnDevInst, ulFlags, pulRegDataType, Buffer, pulLength, FALSE );

	return ret;
}


ULONG WINAPI fake_CM_Get_DevNode_Registry_Property_ExW(DWORD dnDevInst, ULONG ulProperty, PULONG pulRegDataType, PVOID Buffer, PULONG pulLength, ULONG ulFlags, HANDLE hMachine)
{
	ULONG ret = 0;

	if ( hMachine )
		ret = CR_MACHINE_UNAVAILABLE;
	else
		ret = CM_Get_DevNode_Registry_Property_filter( ulProperty, dnDevInst, ulFlags, pulRegDataType, Buffer, pulLength, TRUE );

	return ret;
}


ULONG WINAPI 
fake_CM_Get_Device_Interface_Alias_ExW (
	LPWSTR pszDeviceInterface,
	LPGUID AliasInterfaceGuid, 
	PVOID pszAliasDeviceInterface, 
	PULONG pulLength, 
	ULONG ulFlags, 
	HANDLE hMachine
	)
{
	ULONG DeviceInterfaceLength = 0, DataLength = 0, ErrorCode = 0 ;
	LPRPC_IN_CM_Get_Device_Interface_Alias_ExW pInBuffer = NULL ;
	LPRPC_OUT_CM_Get_Device_Interface_Alias_ExW pOutBuffer = NULL ;

	DeviceInterfaceLength = 2 * wcslen(pszDeviceInterface) + 2 ;
	DataLength = DeviceInterfaceLength + sizeof(RPC_IN_CM_Get_Device_Interface_Alias_ExW) ;
	pInBuffer = (LPRPC_IN_CM_Get_Device_Interface_Alias_ExW) kmalloc( DataLength );

	pInBuffer->RpcHeader.DataLength = DataLength ;
	pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_CM_Get_Device_Interface_Alias_ExW_ ;
	pInBuffer->DeviceInterfaceLength = DeviceInterfaceLength ;
	pInBuffer->ulFlags = ulFlags ;
	memcpy( pInBuffer->Buffer, pszDeviceInterface, DeviceInterfaceLength );
	memcpy( &pInBuffer->AliasInterfaceGuid, AliasInterfaceGuid, sizeof(GUID) );

	pOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_Alias_ExW) PB_CallServer( pInBuffer );
	kfree( pInBuffer );
	if ( NULL == pOutBuffer ) { return CR_REMOTE_COMM_FAILURE; }

	ErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS != ErrorCode )
	{
		PB_FreeReply( pOutBuffer );
		return ErrorCode;
	}

	if ( NULL == pulLength )
	{
		PB_FreeReply( pOutBuffer );
		return CR_INVALID_POINTER;
	}

	if ( *pulLength < pOutBuffer->ReturnLength )
	{
		*pulLength = pOutBuffer->ReturnLength ;
		PB_FreeReply( pOutBuffer );
		return CR_BUFFER_SMALL;
	}

	if ( NULL == pszAliasDeviceInterface )
	{
		*pulLength = pOutBuffer->ReturnLength ;
		PB_FreeReply( pOutBuffer );
		return CR_INVALID_POINTER;
	}

	memcpy( pszAliasDeviceInterface, pOutBuffer->ReturnBuffer, 2 * pOutBuffer->ReturnLength );
	*pulLength = pOutBuffer->ReturnLength ;

	PB_FreeReply( pOutBuffer );
	return ERROR_SUCCESS;
}


ULONG WINAPI 
fake_CM_Get_Device_Interface_Property_ExW (
	LPWSTR DevInst, 
	DEVPROPKEY* pPropertyKey, 
	int a3, 
	PVOID pReturnBuffer, 
	PULONG pulLength, 
	int a6, 
	int a7
	)
{
	ULONG_PTR ErrorCode = 0 ;
	RPC_IN_CM_Get_Device_Interface_Property_ExW pInBuffer ;
	LPRPC_OUT_CM_Get_Device_Interface_Property_ExW pOutBuffer = NULL ;

	if ( a6 || a7 ) { return CR_FAILURE; }

	pInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_CM_Get_Device_Interface_Property_ExW ) ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_CM_Get_Device_Interface_Property_ExW_ ;
	wcscpy( pInBuffer.DevInst, DevInst );
	memcpy( &pInBuffer.PropertyKey, pPropertyKey, sizeof(DEVPROPKEY) );

	pOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_Property_ExW) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return CR_OUT_OF_MEMORY; }

	ErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS != ErrorCode )
	{
		PB_FreeReply( pOutBuffer );
		return ErrorCode;
	}

	if ( *pulLength < pOutBuffer->ReturnLength )
	{
		*pulLength = pOutBuffer->ReturnLength ;
		PB_FreeReply( pOutBuffer );
		return CR_BUFFER_SMALL ;
	}

	memcpy( pReturnBuffer, pOutBuffer->ReturnBuffer, pOutBuffer->ReturnLength );
	*pulLength = pOutBuffer->ReturnLength ;
	PB_FreeReply( pOutBuffer );
	return ERROR_SUCCESS ;
}


ULONG WINAPI 
fake_CM_Get_Class_Property_ExW_Win7 (
	LPGUID ClassGUID1, 
	DEVPROPKEY* pPropertyKey,
	int* a3, 
	PVOID pReturnBuffer, 
	PULONG pulLength, 
	ULONG a6, 
	int a7
	)
{
	ULONG ErrorCode = 0 ;
	RPC_IN_CM_Get_Class_Property_ExW_Win7 pInBuffer ;
	LPRPC_OUT_CM_Get_Class_Property_ExW_Win7 pOutBuffer = NULL ;

	if ( a7 ) { return CR_FAILURE; }

	memcpy( &pInBuffer.ClassGuid1, ClassGUID1, sizeof(GUID) );
	memcpy( &pInBuffer.PropertyKey, pPropertyKey, sizeof(DEVPROPKEY) );
	pInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_CM_Get_Class_Property_ExW_Win7 ) ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_CM_Get_Class_Property_ExW_Win7_ ;
	pInBuffer.Reserved = a6;

	pOutBuffer = (LPRPC_OUT_CM_Get_Class_Property_ExW_Win7) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return CR_OUT_OF_MEMORY; }

	ErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS != ErrorCode )
	{
		PB_FreeReply( pOutBuffer );
		return ErrorCode;
	}

	if ( *pulLength < pOutBuffer->ReturnLength )
	{
		*pulLength = pOutBuffer->ReturnLength ;
		PB_FreeReply( pOutBuffer );
		return CR_BUFFER_SMALL ;
	}

	memcpy( pReturnBuffer, pOutBuffer->ReturnBuffer, pOutBuffer->ReturnLength );
	*pulLength = pOutBuffer->ReturnLength;
	PB_FreeReply( pOutBuffer );
	return ERROR_SUCCESS;
}


ULONG WINAPI fake_VerifyCatalogFile(int a1)
{
	SetLastError(NO_ERROR);
	return 0;
}


ULONG WINAPI  
fake_SetupDiGetDevicePropertyW (
	IN HANDLE DeviceInfoSet,
	IN PSP_DEVINFO_DATA DeviceInfoData,
	IN const DEVPROPKEY *PropertyKey,
	OUT DEVPROPTYPE *PropertyType,
	OUT PBYTE PropertyBuffer,
	IN DWORD PropertyBufferSize,
	OUT PDWORD RequiredSize,
	IN DWORD Flags
	)
{
	ULONG ErrorCode = 0, BufferSize = 0 ;
	RPC_IN_SetupDiGetDevicePropertyW pRpcInBuffer = { 0 } ;
	LPRPC_OUT_SetupDiGetDevicePropertyW pRpcOutBuffer = NULL ;

	pRpcInBuffer.RpcHeader.DataLength = 0x230 ;
	pRpcInBuffer.RpcHeader.Flag = _PBSRV_APINUM_SetupDiGetDevicePropertyW_ ;

	wcscpy( pRpcInBuffer.DevInst, (LPWSTR)DeviceInfoData->DevInst );
	memcpy( &pRpcInBuffer.ClassGuid, &DeviceInfoData->ClassGuid, sizeof(GUID) );
	memcpy( &pRpcInBuffer.PropertyKey.fmtid, &PropertyKey->fmtid, sizeof(GUID) );

	pRpcInBuffer.PropertyKey.pid = PropertyKey->pid ;
	pRpcInBuffer.Flags = Flags ;

	pRpcOutBuffer = (LPRPC_OUT_SetupDiGetDevicePropertyW) PB_CallServer( &pRpcInBuffer );
	if ( NULL == pRpcOutBuffer )
	{
		SetLastError( RPC_S_SERVER_UNAVAILABLE );
		return 0;
	}

	ErrorCode = pRpcOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS == ErrorCode )
	{
		BufferSize = pRpcOutBuffer->BufferSize;
		if ( BufferSize > PropertyBufferSize )
		{
			BufferSize = PropertyBufferSize;
			ErrorCode = ERROR_INSUFFICIENT_BUFFER ;
		}

		if ( PropertyBuffer && BufferSize )
		{
			memcpy( PropertyBuffer, pRpcOutBuffer->PropertyBuffer, BufferSize );
		}

		if ( RequiredSize ) { *RequiredSize = pRpcOutBuffer->BufferSize ; }
		if ( PropertyType ) { *PropertyType = pRpcOutBuffer->PropertyType ; }
	}

	PB_FreeReply( pRpcOutBuffer );
	SetLastError( ErrorCode );
	return (ErrorCode == 0) ;
}


typedef enum _SetupApi_ 
{
	CM_Enumerate_Classes_TAG = 0,
	CM_Enumerate_Classes_Ex_TAG,
	CM_Get_Class_NameA_TAG,
	CM_Get_Class_NameW_TAG,
	CM_Get_Class_Name_ExA_TAG,
	CM_Get_Class_Name_ExW_TAG,
	CM_Get_Device_ID_ExA_TAG,
	CM_Get_Device_ID_ExW_TAG,
	CM_Get_Device_ID_Size_Ex_TAG,
	CM_Get_Device_ID_ListA_TAG,
	CM_Get_Device_ID_ListW_TAG,
	CM_Get_Device_ID_List_ExA_TAG,
	CM_Get_Device_ID_List_ExW_TAG,
	CM_Get_Device_ID_List_SizeA_TAG,
	CM_Get_Device_ID_List_SizeW_TAG,
	CM_Get_Device_ID_List_Size_ExA_TAG,
	CM_Get_Device_ID_List_Size_ExW_TAG,
	CM_Get_Device_Interface_ListA_TAG,
	CM_Get_Device_Interface_ListW_TAG,
	CM_Get_Device_Interface_List_ExA_TAG,
	CM_Get_Device_Interface_List_ExW_TAG,
	CM_Get_Device_Interface_List_SizeA_TAG,
	CM_Get_Device_Interface_List_SizeW_TAG,
	CM_Get_Device_Interface_List_Size_ExA_TAG,
	CM_Get_Device_Interface_List_Size_ExW_TAG,
	CM_Locate_DevNodeA_TAG,
	CM_Locate_DevNodeW_TAG,
	CM_Locate_DevNode_ExA_TAG,
	CM_Locate_DevNode_ExW_TAG,
	CM_Get_DevNode_Status_TAG,
	CM_Get_DevNode_Status_Ex_TAG,
	CM_Get_DevNode_Registry_PropertyA_TAG,
	CM_Get_DevNode_Registry_PropertyW_TAG,
	CM_Get_DevNode_Registry_Property_ExA_TAG,
	CM_Get_DevNode_Registry_Property_ExW_TAG,
	CM_Get_Device_Interface_Alias_ExW_TAG,
};

// SetupApi
static HOOKINFOLittle g_HookInfoSetupApi_Array [] = 
{
	{ Nothing_TAG, CM_Enumerate_Classes_TAG, "CM_Enumerate_Classes", 0, NULL, fake_CM_Enumerate_Classes },
	{ Nothing_TAG, CM_Enumerate_Classes_Ex_TAG, "CM_Enumerate_Classes_Ex", 0, NULL, fake_CM_Enumerate_Classes_Ex },
	{ Nothing_TAG, CM_Get_Class_NameA_TAG, "CM_Get_Class_NameA", 0, NULL, fake_CM_Get_Class_NameA },
	{ Nothing_TAG, CM_Get_Class_NameW_TAG, "CM_Get_Class_NameW", 0, NULL, fake_CM_Get_Class_NameW },
	{ Nothing_TAG, CM_Get_Class_Name_ExA_TAG, "CM_Get_Class_Name_ExA", 0, NULL, fake_CM_Get_Class_Name_ExA },
	{ Nothing_TAG, CM_Get_Class_Name_ExW_TAG, "CM_Get_Class_Name_ExW", 0, NULL, fake_CM_Get_Class_Name_ExW },
	{ Nothing_TAG, CM_Get_Device_ID_ExA_TAG, "CM_Get_Device_ID_ExA", 0, NULL, fake_CM_Get_Device_ID_ExA },
	{ Nothing_TAG, CM_Get_Device_ID_ExW_TAG, "CM_Get_Device_ID_ExW", 0, NULL, fake_CM_Get_Device_ID_ExW },
	{ Nothing_TAG, CM_Get_Device_ID_Size_Ex_TAG, "CM_Get_Device_ID_Size_Ex", 0, NULL, fake_CM_Get_Device_ID_Size_Ex },
	{ Nothing_TAG, CM_Get_Device_ID_ListA_TAG, "CM_Get_Device_ID_ListA", 0, NULL, fake_CM_Get_Device_ID_ListA },
	{ Nothing_TAG, CM_Get_Device_ID_ListW_TAG, "CM_Get_Device_ID_ListW", 0, NULL, fake_CM_Get_Device_ID_ListW },
	{ Nothing_TAG, CM_Get_Device_ID_List_ExA_TAG, "CM_Get_Device_ID_List_ExA", 0, NULL, fake_CM_Get_Device_ID_List_ExA },
	{ Nothing_TAG, CM_Get_Device_ID_List_ExW_TAG, "CM_Get_Device_ID_List_ExW", 0, NULL, fake_CM_Get_Device_ID_List_ExW },
	{ Nothing_TAG, CM_Get_Device_ID_List_SizeA_TAG, "CM_Get_Device_ID_List_SizeA", 0, NULL, fake_CM_Get_Device_ID_List_SizeA },
	{ Nothing_TAG, CM_Get_Device_ID_List_SizeW_TAG, "CM_Get_Device_ID_List_SizeW", 0, NULL, fake_CM_Get_Device_ID_List_SizeW },
	{ Nothing_TAG, CM_Get_Device_ID_List_Size_ExA_TAG, "CM_Get_Device_ID_List_Size_ExA", 0, NULL, fake_CM_Get_Device_ID_List_Size_Ex },
	{ Nothing_TAG, CM_Get_Device_ID_List_Size_ExW_TAG, "CM_Get_Device_ID_List_Size_ExW", 0, NULL, fake_CM_Get_Device_ID_List_Size_Ex },
	{ Nothing_TAG, CM_Get_Device_Interface_ListA_TAG, "CM_Get_Device_Interface_ListA", 0, NULL, fake_CM_Get_Device_Interface_ListA },
	{ Nothing_TAG, CM_Get_Device_Interface_ListW_TAG, "CM_Get_Device_Interface_ListW", 0, NULL, fake_CM_Get_Device_Interface_ListW },
	{ Nothing_TAG, CM_Get_Device_Interface_List_ExA_TAG, "CM_Get_Device_Interface_List_ExA", 0, NULL, fake_CM_Get_Device_Interface_List_ExA },
	{ Nothing_TAG, CM_Get_Device_Interface_List_ExW_TAG, "CM_Get_Device_Interface_List_ExW", 0, NULL, fake_CM_Get_Device_Interface_List_ExW },
	{ Nothing_TAG, CM_Get_Device_Interface_List_SizeA_TAG, "CM_Get_Device_Interface_List_SizeA", 0, NULL, fake_CM_Get_Device_Interface_List_SizeA },
	{ Nothing_TAG, CM_Get_Device_Interface_List_SizeW_TAG, "CM_Get_Device_Interface_List_SizeW", 0, NULL, fake_CM_Get_Device_Interface_List_SizeW },
	{ Nothing_TAG, CM_Get_Device_Interface_List_Size_ExA_TAG, "CM_Get_Device_Interface_List_Size_ExA", 0, NULL, fake_CM_Get_Device_Interface_List_Size_ExA },
	{ Nothing_TAG, CM_Get_Device_Interface_List_Size_ExW_TAG, "CM_Get_Device_Interface_List_Size_ExW", 0, NULL, fake_CM_Get_Device_Interface_List_Size_ExW },
	{ Nothing_TAG, CM_Locate_DevNodeA_TAG, "CM_Locate_DevNodeA", 0, NULL, fake_CM_Locate_DevNodeA },
	{ Nothing_TAG, CM_Locate_DevNodeW_TAG, "CM_Locate_DevNodeW", 0, NULL, fake_CM_Locate_DevNodeW },
	{ Nothing_TAG, CM_Locate_DevNode_ExA_TAG, "CM_Locate_DevNode_ExA", 0, NULL, fake_CM_Locate_DevNode_ExA },
	{ Nothing_TAG, CM_Locate_DevNode_ExW_TAG, "CM_Locate_DevNode_ExW", 0, NULL, fake_CM_Locate_DevNode_ExW },
	{ Nothing_TAG, CM_Get_DevNode_Status_TAG, "CM_Get_DevNode_Status", 0, NULL, fake_CM_Get_DevNode_Status },
	{ Nothing_TAG, CM_Get_DevNode_Status_Ex_TAG, "CM_Get_DevNode_Status_Ex", 0, NULL, fake_CM_Get_DevNode_Status_Ex },
	{ Nothing_TAG, CM_Get_DevNode_Registry_PropertyA_TAG, "CM_Get_DevNode_Registry_PropertyA", 0, NULL, fake_CM_Get_DevNode_Registry_PropertyA },
	{ Nothing_TAG, CM_Get_DevNode_Registry_PropertyW_TAG, "CM_Get_DevNode_Registry_PropertyW", 0, NULL, fake_CM_Get_DevNode_Registry_PropertyW },
	{ Nothing_TAG, CM_Get_DevNode_Registry_Property_ExA_TAG, "CM_Get_DevNode_Registry_Property_ExA", 0, NULL, fake_CM_Get_DevNode_Registry_Property_ExA },
	{ Nothing_TAG, CM_Get_DevNode_Registry_Property_ExW_TAG, "CM_Get_DevNode_Registry_Property_ExW", 0, NULL, fake_CM_Get_DevNode_Registry_Property_ExW },
	{ Nothing_TAG, CM_Get_Device_Interface_Alias_ExW_TAG, "CM_Get_Device_Interface_Alias_ExW", 0, NULL, fake_CM_Get_Device_Interface_Alias_ExW },
};


BOOL Hook_pharase8_setupapidll_dep1( IN HMODULE hModule )
{
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	HANDLE hMapping_DeviceSetupClasses = NULL, hMapping_DeviceIdList = NULL ;
	LPVOID pMappedAddr_DeviceSetupClasses = NULL, pMappedAddr_DeviceIdList = NULL ;

	if ( NULL == hModule || g_bIsWhilte_rpc_control_ntsvcs ) { return FALSE; }

	// 2. 过黑白名单
	for (int Index = 0; Index < ARRAYSIZEOF(g_Hook_pharase8_setupapidll_Arrays); Index++ )
	{
		WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, g_Hook_pharase8_setupapidll_Arrays[Index], &bIsWhite, &bIsBlack );
		if ( bIsWhite )
		{
			g_bIsWhilte_rpc_control_ntsvcs = TRUE ;
			return FALSE;
		}
	}

	//
	if ( 0 == g_pMappedAddr_DeviceSetupClasses_array )
	{
		hMapping_DeviceSetupClasses = OpenFileMappingW( SECTION_MAP_READ, FALSE, L"Global\\Proteinbox_DeviceSetupClasses" );
		if ( hMapping_DeviceSetupClasses )
		{
			pMappedAddr_DeviceSetupClasses = MapViewOfFile( hMapping_DeviceSetupClasses, FILE_MAP_READ, 0, 0, 0 );
			if ( pMappedAddr_DeviceSetupClasses )
			{
				g_pMappedAddr_DeviceSetupClasses_ClassIndex = *(DWORD *)pMappedAddr_DeviceSetupClasses;
				g_pMappedAddr_DeviceSetupClasses_array = (int)((char *)pMappedAddr_DeviceSetupClasses + 4);
			}
		}
	}

	if ( 0 == g_pMappedAddr_DeviceIdList_array )
	{
		hMapping_DeviceIdList = OpenFileMappingW( SECTION_MAP_READ, FALSE, L"Global\\Proteinbox_DeviceIdList" );
		if ( hMapping_DeviceIdList )
		{
			pMappedAddr_DeviceIdList = MapViewOfFile( hMapping_DeviceIdList, FILE_MAP_READ, 0, 0, 0 );
			if ( pMappedAddr_DeviceIdList )
			{
				g_pMappedAddr_DeviceIdList_ClassIndex = *(DWORD *)pMappedAddr_DeviceIdList;
				g_pMappedAddr_DeviceIdList_array = (int)((char *)pMappedAddr_DeviceIdList + 4);
			}
		}
	}

	return TRUE;
}


BOOL Hook_pharase8_setupapidll_dep2( HMODULE hModule, char Flag )
{
	BOOL bRet = FALSE;
	int i = 0, TotalCounts = 0 ;

	g_CM_Is_Version_Available_Ex_addr = (_CM_Is_Version_Available_Ex_) GetProcAddress( hModule, "CM_Is_Version_Available_Ex" );

	if ( __ProcessNode->DllBaseAddr.hModuleArrays[KernelBase_TAG] )
	{
		if ( 0x43 == Flag ) // Server 2008 SP1 and Win7 RC 以后才有以下函数,位于cfgmgr.dll
		{
			g_CM_Get_Device_Interface_Property_ExW_addr = (ULONG_PTR) GetProcAddress( hModule, "CM_Get_Device_Interface_Property_ExW" );
			g_CM_Get_Class_Property_ExW_addr = (ULONG_PTR) GetProcAddress( hModule, "CM_Get_Class_Property_ExW" );
		}
	}
	else
	{
		if ( g_SetupDiGetDevicePropertyW_addr && 0x53 == Flag )
		{
			Get_CM_Get_Device_Interface_Property_ExW_address( hModule );
		}
	}

	// 获取函数原始地址
	TotalCounts = ARRAYSIZEOF( g_HookInfoSetupApi_Array );
	for( i=0; i<TotalCounts; i++ )
	{
		g_HookInfoSetupApi_Array[ i ].OrignalAddress = (PVOID) GetProcAddress( hModule, g_HookInfoSetupApi_Array[ i ].FunctionName );
	}

	// 进行Hook
	for( i=0; i<TotalCounts; i++ )
	{
		bRet = HookOne( &g_HookInfoSetupApi_Array[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase8_setupapidll_dep2() - HookOne(); | \"%s\" \n", g_HookInfoSetupApi_Array[ i ].FunctionName );
			return FALSE ;
		}
	}

	if ( g_CM_Is_Version_Available_Ex_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_CM_Is_Version_Available_Ex_addr, fake_CM_Is_Version_Available_Ex );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase8_setupapidll_dep2() - Mhook_SetHook(); | \"CM_Is_Version_Available_Ex\" \n" );
			return FALSE ;
		}
	}

	if ( g_CM_Get_Device_Interface_Property_ExW_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_CM_Get_Device_Interface_Property_ExW_addr, fake_CM_Get_Device_Interface_Property_ExW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase8_setupapidll_dep2() - Mhook_SetHook(); | \"CM_Get_Device_Interface_Property_ExW\" \n" );
			return FALSE ;
		}
	}

	if ( g_CM_Get_Class_Property_ExW_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_CM_Get_Class_Property_ExW_addr, fake_CM_Get_Class_Property_ExW_Win7 );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase8_setupapidll_dep2() - Mhook_SetHook(); | \"CM_Get_Class_Property_ExW\" \n" );
			return FALSE ;
		}
	}
	
	return TRUE;
}


BOOL WINAPI Hook_pharase8_setupapidll( IN HMODULE hModule )
{
	BOOL bRet = FALSE;

	bRet = Hook_pharase8_setupapidll_dep1( hModule );
	if ( FALSE == bRet ) { return TRUE; }

	g_VerifyCatalogFile_addr = (_VerifyCatalogFile_) GetProcAddress( hModule, "VerifyCatalogFile" );
	g_SetupDiGetDevicePropertyW_addr = (_SetupDiGetDevicePropertyW_) GetProcAddress( hModule, "SetupDiGetDevicePropertyW" );

	if ( NULL == __ProcessNode->DllBaseAddr.hModuleArrays[KernelBase_TAG] )
	{
		Hook_pharase8_setupapidll_dep2( hModule, 0x53 );
	}

	// start hook
	bRet = Mhook_SetHook( (PVOID*)&g_VerifyCatalogFile_addr, fake_VerifyCatalogFile );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase8_setupapidll() - Mhook_SetHook(); | \"VerifyCatalogFile\" \n" );
		return FALSE ;
	}

	if ( NULL == g_SetupDiGetDevicePropertyW_addr ) { return TRUE; }

	bRet = Mhook_SetHook( (PVOID*)&g_SetupDiGetDevicePropertyW_addr, fake_SetupDiGetDevicePropertyW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase8_setupapidll() - Mhook_SetHook(); | \"SetupDiGetDevicePropertyW\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase9_cfgmgr32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE;

	if ( Hook_pharase8_setupapidll_dep1(hModule) )
		bRet = Hook_pharase8_setupapidll_dep2( hModule, 0x43 );
	else
		bRet = TRUE;

	return bRet;
}

///////////////////////////////   END OF FILE   ///////////////////////////////