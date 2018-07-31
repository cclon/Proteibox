/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/28 [28:9:2010 - 11:44]
* MODULE : \Code\Project\ProteinBoxDLL\PBRegs.cpp
* 
* Description:
*      
*   注册表重定向模块                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "MemoryManager.h"
#include "PBFilesData.h"
#include "PBRegsData.h"
#include "PBRegs.h"

#pragma warning(disable : 4995 )

//////////////////////////////////////////////////////////////////////////

BOOL g_Is_NtSetValueKey_Ready = FALSE ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+						注册表过滤函数                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_NtCreateKey (
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    IN PULONG Disposition OPTIONAL
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING KeyName ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNeedRemoveNode = FALSE ;
	_NtCreateKey_ OrignalNtCreateKey = (_NtCreateKey_) GetRegFunc(NtCreateKey) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1.0 避免重入?
	if ( pNode->sFileLock.nLockNtCreateKey )
	{
		if ( _NtOpenKey_Tag_ == CreateOptions )
		{
			status = g_NtOpenKey_addr( KeyHandle, DesiredAccess, ObjectAttributes );
		}
		else
		{
			status = OrignalNtCreateKey( KeyHandle, DesiredAccess, ObjectAttributes, TitleIndex, Class, CreateOptions, Disposition );
		}

		return status ;
	}

	pNode->sFileLock.nLockNtCreateKey = 1 ;

	// 1.1 得到操作的原始键值 & 重定向键值	
	status = GetRegPath (
		ObjectAttributes->RootDirectory,   
		ObjectAttributes->ObjectName,
		&OrignalPath,	// "\Registry\Machine\software\0001"
		&RedirectedPath,	// "\REGISTRY\USER\Sandbox_AV_DefaultBox\Machine\software\0001"
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtCreateKey() - GetRegPath(); | status=0x%08lx, Path:%ws \n", status, ObjectAttributes->ObjectName->Buffer );
		goto _CLEARUP_ ; 
	}

	// 2 黑白名单过滤
	InitializeObjectAttributes (
		&ObjAtr ,
		&KeyName ,
		ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ,
		NULL,
		g_SecurityDescriptor
		);

	WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, OrignalPath, &bIsWhite, &bIsBlack );

	// 2.1 黑名单直接禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtCreateKey(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _CLEARUP_ ; 
	}

	// 2.2 白名单调整权限后放行
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &KeyName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		// 对于 NtOpenKey
		if ( _NtOpenKey_Tag_ == CreateOptions )
		{
			status = g_NtOpenKey_addr( KeyHandle, DesiredAccess, &ObjAtr );
			if ( (STATUS_ACCESS_DENIED == status) && (0x2000000 == DesiredAccess) )
			{
				status = g_NtOpenKey_addr( KeyHandle, KEY_READ, &ObjAtr );
			}
		}
		// 对于 NtCreateKey
		else
		{
			status = OrignalNtCreateKey( KeyHandle, DesiredAccess, &ObjAtr, TitleIndex, Class, CreateOptions, Disposition );
			if ( (STATUS_ACCESS_DENIED == status) && (0x2000000 == DesiredAccess) )
			{
				status = OrignalNtCreateKey( KeyHandle, KEY_READ, &ObjAtr, TitleIndex, Class, CreateOptions, Disposition );
			}
		}

		goto _CLEARUP_ ; 
	}

	// 2.3 灰名单另行处理
	RtlInitUnicodeString( &KeyName, RedirectedPath );
	
	// 2.3.1 校验重定向路径的合法性,不合法就拒绝掉.禁止沙箱中的程序直接访问操作重定向目录下的键值!
	if ( IsRedirectedKeyInvalid( RedirectedPath ) )
	{
		MYTRACE( L"ko! | fake_NtCreateKey() - IsRedirectedKeyInvalid; | 重定向路径不合法,拒绝掉! szPath=\"%ws\" \n", RedirectedPath );
		status = STATUS_OBJECT_NAME_NOT_FOUND ;
		goto _CLEARUP_ ; 
	}

	// 2.3.2 调原始函数打开 | 创建 重定向后的键值
	if ( _NtOpenKey_Tag_ == CreateOptions )
	{
		status = g_NtOpenKey_addr( KeyHandle, DesiredAccess | KEY_READ, &ObjAtr );
	}
	else
	{
		status = OrignalNtCreateKey( KeyHandle, DesiredAccess | KEY_READ, &ObjAtr, TitleIndex, Class, CreateOptions & 0xFFFFFFFB, Disposition );
		if ( NT_SUCCESS(status) )
		{
			if ( (NULL == Disposition) || (REG_CREATED_NEW_KEY == *(DWORD *)Disposition) )
			{
				bNeedRemoveNode = TRUE ;
			}
		}
	}

	//
	// 3.1 若操作重定向后的键值成功,进一步过滤
	//

	if ( NT_SUCCESS(status) )
	{
		BOOL bIsDirtyKey = IsDirtyKey( (HANDLE) *KeyHandle );
		if ( bIsDirtyKey )
		{
			// 3.1.0 操作的是被标记为"已删除"的重定向键值
			if ( _NtOpenKey_Tag_ == CreateOptions )
			{
				// 处理打开操作 - 返回未找到该键
				ZwClose( (HANDLE) *KeyHandle );
				*KeyHandle = NULL ;
				status = STATUS_OBJECT_NAME_NOT_FOUND ;
				goto _CLEARUP_ ; 
			}
			else
			{
				// 处理新建操作 - 重新设置该键值的时间
				MarkKeyTime( (HANDLE) *KeyHandle );
			}
		}

		if ( NULL == Disposition ) { goto _CLEARUP_ ; }

		// 3.1.1 到这里只能是CreateNewKey(新建)操作, Disposition才有值.
		if ( bIsDirtyKey )
		{
			*(PULONG) Disposition = REG_CREATED_NEW_KEY ;
		}
		else
		{
			// 检查原始键值是否存在
			HANDLE hTmpKey ;

			RtlInitUnicodeString( &KeyName, OrignalPath );
			status = g_NtOpenKey_addr( &hTmpKey, KEY_READ, &ObjAtr );
			if ( NT_SUCCESS(status) )
			{
				// 原始键值存在,表明是打开已存在的键值
				ZwClose( hTmpKey );
				*(PULONG) Disposition = REG_OPENED_EXISTING_KEY;
			}
			else
			{
				if ( STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status )
				{
					// 对原始键值的操作不成功,是因为该键不存在,表明要创建新键值.成功返回.
					*(PULONG) Disposition = REG_CREATED_NEW_KEY;
					status = STATUS_SUCCESS;
				}
				else
				{
					// 对原始键值的操作不成功,是因为其他原因,将[out]hKey置0表示失败
					ZwClose( *KeyHandle );
					*KeyHandle = NULL ;
				}
			}

		}

		goto _CLEARUP_ ;
	}

	//
	// 3.2 操作重定向键值失败,说明当前键值不存在
	//

	if ( STATUS_OBJECT_NAME_NOT_FOUND != status && STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _CLEARUP_ ; }

	// 3.2.1 首先检查原始键值是否存在
	BOOL bOrignalKeyIsExsit = FALSE ;

	RtlInitUnicodeString( &KeyName, OrignalPath );
	status = g_NtOpenKey_addr( KeyHandle, KEY_READ, &ObjAtr );
	if ( NT_SUCCESS(status) )
	{
		// 打开原始键值成功
		if ( 0 == (DesiredAccess & 0x7DEDFCE6) )
		{
			// 若打开权限未包含特定值,返回失败
			if ( Disposition ) { *(PULONG) Disposition = REG_OPENED_EXISTING_KEY ; }
			goto _CLEARUP_ ;
		}

		// 原始键值存在
		bOrignalKeyIsExsit = TRUE ;
		ZwClose( *KeyHandle );
		*KeyHandle = NULL ;
	}
	else
	{
		// 打开原始键值失败
		if (   (STATUS_OBJECT_NAME_NOT_FOUND != status && STATUS_OBJECT_PATH_NOT_FOUND != status)
			|| (_NtOpenKey_Tag_ == CreateOptions)
			)
		{
			goto _CLEARUP_ ; 
		}
	}

	// 3.2.2 打开原始键的父路径
	LPWSTR ptr2 = wcsrchr( OrignalPath, '\\' );
	*ptr2 = 0 ;

	RtlInitUnicodeString( &KeyName, OrignalPath );
	status = g_NtOpenKey_addr( KeyHandle, KEY_READ, &ObjAtr );
	*ptr2 = '\\' ;

	if ( NT_SUCCESS(status) )
	{
		ZwClose( *KeyHandle );
		*KeyHandle = NULL ;
		bNeedRemoveNode = TRUE ;
 
		// 原始键存在父路径,递归创建重定向键相应路径  
		RtlInitUnicodeString( &KeyName, RedirectedPath );
		status = CreateRedirectedSubkeys( &ObjAtr );
		
		if ( NT_SUCCESS(status) )
		{
			status = g_NtOpenKey_addr( KeyHandle, DesiredAccess, &ObjAtr );
			if ( NT_SUCCESS(status) && Disposition )
			{
				if ( bOrignalKeyIsExsit )
				{
					*(PULONG) Disposition = REG_OPENED_EXISTING_KEY ;
				}
				else
				{
					*(PULONG) Disposition = REG_CREATED_NEW_KEY ;
				}
			}
		}
	}

