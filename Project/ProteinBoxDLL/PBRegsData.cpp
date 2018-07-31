/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/20 [20:9:2010 - 16:50]
* MODULE : D:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDLL\PBRegsData.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "StdAfx.h"
#include "HookHelper.h"
#include "ProteinBoxDLL.h"
#include "PBRegsData.h"

#pragma warning(disable : 4995 )

//////////////////////////////////////////////////////////////////////////


_NtQueryKey_	g_NtQueryKey_addr		= NULL ;
_NtOpenKey_		g_NtOpenKey_addr		= NULL ;
_NtCreateKey_	g_NtCreateKey_addr	= NULL ;
_NtDeleteKey_	g_NtDeleteKey_addr	= NULL ;
_NtEnumerateKey_ g_NtEnumerateKey_addr = NULL ;
_NtEnumerateValueKey_ g_NtEnumerateValueKey_addr = NULL ;


WCHAR szbfe[ MAX_PATH ] = L"\\registry\\machine\\system\\currentcontrolset\\services\\bfe" ;

LIST_ENTRY_EX g_XREG_INFORMATION_HEAD_Redirected = { NULL, NULL, 0 } ;
LIST_ENTRY_EX g_XREG_INFORMATION_HEAD_Orignal	 = { NULL, NULL, 0 } ;

/*static*/ CRITICAL_SECTION g_cs_Regedit ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS
GetRegPath (
	IN HANDLE hRootKey,
	IN PUNICODE_STRING uniObjectName,
	OUT LPWSTR* OrignalPath,
	OUT LPWSTR* RedirectedPath,
	OUT BOOL* bIsHandlerSelfRegPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 18:16]

Routine Description:
  获取对象的原始 & 重定向路径. 调用成功调用者需释放内存
    
Arguments:
  hRootKey - 对象句柄
  uniObjectName - 对象名
  OrignalPath - 保存原始路径
  RedirectedPath - 保存重定向路径

Return Value:
  NTSTATUS
    
