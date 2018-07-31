/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/02/01 [1:2:2012 - 16:02]
* MODULE : \Code\Project\ProteinBoxDLL\PBSetupapidll\PBSetupapidllData.cpp
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

//////////////////////////////////////////////////////////////////////////

ULONG_PTR g_CM_Get_Device_Interface_Property_ExW_addr = 0 ;
int g_pMappedAddr_DeviceSetupClasses_array = 0 ;
int g_pMappedAddr_DeviceSetupClasses_ClassIndex = 0 ;
int g_pMappedAddr_DeviceIdList_array = 0 ;
int g_pMappedAddr_DeviceIdList_ClassIndex = 0 ;

typedef enum _CM_DRP_INFO_ 
{
	CM_DRP_DEVICEDESC                  = 0x00000001,
	CM_DRP_HARDWAREID                  = 0x00000002,
	CM_DRP_COMPATIBLEIDS               = 0x00000003,
	CM_DRP_UNUSED0                     = 0x00000004,
	CM_DRP_SERVICE                     = 0x00000005,
	CM_DRP_UNUSED1                     = 0x00000006,
	CM_DRP_UNUSED2                     = 0x00000007,
	CM_DRP_CLASS                       = 0x00000008,
	CM_DRP_CLASSGUID                   = 0x00000009,
	CM_DRP_DRIVER                      = 0x0000000A,
	CM_DRP_CONFIGFLAGS                 = 0x0000000B,
	CM_DRP_MFG                         = 0x0000000C,
	CM_DRP_FRIENDLYNAME                = 0x0000000D,
	CM_DRP_LOCATION_INFORMATION        = 0x0000000E,
	CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME = 0x0000000F,
	CM_DRP_CAPABILITIES                = 0x00000010,
	CM_DRP_UI_NUMBER                   = 0x00000011,
	CM_DRP_UPPERFILTERS                = 0x00000012,
	CM_DRP_LOWERFILTERS                = 0x00000013,
	CM_DRP_BUSTYPEGUID                 = 0x00000014,
	CM_DRP_LEGACYBUSTYPE               = 0x00000015,
	CM_DRP_BUSNUMBER                   = 0x00000016,
	CM_DRP_ENUMERATOR_NAME             = 0x00000017
};

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

ULONG
CM_Get_Class_Name_filter (
	ULONG ulFlags,
	LPGUID ClassGuid,
	PVOID Buffer, 
	PULONG pulLength, 
	BOOL bIsUnicode
	)
{
	LPWSTR Temp = NULL ;
	int nIndex = 0, pArray = 0, offset = 0, Length = 0 ;
	UNICODE_STRING uniBuffer ;
	ANSI_STRING AnsiBuffer ;

	if ( ulFlags ) { return CR_INVALID_FLAG; }

	// 1. 在全局数组中查找匹配参数二:@ClassGuid
	
	if ( g_pMappedAddr_DeviceSetupClasses_ClassIndex )
	{
		pArray = g_pMappedAddr_DeviceSetupClasses_array ;
		
		do
		{
			if ( IsEqualGUID( (const GUID &)ClassGuid, (const GUID &)pArray ) ) { break; }
			
			++ nIndex ;
			pArray += 0x50 ;
		}
		while ( nIndex < g_pMappedAddr_DeviceSetupClasses_ClassIndex );
	}

	// 2. 未找到退出,找到则继续
	if ( nIndex == g_pMappedAddr_DeviceSetupClasses_ClassIndex ) { return CR_NO_SUCH_VALUE; }

	// 2.1 找到匹配的GUID后,从当前数组+0x10取出内容填充到@Buffer中
	offset = 0x50 * nIndex ;
	Temp = (LPWSTR)( g_pMappedAddr_DeviceSetupClasses_array + offset + 0x10 ) ;
	Length = wcslen(Temp) + 1 ;
	
	if ( *pulLength < (ULONG)Length )
	{
		*pulLength = Length ;
		return CR_BUFFER_SMALL;
	}

	if ( bIsUnicode )
	{
		// 2.1.1 是Unicode格式
		memcpy(Buffer, (PVOID)Temp, 2 * Length );
	}
	else
	{
		// 2.1.2 是Ansi格式
		RtlInitUnicodeString( &uniBuffer, Temp );

		AnsiBuffer.Length = 0;
		AnsiBuffer.MaximumLength = Length;
		AnsiBuffer.Buffer = (PCHAR)Buffer ;

		RtlUnicodeStringToAnsiString( &AnsiBuffer, &uniBuffer, FALSE );
	}

	*pulLength = Length;
	return CR_SUCCESS;
}