_CLEARUP_:
	if ( bNeedRemoveNode )
	{
		RemoveRegNodeExp( OrignalPath, TRUE );
	}

	kfree( OrignalPath );
	kfree( RedirectedPath );
	pNode->sFileLock.nLockNtCreateKey = 0 ;
 	return status ;
}


ULONG WINAPI
fake_NtOpenKey (
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
	)
{
	return fake_NtCreateKey( KeyHandle,DesiredAccess,ObjectAttributes, 0, NULL, _NtOpenKey_Tag_, NULL );
}


ULONG WINAPI
fake_NtOpenKeyEx (
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG Reserved
	)
{

	if ( Reserved & 0xFFFFFFFB ) { return STATUS_NOT_IMPLEMENTED ; }
	
	return fake_NtCreateKey( KeyHandle,DesiredAccess,ObjectAttributes, 0, NULL, _NtOpenKey_Tag_, NULL );
}


ULONG WINAPI
fake_NtDeleteKey (
    IN HANDLE KeyHandle
	)
{
	return StubNtDeleteKey( KeyHandle, FALSE );
}


ULONG WINAPI
fake_NtDeleteValueKey (
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
	)
{
	ULONG ReturnLength ;
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	UCHAR KeyValueInfo[ sizeof(KEY_VALUE_BASIC_INFORMATION) + 30 ];
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNeedRemoveNode = FALSE ;
	_NtDeleteValueKey_ OrignalAddress = (_NtDeleteValueKey_) GetRegFunc(NtDeleteValueKey) ;

	if ( NULL == KeyHandle ) { return STATUS_INVALID_PARAMETER ; }

	// 1. 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtDeleteValueKey() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2. 过黑白名单
	WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, OrignalPath, &bIsWhite, &bIsBlack );

	// 2.1 黑名单直接禁止
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | fake_NtDeleteValueKey(); | 遇见黑名单,拒绝掉! szPath=\"%ws\" \n", OrignalPath );
		status = STATUS_ACCESS_DENIED ;
		goto _over_ ;
	}

	// 2.2 白名单放行之
	if ( bIsWhite )
	{
		status = OrignalAddress( KeyHandle,ValueName );
		goto _over_ ;
	}

	// 2.3 灰名单
	status = fake_NtQueryValueKey (
		KeyHandle,
		ValueName,
		KeyValueBasicInformation,
		&KeyValueInfo,
		sizeof(KeyValueInfo),
		&ReturnLength
		);

	if ( !status || status == STATUS_BUFFER_OVERFLOW )
	{
		status = ZwSetValueKey( KeyHandle, ValueName, 0, _DirtyValueKeyTag_, 0, 0 ); // 标记为"已删除"的键值
	}
	