--*/
{
	ULONG_PTR length = 0 ;
	LPWSTR DataOrig = NULL, DataRedirect = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 

	// 1. 校验参数合法性
	if ( NULL == uniObjectName || NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | GetRegPath();| 参数不合法  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	length = uniObjectName->Length & 0xFFFE ;

	// 2.1 处理 hRootKey 存在的情况
	if ( hRootKey )
	{
		/*
		查询hRootKey对应的注册表路径,和参数二@uniObjectName->Buffer(操作的子键)进行拼接
		eg: RootKey为"\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user" 子键为"current",则拼接
		后的路径为: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current"
		*/

		// 申请内存
		ULONG Size = 0x100 + length ;
		PKEY_NAME_INFORMATION pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
		if ( NULL == pNameInfo )
		{
			MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| 申请内存失败  \n" );
			return STATUS_ALLOCATE_BUCKET ;
		}

		// 查询
		status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, 0x100, &Size );
		if ( STATUS_BUFFER_OVERFLOW == status )
		{
			Size += length ;
			kfree( pNameInfo );

			pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
			if ( NULL == pNameInfo )
			{
				MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| 申请内存失败(2)  \n" );
				return STATUS_ALLOCATE_BUCKET ;
			}

			status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, Size, &Size );
		}

		// 查询失败则释放内存
		if ( !NT_SUCCESS(status) )
		{
			kfree( pNameInfo );
			MYTRACE( L"error! | GetRegPath() - g_NtQueryKey_addr();| status=0x%08lx \n", status );
			return status ;
		}

		// 查询成功,拼接字符串
		DataOrig = (LPWSTR) kmalloc( pNameInfo->NameLength + length + sizeof(WCHAR)*5 );  // 申请足够的内存

		memcpy( DataOrig, pNameInfo->Name, pNameInfo->NameLength );
		LPWSTR ptr = &DataOrig[ pNameInfo->NameLength / sizeof(WCHAR) ] ;

		if ( length )
		{
			LPWSTR ptr1 = NULL ;
			wcscat( ptr, L"\\" );
			ptr1 = ptr + 1 ;
			memcpy( ptr1, uniObjectName->Buffer, length );
			ptr = &ptr1[ length / sizeof(WCHAR) ] ;
		}
		
		*ptr = UNICODE_NULL ;
		kfree( pNameInfo );
	}
	else
	{
		// 2.2 处理 hRootKey 不存在的情况
		if ( 0 == length ) { return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

		DataOrig = (LPWSTR) kmalloc( length + sizeof(WCHAR)*2 );  // 申请足够的内存
		memcpy( DataOrig, uniObjectName->Buffer, length );
		DataOrig[ length / sizeof(WCHAR) ] = UNICODE_NULL ;
	}

	//
	// 3. 处理得到的注册表全路径
	//

	length = wcslen( DataOrig );
	if ( length < 9 ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	if ( wcsnicmp( DataOrig, L"\\registry", 0x9 ) ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ;  } 
	if ( DataOrig[ 0x9 ] && '\\' != DataOrig[ 0x9 ] ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	// 3.1 获取原始注册表路径
	LPWSTR ptr1 = NULL, ptr2 = NULL, tmp = NULL, FuckedBuffer = NULL ;
	ULONG_PTR KeyRootPathLength = (g_BoxInfo.KeyRootPathLength / sizeof(WCHAR)) - 1 ;
	ULONG_PTR SIDLength = g_RegInfo.RegSIDPathLength ;
	ULONG_PTR FuckedBufferLength = 0, TempLength = 0 ;

	while ( 1 )
	{
		while( TRUE )
		{
			if ( (length < KeyRootPathLength) || (wcsnicmp(DataOrig, g_BoxInfo.KeyRootPath, KeyRootPathLength)) )
			{
				break ; // 若操作的不是 "\REGISTRY\USER\Sandbox_AV_DefaultBox" 下的键值,跳过本段,继续~
			}

			if ( bIsHandlerSelfRegPath ) { *bIsHandlerSelfRegPath = TRUE ; }

			// 此时路径形如: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current",将其转换为真实路径
			ptr1 = &DataOrig[ KeyRootPathLength - 9 ];
			memcpy( ptr1, L"\\registry", 0x12 );

			FuckedBufferLength = (wcslen(ptr1) + 1) * sizeof(WCHAR) ;
			FuckedBuffer = (LPWSTR) kmalloc( FuckedBufferLength );
			memcpy( FuckedBuffer, ptr1, FuckedBufferLength );

			kfree( DataOrig );
			DataOrig = FuckedBuffer ;
			
			length += 9 - KeyRootPathLength ;
		}

		if ( length < 0x16 || wcsnicmp( (LPWSTR) (DataOrig + 0x9), L"\\user\\current", 0xD ) ) { break ; }

		TempLength = SIDLength + (length - 0x15) ;
		tmp = (LPWSTR) kmalloc( TempLength * sizeof(WCHAR) ); 

		memcpy( tmp, g_RegInfo.RegSIDPath, SIDLength * sizeof(WCHAR) );
		memcpy( (LPWSTR)(tmp + SIDLength), (LPWSTR)( DataOrig + 0x16), (length - 0x15) * sizeof(WCHAR) );

		kfree( DataOrig );
		DataOrig = tmp ;

		length = TempLength ;
		-- length ;
	}

	*OrignalPath = DataOrig ;

	// 3.2 获取重定向注册表路径
	if ( g_bIs_Inside_PBRpcSs_exe && 0 == wcsnicmp(DataOrig, szbfe, wcslen(szbfe)) ) { kfree( DataOrig ); return STATUS_OBJECT_NAME_NOT_FOUND ; }

	DataRedirect = (LPWSTR) kmalloc( g_BoxInfo.KeyRootPathLength + length * sizeof(WCHAR) + 0x30 ); // 申请足够的内存

	memcpy( DataRedirect, g_BoxInfo.KeyRootPath, g_BoxInfo.KeyRootPathLength );
	ptr1 = &DataRedirect[ KeyRootPathLength ] ;

	if ( length >= SIDLength && 0 == wcsnicmp( DataOrig, g_RegInfo.RegSIDPath, SIDLength ) )
	{
		memcpy( ptr1, L"\\user\\current", 0x1A );
		ptr1 = (LPWSTR) ( ptr1 + 0x1A / 2) ;
		length -= SIDLength ;
		
		ptr2 = &DataOrig[ SIDLength ];
	}
	else
	{
		length -= 9 ;
		ptr2 = (LPWSTR) (DataOrig + 0x9) ;
	}
	
	memcpy( ptr1, ptr2, length * sizeof(WCHAR) );
	ptr1[ length ] = UNICODE_NULL ;

	*RedirectedPath = DataRedirect ;
	return STATUS_SUCCESS ;
}


NTSTATUS
GetRegPathAAA (
	IN HANDLE hRootKey,
	IN PUNICODE_STRING uniObjectName,
	OUT LPWSTR* OrignalPath,
	OUT LPWSTR* RedirectedPath,
	OUT BOOL* bIsHandlerSelfRegPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 18:16]

Routine Description:
  获取对象的原始 & 重定向路径. 调用成功调用者需释放内存
    
Arguments:
  hRootKey - 对象句柄
  uniObjectName - 对象名
  OrignalPath - 保存原始路径
  RedirectedPath - 保存重定向路径

Return Value:
  NTSTATUS
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	PKEY_NAME_INFORMATION pNameInfo = NULL ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, tmp = NULL, FuckedBuffer = NULL, DataOrig = NULL, DataRedirect = NULL ;
	ULONG_PTR length = 0, KeyRootPathLength = 0, SIDLength = 0, TempLength = 0, Size = 0, FuckedBufferLength = 0 ;


	// 1. 校验参数合法性
	if ( NULL == uniObjectName || NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | GetRegPath();| 参数不合法  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	length = uniObjectName->Length & 0xFFFE ;

	// 2.1 处理 hRootKey 存在的情况
	if ( hRootKey )
	{
		/*
		查询hRootKey对应的注册表路径,和参数二@uniObjectName->Buffer(操作的子键)进行拼接
		eg: RootKey为"\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user" 子键为"current",则拼接
		后的路径为: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current"
		*/

		// 申请内存
		Size = 0x100 + length ;
		pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
		if ( NULL == pNameInfo )
		{
			MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| 申请内存失败  \n" );
			return STATUS_ALLOCATE_BUCKET ;
		}

		// 查询
		status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, 0x100, &Size );
		if ( STATUS_BUFFER_OVERFLOW == status )
		{
			Size += length ;
			kfree( pNameInfo );

			pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
			if ( NULL == pNameInfo )
			{
				MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| 申请内存失败(2)  \n" );
				return STATUS_ALLOCATE_BUCKET ;
			}

			status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, Size, &Size );
		}

		// 查询失败则释放内存
		if ( !NT_SUCCESS(status) )
		{
			kfree( pNameInfo );
			MYTRACE( L"error! | GetRegPath() - g_NtQueryKey_addr();| status=0x%08lx \n", status );
			return status ;
		}

		// 查询成功,拼接字符串
		DataOrig = (LPWSTR) kmalloc( pNameInfo->NameLength + length + 0x20 );  // 申请足够的内存

		memcpy( DataOrig, pNameInfo->Name, pNameInfo->NameLength );
		LPWSTR ptr = &DataOrig[ pNameInfo->NameLength / sizeof(WCHAR) ] ;

		if ( length )
		{
			LPWSTR ptr1 = NULL ;
			wcscat( ptr, L"\\" );
			ptr1 = ptr + 1 ;
			memcpy( ptr1, uniObjectName->Buffer, length );
			ptr = &ptr1[ length / sizeof(WCHAR) ] ;
		}
		
		*ptr = UNICODE_NULL ;
		kfree( pNameInfo );
	}
	else
	{
		// 2.2 处理 hRootKey 不存在的情况
		if ( 0 == length ) { return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

		DataOrig = (LPWSTR) kmalloc( length + 0x20 );  // 申请足够的内存
		memcpy( DataOrig, uniObjectName->Buffer, length );
		DataOrig[ length / sizeof(WCHAR) ] = UNICODE_NULL ;
	}

	//
	// 3. 处理得到的注册表全路径
	//

	TempLength = length = wcslen( DataOrig );
	if ( length < 9 ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	if ( wcsnicmp( DataOrig, L"\\registry", 0x9 ) ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ;  } 
	if ( DataOrig[ 0x9 ] && '\\' != DataOrig[ 0x9 ] ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	// 3.1 获取原始注册表路径
	KeyRootPathLength = (g_BoxInfo.KeyRootPathLength / sizeof(WCHAR)) - 1 ;
	SIDLength = g_RegInfo.RegSIDPathLength ;

	while ( TRUE )
	{
		while( TRUE )
		{
			if ( (length < KeyRootPathLength) || (RtlCompareUnicodeStringDummy(DataOrig, g_BoxInfo.KeyRootPath, (USHORT)KeyRootPathLength)) )
			{
				break ; // 若操作的不是 "\REGISTRY\USER\Sandbox_AV_DefaultBox" 下的键值,跳过本段,继续~
			}

			if ( bIsHandlerSelfRegPath ) { *bIsHandlerSelfRegPath = TRUE ; }

			// 此时路径形如: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current",将其转换为真实路径
			ptr1 = &DataOrig[ KeyRootPathLength - 9 ];
			memcpy( ptr1, L"\\registry", 0x12 );
			
			FuckedBufferLength = (wcslen(ptr1) + 1) * sizeof(WCHAR) ;
			FuckedBuffer = (LPWSTR) kmalloc( FuckedBufferLength );
			memcpy( FuckedBuffer, ptr1, FuckedBufferLength );
			
			kfree( DataOrig );
			DataOrig = FuckedBuffer ;

			length += 9 - KeyRootPathLength ;
		}

		if ( length < 0x16 || wcsnicmp( &DataOrig[ 0x9 ], L"\\user\\current", 0xD ) ) { break ; }

		TempLength = SIDLength + (length - 0x15) ;
		tmp = (LPWSTR) kmalloc( TempLength * sizeof(WCHAR) ); 
		
		memcpy( tmp, g_RegInfo.RegSIDPath, SIDLength * sizeof(WCHAR) );
		memcpy( &tmp[ SIDLength ], &DataOrig[ 0x22 ], (length - 0x15) * sizeof(WCHAR) );

		kfree( DataOrig );
		DataOrig = tmp ;

		length = TempLength ;
		-- length ;
	}

	*OrignalPath = DataOrig ;

	// 3.2 获取重定向注册表路径
	if ( g_bIs_Inside_PBRpcSs_exe && 0 == wcsnicmp(DataOrig, szbfe, wcslen(szbfe)) ) { kfree( DataOrig ); return STATUS_OBJECT_NAME_NOT_FOUND ; }

	DataRedirect = (LPWSTR) kmalloc( g_BoxInfo.KeyRootPathLength + length + 0x30 ); // 申请足够的内存

	memcpy( DataRedirect, g_BoxInfo.KeyRootPath, KeyRootPathLength * sizeof(WCHAR) );
	ptr1 = &DataRedirect[ KeyRootPathLength ] ;

	if ( length >= SIDLength && 0 == wcsnicmp( DataOrig, g_RegInfo.RegSIDPath, SIDLength ) )
	{
		memcpy( ptr1, L"\\user\\current", 0x1A );
		ptr1 += 0x1A / 2;
		
		length -= SIDLength ;
		ptr2 = &DataOrig[ SIDLength ];
	}
	else
	{
		length -= 9 ;
		ptr2 = &DataOrig[ 0x9 ];
	}
	
	memcpy( ptr1, ptr2, length * sizeof(WCHAR) );
	ptr1[ length ] = UNICODE_NULL ;

	*RedirectedPath = DataRedirect ;
	return STATUS_SUCCESS ;
}


BOOL
IsRedirectedKeyInvalid (
	IN LPWSTR RedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/27 [27:9:2010 - 11:06]

Routine Description:
  沙箱会在 \HKEY_USERS\Sandbox_AV_DefaultBox\ 下建立重定向注册表键值,禁止沙箱中的程序直接访问操作当前目录下的键值!
    
Arguments:
  RedirectedPath - 待检测的重定向注册表路径

Return Value:
  TRUE - 拒绝访问
  FALSE - 允许访问
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR ptr, ptr1, szNameTmp = NULL ; 
	OBJECT_ATTRIBUTES	ObjectAttributes ;
	UNICODE_STRING		KeyName ;
	HANDLE				KeyHandle ;
	ULONG				length ;
	ULONG_PTR KeyRootPathLength = (g_BoxInfo.KeyRootPathLength / sizeof(WCHAR)) - 1 ;
	WCHAR tmp[ MAX_PATH ] = L"" ;

	if ( NULL == RedirectedPath ) { return FALSE ; }

	// "\REGISTRY\USER\Sandbox_AV_DefaultBox\Machine\software\0001"
	memcpy( tmp, RedirectedPath, MAX_PATH );

	ptr = &tmp[ KeyRootPathLength ] ;// "\Machine\software\0001"
	ptr1 = wcsrchr(tmp, '\\');  

	if ( NULL == ptr1 || ptr1 == tmp ) { return FALSE ; }

	*ptr1 = 0 ; // "\Machine\software"
	length = wcslen( ptr ); 

	switch ( length )
	{
	case 8:
		szNameTmp = L"\\machine";
		break;

	case 0x11:
		szNameTmp = L"\\machine\\software";
		break;

	case 0x19:
		szNameTmp = L"\\machine\\software\\classes";
		break;

	case 0x1F:
		szNameTmp = L"\\machine\\software\\classes\\clsid";
		break;

	case 0x21:
		szNameTmp = L"\\machine\\software\\classes\\typelib";
		break;

	case 0x23:
		szNameTmp = L"\\machine\\software\\classes\\interface";
		break;

	case 5:
		szNameTmp = L"\\user";
		break;

	case 0xD:
		szNameTmp = L"\\user\\current";
		break;

	case 0x16:
		szNameTmp = L"\\user\\current\\software";
		break;

	case 0x1E:
		szNameTmp = L"\\user\\current\\software\\classes";
		break;

	case 0x24:
		szNameTmp = L"\\user\\current\\software\\classes\\clsid" ;
		break;

	case 0x26:
		szNameTmp = L"\\user\\current\\software\\classes\\typelib" ;
		break;

	case 0x28:
		szNameTmp = L"\\user\\current\\software\\classes\\interface";
		break;

	default:
		break ;
	}

	// 操作的是以上白名单列表,允许访问
	if ( szNameTmp && 0 == wcsicmp(ptr, szNameTmp) ) { return FALSE ; }

	// 对于非白的路径,检查要操作键值的父键是否是Sandbox建立的,若是,则禁止该操作!即不允许访问沙箱自己建立的重定向键值!
	RtlInitUnicodeString( &KeyName, tmp );
	InitializeObjectAttributes( &ObjectAttributes, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = g_NtOpenKey_addr( &KeyHandle, KEY_READ, &ObjectAttributes );
	if ( NT_SUCCESS(status) )
	{
		BOOL bIsDirtyKey = IsDirtyKey( KeyHandle );
		ZwClose( KeyHandle );
		if ( bIsDirtyKey ) { return TRUE ;}  // 返回TRUE表示拒绝 
	}

	return FALSE ; // 默认不阻止
}


BOOL 
IsDirtyKey ( 
	IN HANDLE KeyHandle 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/27 [27:9:2010 - 11:06]

Routine Description:
  检测当前句柄是否为被标记为"Dirty"的键值    
    
Arguments:
  KeyHandle - 待验证的句柄
    
--*/
{
	NTSTATUS status ;
	ULONG ResultLength ;
	KEY_BASIC_INFORMATION KeyInformation = { 0 } ;

	if ( NULL == KeyHandle ) { return FALSE ; }

	status = g_NtQueryKey_addr( KeyHandle, KeyBasicInformation, &KeyInformation, 0x18, &ResultLength );
	
	if ( status && status != STATUS_BUFFER_OVERFLOW ) { return FALSE ; }

	return IsDirtyKeyEx( (PVOID)&KeyInformation ) ;
}


BOOL 
IsDirtyKeyEx ( 
	IN PVOID KeyBaseInfo 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/27 [27:9:2010 - 11:06]

Routine Description:
  检测是否为被标记为"Dirty"的键值    
    
Arguments:
  KeyBaseInfo - 待验证的句柄时间属性
    
--*/
{
	PLARGE_INTEGER LastWriteTime = (PLARGE_INTEGER) KeyBaseInfo ;

	if ( NULL == KeyBaseInfo ) { return FALSE ; }

	return (0x1B01234 == LastWriteTime->HighPart && 0xDEAD44A0 == LastWriteTime->LowPart) ;
}


NTSTATUS MarkDirtyKey( IN HANDLE KeyHandle )
{
	NTSTATUS status ;
	LARGE_INTEGER LastWriteTime = { 0xDEAD44A0, 0x1B01234 }; /*(LARGE_INTEGER)0x1B01234DEAD44A0i64 */
	//								LowPart		HighPart
	
	status = ZwSetInformationKey( KeyHandle, KeyWriteTimeInformation, &LastWriteTime, 8 );
	RemoveRegNodeEx( KeyHandle, TRUE );
	ZwClose( KeyHandle );

	return status ;
}


NTSTATUS MarkKeyTime( IN HANDLE KeyHandle )
{
	NTSTATUS status ;
	FILETIME SystemTimeAsFileTime ;

	if ( NULL == KeyHandle ) { return STATUS_INVALID_PARAMETER ; }

	GetSystemTimeAsFileTime( &SystemTimeAsFileTime );

	status = ZwSetInformationKey( KeyHandle, KeyWriteTimeInformation, &SystemTimeAsFileTime, 8 );
// 	if ( STATUS_ACCESS_DENIED == status )
// 	{
//		HANDLE hKey ;
//		OBJECT_ATTRIBUTES ObjectAttributes ;
//		UNICODE_STRING KeyName ; 

// 		RtlInitUnicodeString(&uniName, (PCWSTR)&g_TmpName_Total );
// 		InitializeObjectAttributes( &ObjectAttributes, &KeyName, OBJ_CASE_INSENSITIVE, KeyHandle, NULL );
// 
// 		status = ZwOpenKey( &hKey, KEY_SET_VALUE, &ObjectAttributes );
// 		if ( NT_SUCCESS(status) )
// 		{
// 			status = NtSetInformationKey( KeyHandle, KeyWriteTimeInformation, &SystemTimeAsFileTime, 8 );
// 			ZwClose( hKey );
// 		}
// 	}

	return status ;
}


NTSTATUS 
CreateRedirectedSubkeys (
	IN POBJECT_ATTRIBUTES objectAttributes
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/28 [28:9:2010 - 19:17]

Routine Description:
  递归创建指定的键值
    
Arguments:
  objectAttributes - 包含待创建键值的属性信息
    
--*/
{
	LPWSTR ptr_start, ptr_end ;
	PUNICODE_STRING uniObjectName ;
	ULONG Disposition, NewLength ;
	USHORT OldMaxLength, OldLength ;
	NTSTATUS status ;
	WCHAR OldData ;
	HANDLE hKey ;

	if ( NULL == objectAttributes )
	{
		MYTRACE( L"error! | CreateRedirectedSubkeys();| 参数不合法  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	uniObjectName = objectAttributes->ObjectName;
	ptr_start = uniObjectName->Buffer ;
	ptr_end = &ptr_start[ uniObjectName->Length / sizeof(WCHAR) ];

	// 1. 逆向行驶,直到成功创建一层注册表键值
	while ( TRUE )
	{
		-- ptr_end ;
		if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; }

		while ( '\\' != *ptr_end ) { -- ptr_end ; if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; } }

		if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; }

		OldData = *ptr_end ;
		OldMaxLength = uniObjectName->MaximumLength ;
		OldLength	 = uniObjectName->Length ;

		*ptr_end = NULL ;
		NewLength = (ULONG)((PCHAR)ptr_end - (PCHAR)ptr_start) ;
		uniObjectName->Length		 = (USHORT) NewLength ;
		uniObjectName->MaximumLength = (USHORT) (NewLength + 2) ;

		status = g_NtCreateKey_addr( &hKey, 0x80000000, objectAttributes, 0, 0, 0, &Disposition );

		uniObjectName->Length		 = OldLength ;
		uniObjectName->MaximumLength = OldMaxLength ;
		*ptr_end = OldData ;

		if ( NT_SUCCESS(status) ) { break ; }

		if ( (STATUS_OBJECT_NAME_NOT_FOUND != status) && (STATUS_OBJECT_PATH_NOT_FOUND != status) ) { return status ; }
	}

	// 2. 开始从找到的那层键值,逐个创建
	ZwClose( hKey );

	if ( REG_OPENED_EXISTING_KEY == Disposition && IsDirtyKey(hKey) ) { return STATUS_OBJECT_NAME_NOT_FOUND ; }
	
	while ( TRUE )
	{
		for ( ++ptr_end; *ptr_end; ++ptr_end )
		{
			if ( '\\' == *ptr_end ) { break ; }
		}

		OldData = *ptr_end ;
		OldMaxLength = uniObjectName->MaximumLength ;
		OldLength	 = uniObjectName->Length ;

		*ptr_end = NULL ;
		NewLength = (ULONG)((PCHAR)ptr_end - (PCHAR)ptr_start) ;
		uniObjectName->Length		 = (USHORT) NewLength ;
		uniObjectName->MaximumLength = (USHORT) (NewLength + 2) ;

		status = g_NtCreateKey_addr( &hKey, 0x80000000, objectAttributes, 0, 0, 0, &Disposition );

		uniObjectName->Length		 = OldLength ;
		uniObjectName->MaximumLength = OldMaxLength ;
		*ptr_end = OldData ;

		if ( ! NT_SUCCESS(status) ) { break ; }
		
		ZwClose( hKey );
		if ( ! OldData ) { return status ; }
	}

	return status ;
}


VOID
RemoveRegNodeEx (
	IN HANDLE KeyHandle,
	IN BOOL bRemoveFatherNode 
	)
{
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;

	if ( NULL == KeyHandle ) { return ; }

	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( NT_SUCCESS(status) )
	{
		RemoveRegNodeExp( OrignalPath, bRemoveFatherNode );
	}
	else
	{
		kfree( OrignalPath );
		kfree( RedirectedPath );
	}

	return ;
}


VOID 
RemoveRegNodeExp ( 
	IN LPWSTR szPath,
	IN BOOL bRemoveFatherNode 
	)
{
	LPWSTR ptr = NULL ;
	LPXREG_INFORMATION_CLASS pCurrentNode, pNextNode ; 

	if ( NULL == szPath ) { return ; }

	ULONG length = wcslen( szPath );
	EnterCriticalSection( &g_cs_Regedit );

	pCurrentNode = (LPXREG_INFORMATION_CLASS) g_XREG_INFORMATION_HEAD_Redirected.Flink ;
	if ( NULL == pCurrentNode ) { LeaveCriticalSection( &g_cs_Regedit ); return ; }
	
	do
	{
		pNextNode = (LPXREG_INFORMATION_CLASS) pCurrentNode->ListEntry.Flink ;
		if ( (2 * length != pCurrentNode->NameLength) || (wcsnicmp(pCurrentNode->wszOrignalRegPath, szPath, length)) ) { goto _WHILE_NEXT_ ; }
		
		// 已在链表中找到待删除的节点
		if ( bRemoveFatherNode )
		{
			ptr = wcsrchr( pCurrentNode->wszOrignalRegPath, '\\' );
			if ( ptr )
			{
				*ptr = NULL ;
				RemoveRegNodeExp( pCurrentNode->wszOrignalRegPath, FALSE );
				*ptr = '\\';
				pNextNode = (LPXREG_INFORMATION_CLASS) pCurrentNode->ListEntry.Flink ;
			}
		}

		RemoveRegNodeCom( (PVOID)pCurrentNode, (PVOID)&g_XREG_INFORMATION_HEAD_Redirected );

_WHILE_NEXT_ :
		pCurrentNode = pNextNode ;
	}
	while ( pNextNode );

	LeaveCriticalSection( &g_cs_Regedit );
	return ;
}


VOID 
RemoveRegNodeCom ( 
	IN PVOID pNode,
	IN PVOID pHead
	)
{
	LPXREG_INFORMATION_SUBKEY   pNodeKey	 = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNodeValue	 = NULL ;
	LPXREG_INFORMATION_CLASS	pCurrentNode = (LPXREG_INFORMATION_CLASS) pNode ;
	LPLIST_ENTRY_EX				pNodeHead	 = (LPLIST_ENTRY_EX) pHead ;

	if ( NULL == pNode || NULL == pHead ) { return ; }

	// 从链表中移除当前节点
	RemoveEntryListEx( pNodeHead, (PLIST_ENTRY)pCurrentNode );

	// 释放当前节点中申请的内存 (所有Key)
	pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
	if ( pNodeKey )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeKey, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeCom(); | 在释放当前节点中申请的内存 (所有Key)过程中,遇到结点指针非法. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNodeKey );
			kfree( pNodeKey );
			pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
		}
		while ( pNodeKey );
	}

	// 释放当前节点中申请的内存 (所有Value)
	pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
	if ( pNodeValue )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeValue, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeCom(); | 在释放当前节点中申请的内存 (所有Value)过程中,遇到结点指针非法. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)pNodeValue );
			kfree( pNodeValue );
			pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
		}
		while ( pNodeValue );
	}

	kfree( pCurrentNode );	
	return ;
}