DWORD 
CM_Get_Device_ID_List_filter (
	BOOL bIsUnicode,
	ULONG ulFlags, 
	PCSTR pszFilter, 
	PVOID Buffer, 
	ULONG BufferLen
	)
{
	UNICODE_STRING uniBuffer ; 
	ANSI_STRING ansiBuffer ;

	if ( pszFilter || ulFlags & 0xFFFFFEFF )
	{
		// 清空Buffer的内容
		RtlZeroMemory( Buffer, BufferLen );
		return CR_SUCCESS;
	}
	
	if ( BufferLen < (ULONG)g_pMappedAddr_DeviceIdList_ClassIndex ) { return CR_BUFFER_SMALL; }
	
	if ( bIsUnicode )
	{
		memcpy( Buffer, (PVOID)g_pMappedAddr_DeviceIdList_array, 2 * g_pMappedAddr_DeviceIdList_ClassIndex );
	}
	else
	{
		RtlInitUnicodeString( &uniBuffer, (PCWSTR)g_pMappedAddr_DeviceIdList_array );

		ansiBuffer.Length = 0;
		ansiBuffer.MaximumLength = (USHORT)BufferLen;
		ansiBuffer.Buffer = (PCHAR)Buffer;

		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	}
	
	return CR_SUCCESS;
}


ULONG
CM_Get_Device_Interface_List_filter (
	LPGUID ClassGuid,
	PULONG pulOutSize, 
	BOOL bIsUnicode,
	PVOID DeviceInterface, 
	ULONG ulSize, 
	int ulFlags
	)
{
	DWORD ErrorCode = 0, Length = 0 ;
	RPC_IN_CM_Get_Device_Interface_List pInBuffer ;
	LPRPC_OUT_CM_Get_Device_Interface_List pOutBuffer = NULL ;

	pInBuffer.RpcHeader.DataLength = sizeof(pInBuffer) ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_CM_Get_Device_Interface_List_filter_ ;

	memcpy( &pInBuffer.ClassGuid, ClassGuid, sizeof(GUID) );
	pInBuffer.ulFlags = ulFlags ;
	pInBuffer.bIsUnicode = bIsUnicode ;

	pOutBuffer = (LPRPC_OUT_CM_Get_Device_Interface_List) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return CR_FAILURE; }

	ErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS == ErrorCode )
	{
		if ( pulOutSize )
		{
			*pulOutSize = pOutBuffer->Length;
			kfree(pOutBuffer);
			return ErrorCode;
		}

		Length = pOutBuffer->Length;
		if ( ulSize < Length )
		{
			kfree( pOutBuffer );
			return CR_BUFFER_SMALL;
		}

		if ( bIsUnicode ) { Length *= 2; }
		memcpy( DeviceInterface, pOutBuffer->DeviceInterface, Length );
	}

	PB_FreeReply( pOutBuffer );
	return ErrorCode;
}