_over_ :
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtSetValueKey (
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
	)
{
	HANDLE hKey ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ; 
	_NtSetValueKey_ OrignalAddress = (_NtSetValueKey_) GetRegFunc(NtSetValueKey) ;

	if ( NULL == ValueName || 0 == ValueName->Length || NULL == ValueName->Buffer )
	{
		RtlInitUnicodeString( &KeyName, NULL );
	}
	else
	{
		KeyName.Length = ValueName->Length ;
		KeyName.MaximumLength = ValueName->MaximumLength ;
		KeyName.Buffer = ValueName->Buffer ;

		if (   FALSE == g_Is_NtSetValueKey_Ready 
			&& 0x14 == KeyName.Length 
			&& 0 == wcsnicmp( KeyName.Buffer, L"StoreDirty", 0xA )
			)
		{
			return STATUS_SUCCESS ;
		}
	}

	status = OrignalAddress( KeyHandle,&KeyName,TitleIndex,Type,Data,DataSize );
	if ( STATUS_ACCESS_DENIED == status )
	{
		RtlInitUnicodeString( &KeyName, NULL );
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, KeyHandle, NULL );
		
		status = ZwOpenKey( &hKey, KEY_WRITE, &ObjAtr );
		if ( NT_SUCCESS(status) )
		{
			status = OrignalAddress( hKey,&KeyName,TitleIndex,Type,Data,DataSize );
			ZwClose( hKey );
		}
	}

	RemoveRegNodeEx( KeyHandle, FALSE );
	return status ;
}