VOID 
RemoveRegNodeComEx ( 
	IN PVOID pNode,
	IN BOOL bFlag_Free_Current_Node
	)
{
	LPXREG_INFORMATION_SUBKEY	pNodeKey	 = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNodeValue	 = NULL ;
	LPXREG_INFORMATION_CLASS	pCurrentNode = (LPXREG_INFORMATION_CLASS) pNode ;

	if ( NULL == pNode ) { return ; }

	// 释放当前节点中申请的内存 (所有Key)
	pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
	if ( pNodeKey )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeKey, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeComEx(); | 在释放当前节点中申请的内存 (所有Key)过程中,遇到结点指针非法. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNodeKey );
			kfree( pNodeKey );
			pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
		}
		while ( pNodeKey );
	}

	// 释放当前节点中申请的内存 (所有Value)
	pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
	if ( pNodeValue )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeValue, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeComEx(); | 在释放当前节点中申请的内存 (所有Value)过程中,遇到结点指针非法. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)pNodeValue );
			kfree( pNodeValue );
			pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
		}
		while ( pNodeValue );
	}

	if ( bFlag_Free_Current_Node ) { kfree( pCurrentNode ); }	
	return ;
}


NTSTATUS
StubNtDeleteKey (
    IN HANDLE KeyHandle,
	IN BOOL bRecursive
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING KeyName ;
	HANDLE hRedirectedKey, hkey ;
	ULONG Length, ReturnLength ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNeedRemoveNode = FALSE ;
	BOOL bIsHandlerSelfRegPath = FALSE ;
	LPKEYINFO KeyInfo = NULL ;

	if ( NULL == KeyHandle ) { return STATUS_INVALID_PARAMETER ; }

	// 1.1 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - GetRegPath(1); | status=0x%08lx \n", status );
		return status ;
	}

	// 1.2 传入重定向句柄 & 键值路径,再次查询
	RtlInitUnicodeString( &KeyName, RedirectedPath );

	status = ZwOpenKey( &hRedirectedKey, KEY_READ | 0x40000000, &ObjAtr );
	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - ZwOpenKey(); | status=0x%08lx \n", status );
		goto _over_ ;
	}

	// 先释放内存,再进行查询
	kfree( OrignalPath );
	RedirectedPath = NULL ;

	status = GetRegPath (
		hRedirectedKey ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		&bIsHandlerSelfRegPath
		);

	kfree( KeyName.Buffer );

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - GetRegPath(2); | status=0x%08lx \n", status );
		return status ;
	}

	//  
	// 2. 若操作的不是"\REGISTRY\USER\Sandbox_AV_DefaultBox" 下的键值,直接调用原始函数删除掉.
	//    因为NtDeleteKey的参数@KeyHandle肯定是调用NtOpenKey得到,此时已经被重定位到沙箱
	//    目录下了.若未被重定向,表明操作的是白名单键值,放行之. 故到这里一定是白名单键值...
	//

	if ( FALSE == bIsHandlerSelfRegPath )
	{
		ZwClose( hRedirectedKey );
		status = g_NtDeleteKey_addr( KeyHandle );
		goto _over_ ;
	}

	// 3. 删除被重定向到沙箱注册表目录下的键值
	Length = 0x100 ;
	KeyInfo = (LPKEYINFO) kmalloc( Length );
	if ( NULL == KeyInfo )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - kmalloc(); | 申请内存失败 \n" );
		status = STATUS_UNSUCCESSFUL  ;
		goto _over_ ;
	}

	if ( FALSE == bRecursive )
	{
		status = ZwQueryKey( hRedirectedKey, KeyFullInformation, KeyInfo, Length, &ReturnLength );
		if ( status && STATUS_BUFFER_OVERFLOW != status )
		{
			MYTRACE( L"error! | StubNtDeleteKey() - ZwQueryKey; | status=0x%08lx \n", status );
			goto _over_ ;
		}

		if ( KeyInfo->u.Full.SubKeys ) // 若存在子键,拒绝删除
		{
			ZwClose( hRedirectedKey );
			status = STATUS_CANNOT_DELETE ;
			goto _over_ ;
		}
	}

	// 4. 删除当前目录下的所有键
	while ( TRUE )
	{
		while ( TRUE )
		{
			status = g_NtEnumerateKey_addr( hRedirectedKey, 0, KeyBasicInformation, KeyInfo, Length, &ReturnLength );
			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( KeyInfo );
			Length += 0x100 ;
			KeyInfo = (LPKEYINFO) kmalloc( Length );
		}

		// 4.1 当前键没有子键,跳出循环
		if ( status ) { break ; }

		// 4.2 当前键拥有子键,调用Orignal_NtOpenKey打开之
		KeyName.Length = LOWORD( KeyInfo->u.Basic.NameLength ) ;
		KeyName.MaximumLength = KeyName.Length ;
		KeyName.Buffer = KeyInfo->u.Basic.Name ;
		ObjAtr.RootDirectory = hRedirectedKey ;
		
		status = g_NtOpenKey_addr( &hkey, 0x40010000, &ObjAtr );
		if ( !NT_SUCCESS(status) )
		{
			ZwClose( hRedirectedKey );
			goto _over_ ;
		}

		// 4.3 若要求递归,则递归直到找到不存在子键的键
		if ( bRecursive ) { StubNtDeleteKey( hkey, TRUE ); }

		// 4.4 若不要求递归,则调用原始函数删除当前键值
		g_NtDeleteKey_addr( hkey );
		ZwClose( hkey );
	}

	// 5. 若没有子键了.设置当前键为特殊时间.即标记为"已删除"
	if ( STATUS_NO_MORE_ENTRIES == status )
	{
		status = MarkDirtyKey( hRedirectedKey );
	}
	else
	{
		ZwClose( hRedirectedKey );
	}