int 
CM_Get_DevNode_Registry_Property_filter (
	ULONG ulProperty,
	DWORD dnDevInst,
	ULONG ulFlags,
	PULONG pulRegDataType, 
	PVOID Buffer, 
	PULONG pulLength, 
	BOOL bIsUnicode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/04/11 [11:4:2011 - 10:21]

Routine Description:
  只处理@ulProperty为 CM_DRP_DEVICEDESC / CM_DRP_SERVICE / CM_DRP_CLASS / CM_DRP_CLASSGUID / CM_DRP_FRIENDLYNAME
  的情况,对于其他情况,直接返回失败. 即通过拼接字符串"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\*",查询
  其中的指定内容,而后根据查询到内容的格式填充参数@Buffer
    
--*/
{
	ANSI_STRING AnsiBuffer ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hKey = NULL ;
	LPWSTR pBuffer = NULL ;
	BOOL bFlag = FALSE ;
	PKEY_VALUE_PARTIAL_INFORMATION KeyInfo = NULL ;
	ULONG DataLength = 0, ReturnLength = 0, Type = 0, size_in = 0 ;

	if ( ulFlags ) { return CR_INVALID_FLAG; }

	switch ( ulProperty )
	{
	case CM_DRP_DEVICEDESC:
		AnsiBuffer.Buffer = (PCHAR)"DeviceDesc";
		break;
	case CM_DRP_SERVICE:
		AnsiBuffer.Buffer = (PCHAR)"Service";
		break;
	case CM_DRP_CLASS:
		AnsiBuffer.Buffer = (PCHAR)"Class";
		break;
	case CM_DRP_CLASSGUID:
		AnsiBuffer.Buffer = (PCHAR)"ClassGUID";
		break;
	case CM_DRP_FRIENDLYNAME:
		AnsiBuffer.Buffer = (PCHAR)"FriendlyName";
		break;
	default:
		return CR_INVALID_PROPERTY;
	}

	// 1. 拼接注册表键值,查询其中的指定内容
	pBuffer = (LPWSTR) kmalloc( 2 * wcslen( (const wchar_t *)dnDevInst ) + 0x80 );

	wcscpy( pBuffer, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\" );
	wcscat( pBuffer, (LPWSTR)dnDevInst );

	RtlInitUnicodeString( &uniBuffer, pBuffer );
	InitializeObjectAttributes( &ObjAtr, &uniBuffer, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwOpenKey( &hKey, KEY_READ, &ObjAtr );
	kfree( pBuffer );

	if ( ! NT_SUCCESS(status) ) 
	{ 
		return (((status != STATUS_OBJECT_NAME_NOT_FOUND) - 1) & 0xFFFFFFE8) + CR_REGISTRY_ERROR;
	}

	DataLength = *pulLength + 0x10;
	KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION) kmalloc( DataLength + 8 );

	RtlInitUnicodeString( &uniBuffer, (PCWSTR)AnsiBuffer.Buffer );
	status = ZwQueryValueKey( hKey, &uniBuffer, KeyValuePartialInformation, KeyInfo, DataLength, &ReturnLength );
	ZwClose( hKey );

	// 2. 根据查询到内容的格式填充参数@Buffer
	if ( !status || status == STATUS_BUFFER_OVERFLOW )
	{
		Type = KeyInfo->Type;
		if ( Type == REG_SZ || Type == REG_EXPAND_SZ || Type == REG_MULTI_SZ ) { bFlag = TRUE; }
		if ( pulRegDataType ) { *pulRegDataType = Type; }

		// 2.1 初始化Buffer,将其中内容清空
		RtlZeroMemory( Buffer, *pulLength );

		// 2.2 填充Buffer
		if ( FALSE == bFlag || bIsUnicode )
		{
			// 是字符串的情况
			if ( Buffer )
			{
				// 比较 "查询到的内容大小" 和 "参数@pulLength指定的大小",取二者中较小的作为最后值
				size_in = *pulLength ;
				DataLength = KeyInfo->DataLength ;
				if ( DataLength > size_in ) { DataLength = size_in; }

				// 将查询到的信息填充到Buffer中
				memcpy( Buffer, KeyInfo->Data, DataLength );
			}

			*pulLength = KeyInfo->DataLength ;
		}
		else
		{
			// 是数字的情况
			if ( Buffer )
			{
				AnsiBuffer.Length = 0 ;
				AnsiBuffer.MaximumLength = (USHORT) *pulLength;
				AnsiBuffer.Buffer = (PCHAR)Buffer;

				uniBuffer.Length = uniBuffer.MaximumLength = LOWORD(KeyInfo->DataLength);
				uniBuffer.Buffer = (PWSTR)KeyInfo->Data ;

				RtlUnicodeStringToAnsiString( &AnsiBuffer, &uniBuffer, FALSE );
			}

			*pulLength = KeyInfo->DataLength >> 1 ;
		}
	}

	// 3. 收尾工作
	kfree( KeyInfo );
	if ( status == STATUS_BUFFER_OVERFLOW ) { return CR_BUFFER_SMALL; }
	
	return ((status < 0) - 1) & CR_REGISTRY_ERROR;
}


VOID Get_CM_Get_Device_Interface_Property_ExW_address( HMODULE hModule )
{
	PBYTE ptr_cur = NULL, ptr_end = NULL, ptr1 = NULL ;
	BYTE pCode[9] = { 0xF7, 0x45, 0x1C, 0xFF, 0xFF, 0xFF, 0x7F, 0xF, 0x85 }; 

	ptr_cur = (PBYTE)((int)hModule + 0x100);
	ptr_end = ptr_cur + 0x30000 ;

	if ( ptr_cur > ptr_end ) { return; }

	while ( TRUE )
	{
		if ( 0 == memcmp( ptr_cur, pCode, 9 )  )
		{
			ptr1 = ptr_cur - 0x30;
			if ( ptr1 != ptr_cur ) { break; }
		}

_search_:
		++ ptr_cur;
		if ( ptr_cur > ptr_end ) { return; }
	}

	while ( *ptr1 != 0x6A || ptr1[1] != 0x2C || ptr1[2] != 0x68 )
	{
		++ ptr1 ;
		if ( ptr1 == ptr_cur ) { goto _search_ ; }
	}

	g_CM_Get_Device_Interface_Property_ExW_addr = (ULONG_PTR) ptr1 ;
	return;
}


///////////////////////////////   END OF FILE   ///////////////////////////////