ULONG WINAPI
fake_NtQueryKey (
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	)
{
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtQueryKey_ OrignalAddress = (_NtQueryKey_) GetRegFunc(NtQueryKey) ;

	// 1. 处理 KeyBasicInformation / KeyNodeInformation / KeyFlagsInformation
	if ( KeyBasicInformation == KeyInformationClass || KeyNodeInformation == KeyInformationClass || KeyFlagsInformation == KeyInformationClass )
	{
		status = OrignalAddress( KeyHandle, KeyInformationClass, KeyInformation, Length, ResultLength );

		if ( (KeyFlagsInformation != KeyInformationClass) && (status >= 0 || STATUS_BUFFER_OVERFLOW == status) )
		{
			if ( IsDirtyKeyEx( KeyInformation ) ) { status = STATUS_KEY_DELETED ; }
		}

		return status ;
	}
	
	// 2. 处理 KeyFullInformation / KeyNameInformation / KeyCachedInformation
	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtQueryKey() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2.1 填充_KEY_NAME_INFORMATION结构体.仅是NameLength & Name
	if ( KeyNameInformation == KeyInformationClass )
	{
		ULONG size = 0, NameLength = 0 ;
		PKEY_NAME_INFORMATION KeyNameInfo = (PKEY_NAME_INFORMATION) KeyInformation ;

		NameLength = wcslen(OrignalPath) * sizeof(WCHAR) ;
		*ResultLength = NameLength + 4;
		
		if ( Length < 4 ) { status = STATUS_BUFFER_TOO_SMALL ; goto _over_ ; }

		KeyNameInfo->NameLength = NameLength ;
		if ( Length - 4 < NameLength )
		{ 
			size = Length - 4 ;
			status = STATUS_BUFFER_OVERFLOW ;
		}
		else
		{
			size = NameLength ;
			status = STATUS_SUCCESS ;
		}
		
		memcpy( KeyNameInfo->Name, OrignalPath, size );
		goto _over_ ;
	}

	// 2.2 对于KeyFullInformation & KeyCachedInformation类型,需查询当前父键拥有子键的个数及键值的数量.调用RegKeyFilte()进行统计.
	if ( KeyFullInformation != KeyInformationClass && KeyCachedInformation != KeyInformationClass )
	{
		status = STATUS_INVALID_PARAMETER ;
		goto _over_ ;
	}

	LPXREG_INFORMATION_CLASS RedirectedNode = NULL ;
	status = RegKeyFilter( KeyHandle, OrignalPath, RedirectedPath, TRUE, TRUE, (PVOID *)&RedirectedNode );

	// 2.3.1 处理统计失败的情况
	if ( !NT_SUCCESS(status) )
	{
		// 白名单返回STATUS_BAD_INITIAL_PC标记放行
		if ( STATUS_BAD_INITIAL_PC == status )
		{
			status = OrignalAddress( KeyHandle, KeyInformationClass, KeyInformation, Length, ResultLength );
			if ( status >= 0 || STATUS_BUFFER_OVERFLOW == status )
			{
				if ( IsDirtyKeyEx( KeyInformation ) ) { status = STATUS_KEY_DELETED ; }
			}
		}

		goto _over_ ;
	}

	// 2.3.2 统计成功
	LPWSTR shortName = NULL ;
	ULONG shortNameLength = 0 ;

	if ( KeyCachedInformation == KeyInformationClass )
	{
		shortName = wcsrchr( RedirectedNode->wszOrignalRegPath, '\\' ) + 1 ;
		shortNameLength = wcslen( shortName ) * sizeof(WCHAR) ;
	}

	// (a) 得到当前父键拥有的所有子键(SubKey)中名字最长值
	ULONG MaxNameLen = 0 ;
	LPXREG_INFORMATION_SUBKEY SubKeyCurrent = NULL ;

	for ( SubKeyCurrent = RedirectedNode->pSubKey_ShowTime_Firstone ;
		;
		SubKeyCurrent = SubKeyCurrent->pFlink 
		)
	{
		if ( NULL == SubKeyCurrent ) { break ; }

		if ( SubKeyCurrent->NameLength > MaxNameLen )
			MaxNameLen = SubKeyCurrent->NameLength ;
	}

	// (b) 得到当前父键拥有的所有键值(ValueKey)中名字最长值 & 数据最大值
	LPXREG_INFORMATION_VALUEKEY ValueKeyCurrent = NULL ;
	ULONG MaxValueNameLen = 0, MaxValueDataLen = 0 ;

	for ( ValueKeyCurrent = RedirectedNode->pValueKey_ShowTime_Lastone ;
		;
		ValueKeyCurrent = ValueKeyCurrent->pFlink 
		)
	{
		if ( NULL == ValueKeyCurrent ) { break ; }

		if ( ValueKeyCurrent->NameLength > MaxValueNameLen )
			MaxValueNameLen = ValueKeyCurrent->NameLength;

		if ( ValueKeyCurrent->ValueDataLength > MaxValueDataLen )
		{
			MaxValueDataLen = ValueKeyCurrent->ValueDataLength ;
		}
	}

	LeaveCriticalSection( &g_cs_Regedit );

	// (c) 对不同类型的结构体进行填充
	if ( KeyFullInformation == KeyInformationClass )
	{
		PKEY_FULL_INFORMATION KeyFullInfo = (PKEY_FULL_INFORMATION) KeyInformation ;

		*ResultLength = 0x2C ;
		if ( Length < 0x2C ) { return STATUS_BUFFER_TOO_SMALL ; }
		
		KeyFullInfo->LastWriteTime.LowPart = RedirectedNode->LastWriteTime.LowPart ;
		KeyFullInfo->LastWriteTime.HighPart = RedirectedNode->LastWriteTime.HighPart ;
		KeyFullInfo->TitleIndex = 0 ;
		KeyFullInfo->ClassOffset = 0xFFFFFFFF ;
		KeyFullInfo->ClassLength = 0;
		KeyFullInfo->SubKeys = RedirectedNode->nCounts_SubKeys_ShowTime ;
		KeyFullInfo->MaxNameLen = MaxNameLen ;
		KeyFullInfo->MaxClassLen = 0;
		KeyFullInfo->Values = RedirectedNode->nCounts_ValueKeys_ShowTime ;
		KeyFullInfo->MaxValueNameLen = MaxValueNameLen ;
		KeyFullInfo->MaxValueDataLen = MaxValueDataLen ;
	}
	else if ( KeyCachedInformation == KeyInformationClass )
	{
		PKEY_CACHED_INFORMATION KeyCacheInfo = (PKEY_CACHED_INFORMATION) KeyInformation;

		*ResultLength = 0x28 ;
		if ( Length < 0x28 ) { return STATUS_BUFFER_TOO_SMALL ; }

		KeyCacheInfo->LastWriteTime.LowPart = RedirectedNode->LastWriteTime.LowPart ;
		KeyCacheInfo->LastWriteTime.HighPart = RedirectedNode->LastWriteTime.HighPart ;
		KeyCacheInfo->TitleIndex = 0 ;
		KeyCacheInfo->SubKeys = RedirectedNode->nCounts_SubKeys_ShowTime ;
		KeyCacheInfo->MaxNameLen = MaxNameLen ;
		KeyCacheInfo->Values = RedirectedNode->nCounts_ValueKeys_ShowTime ;
		KeyCacheInfo->MaxValueNameLen = MaxValueNameLen ;
		KeyCacheInfo->MaxValueDataLen = MaxValueDataLen ;
		KeyCacheInfo->NameLength = shortNameLength ;
	}

	status = STATUS_SUCCESS ;

_over_ :
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtQueryValueKey (
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	)
{
	UNICODE_STRING KeyName ;
	BOOL bUnlock = FALSE ;
	ULONG NameLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR wszValueKeyName = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNode = NULL ;
	_NtQueryValueKey_ OrignalAddress = (_NtQueryValueKey_) GetRegFunc(NtQueryValueKey) ;

	// 1. 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtQueryValueKey() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2. 过一下总的过滤函数
	LPXREG_INFORMATION_CLASS RedirectedNode = NULL ;
	status = RegKeyFilter( KeyHandle, OrignalPath, RedirectedPath, FALSE, TRUE, (PVOID *)&RedirectedNode );

	// 2.1 处理统计失败的情况
	if ( !NT_SUCCESS(status) )
	{
		// 白名单返回STATUS_BAD_INITIAL_PC标记放行
		if ( STATUS_BAD_INITIAL_PC == status )
		{
			status = OrignalAddress( KeyHandle,ValueName,KeyValueInformationClass,KeyValueInformation,Length,ResultLength );
			if ( status >= 0 || STATUS_BUFFER_OVERFLOW == status )
			{
				if ( IsDirtyVauleKey( KeyValueInformation, KeyValueInformationClass ) ) { return STATUS_OBJECT_NAME_NOT_FOUND ; }
			}
		}

		goto _over_ ;
	}

	// 2.1 处理统计成功的情况
	bUnlock = TRUE ;
	if ( ValueName && NULL != ValueName->Buffer )
	{
		wszValueKeyName = ValueName->Buffer ;
		NameLength = ValueName->Length ;
	}

	// 3. 遍历所有的键值(ValueKey),找到匹配的结点
	for ( pNode = RedirectedNode->pValueKey_ShowTime_Lastone; ; pNode = pNode->pFlink )
	{
		if ( NULL == pNode ) { status = STATUS_OBJECT_NAME_NOT_FOUND; break; }

		if ( pNode->NameLength == NameLength && 0 == wcsnicmp( pNode->wszValueKeyName, wszValueKeyName, NameLength / sizeof(WCHAR) ) )
		{
			// 找到后进行填充
			status = FillValueKeyInfo( pNode, KeyValueInformationClass, KeyValueInformation, Length, ResultLength );
			break ;
		}
	}

	if ( bUnlock ) { LeaveCriticalSection( &g_cs_Regedit ); }

_over_ :
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtQueryMultipleValueKey (
    IN HANDLE KeyHandle,
    IN OUT PKEY_VALUE_ENTRY ValueList,
    IN ULONG NumberOfValues,
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    OUT PULONG ReturnLength
	)
{
	UNICODE_STRING KeyName ;
	BOOL bUnlock = FALSE ;
	ULONG NameLength = 0, CurrentTotalLength = 0, ValueDataLength = 0, xx = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPWSTR wszValueKeyName = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNode = NULL ;
	_NtQueryMultipleValueKey_ OrignalAddress = (_NtQueryMultipleValueKey_) GetRegFunc(NtQueryMultipleValueKey) ;

	// 1. 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtQueryMultipleValueKey() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2. 过一下总的过滤函数
	LPXREG_INFORMATION_CLASS RedirectedNode = NULL ;
	status = RegKeyFilter( KeyHandle, OrignalPath, RedirectedPath, FALSE, TRUE, (PVOID *)&RedirectedNode );

	// 2.1 处理统计失败的情况
	if ( !NT_SUCCESS(status) )
	{
		// 白名单返回STATUS_BAD_INITIAL_PC标记放行
		if ( STATUS_BAD_INITIAL_PC == status )
		{
			status = OrignalAddress( KeyHandle,ValueList, NumberOfValues, Buffer, Length, ReturnLength );
		}

		goto _over_ ;
	}

	// 2.1 处理统计成功的情况
	bUnlock = TRUE ;
	while ( NumberOfValues )
	{
		// 1. ValueList数组里保存着要查询的N个键值(ValueKeys). 对于每个键值,在链表中查找匹配项.
		if ( ValueList->ValueName && ValueList->ValueName->Buffer )
		{
			wszValueKeyName = ValueList->ValueName->Buffer ;
			NameLength = 2 * wcslen( wszValueKeyName );
		}
		else
		{
			wszValueKeyName = NULL	;
			NameLength		= 0		;
		}

		for ( pNode = RedirectedNode->pValueKey_ShowTime_Lastone; ; pNode = pNode->pFlink )
		{
			if ( NULL == pNode ) { status = STATUS_OBJECT_NAME_NOT_FOUND; goto _over_ ; }

			if ( NameLength == pNode->NameLength && 0 ==wcsicmp( pNode->wszValueKeyName, wszValueKeyName ) )
				break;
		}

		// 2. 已找到匹配项.将当前项struct _XREG_INFORMATION_VALUEKEY_中的部分信息拷贝到@ValueEntries中
		ValueDataLength = pNode->ValueDataLength ;

		if ( ValueDataLength + CurrentTotalLength > *Length )
		{
			status = STATUS_BUFFER_OVERFLOW ;
		}
		else
		{
			if ( ValueDataLength )
			{
				memcpy( (char *)Buffer + CurrentTotalLength, pNode->ValueData, ValueDataLength );
				
				xx = (pNode->ValueDataLength + xx + 3) & 0xFFFFFFFC ;
			}

			ValueList->DataLength = ValueDataLength		;
			ValueList->DataLength = CurrentTotalLength	;
			ValueList->Type		  = pNode->ValueType	;
		}

		CurrentTotalLength = (pNode->ValueDataLength + CurrentTotalLength + 3) & 0xFFFFFFFC ;
		++ ValueList ;
		-- NumberOfValues ;

	} // end-of-while

	if ( status >= 0 || status == STATUS_BUFFER_OVERFLOW )
	{
		*Length = xx ;
		if ( ReturnLength ) {*ReturnLength = CurrentTotalLength ; }
	}

_over_ :
	if ( bUnlock ) { LeaveCriticalSection( &g_cs_Regedit ); }

	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtEnumerateKey (
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	)
{
	UNICODE_STRING KeyName ;
	BOOL bUnlock = FALSE ;
	HANDLE hKey = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	ULONG nCounts = 0, FullNameLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR wszRegFullPath = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPXREG_INFORMATION_SUBKEY pNode = NULL ;
	_NtEnumerateKey_ OrignalAddress = (_NtEnumerateKey_) GetRegFunc(NtEnumerateKey) ;

	// 只过滤3中类型:
	if (   KeyInformationClass
		&& (KeyInformationClass != KeyNodeInformation) 
		&& (KeyInformationClass != KeyFullInformation) )
	{
		return STATUS_INVALID_PARAMETER ;
	}

	// 1. 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtEnumerateKey() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2. 过一下总的过滤函数
	LPXREG_INFORMATION_CLASS RedirectedNode = NULL ;
	status = RegKeyFilter( KeyHandle, OrignalPath, RedirectedPath, TRUE, FALSE, (PVOID *)&RedirectedNode );

	// 2.1 处理统计失败的情况
	if ( !NT_SUCCESS(status) )
	{
		// 白名单返回STATUS_BAD_INITIAL_PC标记放行
		if ( STATUS_BAD_INITIAL_PC == status )
		{
			status = OrignalAddress( KeyHandle,Index,KeyInformationClass,KeyInformation,Length,ResultLength );
			if ( status >= 0 || STATUS_BUFFER_OVERFLOW == status )
			{
				if ( IsDirtyKeyEx( KeyInformation ) ) { status = STATUS_KEY_DELETED ; }
			}
		}

		goto _over_ ;
	}

	// 2.1 处理统计成功的情况
	if ( 0 == RedirectedNode->PreviousIndex || Index < RedirectedNode->PreviousIndex )
	{
		// 枚举的是第一个Subkey
		nCounts = Index ;
		pNode = RedirectedNode->pSubKey_ShowTime_Firstone ;
	}
	else
	{
		// 若枚举的不是第一个Subkey
		nCounts = Index - RedirectedNode->PreviousIndex ;
		pNode = RedirectedNode->pSubKeys_ShowTime_Previous ;
	}

	while ( nCounts )
	{
		if ( NULL == pNode ) { break ; }

		pNode = pNode->pFlink ;
		-- nCounts ;

	}

	// 得到要查询键值的全路径
	if ( pNode )
	{
		FullNameLength = RedirectedNode->NameLength + pNode->NameLength + 4 ;
		wszRegFullPath = (LPWSTR) kmalloc( FullNameLength );

		KeyName.MaximumLength = (USHORT) FullNameLength ;
		KeyName.Length = (USHORT) (FullNameLength - sizeof(WCHAR)) ;
		KeyName.Buffer = wszRegFullPath ;

		memcpy( wszRegFullPath, RedirectedNode->wszOrignalRegPath, RedirectedNode->NameLength );
		
		wszRegFullPath[ RedirectedNode->NameLength / sizeof(WCHAR) ] = '\\';
		
		memcpy ( 
			&wszRegFullPath[ RedirectedNode->NameLength / sizeof(WCHAR) + 1 ], 
			pNode->wszSubKeyName,
			pNode->NameLength + sizeof(WCHAR)
			);

		FullNameLength = (RedirectedNode->NameLength + pNode->NameLength + 2) / sizeof(WCHAR) ;
	}

	LeaveCriticalSection( &g_cs_Regedit );
	
	if ( NULL == pNode )
	{
		RedirectedNode->PreviousIndex = 0 ;
		RedirectedNode->pSubKeys_ShowTime_Previous = 0 ;
		status = STATUS_NO_MORE_ENTRIES ;
		goto _over_ ;
	}

	RedirectedNode->PreviousIndex = Index ;
	RedirectedNode->pSubKeys_ShowTime_Previous = pNode ;

	BOOL bFixup = FALSE ;

	if ( 0x1C == RedirectedNode->NameLength && 0 == wcsnicmp( RedirectedNode->wszOrignalRegPath, L"\\registry\\user", 0xE ) )
	{
		bFixup = TRUE ;
	}
	else if ( 0x18 == FullNameLength && 0 == wcsnicmp( wszRegFullPath, L"\\registry\\machine\\system", 0x18 ) )
	{
		bFixup = TRUE ;
	}
	else
	{
		if ( KeyBasicInformation == KeyInformationClass || KeyNodeInformation == KeyInformationClass ) 
		{ 
			if (   (NULL == pNode->bClassName)
				|| ( FullNameLength > 0x10 && 0 == wcsnicmp(wszRegFullPath, L"\\registry\\user\\Software\\Classes\\S-", 0x22) )
				) 
			{ 
				bFixup = TRUE ;
			}
		}
		
		if ( FALSE == bFixup )
		{
			InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );
			status = ZwOpenKey( &hKey, KEY_READ, &ObjAtr );

			if ( NT_SUCCESS(status) )
			{
				status = ZwQueryKey( hKey, KeyInformationClass, KeyInformation, Length, ResultLength );
				ZwClose( hKey );
			}
		}
	}

	if ( bFixup )
	{
		status = FillSubKeyInfo( FullNameLength, wszRegFullPath, KeyInformationClass, KeyInformation, Length, ResultLength, &pNode->LastWriteTime );
	}

_over_ :
	kfree( wszRegFullPath );
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtEnumerateValueKey (
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	)
{
	UNICODE_STRING KeyName ;
	BOOL bUnlock = FALSE ;
	ULONG nCounts = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNode = NULL ;
	_NtEnumerateValueKey_ OrignalAddress = (_NtEnumerateValueKey_) GetRegFunc(NtEnumerateValueKey) ;

	// 1. 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtEnumerateValueKey() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2. 过一下总的过滤函数
	LPXREG_INFORMATION_CLASS RedirectedNode = NULL ;
	status = RegKeyFilter( KeyHandle, OrignalPath, RedirectedPath, FALSE, TRUE, (PVOID *)&RedirectedNode );

	// 2.1 处理统计失败的情况
	if ( !NT_SUCCESS(status) )
	{
		// 白名单返回STATUS_BAD_INITIAL_PC标记放行
		if ( STATUS_BAD_INITIAL_PC == status )
		{
			status = OrignalAddress( KeyHandle,Index,KeyValueInformationClass,KeyValueInformation,Length,ResultLength );
			if ( status >= 0 || STATUS_BUFFER_OVERFLOW == status )
			{
				if ( IsDirtyVauleKey( KeyValueInformation, KeyValueInformationClass ) ) { return STATUS_OBJECT_NAME_NOT_FOUND ; }
			}
		}

		goto _over_ ;
	}

	// 2.1 处理统计成功的情况
	bUnlock = TRUE ;
	pNode = RedirectedNode->pValueKey_ShowTime_Lastone ;

	// 得到当前Index对应的ValueKey结点.填充之
	for ( nCounts = Index; nCounts; --nCounts )
	{
		if ( NULL == pNode ) { break ; }
		pNode = pNode->pFlink ;
	}

	if ( pNode )
	{
		status = FillValueKeyInfo( pNode, KeyValueInformationClass, KeyValueInformation, Length, ResultLength );
	}
	else
	{
		status = STATUS_NO_MORE_ENTRIES ;
	}

	if ( bUnlock ) { LeaveCriticalSection( &g_cs_Regedit ); }

_over_ :
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;	
}


ULONG WINAPI
fake_NtNotifyChangeKey (
    IN HANDLE KeyHandle,
    IN HANDLE Event,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN Asynchroneous,
    OUT PVOID ChangeBuffer,
    IN ULONG Length,
    IN BOOLEAN WatchSubtree
	)
{
	return fake_NtNotifyChangeMultipleKeys(KeyHandle,0,0,Event,ApcRoutine,ApcContext,IoStatusBlock,CompletionFilter,Asynchroneous,ChangeBuffer,Length,WatchSubtree );
}


ULONG WINAPI
fake_NtNotifyChangeMultipleKeys (
    IN HANDLE MasterKeyHandle,
    IN ULONG Count,
    IN POBJECT_ATTRIBUTES SlaveObjects,
    IN HANDLE Event,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN Asynchronous
	)
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	BOOL bUnlock = FALSE, bIsHandlerSelfRegPath = FALSE ;
	ULONG nCounts = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	_NtNotifyChangeMultipleKeys_ OrignalAddress = (_NtNotifyChangeMultipleKeys_) GetRegFunc(NtNotifyChangeMultipleKeys) ;

	// 1. 得到操作的原始键值 & 重定向键值	
	RtlInitUnicodeString( &KeyName, NULL );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = GetRegPath (
		MasterKeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		&bIsHandlerSelfRegPath
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtNotifyChangeMultipleKeys() - GetRegPath(); | status=0x%08lx \n", status );
		return status ;
	}

	// 2.
	if ( FALSE == bIsHandlerSelfRegPath )
	{
		// 检查重定向键值是否存在
		RtlInitUnicodeString( &KeyName, RedirectedPath );
		status = ZwOpenKey( &MasterKeyHandle, 0xF003F, &ObjAtr );

		if ( !NT_SUCCESS(status) ) { goto _over_ ; }
	}

	if ( Count && SlaveObjects )
	{
		status = GetRegPath (
			SlaveObjects->RootDirectory ,
			SlaveObjects->ObjectName ,
			&OrignalPath ,
			&RedirectedPath ,
			&bIsHandlerSelfRegPath
			);

		if ( !NT_SUCCESS(status) ) { goto _over_ ; }

		if ( FALSE == bIsHandlerSelfRegPath )
		{
			// 检查重定向键值是否存在
			HANDLE hRedirectedKey = NULL ;

			RtlInitUnicodeString( &KeyName, RedirectedPath );
			status = ZwOpenKey( &hRedirectedKey, 0xF003F, &ObjAtr );

			if ( !NT_SUCCESS(status) ) { goto _over_ ; }

			ZwClose( hRedirectedKey );
			SlaveObjects = &ObjAtr ;
		}
	}

	status = OrignalAddress( MasterKeyHandle,Count,SlaveObjects,Event,ApcRoutine,ApcContext,IoStatusBlock,CompletionFilter,WatchTree,Buffer,Length,Asynchronous );

	if ( STATUS_INVALID_PARAMETER == status && 1 == Count )
	{
		status = OrignalAddress( MasterKeyHandle,0,0,Event,ApcRoutine,ApcContext,IoStatusBlock,CompletionFilter,WatchTree,Buffer,Length,Asynchronous );	
	}

_over_ :
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtSaveKey (
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
	)
{
	// 不响应此函数的操作
	return 0 ;
}


ULONG WINAPI
fake_NtLoadKey (
    IN POBJECT_ATTRIBUTES KeyObjectAttributes,
    IN POBJECT_ATTRIBUTES FileObjectAttributes
	)
{
	HANDLE hFile = NULL ;
	ULONG length = 0, DataLength = 0 ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR RegOrignalPath = NULL, RegRedirectedPath = NULL, szPath = NULL ;
	LPRPC_IN_NtLoadKey pRpcInBuffer = NULL ;
	LPRPC_OUT_HEADER pOutBuffer = NULL ;
	_NtLoadKey_ OrignalAddress = (_NtLoadKey_) GetRegFunc(NtLoadKey) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 调用原始函数
	status =  OrignalAddress( KeyObjectAttributes,FileObjectAttributes );
	if ( STATUS_PRIVILEGE_NOT_HELD != status ) { return status ; }

	DataLength = sizeof(RPC_IN_NtLoadKey);
	pRpcInBuffer = (LPRPC_IN_NtLoadKey) kmalloc( DataLength );
	pRpcInBuffer->RpcHeader.DataLength = DataLength ;
	pRpcInBuffer->RpcHeader.Flag = _PBSRV_APINUM_NtLoadKey_ ;

	szPath = (LPWSTR) kmalloc( 0x4000 );

	// 2. 打开文件,获得句柄
	status = ZwCreateFile (
		&hFile,
		SYNCHRONIZE | FILE_READ_ATTRIBUTES,
		FileObjectAttributes,
		&IoStatusBlock,
		NULL ,
		0 ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ,
		FILE_OPEN ,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT ,
		NULL,
		0
		);

	if ( ! NT_SUCCESS(status) )
	{ 
		MYTRACE( L"error! | fake_NtLoadKey() - ZwCreateFile(); | status=0x%08lx \n", status );
		return status ; 
	}

	// 3. 得到文件句柄对应的原始 & 重定向路径
	status = PB_GetHandlePath( hFile, szPath, NULL );
	if ( ! NT_SUCCESS(status) ) 
	{ 
		MYTRACE( L"error! | fake_NtLoadKey() - PB_GetHandlePath(); | status=0x%08lx \n", status );
		goto _END_ ;
	}

	// 4.1 将重定向路径转换为完整路径
	if ( FALSE == PB_TranslateNtToDosPath( szPath ) )
	{
		MYTRACE( L"error! | fake_NtLoadKey() - PB_TranslateNtToDosPath(); | \n" );
		status = STATUS_ACCESS_DENIED ;
		goto _END_ ;
	}

	length = wcslen( szPath );
	if ( length > 0x7F )
	{
		MYTRACE( L"error! | fake_NtLoadKey(); | RegRedirectedPath(%ws) is to long(%d) \n", RegRedirectedPath, length );
		status = STATUS_ACCESS_DENIED ;
		goto _END_ ;
	}

	wcscpy( pRpcInBuffer->szRedirectedPath, szPath );

	// 4.3 得到操作的原始键值 & 重定向键值
	status = GetRegPath (
		KeyObjectAttributes->RootDirectory,   
		KeyObjectAttributes->ObjectName,
		&RegOrignalPath,	
		&RegRedirectedPath,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | fake_NtLoadKey() - GetRegPath(); | status=0x%08lx \n", status );
		goto _END_ ;
	}

	length = wcslen( RegOrignalPath );
	if ( length > 0x7F )
	{
		MYTRACE( L"error! | fake_NtLoadKey(); | RegOrignalPath(%ws) is to long(%d) \n", RegOrignalPath, length );
		status = STATUS_ACCESS_DENIED ;
		goto _END_ ;
	}

	// 4.4 将OrignalPath拷贝到 +0x8 处,这是LPC通信时待填充的Buffer结构体
	wcscpy( pRpcInBuffer->szOrignalPath, RegOrignalPath );

	// 4.5 和Server通信
	pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( (PVOID)pRpcInBuffer );
	if ( NULL == pOutBuffer )
	{
		MYTRACE( L"error! | fake_NtLoadKey() - PB_CallServer(); |  \n" );
		status = STATUS_ACCESS_DENIED ;
		goto _END_ ;
	}

	status = pOutBuffer->u.Status ;
	PB_FreeReply( pOutBuffer );

_END_ :
	if ( hFile ) { ZwClose( hFile ); }
	if ( pRpcInBuffer ) { kfree( (PVOID)pRpcInBuffer ); }
	if ( szPath )	 { kfree( (PVOID)szPath ); }

	kfree( RegOrignalPath );
	kfree( RegRedirectedPath );
	return status ;
}


ULONG WINAPI
fake_NtRenameKey (
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ReplacementName
	)
{
	// 走原始函数
	_NtRenameKey_ OrignalAddress = (_NtRenameKey_) GetRegFunc(NtRenameKey) ;
	return OrignalAddress( KeyHandle,ReplacementName );
}


///////////////////////////////   END OF FILE   ///////////////////////////////