_over_ :
	if ( KeyInfo ) { kfree( KeyInfo ); }
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


NTSTATUS 
RegKeyFilter (
	IN HANDLE KeyHandle,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN BOOL bIs_NtEnumerateKey_called,
	IN BOOL bIs_NtEnumerateValueKey_called,
	OUT PVOID * out_pCurrentRegNode_Redirected
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 11:38]

Routine Description:
  (1) 通过遍历匹配@KeyHandle查找链表,找到则返回该结点,否则新建之
  黑名单返回STATUS_ACCESS_DENIED拒绝掉; 白名单返回STATUS_BAD_INITIAL_PC标记放行
  (2) 调用Handler_RegNode_Orignal()函数刷新 & 重构结点
  (3) 调用Handler_SubKeyNodes_NtEnumerateKey() 或 Handler_SubValueNodes_NtEnumerateValueKey()函数进一步处理

  注意,返回成功的话,调用者需要释放全局锁,即 LeaveCriticalSection( &g_cs_Regedit );
    
Arguments:
  KeyHandle - 
  OrignalPath - 原始键值路径
  RedirectedPath - 重定向键值路径
  bIs_NtEnumerateKey_called - 是否为NtEnumerateKey的调用
  bIs_NtEnumerateValueKey_called - 是否为NtEnumerateValueKey的调用 
  out_pCurrentRegNode_Redirected - 保存填充完的当前重定向父键结点

Return Value:
  NTSTATUS
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	DWORD TickCount = GetTickCount();
	ULONG NameLength = 0 ;
	HANDLE hRedirectedKey = NULL, out_hKey_Redirected = NULL ;
	BOOL bNeedRemove = FALSE, bBuildNewNode = FALSE ;
	LPXREG_INFORMATION_CLASS pCurrentRedirectedNode = NULL, pNextRedirectedNode = NULL ; 
	LPXREG_INFORMATION_CLASS pCurrentOrignalNode = NULL, pNextOrignalNode = NULL ; 

	EnterCriticalSection( &g_cs_Regedit );

	NameLength = wcslen( OrignalPath ) * sizeof(WCHAR) ;
	pCurrentRedirectedNode = (LPXREG_INFORMATION_CLASS) g_XREG_INFORMATION_HEAD_Redirected.Flink ;
	
	if ( NULL == pCurrentRedirectedNode )
	{ 
		bBuildNewNode = TRUE ; 
	}
	else
	{
		// 通过遍历匹配@hKey查找链表,找到则返回该结点,否则新建之
		while ( TRUE )
		{
			bNeedRemove = FALSE ;
			pNextRedirectedNode = (LPXREG_INFORMATION_CLASS) pCurrentRedirectedNode->ListEntry.Flink ;
			
			if ( KeyHandle == pCurrentRedirectedNode->hKey ) 
			{ 
				if ( (pCurrentRedirectedNode->NameLength == NameLength) && (0 == wcsnicmp(pCurrentRedirectedNode->wszOrignalRegPath, OrignalPath, NameLength / sizeof(WCHAR))) ) { break ; }
				
				bNeedRemove = TRUE ;
			}
			else
			{
				if ( TickCount - pCurrentRedirectedNode->TickCount > 5000 ) { bNeedRemove = TRUE ; }
			}

			// 句柄已匹配到,但名称不匹配或者TickCounts不合法,则必须移除当前的陈旧结点
			if ( bNeedRemove ) { RemoveRegNodeCom( (PVOID)pCurrentRedirectedNode, (PVOID)&g_XREG_INFORMATION_HEAD_Redirected ); }

			pCurrentRedirectedNode = pNextRedirectedNode ;
			if ( NULL == pCurrentRedirectedNode ) { bBuildNewNode = TRUE ; break ; }
		}
	}

	// 新建结点
	if ( bBuildNewNode )
	{
		pCurrentRedirectedNode = (LPXREG_INFORMATION_CLASS) kmalloc( NameLength + sizeof(WCHAR) + sizeof(XREG_INFORMATION_CLASS) );
		
		pCurrentRedirectedNode->TickCount		= TickCount  ;
		pCurrentRedirectedNode->NameLength	= NameLength ;
		pCurrentRedirectedNode->hKey			= KeyHandle  ;
		memcpy( pCurrentRedirectedNode->wszOrignalRegPath, OrignalPath, NameLength + sizeof(WCHAR) );

		InsertListA( &g_XREG_INFORMATION_HEAD_Redirected, NULL, (PLIST_ENTRY)pCurrentRedirectedNode );
	}

	// 处理匹配到的结点
	if ( pCurrentRedirectedNode->bFlag_IsIn_WhiteList )
	{
		LeaveCriticalSection( &g_cs_Regedit );
		return STATUS_BAD_INITIAL_PC ;
	}

	BOOL bNeedHandlerKeysVaules = FALSE ;
	BOOL bPleaseDonotFlush = ( FALSE == bIs_NtEnumerateKey_called || pCurrentRedirectedNode->bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK ) 
				&& ( FALSE == bIs_NtEnumerateValueKey_called || pCurrentRedirectedNode->bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK );

	if ( bPleaseDonotFlush )
	{
		bNeedHandlerKeysVaules = TRUE ;
		hRedirectedKey = NULL ;
		pCurrentOrignalNode = NULL ;
		out_hKey_Redirected = NULL ;
	}
	else
	{
		// 重要的一步,刷新 & 重构结点  
		status = Handler_RegNode_Orignal (
			&out_hKey_Redirected,
			OrignalPath,
			RedirectedPath,
			(PVOID *) &pCurrentOrignalNode
			);

		if ( NT_SUCCESS(status) )
		{
			hRedirectedKey = out_hKey_Redirected;
			bNeedHandlerKeysVaules = TRUE ;
		}

		if ( STATUS_BAD_INITIAL_PC == status ) { pCurrentRedirectedNode->bFlag_IsIn_WhiteList = TRUE ; }
	}

	if ( bNeedHandlerKeysVaules )
	{
		status = STATUS_SUCCESS ;

		// 处理NtEnumerateKey的情况 
		if ( bIs_NtEnumerateKey_called && FALSE == pCurrentRedirectedNode->bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK )
		{
			status = Handler_SubKeyNodes_NtEnumerateKey( pCurrentRedirectedNode, pCurrentOrignalNode, hRedirectedKey );
			pCurrentRedirectedNode->bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK = TRUE ;
			hRedirectedKey = out_hKey_Redirected;
		}

		// 处理NtEnumerateValueKey的情况
		if ( status >= 0 && bIs_NtEnumerateValueKey_called && FALSE == pCurrentRedirectedNode->bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK )
		{
			status = Handler_SubValueNodes_NtEnumerateValueKey( pCurrentRedirectedNode, pCurrentOrignalNode, hRedirectedKey );
			pCurrentRedirectedNode->bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK = 1;
			hRedirectedKey = out_hKey_Redirected;	
		}

		if ( hRedirectedKey ) { ZwClose( hRedirectedKey ); }
	}
 
	if ( !NT_SUCCESS(status) ) { LeaveCriticalSection( &g_cs_Regedit ); }
	*out_pCurrentRegNode_Redirected = (PVOID) pCurrentRedirectedNode ;
	return status ;
}


NTSTATUS 
Handler_SubKeyNodes_NtEnumerateKey (
	IN OUT LPXREG_INFORMATION_CLASS RedirectedNode,
	IN LPXREG_INFORMATION_CLASS OrignalNode,
	IN HANDLE hRedirectedKey
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 16:06]

Routine Description:
  (1) 更新重定向结点即当前父键的LastWriteTime
  (2) "扩充"当前重定向父键+0x20处的内容
  为什么叫"扩充"? 比如在沙箱中操作原始键值(简称A目录),新建了N个子键,
  这些新建的键被重定向到沙箱自己维护的注册表目录下(简称B目录).呈现给
  用户的是A目录+B目录的组合.故若原始键值存在,进行完相应操作后得到的是
  原始键值下的所有子键信息,还需得到重定向目录下所有子键信息,再进行整合.

  (3) 在整合过程中,若发现被标记为"已删除"结点,则从链表中移除之.
  这样做的效果是,在沙箱中展示给用户,用户看不到"已删除"的键值
    
Arguments:
  RedirectedNode - 要填充的重定向父键结点+0x20处的子键链表
  OrignalNode - 原始父键结点
  hRedirectedKey - 重定向父键句柄

Return Value:
  NTSTATUS

--*/
{
	int ret = 0 ;
	BOOL bIsDirtykey = TRUE, bFreeNode = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPXREG_INFORMATION_SUBKEY RedirectedNew = NULL, pNode = NULL ;
	ULONG ResultLength, Length, NameLength, Index = 0 ;
	LPKEYINFO KeyInformation = (LPKEYINFO) kmalloc( 0x80 ) ;

	if ( NULL == hRedirectedKey || NULL == RedirectedNode ) { return STATUS_INVALID_PARAMETER ; }

	// 1.1 打开重定向键值,查询信息
	status = g_NtQueryKey_addr( hRedirectedKey, KeyBasicInformation, KeyInformation, 0x80, &ResultLength );
	if ( !NT_SUCCESS(status) && STATUS_BUFFER_OVERFLOW != status )
	{
		kfree( KeyInformation );
		return status ;
	}

	// 1.2 更新重定向结点即当前父键的LastWriteTime
	RedirectedNode->LastWriteTime.LowPart  = KeyInformation->u.Basic.LastWriteTime.LowPart  ;
	RedirectedNode->LastWriteTime.HighPart = KeyInformation->u.Basic.LastWriteTime.HighPart ;

	// 1.3 对于存在原始键值的情况进行处理
	if ( OrignalNode )
	{
		// 1.3.1 若原始键值的最后写入时间是最新的,则更新到重定向键值中去
		if ( OrignalNode->LastWriteTime.HighPart > RedirectedNode->LastWriteTime.HighPart || OrignalNode->LastWriteTime.LowPart > RedirectedNode->LastWriteTime.LowPart )
		{
			RedirectedNode->LastWriteTime.LowPart  = KeyInformation->u.Basic.LastWriteTime.LowPart  ;
			RedirectedNode->LastWriteTime.HighPart = KeyInformation->u.Basic.LastWriteTime.HighPart ;
		}
  
		// 1.3.2 原始键值存在,之前已经建立了一份原始子键链表.故仅需从原始父键中取出子键链表中的所有节点,挨个插入到重定向父键中即可
		LPXREG_INFORMATION_SUBKEY CopyedOrignalNode = NULL ;
		LPXREG_INFORMATION_SUBKEY CurrentOrignalNode = OrignalNode->pSubKey_ShowTime_Firstone ;
		
		if ( CurrentOrignalNode )
		{
			while ( TRUE )
			{
				Length = CurrentOrignalNode->NameLength + sizeof(XREG_INFORMATION_SUBKEY) + sizeof(WCHAR) ;
				CopyedOrignalNode = (LPXREG_INFORMATION_SUBKEY) kmalloc( Length ) ; // 分配结点

				CopyedOrignalNode->NameLength = NameLength = CurrentOrignalNode->NameLength ;

				memcpy( CopyedOrignalNode->wszSubKeyName, CurrentOrignalNode->wszSubKeyName, NameLength );
				CopyedOrignalNode->wszSubKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

				// 插入到重定向子键链表中去
				InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, 0, (PLIST_ENTRY)CopyedOrignalNode );

				CurrentOrignalNode = CurrentOrignalNode->pFlink ;
				if ( NULL == CurrentOrignalNode ) { break ; }
			}
		}
	}

	// 1.4 "扩充"当前重定向父键+0x20处的内容
	Length = 0x80 ;

	while( TRUE )
	{
		// 1.4.1 调用原始函数查询重定向目录每个子键的信息
		while ( TRUE )
		{
			status = g_NtEnumerateKey_addr( hRedirectedKey, Index, KeyNodeInformation, KeyInformation, Length, &ResultLength );
			
			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( KeyInformation );
			Length += 0x80 ;
			KeyInformation = (LPKEYINFO) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( KeyInformation );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// 1.4.2 将查询得到的子键信息填入新结点 struct _XREG_INFORMATION_SUBKEY_
		ResultLength = KeyInformation->u.Node.NameLength + sizeof(XREG_INFORMATION_SUBKEY) + sizeof(WCHAR) ;
		RedirectedNew = (LPXREG_INFORMATION_SUBKEY) kmalloc( ResultLength );

		RedirectedNew->NameLength = NameLength = KeyInformation->u.Node.NameLength ;

		memcpy( RedirectedNew->wszSubKeyName, KeyInformation->u.Node.Name, NameLength );
		RedirectedNew->wszSubKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		RedirectedNew->LastWriteTime.LowPart  = KeyInformation->u.Node.LastWriteTime.LowPart  ;
		RedirectedNew->LastWriteTime.HighPart = KeyInformation->u.Node.LastWriteTime.HighPart ;
		RedirectedNew->bClassName = (KeyInformation->u.Node.TitleIndex || KeyInformation->u.Node.ClassOffset != 0xFFFFFFFF || KeyInformation->u.Node.ClassLength) ;

		bIsDirtykey = IsDirtyKeyEx( (PVOID) &KeyInformation->u.Basic.LastWriteTime );

		bFreeNode = FALSE ;

		// 1.4.3 填充当前父键+0x20处的子键链表
		pNode = RedirectedNode->pSubKey_ShowTime_Firstone ;
		
		if ( pNode )
		{
			// 1.4.4 按照Subkey的名字长度进行排序,然后插入到当前父键+0x20处的子键链表中
			while ( TRUE )
			{
				ret = wcsicmp( pNode->wszSubKeyName, RedirectedNew->wszSubKeyName );

				if ( 0 == ret ) { break ; }
				if ( ret > 0 ) { goto _INSERT_ ; }

				pNode = pNode->pFlink ;
				if ( NULL == pNode ) { goto _INSERT_ ; }
			}

			// 1.4.5 若在当前父键+0x20处的子键链表中找到当前子键,分2种情况处理
			if ( bIsDirtykey )
			{
				// (A) 被标记为"已删除",则从链表中移除当前结点
				RemoveEntryListEx( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNode );
				kfree( pNode );
			}
			else
			{
				// (B) 是正常键,则更新最后写入时间LastWriteTime
				pNode->LastWriteTime.LowPart	= RedirectedNew->LastWriteTime.LowPart  ;
				pNode->LastWriteTime.HighPart	= RedirectedNew->LastWriteTime.HighPart ;

				if ( RedirectedNew->bClassName ) { pNode->bClassName = RedirectedNew->bClassName ; }
			}

			bFreeNode = TRUE ;
		}
		else
		{
_INSERT_ :
			if ( bIsDirtykey ) 
			{
				bFreeNode = TRUE ;
			}
			else
			{
				// 将枚举得到的当前子键结点插入到链表中.
				if ( pNode )
				{
					InsertListB( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNode, (PLIST_ENTRY)RedirectedNew );
				}
				else 
				{
					InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, NULL, (PLIST_ENTRY)RedirectedNew );
				}
			}
		}

		if ( bFreeNode ) { kfree( RedirectedNew ); }
		++ Index ; // 继续枚举下个子键
		
	} // end-of-while

	kfree( KeyInformation );
	return status ;
}


NTSTATUS 
Handler_SubValueNodes_NtEnumerateValueKey (
	IN LPXREG_INFORMATION_CLASS RedirectedNode,
	IN LPXREG_INFORMATION_CLASS OrignalNode,
	IN HANDLE hRedirectedKey
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 16:06]

Routine Description:
  原理同 Handler_SubKeyNodes_NtEnumerateKey()
    
--*/
{
	int ret = 0 ;
	BOOL bIsDirtyValuekey = TRUE, bFreeNode = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPXREG_INFORMATION_VALUEKEY RedirectedNew = NULL, OrigNode = NULL ;
	ULONG ResultLength, Length, NameLength, ValueDataLength, Index = 0 ;
	PKEY_VALUE_FULL_INFORMATION ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( 0x80 ) ;

	if ( NULL == hRedirectedKey || NULL == RedirectedNode ) { return STATUS_INVALID_PARAMETER ; }

	// 1. 对于存在原始键值的情况进行处理
	if ( OrignalNode )
	{
		// 原始键值存在,之前已经建立了一份原始键值链表.故仅需从原始父键中取出键值链表中的所有节点,挨个插入到重定向父键中即可
		LPXREG_INFORMATION_VALUEKEY CopyedOrignalNode = NULL ;
		LPXREG_INFORMATION_VALUEKEY CurrentOrignalNode = OrignalNode->pValueKey_ShowTime_Lastone ;

		if ( CurrentOrignalNode )
		{
			while ( TRUE )
			{
				Length = CurrentOrignalNode->NameLength + CurrentOrignalNode->ValueDataLength + sizeof(XREG_INFORMATION_VALUEKEY) + sizeof(WCHAR) ;
				CopyedOrignalNode = (LPXREG_INFORMATION_VALUEKEY) kmalloc( Length ) ; // 分配结点

				CopyedOrignalNode->NameLength = NameLength = CurrentOrignalNode->NameLength ;

				memcpy( CopyedOrignalNode->wszValueKeyName, CurrentOrignalNode->wszValueKeyName, NameLength );
				CopyedOrignalNode->wszValueKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

				CopyedOrignalNode->ValueType = CurrentOrignalNode->ValueType ;
				CopyedOrignalNode->ValueData = (char *)&CopyedOrignalNode->wszValueKeyName[0] + CopyedOrignalNode->NameLength + 2 ;
				CopyedOrignalNode->ValueDataLength = ValueDataLength = CurrentOrignalNode->ValueDataLength ;

				memcpy( CopyedOrignalNode->ValueData, CurrentOrignalNode->ValueData, ValueDataLength );

				// 插入到重定向键值链表中去
				InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, 0, (PLIST_ENTRY)CopyedOrignalNode );

				CurrentOrignalNode = CurrentOrignalNode->pFlink ;
				if ( NULL == CurrentOrignalNode ) { break ; }
			}
		}
	}

	// 2. "扩充"内容
	Length = 0x80 ;
	while( TRUE )
	{
		// 2.1 调用原始函数查询重定向目录每个键值的信息
		while ( TRUE )
		{
			status = g_NtEnumerateValueKey_addr( hRedirectedKey, Index, KeyValueFullInformation, ValueInfo, Length, &ResultLength );

			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( ValueInfo );
			Length += 0x80 ;
			ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( ValueInfo );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// 2.2 将查询得到的键值信息填入新结点 struct _XREG_INFORMATION_VALUEKEY_
		ResultLength = ValueInfo->NameLength + ValueInfo->DataLength + sizeof(XREG_INFORMATION_VALUEKEY) + sizeof(WCHAR) ;
		RedirectedNew = (LPXREG_INFORMATION_VALUEKEY) kmalloc( ResultLength );

		RedirectedNew->NameLength = NameLength = ValueInfo->NameLength ;

		memcpy( RedirectedNew->wszValueKeyName, ValueInfo->Name, NameLength );
		RedirectedNew->wszValueKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		RedirectedNew->ValueType = ValueInfo->Type ;
		RedirectedNew->ValueData = (char *)&RedirectedNew->wszValueKeyName[0] + ValueInfo->NameLength + 2 ;
		RedirectedNew->ValueDataLength = ValueInfo->DataLength ;

		memcpy( RedirectedNew->ValueData, (char *)ValueInfo + ValueInfo->DataOffset, ValueInfo->DataLength );

		bFreeNode = FALSE ;

		// 2.3 填充当前父键+0xXX处的键值链表
		OrigNode = RedirectedNode->pValueKey_ShowTime_Lastone ;
		bIsDirtyValuekey = ValueInfo->Type ==  _DirtyValueKeyTag_ ;

		if ( OrigNode )
		{
			// 按照ValueKey的名字长度进行排序,插入到当前父键中
			while ( TRUE )
			{
				ret = wcsicmp( OrigNode->wszValueKeyName, RedirectedNew->wszValueKeyName );

				if ( 0 == ret ) { break ; }
				if ( ret > 0 ) { goto _INSERT_ ; }

				OrigNode = OrigNode->pFlink ;
				if ( NULL == OrigNode ) { goto _INSERT_ ; }
			}

			// 1.4.5 在当前父键的键值链表中找到当前键值
			if ( FALSE == bIsDirtyValuekey )
			{
				InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)OrigNode, (PLIST_ENTRY)RedirectedNew );
				RedirectedNew = NULL ;
			}
			
			RemoveEntryListEx( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)OrigNode );
			kfree( OrigNode );
			OrigNode = NULL ;
		}

_INSERT_ :
		if ( RedirectedNew )
		{
			if ( bIsDirtyValuekey ) 
			{
				bFreeNode = TRUE ;
			}
			else
			{
				// 将枚举得到的当前子键结点插入到链表中.
				if ( OrigNode )
				{
					InsertListB( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)OrigNode, (PLIST_ENTRY)RedirectedNew );
				}
				else 
				{
					InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, NULL, (PLIST_ENTRY)RedirectedNew );
				}
			}
		}

		if ( bFreeNode ) { kfree( RedirectedNew ); }
		++ Index ; // 继续枚举下个子键

	} // end-of-while

	kfree( ValueInfo );
	return status ;
}


NTSTATUS 
Handler_RegNode_Orignal (
	OUT PHANDLE phRedirectedKey,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	OUT PVOID *pOrignalNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 16:06]

Routine Description:
  (1) 黑白名单过滤.黑名单返回STATUS_ACCESS_DENIED拒绝掉; 白名单返回STATUS_BAD_INITIAL_PC标记放行
  (2) 查询重定向键(当前父键)值,若失败返回STATUS_KEY_DELETED;若为特殊时间,则拒绝掉
  (3) 若重定向键值是正常,则查询原始键值,根据情况刷新重建子键(Subkeys)&键值(ValueKeys)
    
Arguments:
     - 
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	HANDLE hOrignalKey = NULL ;
	ULONG ResultLength ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING KeyName ;
	KEY_BASIC_INFORMATION KeyBaseInfo = { 0 } ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNeedRemoveNode = FALSE ;

	if ( NULL == phRedirectedKey || NULL == OrignalPath || NULL == RedirectedPath || NULL == pOrignalNode ) { return STATUS_INVALID_PARAMETER ; }

	*phRedirectedKey = NULL ;
	*pOrignalNode	 = NULL ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	// 1.0 黑白名单过滤
	WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, OrignalPath, &bIsWhite, &bIsBlack );

	// 1.1 黑名单直接禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | Handler_RegNode_Orignal(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		return STATUS_ACCESS_DENIED ;
	}

	// 1.2 白名单放行
	if ( bIsWhite ) { return STATUS_BAD_INITIAL_PC ; }

	// 2.1 查询重定向键(当前父键)值若被标记为"已删除",则拒绝掉
	RtlInitUnicodeString( &KeyName, RedirectedPath );

	status = g_NtOpenKey_addr( phRedirectedKey, KEY_READ, &ObjAtr );
	if ( !NT_SUCCESS(status) )
	{
		if ( STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status )
		{
			status = STATUS_BAD_INITIAL_PC ;
		}

		*phRedirectedKey = NULL ;
		return status ;
	}

	status = g_NtQueryKey_addr( *phRedirectedKey, KeyBasicInformation, &KeyBaseInfo, 0x18, &ResultLength );
	if ( !NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW ) 
	{
		ZwClose( *phRedirectedKey );
		*phRedirectedKey = NULL ;
		return status ;
	}

	if ( IsDirtyKeyEx( (PVOID)&KeyBaseInfo ) )
	{
		ZwClose( *phRedirectedKey );
		*phRedirectedKey = NULL ;
		return STATUS_KEY_DELETED ;
	}

	// 2.2 重定向键值是正常,查询原始键值
	RtlInitUnicodeString( &KeyName, OrignalPath );
	status = g_NtOpenKey_addr( &hOrignalKey, KEY_READ, &ObjAtr );

	if ( !NT_SUCCESS(status) )
	{
		//
		// 2.3.1 原始键值打开失败,status为这2种状态表明:原始键值本来就不存在! 因为是在沙箱中新建键值,会被重定向,
		// 根本就不存在原始键值!若是在沙箱中对原始键值进行操作,则会存在原始键值!
		//

		if ( STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status ) { return STATUS_SUCCESS ; }
	}
	else
	{
		// 2.3.2 查询原始键值的LastWriteTime
		status = g_NtQueryKey_addr( hOrignalKey, KeyBasicInformation, &KeyBaseInfo, 0x18, &ResultLength );
		status = g_NtQueryKey_addr( hOrignalKey, KeyBasicInformation, &KeyBaseInfo, 0x18, &ResultLength );

		// 更新原始键值的结点信息
		if ( status >= 0 || STATUS_BUFFER_OVERFLOW == status )
		{
			status = Refresh_Orignal_RegNode( hOrignalKey, (PLARGE_INTEGER)&KeyBaseInfo, OrignalPath, pOrignalNode );
		}

		ZwClose( hOrignalKey );
	}

	if ( !NT_SUCCESS(status) )
	{
		ZwClose( *phRedirectedKey );
		*phRedirectedKey = NULL ;
		*pOrignalNode	 = NULL ;
		return status ;
	}

	return STATUS_SUCCESS ;
}


NTSTATUS 
Refresh_Orignal_RegNode (
	IN HANDLE hOrignalKey,
	IN PLARGE_INTEGER LastWriteTime,
	IN LPWSTR OrignalPath,
	OUT PVOID *out_RegNode_Orignal
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG NameLength = 0 ;
	DWORD TickCount = GetTickCount();
	BOOL bNeedRemove = FALSE, bBuildNewNode = FALSE ;
	LPXREG_INFORMATION_CLASS pCurrentOrignalNode = NULL, pNextOrignalNode = NULL ; 

	if ( NULL == hOrignalKey || NULL == LastWriteTime || NULL == OrignalPath || NULL == out_RegNode_Orignal ) { return STATUS_INVALID_PARAMETER ; }

	NameLength = wcslen( OrignalPath ) * sizeof(WCHAR) ;
	pCurrentOrignalNode = (LPXREG_INFORMATION_CLASS) g_XREG_INFORMATION_HEAD_Orignal.Flink ;

	if ( NULL == pCurrentOrignalNode )
	{ 
		bBuildNewNode = TRUE ; 
	}
	else
	{
		// 通过遍历匹配@hKey查找链表,找到则返回该结点,否则新建之
		while ( TRUE )
		{
			bNeedRemove = FALSE ;
			pNextOrignalNode = (LPXREG_INFORMATION_CLASS) pCurrentOrignalNode->ListEntry.Flink ;

			if ( TickCount - pCurrentOrignalNode->TickCount > 30000 ) 
			{ 
				if ( (pCurrentOrignalNode->NameLength == NameLength) && (0 == wcsnicmp(pCurrentOrignalNode->wszOrignalRegPath, OrignalPath, NameLength / sizeof(WCHAR))) )
				{
					// 找到了名字相匹配的结点,若LastWriteTime也相同,返回该结点
					if ( LastWriteTime->LowPart == pCurrentOrignalNode->LastWriteTime.LowPart && LastWriteTime->HighPart == pCurrentOrignalNode->LastWriteTime.HighPart )
					{
						*out_RegNode_Orignal = (PVOID) pCurrentOrignalNode ;
						return STATUS_SUCCESS ;
					}
					
					// LastWriteTime不同,则重新建立当前的pCurrentNode,即修正LastWriteTime,更新@pCurrentNode 中的2个子链表!
					RemoveRegNodeComEx( (PVOID)pCurrentOrignalNode, FALSE );
					break ;
				}
			}
			else
			{
				bNeedRemove = TRUE ; // 检查链表中所有结点TickCount的合法性,对于非法结点,移除掉
			}

			if ( bNeedRemove ) { RemoveRegNodeCom( (PVOID)pCurrentOrignalNode, (PVOID)&g_XREG_INFORMATION_HEAD_Orignal ); }

			pCurrentOrignalNode = pNextOrignalNode ;
			if ( NULL == pCurrentOrignalNode ) { bBuildNewNode = TRUE ; break ; }
		}
	}

	// 新建结点
	if ( bBuildNewNode )
	{
		pCurrentOrignalNode = (LPXREG_INFORMATION_CLASS) kmalloc( NameLength + sizeof(XREG_INFORMATION_CLASS) );

		pCurrentOrignalNode->NameLength	= NameLength ;
		memcpy( pCurrentOrignalNode->wszOrignalRegPath, OrignalPath, NameLength + 2 );

		InsertListA( &g_XREG_INFORMATION_HEAD_Orignal, NULL, (PLIST_ENTRY)pCurrentOrignalNode );
	}

	// 刷新结点
	pCurrentOrignalNode->LastWriteTime.LowPart	= LastWriteTime->LowPart  ;
	pCurrentOrignalNode->LastWriteTime.HighPart = LastWriteTime->HighPart ;

	// 更新当前原始键值结点,主要是重新存储其所有子键(SubKeys) & 键值(ValueKeys) 信息 
	status = BuildSubKeysLists( pCurrentOrignalNode, hOrignalKey );
	if ( status < 0 || (status = BuildValueKeysLists( pCurrentOrignalNode, hOrignalKey ), status < 0) )
	{
		RemoveEntryListEx( (LPLIST_ENTRY_EX)&g_XREG_INFORMATION_HEAD_Orignal, (PLIST_ENTRY)pCurrentOrignalNode );
		RemoveRegNodeComEx( (PVOID)pCurrentOrignalNode, TRUE );
	}
	else
	{
		*out_RegNode_Orignal = pCurrentOrignalNode ;
	}

	return status ;
}


NTSTATUS 
BuildSubKeysLists (
	IN LPXREG_INFORMATION_CLASS pCurrentOrignalNode,
	IN HANDLE hOrignalKey
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	BOOL bInsertToHead = FALSE ; 
	ULONG ResultLength, Length = 0x80, NameLength, Index = 0 ;
	LPXREG_INFORMATION_SUBKEY NewNode = NULL, pNode = NULL, FuckedNode = NULL ;
	PKEY_NODE_INFORMATION KeyInformation = (PKEY_NODE_INFORMATION) kmalloc( 0x80 ) ;

	if ( NULL == pCurrentOrignalNode || NULL == hOrignalKey ) { return STATUS_INVALID_PARAMETER ; }

	while( TRUE )
	{
		// 调用原始函数查询原始目录每个子键的信息
		while ( TRUE )
		{
			status = g_NtEnumerateKey_addr( hOrignalKey, Index, KeyNodeInformation, KeyInformation, Length, &ResultLength );

			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( KeyInformation );
			Length += 0x80 ;
			KeyInformation = (PKEY_NODE_INFORMATION) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( KeyInformation );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// 枚举当前目录下的所有子键,对每个子键,申请新结点存在之,组成链表
		bInsertToHead = FALSE ; 

		ResultLength = KeyInformation->NameLength + sizeof(XREG_INFORMATION_SUBKEY) + sizeof(WCHAR) ;
		NewNode = (LPXREG_INFORMATION_SUBKEY) kmalloc( ResultLength );

		NewNode->NameLength = NameLength = KeyInformation->NameLength ;

		memcpy( NewNode->wszSubKeyName, KeyInformation->Name, NameLength );
		NewNode->wszSubKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		NewNode->LastWriteTime.LowPart	= KeyInformation->LastWriteTime.LowPart  ;
		NewNode->LastWriteTime.HighPart	= KeyInformation->LastWriteTime.HighPart ;
		NewNode->bClassName = (KeyInformation->TitleIndex || KeyInformation->ClassOffset != 0xFFFFFFFF || KeyInformation->ClassLength) ;

		FuckedNode = pCurrentOrignalNode->pSubKey_ShowTime_Lastone  ;
		pNode = pCurrentOrignalNode->pSubKey_ShowTime_Firstone ;

		if ( (FuckedNode && wcsicmp( FuckedNode->wszSubKeyName, NewNode->wszSubKeyName ) < 0) || (NULL == pNode) )
		{
			bInsertToHead = TRUE ; 
		}
		else
		{
			while ( wcsicmp( pNode->wszSubKeyName, NewNode->wszSubKeyName) <= 0 )
			{
				pNode = pNode->pFlink ;
				if ( NULL == pNode )
				{
					bInsertToHead = TRUE ; 
					break ;
				}
			}
		}

		if ( bInsertToHead )
		{
			InsertListA( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pSubKey_ShowTime_Firstone, NULL, (PLIST_ENTRY)NewNode );
		}
		else
		{
			InsertListB( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNode, (PLIST_ENTRY)NewNode );
		}

		++ Index ; // 继续枚举下个子键
	} // end-of-while

	kfree( KeyInformation );
	return status ;
}


NTSTATUS 
BuildValueKeysLists (
	IN LPXREG_INFORMATION_CLASS pCurrentOrignalNode,
	IN HANDLE hOrignalKey
	)
{
	BOOL bInsertToHead = FALSE ; 
	NTSTATUS status = STATUS_SUCCESS ; 
	LPXREG_INFORMATION_VALUEKEY NewNode = NULL, pNode = NULL ;
	ULONG ResultLength, Length  = 0x80, NameLength, Index = 0 ;
	PKEY_VALUE_FULL_INFORMATION ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( 0x80 ) ;

	if ( NULL == pCurrentOrignalNode || NULL == hOrignalKey ) { return STATUS_INVALID_PARAMETER ; }

	while( TRUE )
	{
		// 2.1 调用原始函数查询原始目录每个键值的信息
		while ( TRUE )
		{
			status = g_NtEnumerateValueKey_addr( hOrignalKey, Index, KeyValueFullInformation, ValueInfo, Length, &ResultLength );

			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( ValueInfo );
			Length += 0x80 ;
			ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( ValueInfo );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// 遍历当前父键的所有ValueKey,为每个键值建立结点,加入到链表中
		bInsertToHead = FALSE ; 

		ResultLength = ValueInfo->NameLength + ValueInfo->DataLength + sizeof(XREG_INFORMATION_VALUEKEY) + sizeof(WCHAR) ;
		NewNode = (LPXREG_INFORMATION_VALUEKEY) kmalloc( ResultLength );

		NewNode->NameLength = NameLength = ValueInfo->NameLength ;

		memcpy( NewNode->wszValueKeyName, ValueInfo->Name, NameLength );
		NewNode->wszValueKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		NewNode->ValueType = ValueInfo->Type ;
		NewNode->ValueData = (char *)&NewNode->wszValueKeyName[0] + ValueInfo->NameLength + 2 ;
		NewNode->ValueDataLength = ValueInfo->DataLength ;

		memcpy( NewNode->ValueData, (char *)ValueInfo + ValueInfo->DataOffset, ValueInfo->DataLength );

		// 找到链表中可插入点,Insert之
		pNode = pCurrentOrignalNode->pValueKey_ShowTime_Lastone ;
		if ( pNode )
		{
			while ( wcsicmp( pNode->wszValueKeyName, NewNode->wszValueKeyName ) <= 0 )
			{
				pNode = pNode->pFlink ;
				if ( NULL == pNode )
				{
					bInsertToHead = TRUE ; 
					break ;
				}
			}
		}
		else
		{
			bInsertToHead = TRUE ; 
		}

		if ( bInsertToHead )
		{
			InsertListA( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pValueKey_ShowTime_Lastone, NULL, (PLIST_ENTRY)NewNode );
		}
		else
		{
			InsertListB( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)pNode, (PLIST_ENTRY)NewNode );
		}

		++ Index ; // 继续枚举下个子键
	} // end-of-while

	kfree( ValueInfo );
	return status ;
}


BOOL 
IsDirtyVauleKey (
	IN PVOID KeyValueInformation,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass
	)
{
	BOOL bRet ;

	switch ( KeyValueInformationClass )
	{
	case KeyValueBasicInformation :
	case KeyValueFullInformation :
	case KeyValuePartialInformation :
		bRet = *((DWORD *)KeyValueInformation + 1) == _DirtyValueKeyTag_ ;
		break ;

	case KeyValueFullInformationAlign64:
		bRet = _DirtyValueKeyTag_ == 0 ;
		break ;

	case KeyValuePartialInformationAlign64:
		bRet = *(DWORD *)KeyValueInformation == _DirtyValueKeyTag_ ;
		break ;
	
	default :
		break ;
	}

	return bRet ;
}


NTSTATUS 
FillValueKeyInfo (
	IN LPXREG_INFORMATION_VALUEKEY pNode ,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass ,
	OUT PVOID KeyValueInformation,
	IN ULONG Length ,
	OUT PULONG ResultLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/22 [22:10:2010 - 15:07]

Routine Description:
  根据KeyValueInformationClass确定结构体类型,将@pNode中的部分内容填充至@KeyValueInformation   
    
Arguments:
  pNode - 包含当前ValueKey的所有有用信息(沙箱自定义结点)
  KeyValueInformationClass - 枚举类型KEY_VALUE_INFORMATION_CLASS中的一种
  KeyValueInformation - 保存待填充的ValueKey信息
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG StructSize = 0, TotalLength = 0 ;

	if ( NULL == pNode || NULL == KeyValueInformation || NULL == ResultLength ) { return STATUS_INVALID_PARAMETER ; }

	// 1. 根据_KEY_VALUE_INFORMATION类型确定结构体的大小
	switch ( KeyValueInformationClass )
	{
	case KeyValueBasicInformation :
		StructSize = 0xC ;
		TotalLength = pNode->NameLength + StructSize ;
		break ;

	case KeyValueFullInformation :
		StructSize = 0x14 ;
		TotalLength = pNode->NameLength + pNode->ValueDataLength + StructSize ;
		break ;

	case KeyValuePartialInformation :
		StructSize = 0xC ;
		TotalLength = pNode->ValueDataLength + StructSize ;
		break ;

	case KeyValuePartialInformationAlign64:
		StructSize = 8 ;
		TotalLength = pNode->ValueDataLength + StructSize ;
		break ;

	default :
		return STATUS_INVALID_PARAMETER ;
	}

	*ResultLength = TotalLength ;
	if ( Length < StructSize ) { return STATUS_BUFFER_TOO_SMALL ; }

	// 2. 分配新的待填充结构体的内存
	ULONG NameLength = 0 ;
	LPWSTR wszSrc = NULL, wszDest = NULL ;
	LPxx_KEY_VALUE_INFORMATION ptr = (LPxx_KEY_VALUE_INFORMATION) kmalloc( TotalLength );

	if ( KeyValueBasicInformation == KeyValueInformationClass )
	{
		ptr->BasicInfo.Type			= pNode->ValueType  ;
		ptr->BasicInfo.NameLength	= pNode->NameLength ;

		memcpy( ptr->BasicInfo.Name, pNode->wszValueKeyName, pNode->NameLength );
	}
	else if ( KeyValueFullInformation == KeyValueInformationClass )
	{
		ptr->FullInfo.Type		 = pNode->ValueType ;              
		ptr->FullInfo.DataOffset = pNode->NameLength + StructSize ; 
		ptr->FullInfo.DataLength = pNode->ValueDataLength ;  
		ptr->FullInfo.NameLength = pNode->NameLength ;

		memcpy( ptr->FullInfo.Name, pNode->wszValueKeyName, pNode->NameLength );
		memcpy( (LPWSTR)((char *)ptr + ptr->FullInfo.DataOffset), pNode->ValueData, pNode->ValueDataLength );
	}
	else if ( KeyValuePartialInformation == KeyValueInformationClass )
	{
		ptr->PartialInfo.Type		= pNode->ValueType ;
		ptr->PartialInfo.DataLength = pNode->ValueDataLength ;

		memcpy( ptr->PartialInfo.Data, pNode->ValueData, pNode->ValueDataLength );
	}
	else if ( KeyValuePartialInformationAlign64 == KeyValueInformationClass )
	{
		ptr->PartialInfoAlign64.Type		= pNode->ValueType ;
		ptr->PartialInfoAlign64.DataLength  = pNode->ValueDataLength ;

		memcpy( ptr->PartialInfoAlign64.Data, pNode->ValueData, pNode->ValueDataLength );
	}

	// 3. 拷贝临时内容到参数三 @KeyValueInformation
	if ( Length < TotalLength )
	{
		status = STATUS_BUFFER_OVERFLOW ;
	}
	else
	{
		Length = TotalLength ;
		status = STATUS_SUCCESS ;
	}

	memcpy( KeyValueInformation, ptr, Length );

	if ( ptr ) { kfree(ptr); }
	return status ;
}


NTSTATUS 
FillSubKeyInfo (
	IN ULONG RegNameLength,
	IN LPWSTR wszRegFullPath,
	IN KEY_INFORMATION_CLASS KeyInformationClass ,
	OUT PVOID KeyInformation,
	IN ULONG Length ,
	OUT PULONG ResultLength,
	IN PLARGE_INTEGER LastWriteTime
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/25 [25:10:2010 - 17:35]

Routine Description:
  根据@KeyInformationClass类型填充结构体@KeyInformation,填入@LastWriteTime & 子键短名

--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPKEYINFO KeyInfo = (LPKEYINFO) KeyInformation ;
	LPWSTR shortName_ptr = NULL, Name = NULL ;
	ULONG TotalSize = 0, StructSize = 0, size = 0, *NameLength = 0 ;

	if ( KeyFullInformation != KeyInformationClass )
	{
		shortName_ptr = wcsrchr( wszRegFullPath, '\\' ) + 1 ;
		TotalSize = (RegNameLength - ((ULONG)((char *)shortName_ptr - (char *)wszRegFullPath) / sizeof(WCHAR))) * sizeof(WCHAR) ;
	}

	switch ( KeyInformationClass )
	{
	case KeyBasicInformation :
		StructSize = 0x18 ;
		NameLength = &KeyInfo->u.Basic.NameLength ;
		Name = KeyInfo->u.Basic.Name ;
		break ;

	case KeyNodeInformation :
		StructSize = 0x20 ;
		NameLength = &KeyInfo->u.Node.NameLength ;
		Name = KeyInfo->u.Node.Name ;
		break ;

	case KeyFullInformation :
		StructSize = 0x30 ;
		NameLength = 0 ;
		Name = NULL ;
		break ;
	
	default :
		break ;
	}

	*ResultLength = StructSize + TotalSize ;
	if ( Length < StructSize ) { return STATUS_BUFFER_TOO_SMALL; }

	memset( KeyInformation, 0, StructSize );

	KeyInfo->u.LastWriteTime.LowPart  = LastWriteTime->LowPart  ;
	KeyInfo->u.LastWriteTime.HighPart = LastWriteTime->HighPart ;

	if ( KeyNodeInformation == KeyInformationClass )
	{
		KeyInfo->u.Node.ClassOffset = 0xFFFFFFFF ;
	}

	if ( KeyFullInformation != KeyInformationClass )
	{
		size = Length - StructSize ;

		if ( size < TotalSize )
		{
			status = STATUS_BUFFER_OVERFLOW ;
		}
		else
		{
			size = TotalSize ;
		}

		memcpy( Name, shortName_ptr, size );
		*(ULONG *)NameLength = TotalSize ;
	}

	return status ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////