/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/11/01 [1:11:2010 - 11:48]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDLL\PBFilesData.cpp
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
#include "Exportfunc.h"
#include "HookHelper.h"
#include "ProteinBoxDLL.h"
#include "MemoryManager.h"
#include "PBRegsData.h"
#include "PBFilesData.h"

#pragma warning(disable : 4995) 

//////////////////////////////////////////////////////////////////////////

_NtCreateFile_ g_NtCreateFile_addr = NULL ;
_NtDeleteFile_ g_NtDeleteFile_addr = NULL ;
_NtQueryFullAttributesFile_ g_NtQueryFullAttributesFile_addr = NULL ;
_NtQueryInformationFile_ g_NtQueryInformationFile_addr = NULL ;
_NtQueryDirectoryFile_ g_NtQueryDirectoryFile_addr = NULL ;
_NtSetInformationFile_ g_NtSetInformationFile_addr = NULL ;
_NtClose_ g_NtClose_addr = NULL ;


CRITICAL_SECTION g_Lock_DirHandle ;
char g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok = '?';
BOOL g_bFlag_GetVolumePathNamesForVolumeNameW_is_Null = FALSE ;
PVOID g_SymbolicLinkObjectName_Array = NULL ;
ULONG g_Redirectd_FileHandle_Array = 0 ;
ULONG	g_lpszEnv_00000000_SBIE_ALL_USERS_Length	= 0 ;
LPWSTR	g_lpszEnv_00000000_SBIE_ALL_USERS			= NULL ;
ULONG	g_lpszEnv_00000000_SBIE_CURRENT_USER_Length = 0 ;
LPWSTR	g_lpszEnv_00000000_SBIE_CURRENT_USER		= NULL ;

VOLUME_INFO g_pNodeHead_xx  = { 0, {NULL, NULL, 0} } ;

// 保存PIPE 重定向的文件句柄
LONG g_FileHandle_Arrays[ 0x100 ] ;

ULONG g_CopyLimitKb = 0xC000 ;
/*
文件迁移 File Migration Settings
文件迁移设置
默认，沙盘里的程序是不能直接修改系统文件，修改的是沙盘里的替身;先要从真实系统复制一个一模一样的文件到沙盘里，这个过程就是迁移
如果迁移一个过大的文件，比如好几G的文件，这样会影响效率，浪费时间，浪费空间; 所以，要给迁移的文件大小，设置一个上限,超过这个上限，
文件就不会复制到沙盘里，不能修改(只读)，并且会弹出一个提示.默认上限是49152KB,48MB，已经足够大，常用的网络程序不会迁移这么大的文件
*/

LIST_ENTRY_EX g_pNodeHead_RecoverFolder = { NULL, NULL, 0 } ;
LIST_ENTRY_EX g_pNodeHead_AutoRecoverIgnore = { NULL, NULL, 0 } ;
LIST_ENTRY_EX g_pNodeHead_Handles = { NULL, NULL, 0 } ;
LIST_ENTRY_EX g_pNodeHead_DirHandle = { NULL, NULL, 0 } ;

RTL_CRITICAL_SECTION g_HandlesLock ;


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
GetFilePath (
	IN PUNICODE_STRING uniFileName,
	IN HANDLE FileHandle,
	OUT LPWSTR* pOrignalPath,
	OUT LPWSTR* pRedirectedPath,
	OUT BOOL* bIsHandlerSelfFilePath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 18:16]

Routine Description:
  获取对象的原始 & 重定向路径; 调用成功调用者需释放内存
    
Arguments:
  hRootKey - 对象句柄
  uniObjectName - 对象名
  pOrignalPath - 保存原始路径
  pRedirectedPath - 保存重定向路径

Return Value:
  NTSTATUS
    
--*/
{
	ULONG ReturnLength = 0x100 ;
	BOOL bFlag = FALSE, bJoinPathOK = FALSE, bFlag1 = FALSE, bFlag2 = FALSE, bFlag3 = FALSE, bFlag4 = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR szFileName, szNameFileLittle, ptr_end, pBuffer, TempBuffer, OrignalPathFixed ;
	LPWSTR ptr1, ptr2, ptr4, ptr5, ptr6, ptr7, ptr8, ptr9 ;
	ULONG FileNameLength, szPanFu, OrignalPathLength ;
	LPVOLUME_INFO_LITTLE VolumeNode = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( NULL == uniFileName || NULL == pOrignalPath || NULL == pRedirectedPath ) { return STATUS_INVALID_PARAMETER ; }

	*pOrignalPath = *pRedirectedPath = NULL ;
	if ( bIsHandlerSelfFilePath ) { *bIsHandlerSelfFilePath = FALSE ; }

	if ( uniFileName )
	{
		szFileName = uniFileName->Buffer ;
		FileNameLength = uniFileName->Length & 0xFFFE ;
	}
	else
	{
		FileNameLength = 0 ;
		szFileName = NULL  ;
	}

	szNameFileLittle = szFileName ;
	if ( FileHandle )
	{
		PVOID pInfo = NULL ;
		POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;
		POBJECT_TYPE_INFORMATION ObjectTypeInfo = NULL ;

		// 查询句柄对应的对象名
		pInfo = CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, ReturnLength + FileNameLength );

		status = ZwQueryObject( FileHandle, ObjectNameInformation, pInfo, 0x100, &ReturnLength );
		if ( STATUS_OBJECT_PATH_INVALID == status )
		{
			if ( FileNameLength ) { return status; }

			ReturnLength = 0x100 ;
			status = ZwQueryObject( FileHandle, ObjectTypeInformation, pInfo, 0x100, &ReturnLength );
			if ( status < 0 ) { return STATUS_OBJECT_PATH_INVALID; }

			ObjectTypeInfo = (POBJECT_TYPE_INFORMATION) pInfo ;
			LPWSTR ObjectTypeName = ObjectTypeInfo->Name.Buffer ;

			if ( 'F' == *ObjectTypeName && 0 == wcsicmp(ObjectTypeName, L"File") )
			{
				*ObjectNameInfo->Name.Buffer = FileNameLength;
				ObjectNameInfo->Name.Buffer[1] = FileNameLength;
				*pOrignalPath = ObjectNameInfo->Name.Buffer;
				return STATUS_BAD_INITIAL_PC ;
			}
		}

		if ( STATUS_BUFFER_OVERFLOW == status )
		{
			pInfo = CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, ReturnLength + FileNameLength );
			status = ZwQueryObject( FileHandle, ObjectNameInformation, pInfo, ReturnLength, &ReturnLength );
		}

		if ( status < 0 ) { return status; }

		// 查询成功, 拼接对象名 和 文件名
		ObjectNameInfo = (POBJECT_NAME_INFORMATION) pInfo ;
		LPWSTR szObjectName = ObjectNameInfo->Name.Buffer ;
		if ( szObjectName )
		{
			*pOrignalPath = szObjectName;
			ptr_end = &szObjectName[ wcslen(szObjectName) ];
		}
		else
		{
			*pOrignalPath = (LPWSTR) pInfo ;
			ptr_end = (LPWSTR) pInfo ;
		}

		if ( FileNameLength )
		{
			if ( *szFileName != ':' )
			{
				*ptr_end = '\\';
				++ptr_end ;
			}

			memcpy( ptr_end, szFileName, FileNameLength );
			ptr_end += FileNameLength / sizeof(WCHAR) ;
		}

		*ptr_end = 0 ;

		JointPath( pOrignalPath );
		goto _next1_ ;
	}

	if ( 0 == FileNameLength ) { return STATUS_OBJECT_PATH_SYNTAX_BAD; }

	if ( FileNameLength >= 0xC
		&& *szFileName == '\\'
		&& szFileName[1] == '?'
		&& szFileName[2] == '?'
		&& szFileName[3] == '\\'
		&& szFileName[5] == ':' 
		)
	{
		szPanFu = szFileName[4] ;

		// 将盘符装换为小写的.
		if ( szPanFu >= 'A' && szPanFu <= 'Z' ) { szPanFu += 0x20 ; }

		if ( szPanFu < 'a' || szPanFu > 'z' ) { return STATUS_OBJECT_PATH_NOT_FOUND; }

		VolumeNode = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4*(szPanFu - 0x61) );
		FileNameLength -= 0xC ;
		szNameFileLittle = szFileName + 6 ;

		if ( VolumeNode )
		{
			pBuffer = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( 
				pNode, 0,  FileNameLength + (VolumeNode->VolumePathLength + 1) * sizeof(WCHAR) );
			*pOrignalPath = pBuffer ;
			
			memcpy( pBuffer, VolumeNode->VolumePath, VolumeNode->VolumePathLength * sizeof(WCHAR) );
			
			ptr_end = &pBuffer[ VolumeNode->VolumePathLength ];
			memcpy( ptr_end, szNameFileLittle, FileNameLength );
			ptr_end[ FileNameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

			JointPath( pOrignalPath );
			goto _next1_;
		}
	}

	if ( GetVolumeNode( szNameFileLittle, FileNameLength ) )
	{
		pBuffer = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0,  FileNameLength + sizeof(WCHAR) );
		*pOrignalPath = pBuffer ;

		memcpy( pBuffer, szNameFileLittle, FileNameLength );
		pBuffer[ FileNameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		JointPath( pOrignalPath );
		goto _next1_;
	}

	pBuffer = QuerySymbolicLinkObject( pNode, szNameFileLittle, FileNameLength, &bFlag );
	*pOrignalPath = pBuffer ;
	if ( NULL == pBuffer ) { return STATUS_OBJECT_PATH_SYNTAX_BAD; }

	bJoinPathOK = JointPath( pOrignalPath );

	if ( FALSE == (bJoinPathOK | bFlag) && wcsnicmp(*pOrignalPath, L"\\device\\mup\\", 0xC) ) { return STATUS_BAD_INITIAL_PC; }

_next1_:
	ptr1 = *pOrignalPath;
	ReturnLength = OrignalPathLength = wcslen( ptr1 );

	WCHAR i ;
	for ( i = *ptr1; *ptr1; i = *ptr1 )
	{
		if ( i != '\\' || ptr1[1] != '\\' )
		{
			++ptr1 ;
		}
		else
		{
			memmove( ptr1, ptr1 + 1, 2 * (OrignalPathLength - ((ULONG)((PCHAR)ptr1 - (PCHAR)*pOrignalPath) / sizeof(WCHAR)) ) + 2);
			OrignalPathLength = ReturnLength-- - 1 ;
		}
	}

	ptr2 = *pOrignalPath ;
	if ( OrignalPathLength && ptr2[OrignalPathLength - 1] == '\\' )
	{
		OrignalPathLength -- ;
		ReturnLength = OrignalPathLength ;
		ptr2[ OrignalPathLength ] = 0 ;
		bFlag1 = TRUE ;
	}
	else
	{
		bFlag1 = FALSE ;
	}

	if ( OrignalPathLength < 8 || wcsnicmp( ptr2, L"\\device\\", 8 ) )
		return STATUS_OBJECT_PATH_SYNTAX_BAD;

	if ( GetNameType( ptr2, NULL ) )
		return STATUS_BAD_INITIAL_PC;

	if ( wcschr( ptr2, '~' ) )
	{
		bFlag3 = TRUE ;

		OrignalPathFixed = FixupOrignalPath( pNode, *pOrignalPath );
		ReturnLength = wcslen( OrignalPathFixed );
		*pOrignalPath = OrignalPathFixed ;
	}
	else
	{
		bFlag3 = FALSE ;
	}

	ULONG FileRootPathLength = g_BoxInfo.FileRootPathLength / sizeof(WCHAR) - 1 ;

	for (int j = 0; ; j = 1 )
	{
		if ( ReturnLength >= FileRootPathLength && 0 == wcsnicmp( *pOrignalPath, g_BoxInfo.FileRootPath, FileRootPathLength ) )
		{
			if ( ReturnLength == FileRootPathLength ) { return STATUS_BAD_INITIAL_PC; }

			*pOrignalPath = &(*pOrignalPath)[ FileRootPathLength ];
			ReturnLength -= FileRootPathLength ;

			if ( bIsHandlerSelfFilePath ) { *bIsHandlerSelfFilePath = TRUE; }
		}

		if ( ReturnLength < 6 || wcsnicmp(*pOrignalPath, L"\\drive\\", 6) ) { break; }

		if ( (*pOrignalPath)[6] != '\\' )
		{
			*pOrignalPath -= FileRootPathLength ;
			return STATUS_BAD_INITIAL_PC;
		}

		szPanFu = (*pOrignalPath)[ 7 ] ;

		// 将盘符装换为小写的.
		if ( szPanFu >= 'A' && szPanFu <= 'Z' ) { szPanFu += 0x20 ; }
		if ( szPanFu < 'a' || szPanFu > 'z' )
		{
			*pOrignalPath -= FileRootPathLength ;
			return STATUS_BAD_INITIAL_PC;
		}

		VolumeNode = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4*(szPanFu - 0x61) );
		if ( NULL == VolumeNode ) 
		{
			*pOrignalPath -= FileRootPathLength ;
			return STATUS_BAD_INITIAL_PC;
		}

		JointPathEx( pOrignalPath, &ReturnLength, 8, VolumeNode->VolumePath, VolumeNode->VolumePathLength );

	}

	if ( ReturnLength >= 5 && 0 == wcsnicmp(*pOrignalPath, L"\\user", 5) )
	{
		if ( ReturnLength >= 9 && 0 == wcsnicmp(*pOrignalPath, L"\\user\\all", 9) )
		{
			JointPathEx( pOrignalPath, &ReturnLength, 9, g_lpszEnv_00000000_SBIE_ALL_USERS, g_lpszEnv_00000000_SBIE_ALL_USERS_Length );
			bFlag4 = TRUE ;
		}
		else if ( ReturnLength >= 0xD && 0 == wcsnicmp(*pOrignalPath, L"\\user\\current", 0xD) )
		{
			JointPathEx( pOrignalPath, &ReturnLength, 0xD, g_lpszEnv_00000000_SBIE_CURRENT_USER, g_lpszEnv_00000000_SBIE_CURRENT_USER_Length );
			bFlag4 = TRUE ;
		}
		else
		{
			TempBuffer = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory(
				pNode, 0, 2 * (ReturnLength + FileRootPathLength + 1) );
			
			memcpy( TempBuffer, g_BoxInfo.FileRootPath, 2 * FileRootPathLength );
			memmove( &TempBuffer[ FileRootPathLength ], *pOrignalPath, 2 * ReturnLength + 2 );

			*pOrignalPath = TempBuffer ;
			return STATUS_BAD_INITIAL_PC ;
		}
	}
	else if ( ReturnLength >= 7 && 0 == wcsnicmp(*pOrignalPath, L"\\share\\", 7) )
	{
		JointPathEx( pOrignalPath, &ReturnLength, 7, L"\\device\\mup\\", 0xC );
		bFlag4 = TRUE ;
	}

	if ( bFlag3 ) 
	{
		OrignalPathFixed = FixupOrignalPath( pNode, *pOrignalPath );
		*pOrignalPath = OrignalPathFixed ;
		ReturnLength = wcslen( OrignalPathFixed );
	}

	if ( bFlag4 )
	{
		JointPath( pOrignalPath );
		ReturnLength = wcslen( *pOrignalPath );
	}

	//
	// 获取重定向路径
	//

	ptr4 = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 1, ReturnLength + FileRootPathLength + 0x100 );
	*pRedirectedPath = ptr4 ;

	memcpy( ptr4, g_BoxInfo.FileRootPath, 2 * FileRootPathLength );
	ptr5 = &ptr4[ FileRootPathLength ];

	if ( ReturnLength >= 0x19 && 0 == wcsnicmp(*pOrignalPath, L"\\device\\lanmanredirector\\", 0x19) )
	{
		ptr6 = &(*pOrignalPath)[ 0x19 ];
		if ( ';' == *ptr6 )
		{
			ptr7 = wcschr(ptr6, '\\');
			if ( NULL == ptr7 ) { return STATUS_BAD_INITIAL_PC; }
			ptr6 = ptr7 + 1 ;
		}

		memcpy( ptr5, "\\share\\", 0xE );

		ReturnLength = wcslen( ptr6 );
		ptr8 = ptr5 + 7 ;
		memcpy( ptr8, ptr6, 2 * ReturnLength );
		ptr8[ ReturnLength ] = 0 ;
	}
	else if ( ReturnLength >= 0x1E && 0 == wcsnicmp(*pOrignalPath, L"\\device\\mup\\;lanmanredirector\\", 0x1E) )
	{
		ptr6 = &(*pOrignalPath)[ 0x1E ];
		if ( ';' == *ptr6 )
		{
			ptr7 = wcschr(ptr6, '\\');
			if ( NULL == ptr7 ) { return STATUS_BAD_INITIAL_PC; }
			ptr6 = ptr7 + 1 ;
		}

		memcpy( ptr5, "\\share\\", 0xE );

		ReturnLength = wcslen( ptr6 );
		ptr8 = ptr5 + 7 ;
		memcpy( ptr8, ptr6, 2 * ReturnLength );
		ptr8[ ReturnLength ] = 0 ;
	}
	else if ( ReturnLength >= 0xC && 0 == wcsnicmp(*pOrignalPath, L"\\device\\mup\\", 0xC) )
	{
		ptr6 = &(*pOrignalPath)[ 0xC ];
		ptr7 = wcschr(ptr6, '\\');
		if ( ptr7 && 0 == wcsnicmp(ptr7, L"\\PIPE\\", 6) ) { return STATUS_BAD_INITIAL_PC; }

		memcpy( ptr5, "\\share\\", 0xE );

		ReturnLength -= 0xC ;
		ptr8 = ptr5 + 7 ;
		memcpy( ptr8, ptr6, 2 * ReturnLength );
		ptr8[ ReturnLength ] = 0 ;
	}
	else if ( ReturnLength >= g_lpszEnv_00000000_SBIE_CURRENT_USER_Length 
		&& 0 == wcsnicmp(*pOrignalPath, g_lpszEnv_00000000_SBIE_CURRENT_USER, g_lpszEnv_00000000_SBIE_CURRENT_USER_Length) )
	{
		ptr6 = &(*pOrignalPath)[ g_lpszEnv_00000000_SBIE_CURRENT_USER_Length ];
		
		memcpy( ptr5, L"\\user\\current", 0x1A );

		ReturnLength -= g_lpszEnv_00000000_SBIE_CURRENT_USER_Length ;
		ptr8 = ptr5 + 0x1A / 2 ;
		memcpy( ptr8, ptr6, 2 * ReturnLength );
		ptr8[ ReturnLength ] = 0 ;
	}
	else if ( ReturnLength >= g_lpszEnv_00000000_SBIE_ALL_USERS_Length 
		&& 0 == wcsnicmp(*pOrignalPath, g_lpszEnv_00000000_SBIE_ALL_USERS, g_lpszEnv_00000000_SBIE_ALL_USERS_Length) )
	{
		ptr6 = &(*pOrignalPath)[ g_lpszEnv_00000000_SBIE_ALL_USERS_Length ];

		memcpy( ptr5, L"\\user\\all", 0x12 );

		ReturnLength -= g_lpszEnv_00000000_SBIE_ALL_USERS_Length ;
		ptr8 = ptr5 + 0x12 / 2 ;
		memcpy( ptr8, ptr6, 2 * ReturnLength );
		ptr8[ ReturnLength ] = 0 ;
	}
	else
	{
		//
		// 这里面可能有BUG,需要调试,请注意!!!!!
		//

		VolumeNode = (LPVOLUME_INFO_LITTLE) GetVolumeNode( *pOrignalPath, 2 * ReturnLength );
		if ( NULL == VolumeNode ) { return STATUS_BAD_INITIAL_PC; }

		memcpy( ptr5, L"\\drive\\", 0xE );

		ptr8 = ptr5 + 0xE / sizeof(WCHAR) ;
		*(WORD *)ptr8 = LOWORD( VolumeNode->VolumeNumber );
		++ ptr8 ;
		*ptr8 = 0 ;

		if ( ReturnLength == VolumeNode->VolumePathLength )
		{
			if ( FALSE == bFlag1 ) { return STATUS_BAD_INITIAL_PC; }
			bFlag2 = TRUE ;
		} 

		ptr6 = &(*pOrignalPath)[ VolumeNode->VolumePathLength ];
		ReturnLength -= VolumeNode->VolumePathLength ;
		memcpy( ptr8, ptr6, 2 * ReturnLength );
		ptr8[ ReturnLength ] = 0 ;
	}

	if ( bFlag2 )
	{
		ptr9 = &(*pOrignalPath)[ wcslen(*pOrignalPath) ];
		*ptr9 = '\\';
		ptr9[1] = 0 ;
	}

	return STATUS_SUCCESS ;
}


PVOID 
GetVolumeNode (
	IN LPWSTR szFileName,
	IN ULONG FileNameLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/18 [18:11:2010 - 10:48]

Routine Description:
  在数组中查找@szFileName对应的结点

--*/
{
	WCHAR Data ;
	ULONG Index = 0 ;
	LPVOLUME_INFO_LITTLE pNode = NULL ;

	while ( TRUE )
	{
		pNode = *(LPVOLUME_INFO_LITTLE *)( Index + (int)g_SymbolicLinkObjectName_Array );
		if ( pNode )
		{
			if ( FileNameLength >= pNode->VolumePathLength * sizeof(WCHAR) )
			{
				if ( 0 == wcsnicmp(szFileName, pNode->VolumePath, pNode->VolumePathLength) )
				{
					Data = szFileName[ pNode->VolumePathLength ];
					if ( Data == '\\' || !Data ) { break; }
				}
			}
		}

		Index += 4 ;
		if ( Index >= 0x68 ) { return NULL ; }
	}

	return (PVOID)pNode;
}


BOOL HandlerPanfu( ULONG a1 )
{
	int nIndex = 0, VolumeName = 0 ; 
	ULONG ReturnedLength = 0x10 ;
	LPWSTR szName = NULL, SymbolicLinkObjectName = NULL ;
	HANDLE LinkHandle = NULL ;
	LPVOLUME_INFO_LITTLE VolumeNode = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING FileName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	if ( NULL == g_SymbolicLinkObjectName_Array )
	{
		g_SymbolicLinkObjectName_Array = CMemoryManager::GetInstance().kmallocEx( 0x68 );
		memset( g_SymbolicLinkObjectName_Array, 0, 0x68 );
	}

	InitializeObjectAttributes( &ObjAtr, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	while ( TRUE )
	{
		if ( !((1 << nIndex) & a1) ) { goto _While_Next_; }

		VolumeNode = (LPVOLUME_INFO_LITTLE) *(DWORD*) ((int)g_SymbolicLinkObjectName_Array + nIndex * 4 );
		if ( VolumeNode )
		{
			RemoveVolumeNode( VolumeNode->VolumePath );
			*(DWORD *)((int)g_SymbolicLinkObjectName_Array + nIndex * 4) = 0 ;
		}

		VolumeName = nIndex + 'A';
		szName = (LPWSTR) CMemoryManager::GetInstance().kmallocEx( 0x10 );
		swprintf( szName, L"\\??\\%c:", VolumeName );
		RtlInitUnicodeString( &FileName, szName );

		status = ZwOpenSymbolicLinkObject( &LinkHandle, 0x1, &ObjAtr );
		if ( ! NT_SUCCESS(status) ) { goto _While_Next_ ; }

		FileName.Buffer = NULL ;
		FileName.Length = 0 ;
		FileName.MaximumLength = 0 ;

		status = ZwQuerySymbolicLinkObject( LinkHandle, &FileName, &ReturnedLength );
		if ( STATUS_BUFFER_TOO_SMALL == status )
		{
			CMemoryManager::GetInstance().kfreeEx( (int)szName );
			ReturnedLength += 0x20 ;
			
			FileName.MaximumLength = (USHORT) (ReturnedLength - 8) ;
			FileName.Length = (USHORT) (ReturnedLength - 10) ;
			FileName.Buffer = szName = (LPWSTR) CMemoryManager::GetInstance().kmallocEx( ReturnedLength );

			status = ZwQuerySymbolicLinkObject( LinkHandle, &FileName, NULL );
			if ( NT_SUCCESS(status) )
			{
				// 比如符号链接为"\??\A:",则调用函数NtQuerySymbolicLinkObject()之后得到的名字为"\Device\Floppy0"
				szName[ FileName.Length / sizeof(WCHAR) ] = UNICODE_NULL ;

				SymbolicLinkObjectName = QuerySymbolicLinkObject( pNode, szName, FileName.Length, NULL );
				if ( SymbolicLinkObjectName )
				{
					ReturnedLength = wcslen( SymbolicLinkObjectName );
					VolumeNode = (LPVOLUME_INFO_LITTLE) CMemoryManager::GetInstance().kmallocEx( 2*ReturnedLength + 10 );

					VolumeNode->VolumeNumber = nIndex + 'A' ;
					VolumeNode->VolumePathLength = ReturnedLength ;
					wcscpy( VolumeNode->VolumePath, SymbolicLinkObjectName );

					*(DWORD *)( (int)g_SymbolicLinkObjectName_Array + nIndex * 4 ) = (DWORD) VolumeNode ;
				}
			}
		}

		ZwClose( LinkHandle );

_While_Next_:
		CMemoryManager::GetInstance().kfreeEx( (int)szName );
		++ nIndex ;
		if ( nIndex >= 0x1A ) { break; } // 支持的分区数量不超过0x1A个
	}

	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);

	if ( FALSE == g_pNodeHead_xx.bInited )
	{
		g_pNodeHead_xx.bInited = TRUE ;
		HandlerDevices();
	}

	return TRUE ;
}


VOID HandlerFileList()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/01 [1:11:2010 - 17:03]

Routine Description:
  初始化文件的黑白名单,其实驱动中已经初始化好了.此函数可有可无      
    
--*/
{
	HANDLE hDirectory = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING FileName ;
	IO_STATUS_BLOCK IostatusBlock ;

	RtlInitUnicodeString( &FileName, L"\\SystemRoot" );
	InitializeObjectAttributes( &ObjAtr, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	ZwOpenFile( 
		&hDirectory,
		FILE_READ_DATA,
		&ObjAtr ,
		&IostatusBlock ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		0
		);

	if ( hDirectory ) { ZwClose( hDirectory ); }
	return ;
}


void Handler_Configration_AutoRecover()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 15:06]

Routine Description:
  查询配置文件Sandboxie.ini中的项"AutoRecover",若为真,即内容为"y"等,则调用PB_QueryConf()函数查询
  "RecoverFolder" & "AutoRecoverIgnore"项包含的内容,依次插入到链表中.
    
--*/
{
	InitializeCriticalSectionAndSpinCount( &g_HandlesLock, 0x3E8 );

	g_Redirectd_FileHandle_Array = (ULONG) kmalloc( 0x200 );

	*(ULONG *)(g_Redirectd_FileHandle_Array + 0x1FC) = 0xFFFFFFFF ; // 数组结束的标记位

	//
	// TODO{sudami}: 查询配置文件
	//

	return ;
}


BOOL CsrPopulateDosDevices()
{
	return TRUE ; // 无用
}


LPWSTR
QuerySymbolicLinkObject (
	IN PVOID _pNode,
	IN LPWSTR szName,
	IN ULONG NameLength,
	OUT BOOL* bFlag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/04 [4:11:2010 - 16:16]

Routine Description:
  查询成功,调用者来释放内存    
  
Return Value:
  查询到的symbolic对应的字符串路径
    
--*/
{
	ULONG nSize = 0, Length = 0, TempLength = 0 ;
	BOOL bRet = TRUE ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING ObjectName ;
	HANDLE LinkHandle = NULL ;
	LPWSTR ptr = NULL, pSzName = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE)_pNode;

	if ( NULL == szName ) { return NULL; }

	if ( NameLength >= 0x12 && GetNameType(szName, 0) ) { goto _END_; }

	// 1. 以此递归打开symbol path,直到成功获得句柄为止
	ObjectName.MaximumLength = ObjectName.Length = (USHORT) NameLength ;
	ObjectName.Buffer = szName ;

	InitializeObjectAttributes( &ObjAtr, &ObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	for ( ;
		ZwOpenSymbolicLinkObject( &LinkHandle, SYMBOLIC_LINK_QUERY, &ObjAtr ) < 0;
		ObjectName.MaximumLength = (USHORT) nSize 
		)
	{
		nSize = ObjectName.Length ;
		LinkHandle = NULL ;
		if ( nSize <= 2 ) { goto _END_ ; }

		// 根据"\"从右到左依次截断. 比如开始是"A\B\C\D",则经过下面代码的截断后,变成"A\B\C". 
		do
		{
			bRet = nSize == 2 ;
			nSize -= 2 ;
			ObjectName.Length = (USHORT) nSize ;
		}
		while ( FALSE == bRet && '\\' != szName[nSize / sizeof(WCHAR)] );
		
		if ( nSize <= 2 ) { goto _END_ ; }
	}

	if ( NULL == LinkHandle ) { goto _END_ ; }

	// 2. 查询句柄对应的对象名
	Length = NameLength - ObjectName.Length ;
	TempLength =  ObjectName.Length / sizeof(WCHAR) ;

	ObjectName.Length = ObjectName.MaximumLength = 0 ;
	ObjectName.Buffer = NULL ;

	NameLength = 0;
	if ( ZwQuerySymbolicLinkObject( LinkHandle, &ObjectName, &NameLength ) != STATUS_BUFFER_TOO_SMALL )
	{
		ZwClose( LinkHandle );
		return NULL ;
	}

	ObjectName.MaximumLength = (USHORT) NameLength ;
	ObjectName.Length = (USHORT) (NameLength - 2) ;
	ObjectName.Buffer = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, NameLength + Length + 2 ) ;

	status = ZwQuerySymbolicLinkObject( LinkHandle, &ObjectName, NULL );
	
	NameLength = ObjectName.Length ;
	ZwClose( LinkHandle );
	if ( !NT_SUCCESS(status) ) { return NULL ; }

	// 3. 拼接名称
	ptr = &ObjectName.Buffer[NameLength / sizeof(WCHAR)] ;
	memcpy( ptr, &szName[ TempLength ], Length );
	*(WORD *)( (ULONG)ptr + Length ) = 0 ;

	LPWSTR szName_new = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 1, NameLength + Length + 2 );
	memcpy( szName_new, ObjectName.Buffer, NameLength + Length + 2 );

	pSzName = QuerySymbolicLinkObject( pNode, szName_new, NameLength + Length, bFlag );
	if ( pSzName ) { *bFlag = TRUE ; }
	return pSzName ;

_END_ :
	pSzName = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, NameLength + sizeof(WCHAR) );
	memcpy( pSzName, szName, NameLength );
	pSzName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;
	return pSzName ;
}


ULONG 
GetNameType (
	IN LPWSTR wszName,
	OUT LPWSTR* Buffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/03 [3:11:2010 - 17:44]

Routine Description:
  确定@wszName的类型,进行部分字符串匹配    
  typedef enum _Name_Type_Class_ 
  {
        Is_Error_Type    = 0,
        Is_Pipe_Type     = 1, // eg:"\\device\\namedpipe\\", "\\device\\lanmanredirector\\PIPE\\", "\\device\\mup\\;lanmanredirector\\PIPE\\", // "\\device\\mup\\PIPE\\"
        Is_Mailslot_Type = 2
  };
    
Arguments:
  wszName - 要检查的字符串
  Buffer - 若是Pipe类型,保存的字符串形如"\\PIPE\\..."

Return Value:
  0,1,2
    
--*/
{
	LPWSTR ptr = NULL ;

	if ( NULL == wszName || wcslen(wszName) < 0x12 ) { return Is_Error_Type; }

	if ( 0 == wcsnicmp(wszName, L"\\device\\namedpipe\\", 0x12) ) { return Is_Pipe_Type; }

	if ( 0 == wcsnicmp(wszName, L"\\device\\mailslot\\", 0x11) ) { return Is_Mailslot_Type; }

	if ( 0 == wcsnicmp(wszName, L"\\device\\lanmanredirector\\", 0x19) )
	{
		ptr = wcschr( wszName + 0x19, '\\' );
		if ( ptr && 0 == wcsnicmp(ptr, L"\\PIPE\\", 6) )
		{
			if ( Buffer ) { *Buffer = wszName + 0x19 ; }
			return Is_Pipe_Type ;
		}
	}

	ptr = NULL ;
	if (   wcsnicmp( wszName, L"\\device\\mup\\;lanmanredirector\\", 0x1E )
		|| (ptr = wcschr( wszName + 0x1E, '\\' ), NULL == ptr)
		|| wcsnicmp( ptr, L"\\PIPE\\", 6 ) 
		)
	{
		if ( 0 == wcsnicmp( wszName, L"\\device\\mup\\", 0xC ) )
		{
			LPWSTR tmp = wszName + 0xC ;

			ptr = wcschr( tmp, '\\' );
			if ( ptr && 0 == wcsnicmp(ptr, L"\\PIPE\\", 6) )
			{
				if ( Buffer ) { *Buffer = tmp ; }
				return Is_Pipe_Type ;
			}
		}

		return Is_Error_Type ;
	}

	if ( Buffer ) { *Buffer = wszName + 0x1E ; }
	return Is_Pipe_Type ;
}


VOID HandlerDevices()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/03 [3:11:2010 - 17:36]

Routine Description:
  操作Volume Path相关    

--*/
{
	UCHAR OldData ;
	PVOID addr = NULL ;
	HANDLE hFile = NULL ;
	LPWSTR VolumePathBegin = NULL, VolumePathCurrent = NULL ;
	ULONG Index = 0, VolumePathLength = 0 ;
	PUCHAR SymbolicLinkName = NULL, DeviceName = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	UNICODE_STRING ObjectName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IoStatusBlock ;
	MOUNTMGR_MOUNT_POINT InMountPoint = { 0 };
	PMOUNTMGR_MOUNT_POINT	MountPoint	   = NULL ;
	PMOUNTMGR_MOUNT_POINTS	OutMountPoints = NULL ;
	PMOUNTMGR_TARGET_NAME	TargetNameInfo = NULL ;
	PMOUNTMGR_VOLUME_PATHS	VolumePaths	   = NULL ;

	// 1. 获取函数地址,成功则继续
	addr = GetProcAddress( __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG], "GetVolumePathNamesForVolumeNameW" );
	if ( NULL == addr )
	{
		g_bFlag_GetVolumePathNamesForVolumeNameW_is_Null = TRUE ;
		return ;
	}

	// 2. 打开MPM,获得文件句柄,向其发送Ioctl
	RtlInitUnicodeString( &ObjectName, L"\\Device\\MountPointManager" );
	InitializeObjectAttributes( &ObjAtr, &ObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile (
		&hFile,
		0x80100000 ,
		&ObjAtr,
		&IoStatusBlock,
		NULL ,
		FILE_ATTRIBUTE_NORMAL ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ,
		FILE_OPEN ,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT ,
		NULL ,
		0
		);

	if( ! NT_SUCCESS( status ) )
	{
		MYTRACE( L"error! | HandlerDevices() - ZwCreateFile();| status=0x%08lx \n", status );
		return ;
	}

	// 2.1 发送IOCTL_MOUNTMGR_QUERY_POINTS
	OutMountPoints = (PMOUNTMGR_MOUNT_POINTS) kmalloc( 0x2000 );

	status = ZwDeviceIoControlFile (
		hFile,
		0,
		0,
		0,
		&IoStatusBlock,
		IOCTL_MOUNTMGR_QUERY_POINTS,
		&InMountPoint,
		0x18 ,
		OutMountPoints,
		0x2000
		);

	if( ! NT_SUCCESS( status ) )
	{
		MYTRACE( L"error! | HandlerDevices() - ZwDeviceIoControlFile( IOCTL_MOUNTMGR_QUERY_POINTS );| status=0x%08lx \n", status );
		goto _END_ ;
	}

	TargetNameInfo	= (PMOUNTMGR_TARGET_NAME) kmalloc( 0x80 );
	VolumePaths		= (PMOUNTMGR_VOLUME_PATHS) kmalloc( 0x2000 );

	if ( 0 == OutMountPoints->NumberOfMountPoints )
	{
		MYTRACE( L"ko! | HandlerDevices();| 0 == OutMountPoints->NumberOfMountPoints \n" );
		goto _END_ ;
	}

	// 2.2 遍历 xx
	for ( Index = 0; Index < OutMountPoints->NumberOfMountPoints; Index++ )
	{
		MountPoint = OutMountPoints->MountPoints + Index ;

		SymbolicLinkName = (PUCHAR)OutMountPoints + MountPoint->SymbolicLinkNameOffset ;
		DeviceName = (PUCHAR)OutMountPoints + MountPoint->DeviceNameOffset ;

		if (   ( MountPoint->SymbolicLinkNameLength == 48 * sizeof(WCHAR) )
			|| ( MountPoint->SymbolicLinkNameLength == 49 * sizeof(WCHAR) && L'\\' == SymbolicLinkName[48] )
			)
		{
			if ( RtlCompareMemory(SymbolicLinkName, L"\\??\\Volume{", 11 * sizeof(WCHAR)) != 11 * sizeof(WCHAR) ) { continue ; }

			TargetNameInfo->DeviceNameLength = (USHORT) 0x60 ;
			memcpy( TargetNameInfo->DeviceName, SymbolicLinkName, TargetNameInfo->DeviceNameLength ); // SymbolicLinkName --> "\??\Volume{e1b98053-847a-11db-840c-806d6172696f}"
			TargetNameInfo->DeviceName[ TargetNameInfo->DeviceNameLength / sizeof(WCHAR)  ] = UNICODE_NULL ;

			status = ZwDeviceIoControlFile (
				hFile,
				0,
				0,
				0,
				&IoStatusBlock,
				IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS,
				TargetNameInfo,
				0x62 ,
				VolumePaths,
				0x2000
				);

			if( ! NT_SUCCESS( status ) )
			{
				MYTRACE( L"error! | HandlerDevices() - ZwDeviceIoControlFile( IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS );| status=0x%08lx \n", status );
				continue ;
			}

			if ( 0 == VolumePaths->MultiSzLength ) { continue ; }

			OldData = DeviceName[ MountPoint->DeviceNameLength / sizeof(WCHAR) ] ;	// DeviceName --> "\Device\HarddiskVolume1\"
			DeviceName[ MountPoint->DeviceNameLength / sizeof(WCHAR) ] = 0 ;		// 此时变为 DeviceName --> "\Device\HarddiskVolume1"

			// 调用 IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS 查询后,DevicePath已转换为了DosPath. 即 "\Device\HarddiskVolume1" --> "C"
			VolumePathBegin	 = VolumePaths->MultiSz ;
			VolumePathLength = wcslen( VolumePathBegin );

			if ( VolumePathLength > 3 )
			{
				HandlerDevicesEx( (LPWSTR)DeviceName, (LPWSTR)VolumePathBegin );

				if ( VolumePathBegin[ VolumePathLength + 1 ] )
				{
					VolumePathCurrent = &VolumePathBegin[ VolumePathLength + 1 ] ;

					do
					{
						HandlerDevicesEx( (LPWSTR)VolumePathCurrent, (LPWSTR)VolumePathBegin );
						VolumePathCurrent += wcslen( VolumePathCurrent ) + 1 ;
					}
					while ( *VolumePathCurrent );
				}
			}
			else
			{
				if ( VolumePathBegin[ VolumePathLength + 1 ] )
				{
					VolumePathCurrent = &VolumePathBegin[ VolumePathLength + 1 ] ;

					do
					{
						HandlerDevicesEx( (LPWSTR)VolumePathCurrent, (LPWSTR)DeviceName );
						VolumePathCurrent += wcslen( VolumePathCurrent ) + 1 ;
					}
					while ( *VolumePathCurrent );
				}
			}

			DeviceName[ MountPoint->DeviceNameLength / sizeof(WCHAR) ] = OldData ;
		}
	}

_END_ :
	if ( hFile ) { ZwClose( hFile ); }
	kfree( OutMountPoints );
	kfree( TargetNameInfo );
	kfree( VolumePaths );
	return ;
}


BOOL 
HandlerDevicesEx (
	IN LPWSTR szNameFile,
	IN LPWSTR szNameDirectory
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/03 [3:11:2010 - 17:35]

Routine Description:
  将所给的字符串填充到新建结点中,而后插入至链表
    
--*/
{
	ULONG szPanFu = 0 ;
	LPWSTR ptr = NULL ;
	LPVOLUME pNode = NULL, pCurrentNode = NULL ;
	LPVOLUME_INFO_LITTLE VolumeNode1 = NULL, VolumeNode2 = NULL ;

	// 1. 检测参数合法性
	if ( NULL == szNameFile || NULL == szNameDirectory ) { return FALSE ; }

	// 2. 计算File & Directory 字符串的长度
	ULONG FileLength = wcslen( szNameFile );
	if ( ':' == szNameFile[1] )
	{
		szPanFu = *szNameFile ;

		if ( szPanFu >= 'A' && szPanFu <= 'Z' )  // 将盘符装换为小写的.
		{
			szPanFu += 0x20 ;
		}

		if ( szPanFu >= 'a' && szPanFu <= 'z' )
		{
			VolumeNode1 = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4*(szPanFu - 0x61) );
			if ( VolumeNode1 ) { FileLength += VolumeNode1->VolumePathLength ; }
		}
	}

	ULONG DirectoryLength = wcslen( szNameDirectory );
	if ( ':' == szNameDirectory[1] )
	{
		szPanFu = *szNameDirectory ;

		if ( szPanFu >= 'A' && szPanFu <= 'Z' )  // 将盘符装换为小写的.
		{
			szPanFu += 0x20 ;
		}

		if ( szPanFu >= 'a' && szPanFu <= 'z' )
		{
			VolumeNode2 = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4*(szPanFu - 0x61) );
			if ( VolumeNode2 ) { DirectoryLength += VolumeNode2->VolumePathLength ; }
		}
	}

	// 3. 创建新节点,填充FileName & DirectoryName
	pNode = (LPVOLUME) kmalloc( (FileLength + DirectoryLength) * sizeof(WCHAR) + 0x28 );

	// 3.1 填充 FileName
	if ( VolumeNode1 )
	{
		wcscpy( pNode->Buffer, VolumeNode1->VolumePath );
		wcscat( pNode->Buffer, szNameFile + 2 );
	}
	else
	{
		wcscpy( pNode->Buffer, szNameFile );
	}

	pNode->FileNameLength = wcslen(pNode->Buffer) ;
	if ( pNode->FileNameLength )
	{
		do
		{
			if ( *((WORD *)&pNode->DirectoryNameAddr + pNode->FileNameLength + 1) != '\\' )
				break;

			*((WORD *)&pNode->DirectoryNameAddr + pNode->FileNameLength + 1) = 0 ;
		}
		while ( pNode->FileNameLength-- != 1 );
	}

	pNode->Buffer[ pNode->FileNameLength + 1 ] = UNICODE_NULL ;

	// 3.2 填充 DirectoryName
	pNode->DirectoryNameAddr = &pNode->Buffer[ pNode->FileNameLength + 2 ] ; 

	if ( VolumeNode2 )
	{
		wcscpy( pNode->DirectoryNameAddr, VolumeNode2->VolumePath );
		wcscat( pNode->DirectoryNameAddr, szNameDirectory + 2 );
	}
	else
	{
		wcscpy( pNode->DirectoryNameAddr, szNameDirectory );
	}

	pNode->DirectoryNameLength = wcslen( pNode->DirectoryNameAddr );
	if ( pNode->DirectoryNameLength )
	{
		do
		{
			ptr = &pNode->DirectoryNameAddr[ pNode->DirectoryNameLength - 1 ] ;
			if ( *ptr != '\\' ) { break; }

			*ptr = 0 ;
		}
		while ( pNode->DirectoryNameLength-- != 1 );
	}

	// 4. 结点建立完毕,插入到链表中(已存在则不会重复插入)
	if ( 0 == pNode->DirectoryNameLength || 0 == pNode->FileNameLength ) { goto _ERROR_ ; }

	if ( pNode->DirectoryNameLength == pNode->FileNameLength && 0 == wcsicmp(pNode->Buffer, pNode->DirectoryNameAddr) ) { goto _ERROR_ ; }
	
	pCurrentNode = (LPVOLUME)g_pNodeHead_xx.NodeHead.Flink ;
	if ( pCurrentNode )
	{
		while ( TRUE )
		{
			if ( pCurrentNode->FileNameLength == pNode->FileNameLength && 0 == wcsicmp(pCurrentNode->Buffer, pNode->Buffer) )
			{
				// 链表中已存在当前结点,无需插入
				goto _ERROR_ ;
			}

			pCurrentNode = (LPVOLUME) pCurrentNode->ListEntry.Flink ;
			if ( NULL == pCurrentNode ) { break; }
		}
	}

	InsertListA( &g_pNodeHead_xx.NodeHead, 0, &pNode->ListEntry );
	return TRUE ;

_ERROR_ :
	kfree( pNode );
	return FALSE ;
}


VOID RemoveVolumeNode( IN LPWSTR szName )
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/04 [4:11:2010 - 16:54]

Routine Description:
  在链表 g_pNodeHead_xx 中找到@szName对应的结点,删除之
    
Arguments:
  szName - 待查找的字符串
 
--*/
{
	LPVOLUME pCurrentNode = NULL ;
	ULONG Length = 0 ;

	if ( NULL == szName ) { return ; }

	Length = wcslen( szName );
	pCurrentNode = (LPVOLUME)g_pNodeHead_xx.NodeHead.Flink ;
	if ( NULL == pCurrentNode ) { return ; }

	while ( TRUE )
	{
		if ( pCurrentNode->FileNameLength >= Length )
		{
			if ( NULL == pCurrentNode->Buffer[Length] || '\\' == pCurrentNode->Buffer[Length] )
			{
				if ( 0 == wcsnicmp( szName, pCurrentNode->Buffer, Length ) )
				{
					RemoveEntryListEx( &g_pNodeHead_xx.NodeHead, &pCurrentNode->ListEntry );
				}
			}
		}

		pCurrentNode = (LPVOLUME) pCurrentNode->ListEntry.Flink ;
		if ( NULL == pCurrentNode ) { break; }
	}
	
	return ;
}


BOOL
GetClearEnviro (
	IN LPWSTR lpszValue,
	IN ULONG MaxLength
	)
{
	DWORD cchMax = (MaxLength / sizeof(WCHAR) ) - 8 ;

	if ( NULL == lpszValue ) { return FALSE ; }

	// 1. 得到"00000000_SBIE_ALL_USERS"环境变量的内容
	RtlZeroMemory( lpszValue, 0x10 );
	GetEnvironmentVariableW( L"00000000_SBIE_ALL_USERS", lpszValue, cchMax );
	
	if ( *lpszValue )
	{
		// "\Device\HarddiskVolume1\Documents and Settings\All Users"
		g_lpszEnv_00000000_SBIE_ALL_USERS_Length = wcslen( lpszValue );
		g_lpszEnv_00000000_SBIE_ALL_USERS = (LPWSTR) kmalloc( ( wcslen( lpszValue ) + 1 ) * sizeof(WCHAR) );
		wcscpy( g_lpszEnv_00000000_SBIE_ALL_USERS, lpszValue );
	}

	// 2. 得到"00000000_SBIE_CURRENT_USERS"环境变量的内容
	RtlZeroMemory( lpszValue, 0x10 );
	GetEnvironmentVariableW( L"00000000_SBIE_CURRENT_USER", lpszValue, cchMax );
	
	if ( *lpszValue && g_lpszEnv_00000000_SBIE_ALL_USERS_Length )
	{
		// "\Device\HarddiskVolume1\Documents and Settings\KHacker"
		g_lpszEnv_00000000_SBIE_CURRENT_USER_Length = wcslen(lpszValue);
		g_lpszEnv_00000000_SBIE_CURRENT_USER = (LPWSTR) kmalloc( ( wcslen( lpszValue ) + 1 ) * sizeof(WCHAR) );
		wcscpy(g_lpszEnv_00000000_SBIE_CURRENT_USER, lpszValue);
	}

	// 3. 清除这2个环境变量的内容
	SetEnvironmentVariableW( L"00000000_SBIE_ALL_USERS", NULL );
	SetEnvironmentVariableW( L"00000000_SBIE_CURRENT_USER", NULL );

	return g_lpszEnv_00000000_SBIE_ALL_USERS_Length && g_lpszEnv_00000000_SBIE_CURRENT_USER_Length ;
}


BOOL GetPBEnvironment()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/04 [4:11:2010 - 17:13]

Routine Description:
  获取"00000000_SBIE_ALL_USERS" & "00000000_SBIE_CURRENT_USERS"环境变量的内容,保存至全局变量.
    
--*/
{
	int DriverIndex = 0 ;
	UNICODE_STRING EntryContext ;
	RTL_QUERY_REGISTRY_TABLE QueryTable[2] ;
	LPVOLUME_INFO_LITTLE pCurrentNode = NULL ;
	LPWSTR pBuffer = NULL, AllUsersProfile = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 

	// 1. 获取环境变量的内容
	pBuffer = (LPWSTR) kmalloc( 0x400 );
	if ( GetClearEnviro( pBuffer, 0x400 ) )
	{
		kfree( pBuffer );
		return TRUE ;  // 获取成功则直接返货TRUE
	}

	// 2. 获取环境变量的内容失败,尝试其他方法
	RtlZeroMemory( QueryTable, (sizeof(RTL_QUERY_REGISTRY_TABLE)*2) );

	// 2.1 查询"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList"下"ProfilesDirectory"的内容,对应"%SystemDrive%\Documents and Settings"
	EntryContext.Length			= 0x3F8 ;
	EntryContext.MaximumLength	= 0x3FA ;
	EntryContext.Buffer			= pBuffer ;

	QueryTable[0].DefaultType = REG_NONE ;
	QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED ;
	QueryTable[0].Name = L"ProfilesDirectory" ;
	QueryTable[0].EntryContext = &EntryContext ;

	status = RtlQueryRegistryValues( RTL_REGISTRY_WINDOWS_NT, L"ProfileList", QueryTable, NULL, NULL );
	if ( !NT_SUCCESS(status) ) { goto _ERROR_ ; }
	
	DriverIndex = *pBuffer ;
	if ( DriverIndex >= 'A' && DriverIndex <= 'Z' ) { DriverIndex += 0x20 ; }

	if ( DriverIndex >= 'a' && DriverIndex <= 'z' &&  ':' == pBuffer[1] )
	{
		pCurrentNode = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4 * (DriverIndex - 0x61) );
	}

	if ( NULL == pCurrentNode ) { goto _ERROR_ ; }

	// 2.2 查询"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList"下"AllUsersProfile"的内容,对应"All Users"
	AllUsersProfile = (LPWSTR) kmalloc( 0x400 );

	EntryContext.Length = 0x3F8 ;
	EntryContext.MaximumLength = 0x3FA ;
	EntryContext.Buffer = AllUsersProfile ;
	QueryTable[0].Name = L"AllUsersProfile" ;

	status = RtlQueryRegistryValues ( RTL_REGISTRY_WINDOWS_NT, L"ProfileList", QueryTable, NULL, NULL );

	if ( STATUS_OBJECT_NAME_NOT_FOUND == status )
	{
		QueryTable[0].Name = L"ProgramData" ;
		status = RtlQueryRegistryValues ( RTL_REGISTRY_WINDOWS_NT, L"ProfileList", QueryTable, NULL, NULL );
	}

	if ( !NT_SUCCESS(status) ) { goto _ERROR_ ; }

	// 2.3 拼接查询的键值,得到最终结果
	if ( 'A' == *QueryTable[0].Name )
	{
		g_lpszEnv_00000000_SBIE_ALL_USERS_Length = wcslen(AllUsersProfile) + wcslen(QueryTable[0].Name) + pCurrentNode->VolumePathLength + 1 ;
		
		g_lpszEnv_00000000_SBIE_ALL_USERS = (LPWSTR) kmalloc( 2 * g_lpszEnv_00000000_SBIE_ALL_USERS_Length + 2 );
		swprintf( g_lpszEnv_00000000_SBIE_ALL_USERS, L"%s%s\\%s", pCurrentNode->VolumePath, pBuffer + 2, AllUsersProfile );
	}
	else if ( 'P' == *QueryTable[0].Name )
	{
		pCurrentNode = NULL ;
		DriverIndex = *AllUsersProfile ;
		if ( DriverIndex >= 'A' && DriverIndex <= 'Z' ) { DriverIndex += 0x20 ; }

		if ( DriverIndex >= 'a' && DriverIndex <= 'z' &&  ':' == pBuffer[1] )
		{
			pCurrentNode = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4 * (DriverIndex - 0x61) );
		}

		if ( NULL == pCurrentNode ) { goto _ERROR_ ; }

		g_lpszEnv_00000000_SBIE_ALL_USERS_Length = pCurrentNode->VolumePathLength + wcslen( AllUsersProfile + 2 ) ;
		
		g_lpszEnv_00000000_SBIE_ALL_USERS = (LPWSTR) kmalloc( 2 * g_lpszEnv_00000000_SBIE_ALL_USERS_Length + 2 );
		swprintf( g_lpszEnv_00000000_SBIE_ALL_USERS, L"%s%s", pCurrentNode->VolumePath, AllUsersProfile + 2 );
	}
	else
	{
		goto _ERROR_ ;
	}

	// 2.4
	swprintf( AllUsersProfile, L"%s\\%s", L"ProfileList", g_BoxInfo.SID );
	
	EntryContext.Length = 0x3F8 ;
	EntryContext.MaximumLength = 0x3FA ;
	EntryContext.Buffer = pBuffer ;
	QueryTable[0].Name = L"ProfileImagePath";
	
	status = RtlQueryRegistryValues( RTL_REGISTRY_WINDOWS_NT, AllUsersProfile, QueryTable, NULL, NULL );
	if ( !NT_SUCCESS(status) ) { goto _ERROR_ ; }

	pCurrentNode = NULL ;
	DriverIndex = *pBuffer ;
	if ( DriverIndex >= 'A' && DriverIndex <= 'Z' ) { DriverIndex += 0x20 ; }

	if ( DriverIndex >= 'a' && DriverIndex <= 'z' &&  ':' == pBuffer[1] )
	{
		pCurrentNode = *(LPVOLUME_INFO_LITTLE *)( (int)g_SymbolicLinkObjectName_Array + 4 * (DriverIndex - 0x61) );
	}

	if ( NULL == pCurrentNode ) { goto _ERROR_ ; }

	g_lpszEnv_00000000_SBIE_CURRENT_USER_Length = pCurrentNode->VolumePathLength + wcslen(pBuffer + 2);

	g_lpszEnv_00000000_SBIE_CURRENT_USER = (LPWSTR) kmalloc( 2 * g_lpszEnv_00000000_SBIE_ALL_USERS_Length + 2 );
	swprintf( g_lpszEnv_00000000_SBIE_CURRENT_USER, L"%s%s", pCurrentNode->VolumePath, pBuffer + 2 );

	kfree( pBuffer );
	kfree( AllUsersProfile );
	return TRUE ;

_ERROR_ :
	kfree( pBuffer );
	kfree( AllUsersProfile );
	return FALSE ;
}


NTSTATUS
Handler_PipeMailslot_Communication (
	IN int CreateOptions ,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PHANDLE FileHandle,
	IN ULONG DesiredAccess,
	IN PISECURITY_DESCRIPTOR SecurityDescriptor,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG ShareAccess,
	IN ULONG CreateDisposition,
	IN LPWSTR OrignalPath,
	IN ULONG NameType,
	IN LPWSTR PipeName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/09 [9:11:2010 - 14:09]

Routine Description:
  处理 油槽 | 管道 通信; 判断黑白名单; 对于灰白名单,模拟管道通信,重定位油槽通信   
    
Arguments:
  NameType - 油槽 | 管道
  PipeName - 形如"\\PIPE\\***"

--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR RedirectedPath = NULL ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;

	WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );

	// 黑白名单
	if ( bIsBlack ) { return STATUS_ACCESS_DENIED ; }

	if ( bIsWhite )
	{
		RtlInitUnicodeString( ObjectAttributes->ObjectName, OrignalPath );
		ObjectAttributes->SecurityDescriptor = SecurityDescriptor ;
		
		status = g_NtCreateFile_addr ( FileHandle, DesiredAccess, ObjectAttributes,
			IoStatusBlock, 0, 0, ShareAccess, CreateDisposition, CreateOptions, 0, 0 );
		
		return status ;
	}

	// 灰名单

	// 1.处理 管道通信
	if ( Is_Pipe_Type == NameType )
	{
		LPWSTR ptr = wcsrchr( OrignalPath, '\\' );
		if ( ptr )
		{
			++ ptr ;
			if (   0 == wcsicmp(ptr, L"lsarpc")
				|| 0 == wcsicmp(ptr, L"srvsvc")
				|| 0 == wcsicmp(ptr, L"wkssvc")
				|| 0 == wcsicmp(ptr, L"samr")
				|| 0 == wcsicmp(ptr, L"netlogon") 
				)
			{
				status = Imitate_Pipe_Communication( ptr, PipeName, IoStatusBlock, CreateOptions, FileHandle );
				return status ;
			}
		}
	}

	// 2.处理 邮槽通信
	RedirectedPath = Get_Redirected_PipeMailslot_Name( OrignalPath, NameType );
	if ( NULL == RedirectedPath ) { return STATUS_INVALID_PARAMETER ; }

	RtlInitUnicodeString( ObjectAttributes->ObjectName, RedirectedPath );
	status = g_NtCreateFile_addr ( FileHandle, DesiredAccess, ObjectAttributes, 
		IoStatusBlock, 0, 0, ShareAccess, CreateDisposition, CreateOptions, 0, 0 );

	return status ;
}


NTSTATUS
Imitate_Pipe_Communication (
	IN LPWSTR szName,
	IN LPWSTR PipeName,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG CreateOptions,
	OUT HANDLE* FileHandle
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/09 [9:11:2010 - 17:20]

Routine Description:
  模拟进程间的通信,即LPC部分  
    
Arguments:
  szName - 可以为以下字符串:"lsarpc","srvsvc", "wkssvc", "samr", "netlogon"
    
--*/
{
	PLONG FileHandlePtr ;
	LPWSTR ptr1 = NULL ;
	ULONG length = 0, n = 1  ;
	NTSTATUS status = STATUS_SUCCESS ; 
	RPC_IN_Imitate_Pipe_Communication pInBuffer ;
	LPRPC_OUT_Imitate_Pipe_Communication pOutBuffer = NULL ;

	pInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_Imitate_Pipe_Communication ) ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_PipeCommunication_ ;
	pInBuffer.CreateOptions = CreateOptions ;
	
	wcscpy( pInBuffer.szName, szName );
	memset( pInBuffer.PipeName, 0, sizeof(pInBuffer.PipeName) );
	if ( PipeName )
	{
		ptr1 = wcschr( PipeName, '\\' );
		if ( ptr1 )
		{
			length = (ULONG) ((char *)ptr1 - (char *)PipeName) / sizeof(WCHAR) ;
			if ( length > 0x2E ) { length = 0x2E ; }
			
			wcsncpy( pInBuffer.PipeName, PipeName, length );
		}
	}

	pOutBuffer = (LPRPC_OUT_Imitate_Pipe_Communication) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return STATUS_OBJECT_NAME_NOT_FOUND ; }
	
	status = pOutBuffer->Result ;

	if ( pOutBuffer->ReturnLength > 8 )
	{
		IoStatusBlock->Status = (NTSTATUS)pOutBuffer->IoStatusBlock_Pointer ;
		IoStatusBlock->Information = pOutBuffer->IoStatusBlock_Information ;
		
		if ( status >= 0 )
		{
			FileHandlePtr = g_FileHandle_Arrays ;
			while ( *FileHandlePtr || InterlockedCompareExchange(FileHandlePtr, (LONG)pOutBuffer->RedirectedFileHandle, 0) )
			{
				++ n ;
				++ FileHandlePtr ;
				if ( n >= 0x100 )
				{
					status = STATUS_INSUFFICIENT_RESOURCES ;
					break ;
				}
			}

			if ( status >= 0 )
			{
				*FileHandle = (HANDLE)(n | 0xFFFFFF00);
			}
		}
	}

	PB_FreeReply( pOutBuffer );
	return status;
}


LPWSTR
Get_Redirected_PipeMailslot_Name (
	IN LPWSTR szPath,
	IN ULONG NameType
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/09 [9:11:2010 - 17:37]

Routine Description:
  重定向油槽 | 管道的路径; 调用者需要释放内存
    
Arguments:
  szPath - 原始的Pipe 或 Mailslot 的路径
  NameType - 名字类型, Pipe:1; Mailslot:2

Return Value:
  重定向拼接后的字符串指针,eg:"_device_namedpipe_Sandbox_AV_DefaultBox_Session_0\\***"
    
--*/
{
	ULONG length = 0 ;
	LPWSTR ptr = NULL, ptr1 = NULL, ptr2 = NULL, ptrOld = NULL, Name = NULL ;

	if ( NULL == szPath ) { return NULL ; }

	if ( Is_Pipe_Type == NameType )
	{
		ptr = szPath + 18 ;
	}
	else if ( Is_Mailslot_Type == NameType )
	{		
		ptr = szPath + 17 ;
	}

	if ( NULL == ptr ) { return NULL ; }
	length = wcslen( ptr );
	Name = (LPWSTR) kmalloc( (g_BoxInfo.LpcRootPathLength + (length+2) * sizeof(WCHAR)  ) );

	wcscpy( Name, L"\\device\\" );
	
	if ( Is_Pipe_Type == NameType ) {
		wcscat( Name, L"namedpipe" );
	} else {
		wcscat( Name, L"mailslot" );
	}

	ptr1 = &Name[ wcslen(Name) ];
	*ptr1 = '\\';
	++ ptr1 ;
	ptrOld = ptr1 ;
	wcscpy( ptr1, g_BoxInfo.LpcRootPath ); 

	//
	// 拼接完成后,内容形如 eg:"\\device\\namedpipe\\Sandbox\\AV\\DefaultBox\\Session_0"
	// 将上面的字符串转换成下划线的格式,形如: "_device_namedpipe_Sandbox_AV_DefaultBox_Session_0\\"
	//

	while ( *ptr1 )
	{
		ptr2 = wcschr(ptr1, '\\');
		if ( ptr2 )
		{
			ptr1 = ptr2 ;
			*ptr2 = '_' ;
		}
		else
		{
			ptr1 += wcslen( ptr1 );
		}
	}

	*ptr1 = '\\';
	++ ptr1 ;
	*ptr1 = 0 ;

	length = wcslen( ptrOld );
	if ( wcsnicmp( ptr, ptrOld, length) )
	{
		wcscpy( ptr1, ptr );
	}
	else
	{
		wcscpy( ptr1, &ptr[length] );
	}

	return Name ;
}


NTSTATUS
NtCreateFileFilter (
	IN ULONG Flag,
	IN ULONG CreateDisposition ,
	IN ULONG DesiredAccess, 
	IN ULONG CreateOptions
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/10 [10:11:2010 - 14:33]

Routine Description:
  仅过滤灰名单程序调用NtCreateFile / NtOpenFile的参数权限  
    
Arguments:
  CreateDisposition - NtCreateFile / NtOpenFile 的三个重要权限参数
  DesiredAccess -
  CreateOptions - 

--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	BOOL FlagIsOK = FALSE, bIs_FILE_ATTRIBUTE_SYSTEM = FALSE, bIs_FILE_ATTRIBUTE_READONLY = FALSE ;
	ULONG InvalidOP = FILE_NON_DIRECTORY_FILE | FILE_DIRECTORY_FILE ;
	ULONG AlertOP	= FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT ;
	BOOL bWROP = (FILE_SUPERSEDE == CreateDisposition || FILE_OVERWRITE == CreateDisposition || FILE_OVERWRITE_IF == CreateDisposition) ;

	if (   (CreateOptions & FILE_OPEN_BY_FILE_ID) // 通过MFT号打开文件,拒绝掉!
		|| ( (CreateOptions & AlertOP) && !(DesiredAccess & SYNCHRONIZE) )	// 若存在"AlertOP"中包含的任意一个权限,而又没有SYNCHRONIZE权限,拒绝掉!
		|| InvalidOP == (CreateOptions & InvalidOP)	// 若同时包含2个权限(打开文件 & 打开目录),表明参数错误!
		)
	{
		return STATUS_INVALID_PARAMETER ;
	}

	// 若对目录有写权限 或者 删除权限不合法,禁止掉!
	if (   ( CreateOptions & FILE_DIRECTORY_FILE  && bWROP )
		|| ( CreateOptions & FILE_DELETE_ON_CLOSE && !(DesiredAccess & DELETE) )
		)
	{
		return STATUS_INVALID_PARAMETER ;
	}
	
	if ( 0xFFFFFFFF == Flag ) { return STATUS_SUCCESS ; }

	if ( Flag & SBFILE_MarkedAsDirty ) 
	{ 
		Flag = 0 ;
	}
	else
	{
		if ( Flag )
		{
			FlagIsOK = TRUE ;
			
			if ( Flag & SBFILE_ATTRIBUTE_READONLY ) { bIs_FILE_ATTRIBUTE_READONLY = TRUE ; }
			if ( Flag & SBFILE_ATTRIBUTE_SYSTEM ) { bIs_FILE_ATTRIBUTE_SYSTEM = TRUE ; }
			
			Flag = Flag & ( SBFILE_DIRECTORY | SBFILE_NON_DIRECTORY );
		}
	}

	if ( FALSE == FlagIsOK )
	{
		if ( FILE_OPEN == CreateDisposition || FILE_OVERWRITE == CreateDisposition )
			return STATUS_OBJECT_NAME_NOT_FOUND;
	}

	if ( (Flag & SBFILE_DIRECTORY) && bWROP )
	{
		return ((CreateOptions & FILE_NON_DIRECTORY_FILE) != 0 ? 0x85 : 0) - 0x3FFFFFCB ;
	}

	if ( FlagIsOK && FILE_CREATE == CreateDisposition )
	{
		if ( Flag & SBFILE_DIRECTORY && CreateOptions & FILE_NON_DIRECTORY_FILE )
		{
			return STATUS_FILE_IS_A_DIRECTORY ;
		}
		else
		{
			return STATUS_OBJECT_NAME_COLLISION ;
		}
	}
	else
	{
		if ( FILE_OPEN == CreateDisposition || FILE_OPEN_IF == CreateDisposition )
		{
			if ( CreateOptions & FILE_DIRECTORY_FILE && Flag & SBFILE_NON_DIRECTORY )
				return STATUS_NOT_A_DIRECTORY ;

			if ( CreateOptions & FILE_NON_DIRECTORY_FILE && Flag & SBFILE_DIRECTORY )
				return STATUS_FILE_IS_A_DIRECTORY ;
		}

		// 这层过滤很关键.
		ULONG SpecialAccess = DesiredAccess & 0x7FEDFF56 ;

		if (  ( (bIs_FILE_ATTRIBUTE_SYSTEM && bWROP ) || bIs_FILE_ATTRIBUTE_READONLY )  // 1. 该文件是只读属性 或者 要暂停/覆盖 系统文件

			// 2.
			&& (  (CreateDisposition != FILE_OPEN && CreateDisposition != FILE_OPEN_IF)
			|| (SpecialAccess != FILE_WRITE_ATTRIBUTES && SpecialAccess != ACCESS_SYSTEM_SECURITY && SpecialAccess != 0x1000100)
			)

			// 3. 除了普通权限外,还存在其他危险权限
			&& ( SpecialAccess || bWROP) 
			)
		{
			status = ((CreateOptions & FILE_DELETE_ON_CLOSE) != 0 ? 0xFF : 0) - 0x3FFFFFDE ; // 0xc0000022 - STATUS_ACCESS_DENNY
		}
		else
		{
			status = STATUS_SUCCESS ;
		}
	}

	return status ;
}


BOOL
IsDirtyDirectory (
	IN LPWSTR szPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/11 [11:11:2010 - 16:39]

Routine Description:
  以@szPath开始,递归遍历上层目录,直到沙箱自己的根目录为止! 校验目录是否被标记为"已删除"  
    
Arguments:
  szPath - 待校验的目录路径

Return Value:
  TRUE - 是"已删除"目录,禁止操作; FALSE - 未发现异常,可正常操作
    
--*/
{
	LPWSTR ptrCur = NULL, ptrOld = NULL ;
	ULONG FileAttributes ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;
	OBJECT_ATTRIBUTES ObjAtr ; 
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ;

	if ( NULL == szPath ) { return TRUE ; }

	// 递归遍历上层目录,直到沙箱自己的根目录为止! 
	while ( TRUE )
	{
		ptrOld = ptrCur;
		ptrCur = wcsrchr( szPath, '\\' );   // eg: "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\Program Files\Sandboxie"
		if ( ptrOld ) { *ptrOld = '\\'; }

		if ( NULL == ptrCur || ptrCur == szPath ) { break; }

		*ptrCur = 0;  // eg: 截断为"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\Program Files"

		RtlInitUnicodeString( &KeyName, szPath );
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		if ( KeyName.Length <= g_BoxInfo.FileRootPathLength - sizeof(WCHAR) )
		{
			*ptrCur = '\\';  // 已遍历至沙箱自己的根目录,没有发现目录创建时间上的异常现象,正常返回!
			return FALSE ;
		}

		// 获取当前目录的时间信息
		FileAttributes = 0 ;
		status = g_NtQueryFullAttributesFile_addr( &ObjAtr, &FileInfo );
		if ( NT_SUCCESS(status) )
		{
			FileAttributes = (FileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? SBFILE_DIRECTORY : SBFILE_NON_DIRECTORY ;

			if ( IsDirtyKeyEx( (PVOID)&FileInfo.CreationTime ) )
			{
				FileAttributes |= SBFILE_MarkedAsDirty ;
			}
		}

		if ( FileAttributes & SBFILE_MarkedAsDirty )
		{
			*ptrCur = '\\';
			return TRUE ;
		}
	}

	return FALSE ;
}


NTSTATUS 
QueryAttributesFile (
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT ULONG *Flag,
	OUT BOOL *bNotEndOfFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:41]

Routine Description:
  根据@ObjectAttributes查询文件信息,将文件属性转换为自定义的Flag,返回之
    
--*/
{
	NTSTATUS status ;
	ULONG SBFlag, FileAttributes ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;

	if ( NULL == Flag || NULL == ObjectAttributes ) { return STATUS_INVALID_PARAMETER ; }

	*Flag = 0 ;
	status = g_NtQueryFullAttributesFile_addr( ObjectAttributes, &FileInfo );
	if ( !NT_SUCCESS(status) )
	{
		if ( STATUS_NO_SUCH_FILE == status ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
		return status ;
	}

	FileAttributes = FileInfo.FileAttributes ;
	if ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		SBFlag = SBFILE_DIRECTORY ;
	}
	else
	{
		SBFlag = SBFILE_NON_DIRECTORY ;
		if ( bNotEndOfFile && (FileInfo.EndOfFile.LowPart && FileInfo.EndOfFile.HighPart) )
		{
			*bNotEndOfFile = TRUE ;
		}
	}

	if ( FileAttributes & FILE_ATTRIBUTE_READONLY )
		SBFlag |= SBFILE_ATTRIBUTE_READONLY ;

	if ( FileAttributes & FILE_ATTRIBUTE_SYSTEM )
		SBFlag |= SBFILE_ATTRIBUTE_SYSTEM ;

	if ( IsDirtyKeyEx( (PVOID)&FileInfo.CreationTime ) )
	{
		SBFlag |= SBFILE_MarkedAsDirty ;
	}

	*Flag = SBFlag;
	return STATUS_SUCCESS ;
}


NTSTATUS 
CreateRedirectedDirectorys ( 
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/15 [15:11:2010 - 10:21]

Routine Description:
  递归创建重定位后文件的上层目录,把没有的目录都创建好
    
--*/
{
	NTSTATUS status ;
	HANDLE hDirectory ;
	OBJECT_ATTRIBUTES ObjAtr ;
	LPWSTR ptr, ptr_start, ptr_end ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	ULONG NewLength, OrignalPathLength, RedirectedPathLength ;
	FILE_BASIC_INFORMATION FileBaseInfo ;
	USHORT OldMaxLength, OldLength ;
	WCHAR OldData, OldDataEx ;

	if ( NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | CreateRedirectedDirectorys();| 参数不合法  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, g_SecurityDescriptor );
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );

	OrignalPathLength = wcslen( OrignalPath );
	RedirectedPathLength = uniObjectName.Length / sizeof(WCHAR) ;
	ptr_start = uniObjectName.Buffer ;
	ptr_end = &ptr_start[ RedirectedPathLength ];

	// 1. 递归遍历重定位后的文件上层目录,直到找到一个已存在的目录为止!
	while ( TRUE )
	{
		-- ptr_end ;
		if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; }

		while ( '\\' != *ptr_end ) { -- ptr_end ; if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; } }

		if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; }

		OldData = *ptr_end ;
		OldMaxLength = uniObjectName.MaximumLength ;
		OldLength	 = uniObjectName.Length ;

		*ptr_end = NULL ;
		NewLength = (ULONG)((PCHAR)ptr_end - (PCHAR)ptr_start) ;
		uniObjectName.Length		 = (USHORT) NewLength ;
		uniObjectName.MaximumLength = (USHORT) (NewLength + 2) ;

		status = g_NtCreateFile_addr( &hDirectory, 0x10080, &ObjAtr, &IoStatusBlock, NULL, 0, 7, FILE_OPEN_IF, FILE_DIRECTORY_FILE, NULL, 0 );

		uniObjectName.Length		 = OldLength ;
		uniObjectName.MaximumLength = OldMaxLength ;
		*ptr_end = OldData ;

		if ( NT_SUCCESS(status) ) { break ; }

		if ( (STATUS_OBJECT_NAME_NOT_FOUND != status) && (STATUS_OBJECT_PATH_NOT_FOUND != status) ) { return status ; }
	}

	// 2. 开始从找到的那层键值,逐个创建
	status = g_NtQueryInformationFile_addr( hDirectory, &IoStatusBlock, &FileBaseInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	ZwClose( hDirectory );

	if ( NT_SUCCESS(status) && IsDirtyKeyEx( (PVOID)&FileBaseInfo.CreationTime ) ) { return STATUS_OBJECT_NAME_NOT_FOUND ; }

	ptr = &OrignalPath[ OrignalPathLength + ((ULONG)((char *)ptr_end - (char *)RedirectedPath) >> 1) - RedirectedPathLength ];
	OldDataEx= *ptr ;
	OldData = *ptr_end ;
	*ptr = NULL ;
	*ptr_end = NULL ;

	if ( 0 == wcsicmp( ptr + 1, ptr_end + 1 ) ) { CreateRedirectedDirectorysEx( OrignalPath, RedirectedPath ); }

	*ptr_end = OldData ;
	*ptr	 = OldDataEx ;

	while ( TRUE )
	{
		++ ptr_end ;
		++ ptr ;

		if ( NULL == *ptr_end ) { break; }

		do
		{
			if ( '\\' == *ptr_end ) { break; }

			++ ptr_end ;
			++ ptr ;
		}
		while ( *ptr_end );

		if ( NULL == *ptr_end ) { break; }
		
		OldData = *ptr_end ;
		*ptr_end = NULL ;
		OldMaxLength = uniObjectName.MaximumLength ;
		OldLength	 = uniObjectName.Length ;

		NewLength = (ULONG)((PCHAR)ptr_end - (PCHAR)ptr_start) ;
		uniObjectName.Length		 = (USHORT) NewLength ;
		uniObjectName.MaximumLength = (USHORT) (NewLength + 2) ;

		status = g_NtCreateFile_addr( &hDirectory, 0x10080, &ObjAtr, &IoStatusBlock, NULL, 0x80, 7, FILE_OPEN_IF, FILE_DIRECTORY_FILE, NULL, 0 );

		if ( status >= 0 )
		{
			ZwClose( hDirectory );
			OldDataEx = *ptr ;
			*ptr = NULL ;
			
			// 到最后一层时候会出现2个字符串相等的情况,eg: 都为"555.txt"
			if ( 0 == wcsicmp( ptr + 1, ptr_end + 1 ) ) { CreateRedirectedDirectorysEx( OrignalPath, RedirectedPath ); }

			*ptr = OldDataEx ;
		}

		uniObjectName.Length		 = OldLength ;
		uniObjectName.MaximumLength = OldMaxLength ;
		*ptr_end = OldData ;

		if ( ! NT_SUCCESS(status) ) { break ; }
	}

	return status ;
}


NTSTATUS
CreateRedirectedDirectorysEx (
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	)
{
	WCHAR Data ;
	LPWSTR ptr1, ptr2 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG DataLength, RedirectedPathLength ;
	
	IO_STATUS_BLOCK IoStatusBlock ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName; 
	PVOID pOutBuffer = NULL ;
	HANDLE hDirectory = NULL ;
	LPRPC_IN_CreateRedirectedDirectorysEx pInBuffer = NULL ;
	PFILE_BOTH_DIR_INFORMATION FBI = NULL ;
	struct SEARCH_BUFFER 
	{
		FILE_BOTH_DIR_INFORMATION DirInfo ;
		WCHAR Names[ MAX_PATH ];
	} Buffer ;

	if ( NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | CreateRedirectedDirectorysEx();| 参数不合法  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	ptr1 = wcsrchr( OrignalPath, '\\' );
	if ( NULL == ptr1 || NULL == ptr1[1] ) { return STATUS_SUCCESS ; }

	ptr2 = ptr1 + 1 ;
	Data = *ptr2 ;
	*ptr2 = 0;

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	
	// 打开原文件的父目录,得到句柄
	status = g_NtCreateFile_addr( &hDirectory, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );

	*ptr2 = Data;
	if( ! NT_SUCCESS( status ) ) { return status ; }

	// 
	FBI = &Buffer.DirInfo ;
	RtlInitUnicodeString( &uniObjectName, ptr2 );
	status = g_NtQueryDirectoryFile_addr( hDirectory, 0,0,0, &IoStatusBlock, FBI, sizeof(Buffer), FileBothDirectoryInformation, TRUE, &uniObjectName, FALSE );

	if ( STATUS_BUFFER_OVERFLOW == status ) { status = STATUS_SUCCESS ; }
	ZwClose( hDirectory );

	if( ! NT_SUCCESS( status ) ) { return status ; }

	if ( FBI->ShortNameLength )
	{
		RedirectedPathLength = ( wcslen(RedirectedPath) + 1 ) * sizeof(WCHAR) ;
		DataLength = RedirectedPathLength + sizeof( RPC_IN_CreateRedirectedDirectorysEx ) ;
		
		pInBuffer = (LPRPC_IN_CreateRedirectedDirectorysEx) kmalloc( DataLength );
		if ( NULL == pInBuffer ) { return status ; }

		if ( FBI->ShortNameLength <= 0x18 )
		{
			pInBuffer->RpcHeader.DataLength = DataLength ;
			pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_SetFileShortName_ ;

			pInBuffer->ShortNameLength = FBI->ShortNameLength;
			memcpy( pInBuffer->ShortName, FBI->ShortName, FBI->ShortNameLength );
			
			pInBuffer->RedirectedPathLength = RedirectedPathLength ;
			wcscpy( pInBuffer->RedirectedPath, RedirectedPath );

			pOutBuffer = PB_CallServer( (PVOID)pInBuffer );
			if ( pOutBuffer ) { PB_FreeReply( pOutBuffer ); }
		}

		kfree( (PVOID)pInBuffer );
	}

	return status ;
}


NTSTATUS
Copy_OrignalFileData_to_RedirectedFileData (
	IN BOOL bFlag,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG TotalSize = 0, DesiredAccess, CreateOptions, Length ;
	HANDLE hOrignalFile, hRedirectedFile ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;
	BOOL bCopy = FALSE ;
	PVOID pBuffer = NULL ;
	BOOL bIsnot_need_to_write_data_to_Redirected_Area = bFlag ;

	if ( NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | Copy_OrignalFileData_to_RedirectedFileData();| 参数不合法  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, g_SecurityDescriptor );
	RtlInitUnicodeString( &uniObjectName, OrignalPath );

	status = g_NtCreateFile_addr( &hOrignalFile, 0x120089, &ObjAtr, &IoStatusBlock, 0,0,7, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, 0, 0 );

	if ( STATUS_SHARING_VIOLATION == status )
	{
		// 若普通方式打不开原始文件,用驱动去打开之
	//	if ( DeviceIoControl_0x12340022( &hOrignalFile, (DWORD)OrignalPath, v6, v7, v8, NetworkInformation, v18, v19) < 0 )
		{
			// 还是打不开,去掉些权限再次打开
			bIsnot_need_to_write_data_to_Redirected_Area = FALSE ;
			status = g_NtCreateFile_addr( &hOrignalFile, 0x100080, &ObjAtr, &IoStatusBlock, 0,0,7, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, 0, 0 );
		}
	}

	if ( ! NT_SUCCESS(status) ) { return status; }

	// 获取文件大小信息
	status = g_NtQueryInformationFile_addr( hOrignalFile, &IoStatusBlock, &FileInfo, sizeof(FILE_NETWORK_OPEN_INFORMATION), FileNetworkOpenInformation );
	if ( ! NT_SUCCESS(status) )
	{
		ZwClose( hOrignalFile );
		return status;
	}

	// 需要将原始文件数据拷贝到重定向区域
	bCopy = FALSE ;
	if ( FALSE == bIsnot_need_to_write_data_to_Redirected_Area )
	{
		TotalSize = 0 ;
		bCopy = TRUE ;
	}
	else
	{
		TotalSize = FileInfo.EndOfFile.LowPart ;
		if ( TotalSize <= (g_CopyLimitKb << 10) )
		{
			bCopy = TRUE ;
		}
	}

	if ( FALSE == bCopy )
	{
		ZwClose( hOrignalFile );
		return STATUS_BAD_INITIAL_PC ;
	}

	//
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	if ( FileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		DesiredAccess = 0x120089 ;
		CreateOptions = FILE_DIRECTORY_FILE ;
	}
	else
	{
		DesiredAccess = 0x120116 ;
		CreateOptions = FILE_NON_DIRECTORY_FILE ;
	}

	status = g_NtCreateFile_addr (
		&hRedirectedFile,
		DesiredAccess,
		&ObjAtr,
		&IoStatusBlock,
		0,
		0x80,
		7,
		FILE_CREATE,
		CreateOptions | FILE_SYNCHRONOUS_IO_NONALERT,
		0,
		0
		);

	if ( ! NT_SUCCESS(status) )
	{
		ZwClose( hOrignalFile );
		return status;
	}

	if ( TotalSize )
	{
		pBuffer = kmalloc( 0x1000 );
		if ( NULL == pBuffer )
		{
			ZwClose( hOrignalFile );
			ZwClose( hRedirectedFile );
			return STATUS_INSUFFICIENT_RESOURCES ;
		}

		while ( TotalSize )
		{
			Length = 0x1000;
			if ( TotalSize <= 0x1000 ) { Length = TotalSize ; }

			status = ZwReadFile( hOrignalFile, 0, 0, 0, &IoStatusBlock, pBuffer, Length, 0, 0 );
			if ( status < 0 ) { break; }

			TotalSize -= IoStatusBlock.Information ;
			status = ZwWriteFile( hRedirectedFile, 0, 0, 0, &IoStatusBlock, pBuffer, IoStatusBlock.Information, 0, 0 );
			if ( status < 0 ) { break; }
		}

		kfree( (PVOID)pBuffer );
	}

	if ( NT_SUCCESS(status) )
	{
		status = CreateRedirectedDirectorysEx( OrignalPath, RedirectedPath );
		if ( NT_SUCCESS(status) )
		{
			memcpy( &FileBasicInfo, &FileInfo, sizeof(FILE_BASIC_INFORMATION) );
			status = MarkFileTime( &FileBasicInfo, hRedirectedFile, RedirectedPath );
		}
	}

	ZwClose( hOrignalFile );
	ZwClose( hRedirectedFile );
	return status ;
}


NTSTATUS
MarkFileTime (
	IN PVOID FileInformation, 
	IN HANDLE hRedirectedFile,
	IN LPWSTR RedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:21]

Routine Description:
  将@FileInformation中包含的时间,赋予@hRedirectedFile代表的文件
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	ULONG LastAccessTime_HighPart, DataLength, RedirectedPathLength ;
	LPWSTR OrignalPath = NULL ;
	LPRPC_OUT_HEADER pOutBuffer = NULL ;
	LPRPC_IN_MarkFileTime pInBuffer = NULL ;
	PFILE_BASIC_INFORMATION FileBasicInfo = (PFILE_BASIC_INFORMATION) FileInformation ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

//	if ( NULL == FileInformation || NULL == hRedirectedFile ) { return STATUS_INVALID_PARAMETER ; }

	status = g_NtSetInformationFile_addr( hRedirectedFile, &IoStatusBlock, FileBasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	if ( STATUS_ACCESS_DENIED != status ) { return status ; }

	if ( !*(QWORD *)&FileBasicInfo->CreationTime
		&& *(QWORD *)&FileBasicInfo->LastAccessTime
		&& !*(QWORD *)&FileBasicInfo->LastWriteTime
		&& !*(QWORD *)&FileBasicInfo->ChangeTime
		&& !FileBasicInfo->FileAttributes

		|| (FileBasicInfo->CreationTime.HighPart & FileBasicInfo->CreationTime.LowPart) == 0xFFFFFFFF
		&& (LastAccessTime_HighPart = FileBasicInfo->LastAccessTime.HighPart & FileBasicInfo->LastAccessTime.LowPart, LastAccessTime_HighPart == 0xFFFFFFFF)
		&& (FileBasicInfo->LastWriteTime.HighPart & FileBasicInfo->LastWriteTime.LowPart) == 0xFFFFFFFF
		&& (FileBasicInfo->ChangeTime.HighPart & FileBasicInfo->ChangeTime.LowPart) == LastAccessTime_HighPart
		&& !FileBasicInfo->FileAttributes 
		)
	{
		return STATUS_SUCCESS ;
	}

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	if ( NULL == RedirectedPath )
	{
		RtlInitUnicodeString( &uniObjectName, NULL );
		status = GetFilePath ( &uniObjectName, hRedirectedFile, &OrignalPath, &RedirectedPath, NULL );
		if ( ! NT_SUCCESS(status) ) { return status; }

		InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
		RtlInitUnicodeString( &uniObjectName, OrignalPath );

		++ pNode->sFileLock.nLockNtDeleteFile ;

		status = ZwCreateFile (
			&hRedirectedFile,
			0x100100,
			&ObjAtr,
			&IoStatusBlock,
			0,
			0,
			7,
			FILE_OPEN_IF,
			FILE_SYNCHRONOUS_IO_NONALERT,
			0,
			0
			);

		-- pNode->sFileLock.nLockNtDeleteFile ;

		if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

		status = g_NtSetInformationFile_addr( hRedirectedFile, &IoStatusBlock, FileBasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
		ZwClose( hRedirectedFile );

		if ( STATUS_ACCESS_DENIED != status || NULL == RedirectedPath ) { goto _over_ ; }
	}
	
	RedirectedPathLength = ( wcslen(RedirectedPath) + 1 ) * sizeof(WCHAR) ;
	DataLength = RedirectedPathLength + 0x38 ;
	pInBuffer = (LPRPC_IN_MarkFileTime) kmalloc( DataLength );
	if ( pInBuffer )
	{
		pInBuffer->RpcHeader.DataLength = DataLength;
		pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_MarkFileTime_ ;

		memcpy( &pInBuffer->FileBasicInfo, FileBasicInfo, 0x24 );
		pInBuffer->RedirectedPathLength = RedirectedPathLength ;
		wcscpy( pInBuffer->RedirectedPath, RedirectedPath );

		pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( pInBuffer );
		if ( pOutBuffer )
		{
			status = pOutBuffer->u.Status ;
			PB_FreeReply( pOutBuffer );
		}
		else
		{
			status = STATUS_ACCESS_DENIED;
		}
	}

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);

	kfree( pInBuffer );
	return status ;
}


NTSTATUS 
DeleteDirectory (
	IN LPWSTR RedirectedDirectoryPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:44]

Routine Description:
  删除重定向目录下的所有文件

--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	HANDLE hDirectory ;
	BOOLEAN bRestartScan ;
	ULONG FileNameLength ;
	BOOL bIs_self_Directory, bIs_father_Directory ;
	PFILE_DIRECTORY_INFORMATION FileInfo = NULL ;

	if ( NULL == RedirectedDirectoryPath ) { return STATUS_INVALID_PARAMETER ; }

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = g_NtCreateFile_addr( &hDirectory, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );
	if ( ! NT_SUCCESS(status) ) { return status; }

	FileInfo = (PFILE_DIRECTORY_INFORMATION) kmalloc( 0x2A0 );
	bRestartScan = TRUE ;

	while ( TRUE )
	{
		status = ZwQueryDirectoryFile (
			hDirectory,
			NULL,
			NULL,
			NULL,
			&IoStatusBlock,
			FileInfo,
			0x2A0,
			FileDirectoryInformation,
			TRUE,
			NULL,
			bRestartScan
			);

		bRestartScan = FALSE ;
		if ( ! NT_SUCCESS(status) ) { break; }

		FileNameLength = FileInfo->FileNameLength ;
		bIs_self_Directory = FileNameLength == 2 && FileInfo->FileName[0] == '.';
		bIs_father_Directory = FileNameLength == 4 && FileInfo->FileName[0] == '.' && FileInfo->FileName[1] == '.';

		if ( FALSE == bIs_self_Directory && FALSE == bIs_father_Directory )
		{
			kfree( FileInfo );
			ZwClose( hDirectory );
			return STATUS_DIRECTORY_NOT_EMPTY ;
		}
	}

	if ( status != STATUS_NO_MORE_FILES && status != STATUS_NO_SUCH_FILE )
	{
		kfree( FileInfo );
		ZwClose( hDirectory );
		return status ;
	}

	ObjAtr.RootDirectory = hDirectory ;
	bRestartScan = TRUE ;
	
	while ( TRUE )
	{
		status = g_NtQueryDirectoryFile_addr (
			hDirectory,
			0,
			0,
			0,
			&IoStatusBlock,
			FileInfo,
			0x2A0,
			FileDirectoryInformation,
			TRUE,
			NULL,
			bRestartScan
			);

		bRestartScan = FALSE ;
		if ( ! NT_SUCCESS(status) ) { break; }

		FileNameLength = FileInfo->FileNameLength;
		bIs_self_Directory = FileNameLength == 2 && FileInfo->FileName[0] == '.';
		bIs_father_Directory = FileNameLength != 4 || FileInfo->FileName[0] != '.' || FileInfo->FileName[1] != '.' ? 0 : 1;

		if ( FALSE == bIs_self_Directory && FALSE == bIs_father_Directory )
		{
			uniObjectName.Length = LOWORD( FileInfo->FileNameLength );
			uniObjectName.MaximumLength = uniObjectName.Length ;
			uniObjectName.Buffer = FileInfo->FileName ;

			status = g_NtDeleteFile_addr( &ObjAtr );

			if ( STATUS_OBJECT_NAME_NOT_FOUND == status ) { continue; }
		}

		if ( ! NT_SUCCESS(status) ) { break; }
	}

	kfree( FileInfo );
	ZwClose( hDirectory );
	
	if ( STATUS_NO_MORE_FILES == status || STATUS_NO_SUCH_FILE == status ) { status = STATUS_SUCCESS ; }
	return status ;
}


NTSTATUS
MarkAsDirtyFile (
	IN HANDLE hFile,
	IN LPWSTR RedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:26]

Routine Description:
  查询@hFile的信息,若是文件直接标记为"已删除"; 若是目录,删除其中的所有文件,而后标记为"已删除" 
    
Arguments:
  hFile - 待操作的文件(目录)句柄 

--*/
{
	NTSTATUS status ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	IO_STATUS_BLOCK IoStatusBlock ;

	status = g_NtQueryInformationFile_addr( hFile, &IoStatusBlock, &FileBasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	if ( ! NT_SUCCESS(status) ) { return status; }

	if ( !(FileBasicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		|| (status = DeleteDirectory(RedirectedPath), status == STATUS_DELETE_PENDING)
		|| status >= 0 )
	{
		*(QWORD *)&FileBasicInfo.CreationTime = 0x1B01234DEAD44A0ui64 ;

		status = MarkFileTime( &FileBasicInfo, hFile, RedirectedPath );

		if ( STATUS_DELETE_PENDING == status )
			status = STATUS_SUCCESS;
	}

	return status ;
}


NTSTATUS
MarkAsDirtyFileEx (
	IN LPWSTR RedirectedDirectoryPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:39]

Routine Description:
  遍历@szDirectoryPath目录下的文件,标记"灰名单文件"为"已删除"  
    
Arguments:
  RedirectedDirectoryPath - 目录的重定向路径
   
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	HANDLE hDirectory, hFile ;
	BOOLEAN bRestartScan ;
	ULONG FileNameLength ;
	LPWSTR OrignalPath, RedirectedPath ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	BOOL bIs_self_Directory, bIs_father_Directory ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	PFILE_DIRECTORY_INFORMATION FileInfo = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( NULL == RedirectedDirectoryPath ) { return STATUS_INVALID_PARAMETER ; }

	// 1. 打开目录获得句柄
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &uniObjectName, RedirectedDirectoryPath );

	status = g_NtCreateFile_addr( &hDirectory, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE, 0, 0 );
	if ( ! NT_SUCCESS(status) ) { return status; }

	FileInfo = (PFILE_DIRECTORY_INFORMATION) kmalloc( 0x2A0 );
	bRestartScan = TRUE ;

	// 2. 遍历当前目录下的文件
	while ( TRUE )
	{
		status = g_NtQueryDirectoryFile_addr (
			hDirectory,
			NULL,
			NULL,
			NULL,
			&IoStatusBlock,
			FileInfo,
			0x2A0,
			FileDirectoryInformation,
			TRUE,
			NULL,
			bRestartScan
			);

		bRestartScan = FALSE ;
		if ( ! NT_SUCCESS(status) ) { break; }

		FileNameLength = FileInfo->FileNameLength ;
		bIs_self_Directory = FileNameLength == 2 && FileInfo->FileName[0] == '.';
		bIs_father_Directory = FileNameLength == 4 && FileInfo->FileName[0] == '.' && FileInfo->FileName[1] == '.';

		// 2.1 刨去 "." 和 ".."
		if ( bIs_self_Directory || bIs_father_Directory ) { continue; }
		
		uniObjectName.MaximumLength = uniObjectName.Length = LOWORD( FileNameLength );
		uniObjectName.Buffer = FileInfo->FileName ;

		// 2.2 得到文件原始 & 重定向 路径
		CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

		status = GetFilePath( &uniObjectName, hDirectory, &OrignalPath, &RedirectedPath, NULL );
		if( NT_SUCCESS( status ) )
		{
			// 2.3 黑白名单过滤
			WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );
			if ( !bIsWhite && !bIsBlack ) 

			// 2.4 标记灰名单文件为"已删除"状态
			RtlInitUnicodeString( &uniObjectName, RedirectedPath );
			status = g_NtCreateFile_addr( &hFile, 0x120116, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_SUPERSEDE, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, 0, 0 );
			if( NT_SUCCESS( status ) )
			{
				RtlZeroMemory( &FileBasicInfo, sizeof(FILE_BASIC_INFORMATION) );
				*(QWORD *)&FileBasicInfo.CreationTime = 0x1B01234DEAD44A0ui64 ;

				g_NtSetInformationFile_addr( hFile, &IoStatusBlock, &FileBasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
				ZwClose( hFile );
			}
		}

		CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	} // end-of-while

	kfree( FileInfo );
	ZwClose( hDirectory );
	return STATUS_SUCCESS ;
}


NTSTATUS
RemoveDirtyTag (
	IN HANDLE hFile,
	IN LPWSTR RedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:31]

Routine Description:
  若当前@hFile被标记为"已删除",则去除改标记  

--*/
{
	NTSTATUS status ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	IO_STATUS_BLOCK IoStatusBlock ;
	FILETIME SystemTimeAsFileTime ;

	status = g_NtQueryInformationFile_addr( hFile, &IoStatusBlock, &FileBasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	if ( ! NT_SUCCESS(status) ) { return status; }

	if ( IsDirtyKeyEx( (PVOID)&FileBasicInfo.CreationTime ) )
	{
		GetSystemTimeAsFileTime( &SystemTimeAsFileTime );
		FileBasicInfo.CreationTime.LowPart	= SystemTimeAsFileTime.dwLowDateTime ;
		FileBasicInfo.CreationTime.HighPart = SystemTimeAsFileTime.dwHighDateTime ;

		status = MarkFileTime( &FileBasicInfo, hFile, RedirectedPath );
	}
	
	return status ;
}


BOOL
IsWinsxsFile (
	IN LPWSTR szName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/16 [16:11:2010 - 14:58]

Routine Description: 
  若操作的是 *\winsxs\*\目录下的以下后缀名文件,返回TRUE
  ".manifest" & ".cat" & ".dll"

--*/
{
	LPWSTR ptr = NULL, ptr1 = NULL, ptr2 = NULL ;

	if ( NULL == szName ) { return FALSE ; }
	
	ptr = wcsrchr( szName, '.' );
	if ( NULL == ptr ) { return FALSE ; }
	
	if ( wcsicmp(ptr, L".manifest") && wcsicmp(ptr, L".cat") && wcsicmp(ptr, L".dll") ) { return FALSE ; }

	ptr1 = wcschr( szName, '\\' );
	if ( NULL == ptr1 ) { return FALSE ; }

	while ( TRUE )
	{
		ptr2 = ptr1 + 1 ;
		if ( 0 == wcsnicmp( ptr2, L"winsxs\\", 7 ) ) { break; }

		ptr1 = wcschr( ptr2, '\\' );
		if ( NULL == ptr1 ) { return FALSE ; }		
	}

	g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok = '?';
	return TRUE ;
}


BOOL 
IsRecoverFolder (
	IN LPWSTR szName 
	)
{
	BOOL bResult = FALSE, bNeedFree = FALSE ;
	ULONG length = 0, NameLength = 0, nSize = 0 ;
	WCHAR Data1 ;
	LPWSTR Name, ptr1, ptr2, ptr3, ptr4 ;
	LPRecoverFolder_INFO pCurrentNode = NULL ;

	if ( NULL == szName ) { return FALSE ; }

	// 1. 若是以下2种路径,修正以下路径名
	if ( 0 == wcsnicmp(szName, L"\\device\\lanmanredirector\\", 0x19) )
	{
		length = 0x19 ;
	}
	else if ( 0 == wcsnicmp(szName, L"\\device\\mup\\;lanmanredirector\\", 0x1E) )
	{
		length = 0x1E ;
	}
	
	if ( length ) 
	{ 
		ptr1 = &szName[ length ] ;
		if ( ';' == *ptr1 )
		{
			ptr2 = wcschr( ptr1, '\\' );
			if ( *ptr2 && ptr2[1] )
			{
				ptr3 = ptr2 + 1 ;
				length = wcslen( ptr3 );
				Name = (LPWSTR) kmalloc( length + 0x30 );

				memcpy( Name, L"\\device\\mup\\", 0x18 );
				memcpy( Name+0x18, ptr3, (length + 1) * sizeof(WCHAR) );
				bNeedFree = TRUE ;
			}
		}
	}
	else
	{
		Name = szName ;
	}

	// 2. 在RecoverFolder链表中匹配. 看@szName是否为其中的一个结点
	pCurrentNode = (LPRecoverFolder_INFO) g_pNodeHead_RecoverFolder.Flink ;
	if ( NULL == pCurrentNode ) { goto _over_; }

	while ( TRUE )
	{
		if ( 0 == wcsnicmp( pCurrentNode->szPath, Name, pCurrentNode->PathLength ) )
		{
			Data1 = Name[ pCurrentNode->PathLength ];
			if ( '\\' == Data1 || !Data1  ) { break; }
		}

		pCurrentNode = (LPRecoverFolder_INFO) pCurrentNode->ListEntry.Flink ;
		if ( NULL == pCurrentNode ) { goto _over_; }
	}

	// 3. @szName在RecoverFolder链表中
	bResult = TRUE ;
	ptr4 = wcsrchr( Name, '\\' );
	if ( !ptr4 || ptr4[1] != '~' || ptr4[2] != '$' )
	{
		// 3.1 排除Ignore链表
		pCurrentNode = (LPRecoverFolder_INFO) g_pNodeHead_AutoRecoverIgnore.Flink ;
		NameLength = wcslen( Name );

		if ( pCurrentNode ) 
		{ 
			while ( TRUE )
			{
				if ( 0 == wcsnicmp(pCurrentNode->szPath, Name, pCurrentNode->PathLength) )
				{
					Data1 = Name[ pCurrentNode->PathLength ];
					if ( '\\' == Data1 || !Data1  ) { break; }
				}

				nSize = pCurrentNode->PathLength ;
				if ( (NameLength >= nSize) && (0 == wcsicmp( pCurrentNode->szPath, &Name[NameLength - nSize] )) ) { break ; }

				pCurrentNode = (LPRecoverFolder_INFO) pCurrentNode->ListEntry.Flink ;
				if ( NULL == pCurrentNode ) { goto _over_; }
			}

			bResult = FALSE ;
		}
	}

_over_ :
	if ( bNeedFree ) { kfree( (PVOID)Name ) ; }
	return bResult ;
}


BOOL IsArrayEnd( IN ULONG ptr )
{
	if ( 0 == ptr || TRUE == IsBadReadPtr((PVOID)ptr, sizeof(DWORD)) ) { return TRUE ; }

	if ( 0xFFFFFFFF == *(ULONG *) ptr ) { return TRUE ; }

	return FALSE ;
}


BOOL
InsertFileHandle (
	IN HANDLE FileHandle,
	IN LPWSTR szName,
	BOOL bInsert
	)
{
	int Index = 0 ;
	HANDLE CurrentHandle = NULL ;
	BOOL bRet = FALSE, bFlag = FALSE ;
	ULONG pNodeHead, pNode ;

	if ( NULL == FileHandle ) { return TRUE ; }
	if ( 0 == g_Redirectd_FileHandle_Array || NULL == szName ) { return FALSE ; }

	if ( FALSE == bInsert && FALSE == IsRecoverFolder(szName) ) { return FALSE ; }

	EnterCriticalSection( &g_HandlesLock );
	
	// 1. 若数组已满了,不再操作
	if ( IsArrayEnd(g_Redirectd_FileHandle_Array) ) { goto _over_ ; }

	// 2. 遍历数组,查找是否已存在@FileHandle
	pNodeHead = g_Redirectd_FileHandle_Array ;
	CurrentHandle = *(HANDLE *) g_Redirectd_FileHandle_Array ;
	
	while ( CurrentHandle != FileHandle )
	{
		CurrentHandle = *(HANDLE *)( pNodeHead + 4 );
		pNodeHead += 4 ;

		if ( (HANDLE)0xFFFFFFFF == CurrentHandle ) 
		{ 
			// 已遍历至数组末尾,仍未找到对应的句柄
			bFlag = TRUE ; 
		}
	}

	if ( FALSE == bFlag ) { goto _over_ ; }
	if ( IsArrayEnd(g_Redirectd_FileHandle_Array) ) { goto _over_ ; }
	
	// 3. 不存在@FileHandle,故在数组的空闲位置填入该值
	pNode = g_Redirectd_FileHandle_Array ;
	while ( *(ULONG *)pNode )
	{
		CurrentHandle = *(HANDLE *)(pNode + 4);
		pNode += 4 ;
		++Index ;

		if ( (HANDLE)0xFFFFFFFF == CurrentHandle ) { goto _over_ ; }
	}

	*(ULONG *)( g_Redirectd_FileHandle_Array + 4 * Index ) = (ULONG) FileHandle ;

_over_ :
	LeaveCriticalSection(&g_HandlesLock);	
	return TRUE ;
}


BOOL
Handler_RedirectedFile (
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN HANDLE hRedirectedFile
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	WCHAR OldData ;
	ULONG TickCount, RedirectedPathLength, DataLength ;
	HANDLE hDirectory ;
	PVOID pOutBuffer = NULL ;
	OBJECT_ATTRIBUTES ObjAtr;
	IO_STATUS_BLOCK IoStatusBlock ;
	UNICODE_STRING uniObjectName ;
	LPWSTR ptr1, ptr2, ptr3, ptr4 ;
	LPRPC_IN_RedirectedFile pInBuffer = NULL ;
	PFILE_BOTH_DIR_INFORMATION FBI	  = NULL ;
	struct SEARCH_BUFFER 
	{
		FILE_BOTH_DIR_INFORMATION DirInfo ;
		WCHAR Names[ MAX_PATH ];
	} Buffer ;

	if ( NULL == OrignalPath || NULL == RedirectedPath ) { return FALSE ; }

	// 1.1 截断重定向目录,得到父路径
	ptr1 = wcsrchr( RedirectedPath, '\\' );
	if ( NULL == ptr1 ) { return FALSE ; }

	ptr2 = ptr1 + 1 ;
	if ( NULL == *ptr2 ) { return FALSE ; }

	OldData = *ptr2 ;
	*ptr2 = 0 ;

	// 1.2 打开该父目录
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );

	status = g_NtCreateFile_addr( &hDirectory, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );
	if ( STATUS_NOT_A_DIRECTORY == status )
	{
		status = g_NtCreateFile_addr( &hDirectory, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x60, 0, 0 );
	}

	*ptr2 = OldData ;
	if ( ! NT_SUCCESS(status) ) { return FALSE ; }

	// 1.3 查询重定向文件信息

	FBI = &Buffer.DirInfo ;
	RtlInitUnicodeString( &uniObjectName, ptr2 ); // 文件短名
	status = g_NtQueryDirectoryFile_addr( hDirectory,0,0,0, &IoStatusBlock, FBI, sizeof(Buffer), FileBothDirectoryInformation, TRUE, &uniObjectName, FALSE );

	if ( status == STATUS_BUFFER_OVERFLOW == status ) { status = STATUS_SUCCESS; }
	ZwClose( hDirectory );
	
	if ( ! NT_SUCCESS(status) ) { return FALSE ; }
	if ( 0 == FBI->ShortNameLength ) { return FALSE; }

	// 2.1 修正原始文件全路径
	ptr3 = wcsrchr( OrignalPath, '\\' );
	if ( NULL == ptr3 || NULL == ptr3[1] ) { return FALSE ; }

	ptr4 = ptr3 + 1 ;
	memcpy( ptr4, FBI->ShortName, FBI->ShortNameLength );
	ptr4[ FBI->ShortNameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

	// 2.2 验证修正后的原始文件是否存在
	RtlInitUnicodeString( &uniObjectName, OrignalPath );
	status = g_NtCreateFile_addr( &hDirectory, 0x120089, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );
	if ( ! NT_SUCCESS(status) ) { return FALSE ; }

	ZwClose( hDirectory );
	ZwClose( hRedirectedFile );

	// 2.3 RPC通信
	TickCount = GetTickCount();
	RedirectedPathLength = (wcslen( RedirectedPath ) + 1) * sizeof(WCHAR) ;
	DataLength = RedirectedPathLength + sizeof( RPC_IN_RedirectedFile ) ;

	pInBuffer = (LPRPC_IN_RedirectedFile) kmalloc( DataLength );
	
	pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_SetFileShortName_ ;
	pInBuffer->RpcHeader.DataLength = DataLength ;

	pInBuffer->TempNameLength = 0x18 ;
	swprintf( pInBuffer->TempName, L"SB~%05X.%03X", TickCount >> 12, TickCount & 0xFFF );
	
	pInBuffer->RedirectedPathLength = RedirectedPathLength ;
	wcscpy( pInBuffer->RedirectedPath, RedirectedPath );

	pOutBuffer = PB_CallServer( pInBuffer );
	
	if ( pOutBuffer ) { PB_FreeReply( pOutBuffer ); }
	kfree( pInBuffer );
	return TRUE ;
}


BOOL
CallHandlerDevicesEx (
	IN LPWSTR szName
	)
{
	BOOL bRet = FALSE ;
	WCHAR OldData ;
	ULONG ResultLength ;
	HANDLE hDirectory ;
	LPWSTR pstart, pend ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	POBJECT_NAME_INFORMATION NameInfo ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	NTSTATUS status = STATUS_SUCCESS ; 

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, g_SecurityDescriptor );

	pstart = wcschr( szName + 1, '\\' );
	if ( NULL == pstart ) { pstart = szName; }

	pend = &szName[ wcslen(szName) ];
	uniObjectName.Buffer = szName ;
	while ( TRUE )
	{
		OldData = *pend ;
		*pend = 0;

		uniObjectName.Length = (USHORT)((char *)pend - (char *)szName);
		uniObjectName.MaximumLength = uniObjectName.Length + 2 ;

		status = g_NtCreateFile_addr( &hDirectory, 0x100080, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 );
		if ( NT_SUCCESS(status) ) 
		{
			status = g_NtQueryInformationFile_addr( hDirectory, &IoStatusBlock, &FileBasicInfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
			if ( NT_SUCCESS(status) ) 
			{
				ResultLength = 0x1000 ;
				NameInfo = (POBJECT_NAME_INFORMATION) kmalloc( 0x1000 );

				status = ZwQueryObject( hDirectory, ObjectNameInformation, NameInfo, ResultLength - 8, &ResultLength );
				if ( NT_SUCCESS(status) )
				{
					NameInfo->Name.Buffer[ NameInfo->Name.Length / sizeof(WCHAR) ] = UNICODE_NULL ;
					bRet = HandlerDevicesEx( szName, NameInfo->Name.Buffer );
				}

				kfree( NameInfo );
			}

			ZwClose( hDirectory );
		}

		*pend = OldData;
		if ( bRet ) { break; }

		-- pend;
		if ( pend <= pstart ) { break; }

		while ( *pend != '\\' )
		{
			-- pend ;
			if ( pend <= pstart ) { return bRet ; }
		}

		if ( pend <= pstart ) { break; }
	}

	return bRet ;
}


BOOL
JointPath (
	IN LPWSTR *pOrignalPath
	)
{
	WCHAR Data ;
	BOOL bRet = FALSE ;
	ULONG OrignalPathLength = 0, FileNameLength = 0 ;
	LPWSTR OrignalPath = NULL  ;
	LPVOLUME pCurrentNode  = NULL ;

	if ( NULL == pOrignalPath ) { return FALSE; }

	OrignalPath = *pOrignalPath ;
	OrignalPathLength = wcslen( OrignalPath );

	pCurrentNode = (LPVOLUME) g_pNodeHead_xx.NodeHead.Flink ;
	if ( NULL == pCurrentNode ) { return FALSE; }

	do
	{
		FileNameLength = pCurrentNode->FileNameLength ;
		
		if ( OrignalPathLength < FileNameLength
			|| (Data = OrignalPath[FileNameLength], Data != '\\') && Data
			|| wcsnicmp(OrignalPath, pCurrentNode->Buffer, pCurrentNode->FileNameLength) )
		{
			pCurrentNode = (LPVOLUME)pCurrentNode->ListEntry.Flink ;
		}
		else
		{
			JointPathEx( &OrignalPath, &OrignalPathLength, FileNameLength, pCurrentNode->DirectoryNameAddr, pCurrentNode->DirectoryNameLength );

			*pOrignalPath = OrignalPath;
			pCurrentNode = (LPVOLUME) g_pNodeHead_xx.NodeHead.Flink ;
			bRet = TRUE ;
		}
	}
	while ( pCurrentNode );

	return bRet;
}


ULONG 
JointPathEx (
	IN LPWSTR *OrignalPath,
	IN ULONG *OrignalPathLength,
	IN ULONG FileNameLength,
	IN LPWSTR DirectoryNameAddr,
	IN ULONG DirectoryNameLength
	)
{
	LPWSTR pBuffer = NULL ;
	ULONG NewLength, size, TotalSize ;

	size = *OrignalPathLength - FileNameLength + 1 ;
	TotalSize = size + DirectoryNameLength ;

	pBuffer = (LPWSTR) kmalloc( 2 * TotalSize );

	memcpy( pBuffer, DirectoryNameAddr, 2 * DirectoryNameLength );
	memcpy( &pBuffer[ DirectoryNameLength ], &(*OrignalPath)[ FileNameLength ], 2 * size );

	*OrignalPath = pBuffer ;
	*OrignalPathLength = NewLength = TotalSize - 1;
	return NewLength ;
}


LPWSTR 
FixupOrignalPath (
	IN PVOID _pNode,
	IN LPWSTR szOrignalPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/11/18 [18:11:2010 - 11:21]

Routine Description:
  修正@OrignalPath,返回修正后的路径   
    
Arguments:
  pOrignalPath - 待修正的原始路径

--*/
{
	WCHAR OldData ;
	NTSTATUS status ;
	HANDLE hDirectory = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	LPWSTR OrignalPath, pBuffer ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	LPWSTR ptr1, ptr2, ptr3, ptr4 ;
	PFILE_BOTH_DIR_INFORMATION FileInfo = NULL ;
	ULONG n, PointCounts, TotalSize, SlashLength, length1, length2 ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE)_pNode;

	OrignalPath = szOrignalPath ;
	if ( NULL == OrignalPath || 0 == *OrignalPath ) { return OrignalPath; }

	n = 0 ;
	do
	{
		// 1. 每次定位到路径中包含 '~'的位置
		for ( ptr1 = &OrignalPath[ n ]; *ptr1; ++n )
		{
			if ( *ptr1 == '~' ) { break; }
			if ( *ptr1 == '\\' ) { SlashLength = n ; }

			++ ptr1 ;
		}

		// 2. 若路径中没有'~',会跳出循环,否则予以处理
		ptr2 = &OrignalPath[n];
		if ( 0 == OrignalPath[n] ) { break; }

		// 3. 计算路径中小数点'.'的数量
		PointCounts = ((n != SlashLength + 1) - 1) & 0x63;
		do
		{
			// 定位到'~'之后第一次出现'\\'的位置
			if ( *ptr2 == '\\' ) { break; }
			if ( *ptr2 == '.' ) { ++ PointCounts; }

			++ ptr2 ;
			++ n ;
		}
		while ( *ptr2 );

		if ( (PointCounts > 1) || (n - SlashLength - 1 > 0xC) ) { continue; }

		// 4. 截断 第一个'\\'后的路径,打开当前的目录,获取目录句柄
		ptr3 = &OrignalPath[ SlashLength + 1 ];
		OldData = *ptr3 ;
		*ptr3 = 0;

		uniObjectName.Buffer = OrignalPath;
		uniObjectName.Length = 2 * wcslen( OrignalPath );
		uniObjectName.MaximumLength = uniObjectName.Length + 2 ;

		InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
		status = g_NtCreateFile_addr( &hDirectory, 0x80100000, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x21, 0, 0 ); 
		*ptr3 = OldData ;
		if ( status < 0 ) { break; }

		if ( NULL == FileInfo ) { FileInfo = (PFILE_BOTH_DIR_INFORMATION) CMemoryManager::GetInstance().kmallocEx( 0x400 ); }

		// 5. 截断 第二个'\\'后的路径, 查询文件信息
		ptr4 = &OrignalPath[ n ];
		OldData = *ptr4 ;
		*ptr4 = 0;
		
		uniObjectName.Buffer = ptr3;
		uniObjectName.Length = 2 * wcslen( ptr3 );
		uniObjectName.MaximumLength = uniObjectName.Length + 2 ;

		status = g_NtQueryDirectoryFile_addr (
			hDirectory,
			0,
			0,
			0,
			&IoStatusBlock,
			FileInfo,
			0x400,
			FileBothDirectoryInformation,
			TRUE,
			&uniObjectName,
			FALSE
			);

		ZwClose( hDirectory );
		*ptr4 = OldData ;

		if ( STATUS_NO_SUCH_FILE == status ) { continue; }
		if ( status < 0 ) { break; }

		// 修正路径
		pBuffer = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( 
			pNode, 1, FileInfo->FileNameLength + 2 * (wcslen(OrignalPath) + 1) );

		memcpy( pBuffer, OrignalPath, 2 * (SlashLength + 1) );
		memcpy( &pBuffer[ SlashLength + 1 ], FileInfo->FileName, FileInfo->FileNameLength );

		length1 = SlashLength + 1 ;
		length2 = FileInfo->FileNameLength / sizeof(WCHAR) ;
		wcscpy( &pBuffer[ length1 + length2 ], ptr4 );

		TotalSize = 2 * wcslen(pBuffer) + 2;
		OrignalPath = (LPWSTR) CMemoryManager::GetInstance().kmalloc_for_frequently_used_memory( pNode, 0, TotalSize );
		memcpy( OrignalPath, pBuffer, TotalSize );
		
		n = (FileInfo->FileNameLength / sizeof(WCHAR)) + SlashLength + 1 ;
	}
	while ( OrignalPath[ n ] );

	if ( FileInfo ) { CMemoryManager::GetInstance().kfreeEx( (int)FileInfo ); }
	return OrignalPath ;
}


void pfnAPC( IN LPAPCINFO APC )
{
	((PIO_APC_ROUTINE)APC->ApcRoutine)( APC->ApcContext, APC->IoStatusBlock, 0 );
	kfree( APC );
}


NTSTATUS 
GetDirNode ( 
	IN HANDLE FileHandle,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN BOOLEAN RestartScan,
	IN PUNICODE_STRING FileName,
	OUT LPDIR_HANDLE_INFO *pNode
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG OrignalPathLength = 0, tsclient_Length = 0 ;
	LPWSTR szTsclient = L"\\device\\mup\\tsclient\\" ;
	LPDIR_HANDLE_INFO pCurrentNode = NULL, pNextNode = NULL ;

	EnterCriticalSection( &g_Lock_DirHandle );

	OrignalPathLength = 2 * wcslen( OrignalPath );

	pCurrentNode = (LPDIR_HANDLE_INFO)g_pNodeHead_DirHandle.Flink;
	if ( pCurrentNode )
	{
		do
		{
			pNextNode = (LPDIR_HANDLE_INFO) pCurrentNode->ListEntry.Flink ;
			if ( pCurrentNode->FileHandle == FileHandle )
			{
				if (   FALSE == RestartScan
					&& OrignalPathLength == pCurrentNode->OrignalPathLength
					&& 0 == wcsicmp( pCurrentNode->OrignalPath, OrignalPath ) 
					)
				{
					goto _next_ ;
				}

				RemoveEntryListEx( &g_pNodeHead_DirHandle, &pCurrentNode->ListEntry );
				FreeDirNode( pCurrentNode );
			}

			pCurrentNode = pNextNode;
		}
		while ( pNextNode );
	}

	pCurrentNode = (LPDIR_HANDLE_INFO) kmalloc( OrignalPathLength + sizeof(DIR_HANDLE_INFO) );

	pCurrentNode->FileHandle = FileHandle ;
	pCurrentNode->bFlag1 = FALSE ;
	pCurrentNode->bFlag2 = TRUE ;

	if ( FileName && FileName->Length && FileName->Buffer ) 
	{
		LPWSTR pName = (LPWSTR) kmalloc( FileName->Length + 0x10 );

		memcpy( pName, FileName->Buffer, FileName->Length );
		pName[ FileName->Length / sizeof(WCHAR) ] = UNICODE_NULL ;

		RtlInitUnicodeString( &pCurrentNode->SearchName, pName );
	}

	pCurrentNode->OrignalPathLength = OrignalPathLength ;
	memcpy( pCurrentNode->OrignalPath, OrignalPath, OrignalPathLength + 2 );
	
	tsclient_Length = wcslen( szTsclient );
	if ( OrignalPathLength >= 2 * tsclient_Length && 0 == wcsnicmp(OrignalPath, szTsclient, tsclient_Length) )
	{
		pCurrentNode->OrignalDataInfo.bFlag_Is_FileBothDirectoryInformation = TRUE ;
	}

	if ( g_bFlag_GetVolumePathNamesForVolumeNameW_is_Null )
	{
		pCurrentNode->OrignalDataInfo.bFlag_Is_FileBothDirectoryInformation		= TRUE ;
		pCurrentNode->RedirectedDataInfo.bFlag_Is_FileBothDirectoryInformation	= TRUE ;
	}

	InsertListA( &g_pNodeHead_DirHandle, NULL, &pCurrentNode->ListEntry );

_next_ :
	if ( pCurrentNode->bFlag1 )
	{
		status = STATUS_BAD_INITIAL_PC ;
		LeaveCriticalSection( &g_Lock_DirHandle );
	}
	else
	{
		if ( pCurrentNode->RedirectedDataInfo.hFile )
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = FixDirNode( RedirectedPath, OrignalPath, pCurrentNode );
			if ( STATUS_BAD_INITIAL_PC == status ) { pCurrentNode->bFlag1 = TRUE ; }

			if ( status < 0 ) { LeaveCriticalSection( &g_Lock_DirHandle ); }	
		}
	}

	*pNode = pCurrentNode ;
	return status ;
}


void 
FreeDirNode (
	IN LPDIR_HANDLE_INFO pNode
	)
{
	if ( pNode->OrignalDataInfo.hFile ) { g_NtClose_addr( pNode->OrignalDataInfo.hFile ); }

	if ( pNode->OrignalDataInfo.FileInfo ) { kfree(pNode->OrignalDataInfo.FileInfo); }

	if ( pNode->OrignalDataInfo.wszFileName ) { kfree(pNode->OrignalDataInfo.wszFileName); }

	if ( pNode->RedirectedDataInfo.hFile ) { g_NtClose_addr( pNode->RedirectedDataInfo.hFile ); }

	if ( pNode->RedirectedDataInfo.FileInfo ) { kfree(pNode->RedirectedDataInfo.FileInfo); }

	if ( pNode->RedirectedDataInfo.wszFileName ) { kfree(pNode->RedirectedDataInfo.wszFileName); }

	if ( pNode->SearchName.Buffer ) { kfree(pNode->SearchName.Buffer); }

	kfree( pNode );
}


NTSTATUS 
FixDirNode (
	IN LPWSTR RedirectedPath,
	IN LPWSTR OrignalPath,
	IN LPDIR_HANDLE_INFO pNode
	)
{
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IostatusBlock ;
	FILE_BASIC_INFORMATION FileBasicInfo ;
	LPWSTR ptr1 = NULL ;
	ULONG OrignalPathLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	// 1. 检测重定向目录的合法性
	if ( IsDirtyDirectory( RedirectedPath ) ) { return STATUS_OBJECT_PATH_NOT_FOUND; }

	// 2. 打开重定向目录
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );

	status = g_NtCreateFile_addr (
		&pNode->RedirectedDataInfo.hFile ,
		FILE_GENERIC_READ,
		&ObjAtr,
		&IostatusBlock,
		0,
		0,
		7,
		1,
		0x20,
		0,
		0
		);

	if ( status < 0 )
	{
		if ( status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND )
			status = STATUS_BAD_INITIAL_PC ;

		return status;
	}

	// 3. 查询重定向目录的信息
	status = g_NtQueryInformationFile_addr (
		pNode->RedirectedDataInfo.hFile ,
		&IostatusBlock,
		&FileBasicInfo,
		sizeof(FILE_BASIC_INFORMATION) ,
		FileBasicInformation
		);

	if ( status < 0 )
	{
_end_:
		g_NtClose_addr( pNode->RedirectedDataInfo.hFile );
		pNode->RedirectedDataInfo.hFile = NULL ;
		return status ;
	}

	// 4. 若有"已删除"标记,不再操作,返回相应错误码
	if ( IsDirtyKeyEx( (PVOID)&FileBasicInfo.CreationTime ) )
	{
		status = STATUS_OBJECT_NAME_NOT_FOUND ;
		goto _end_ ;
	}

	// 5. 若不是目录,返回失败
	if ( !(FileBasicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
	{
		status = STATUS_INVALID_PARAMETER ;
		goto _end_ ;
	}

	pNode->RedirectedDataInfo.bFlag2			= TRUE ;
	pNode->RedirectedDataInfo.bFlag_RestartScan = TRUE ;

	// 若目录的全路径末尾没有'\\',强行加上去
	OrignalPathLength = sizeof(WCHAR) * wcslen( OrignalPath );
	if (   OrignalPathLength <= 2
		|| (ptr1 = &OrignalPath[ (OrignalPathLength >> 1) - 1 ], '\\' == *ptr1)
		)
	{
		ptr1 = NULL ;
	}
	else
	{
		OrignalPath[ OrignalPathLength / sizeof(WCHAR) ] = '\\' ;
		ptr1[2] = 0 ;
		OrignalPathLength += 2 ;
	}

	uniObjectName.Length		= (USHORT) OrignalPathLength ;
	uniObjectName.MaximumLength = (USHORT) (OrignalPathLength + sizeof(WCHAR)) ;
	uniObjectName.Buffer		= OrignalPath ;

	status = g_NtCreateFile_addr (
		&pNode->OrignalDataInfo.hFile ,
		FILE_GENERIC_READ,
		&ObjAtr,
		&IostatusBlock,
		0,
		0,
		7,
		1,
		0x21,
		0,
		0
		);

	if ( ptr1 ) { ptr1[1] = 0 ; }

	if ( status >= 0 )
	{
		pNode->OrignalDataInfo.bFlag2				= TRUE ;
		pNode->OrignalDataInfo.bFlag_RestartScan	= TRUE ;
	}
	else
	{
		if ( status != STATUS_NOT_A_DIRECTORY
			&& status != STATUS_OBJECT_NAME_NOT_FOUND
			&& status != STATUS_OBJECT_PATH_NOT_FOUND )
		{
			goto _end_ ;
		}

		status = STATUS_SUCCESS ;
	}

	if ( NULL == pNode->OrignalDataInfo.hFile ) { return status; }

	status = FixDirNodeEx( &pNode->OrignalDataInfo, &pNode->SearchName, 0 );
	if ( NT_SUCCESS(status) && pNode->OrignalDataInfo.FuckedFlag )
	{
		status = FixDirNodeEx( &pNode->RedirectedDataInfo, &pNode->SearchName, 1 );
	}

	if ( ! NT_SUCCESS(status) )
	{
		if ( pNode->RedirectedDataInfo.hFile )
		{
			g_NtClose_addr( pNode->RedirectedDataInfo.hFile );
			pNode->RedirectedDataInfo.hFile = NULL ;
		}

		if ( pNode->OrignalDataInfo.hFile )
		{
			g_NtClose_addr( pNode->OrignalDataInfo.hFile );
			pNode->OrignalDataInfo.hFile = NULL ;
		}
	}

	return status ;
}


NTSTATUS 
FixDirNodeEx(
	IN LPDIR_HANDLE_INFO_LITTLE pNode, 
	IN PUNICODE_STRING pSearchName,
	IN BOOLEAN bFlag
	)
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	IO_STATUS_BLOCK IoStatusBlock ;
	FILE_ATTRIBUTE_TAG_INFORMATION FileTagInfo ;
	PFILE_ID_BOTH_DIR_INFORMATION FileDirInfo = NULL ;
	LPDIR_HANDLE_INFO_LITTLEEX NodeNew = NULL, NodeCur = NULL ;

	if ( NULL == pNode ) { return STATUS_INVALID_PARAMETER ; }

	if ( pNode->FuckedFlag ) { pNode->FuckedFlag = FALSE ; }

	if ( bFlag )
	{ 
		bRet = TRUE ;
	}
	else
	{
		status = g_NtQueryInformationFile_addr( pNode->hFile, &IoStatusBlock, &FileTagInfo, 8, FileAttributeTagInformation );
		if ( STATUS_INVALID_PARAMETER == status ) { bRet = TRUE ; }
	}

	if ( FALSE == bRet ) { return status; }

	pNode->FuckedFlag = TRUE ;
	ClearStruct( &pNode->ListEntry );

	//
	// 1. 查询目录信息,将其中包含的所有文件挨个插入到链表中保存起来
	//

	FileDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION) kmalloc( 0x800 );
	if ( NULL == FileDirInfo ) { return STATUS_INSUFFICIENT_RESOURCES; }

	status = g_NtQueryDirectoryFile_addr (
		pNode->hFile,
		0,
		0,
		0,
		&IoStatusBlock,
		FileDirInfo,
		0x800,
		FileIdBothDirectoryInformation,
		FALSE,
		pSearchName,
		pNode->bFlag_RestartScan
		);

	//
	// 2. Do Work
	//

	while ( TRUE )
	{
		if ( status < 0 ) { break; }

		pNode->bFlag_RestartScan = FALSE ; // 表明要扫描单个文件

		// 申请新节点,用来存放当前查询到的文件信息
		NodeNew = (LPDIR_HANDLE_INFO_LITTLEEX) kmalloc( FileDirInfo->FileNameLength + 0x88 );
		if ( NULL == NodeNew ) 
		{
			status = STATUS_INSUFFICIENT_RESOURCES ;
			goto _over_ ;
		}

		while ( TRUE )
		{
			NodeNew->StructLength = FileDirInfo->FileNameLength + sizeof(WCHAR) + sizeof(FILE_ID_BOTH_DIR_INFORMATION) ;

			memcpy(  &NodeNew->FileDirInfo, FileDirInfo, NodeNew->StructLength );

			NodeNew->FileDirInfo.NextEntryOffset = 0 ;

			NodeNew->SearchName.MaximumLength = NodeNew->SearchName.Length = (USHORT) FileDirInfo->FileNameLength ;
			NodeNew->SearchName.Buffer		  = NodeNew->FileDirInfo.FileName;

			NodeCur = (LPDIR_HANDLE_INFO_LITTLEEX) pNode->ListEntry.Flink ;
			if ( NULL == NodeCur ) { goto _InsertA_; }

			while ( RtlCompareUnicodeString( &NodeCur->SearchName, &NodeNew->SearchName, TRUE ) <= 0 )
			{
				NodeCur = (LPDIR_HANDLE_INFO_LITTLEEX) NodeCur->ListEntry.Flink ;
				if ( NULL == NodeCur ) { goto _InsertA_; }
			}

			if ( NodeCur )
			{
				InsertListB( &pNode->ListEntry, &NodeCur->ListEntry, &NodeNew->ListEntry );
			}
			else
			{
_InsertA_:
				InsertListA( &pNode->ListEntry, NULL, &NodeNew->ListEntry );
			}

			if ( 0 == FileDirInfo->NextEntryOffset ) { break; }
			
			FileDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION)( (PCHAR)FileDirInfo + FileDirInfo->NextEntryOffset );

			NodeNew = (LPDIR_HANDLE_INFO_LITTLEEX) kmalloc( FileDirInfo->FileNameLength + 0x88 );
			if ( NULL == NodeNew ) 
			{
				status = STATUS_INSUFFICIENT_RESOURCES ;
				goto _over_ ;
			}
		} // end-of-while (Small)

		status = g_NtQueryDirectoryFile_addr (
			pNode->hFile,
			0,
			0,
			0,
			&IoStatusBlock,
			FileDirInfo,
			0x800,
			FileIdBothDirectoryInformation,
			FALSE,
			pSearchName,
			pNode->bFlag_RestartScan
			);

	} // end-of-while (Big)

	if ( STATUS_INVALID_INFO_CLASS == status )
	{
		status = FixDirNodeExp( pNode, pSearchName, (PFILE_BOTH_DIR_INFORMATION)FileDirInfo );
	}

	if ( STATUS_NO_MORE_FILES == status || STATUS_NO_SUCH_FILE == status )
	{
		status = STATUS_SUCCESS ;
	}

_over_ :
	kfree( FileDirInfo );
	return status;
}


NTSTATUS 
FixDirNodeExp (
	IN LPDIR_HANDLE_INFO_LITTLE pNode,
	IN PUNICODE_STRING pSearchName,
	IN PFILE_BOTH_DIR_INFORMATION FileDirInfo
	)
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	IO_STATUS_BLOCK IoStatusBlock ;
	LPDIR_HANDLE_INFO_LITTLEEX NodeNew = NULL, NodeCur = NULL ;

	status = g_NtQueryDirectoryFile_addr (
		pNode->hFile,
		0,
		0,
		0,
		&IoStatusBlock,
		FileDirInfo,
		0x800,
		FileBothDirectoryInformation,
		FALSE,
		pSearchName,
		pNode->bFlag_RestartScan
		);

	if ( ! NT_SUCCESS(status) ) { return status ; }

	while ( TRUE )
	{
		pNode->bFlag_RestartScan = FALSE ; // 表明要扫描单个文件

		// 申请新节点,用来存放当前查询到的文件信息
		NodeNew = (LPDIR_HANDLE_INFO_LITTLEEX) kmalloc( FileDirInfo->FileNameLength + 0x88 );
		if ( NULL == NodeNew ) { return STATUS_INSUFFICIENT_RESOURCES ; }

		while ( TRUE )
		{
			memcpy( &NodeNew->FileDirInfo, FileDirInfo, 0x60 );

			NodeNew->FileDirInfo.FileId.LowPart  = 0 ;
			NodeNew->FileDirInfo.FileId.HighPart = 0 ;

			memcpy( NodeNew->FileDirInfo.FileName, FileDirInfo->FileName, FileDirInfo->FileNameLength );

			NodeNew->StructLength = FileDirInfo->FileNameLength + sizeof(WCHAR) + sizeof(FILE_ID_BOTH_DIR_INFORMATION) ;
			NodeNew->FileDirInfo.NextEntryOffset = 0 ;

			NodeNew->SearchName.MaximumLength = NodeNew->SearchName.Length = (USHORT) FileDirInfo->FileNameLength ;
			NodeNew->SearchName.Buffer		  = NodeNew->FileDirInfo.FileName;

			NodeCur = (LPDIR_HANDLE_INFO_LITTLEEX) pNode->ListEntry.Flink ;
			if ( NULL == NodeCur ) { goto _InsertA_; }

			while ( RtlCompareUnicodeString( &NodeCur->SearchName, &NodeNew->SearchName, TRUE ) <= 0 )
			{
				NodeCur = (LPDIR_HANDLE_INFO_LITTLEEX) NodeCur->ListEntry.Flink ;
				if ( NULL == NodeCur ) { goto _InsertA_; }
			}

			if ( NodeCur )
			{
				InsertListB( &pNode->ListEntry, &NodeCur->ListEntry, &NodeNew->ListEntry );
			}
			else
			{
_InsertA_:
				InsertListA( &pNode->ListEntry, NULL, &NodeNew->ListEntry );
			}

			if ( 0 == FileDirInfo->NextEntryOffset ) { break; }

			FileDirInfo = (PFILE_BOTH_DIR_INFORMATION)( (PCHAR)FileDirInfo + FileDirInfo->NextEntryOffset );

			NodeNew = (LPDIR_HANDLE_INFO_LITTLEEX) kmalloc( FileDirInfo->FileNameLength + 0x88 );
			if ( NULL == NodeNew ) { return STATUS_INSUFFICIENT_RESOURCES ; }

		} // end-of-while (Small)

		status = g_NtQueryDirectoryFile_addr (
			pNode->hFile,
			0,
			0,
			0,
			&IoStatusBlock,
			FileDirInfo,
			0x800,
			FileBothDirectoryInformation,
			FALSE,
			pSearchName,
			pNode->bFlag_RestartScan
			);

		if ( ! NT_SUCCESS(status) ) { return status ; }

	} // end-of-while (Big)

	return status ;
}


NTSTATUS 
NtQueryDirectoryFileFilter ( 
	IN LPDIR_HANDLE_INFO pNode, 
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN int FileInformation,
	IN ULONG Length,
	IN int FileInformationClass,
	IN BOOLEAN ReturnSingleEntry
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	BOOLEAN bFlag_Dont_need_to_get_RedirectedDirectory_nextFileInfo ;
	BOOLEAN bFlag_Dont_need_to_get_OrignalDirectory_nextFileInfo ;
	ULONG StructSize = 0, next_file_needed_size = 0, FileNameLength = 0 ;
	ULONG CurrentTotal_FileInformation_Size = 0 ;
	LPWSTR pName = NULL ;
	PUNICODE_STRING SearchName = NULL ;
	LPFILE_COM_DIR_INFORMATION FileInformation_new = NULL ;
	int FileInformationCur, FileInformationNext, FileInformationDummy ;

	switch ( FileInformationClass )
	{
	case FileDirectoryInformation:
	case FileFullDirectoryInformation:
		StructSize = 0x48 ;
		break;

	case FileBothDirectoryInformation:
		StructSize = 0x60 ;
		break;

	case FileNamesInformation:
		StructSize = 0x10 ;
		break;

	case FileIdBothDirectoryInformation:
		StructSize = 0x70 ;
		break;

	case FileIdFullDirectoryInformation:
		StructSize = 0x58 ;
		break;

	default:
		return STATUS_INVALID_INFO_CLASS;
	}

	if ( StructSize > Length ) { return STATUS_INFO_LENGTH_MISMATCH; }

	FileInformationCur = FileInformationDummy = FileInformation ;
	IoStatusBlock->Information = 0 ;
	SearchName = &pNode->SearchName ;

	//
	// 1.1 每次都是先在重定向目录中进行查询,将得到的下个文件名存放到 pNode->RedirectedDataInfo 中
	//

	status = Handler_NtQueryDirectoryFile_FileInformation( &pNode->RedirectedDataInfo, SearchName );
	if ( ! NT_SUCCESS(status) )  { goto _END_ ; }

	//  
	// 1.2 循环在原目录中进行查询.若找到名字相同的文件(夹),分2种情况:
	//   1.2.1 若文件(夹)时间是特殊的,表明是沙箱中已被删除而真实系统中未被删除的文件,需在显示界面中隐藏该文件! 则将该文件的信息抹除掉
	//   1.2.2 若不是特殊时间,表明是沙箱中新建而真实系统中不存在的文件.则将重定向目录中该文件信息添加到原始信息中
	//

	while ( TRUE )
	{
		status = Handler_NtQueryDirectoryFile_FileInformation( &pNode->OrignalDataInfo, SearchName );
		if ( ! NT_SUCCESS(status) ) { goto _END_ ; }

		bFlag_Dont_need_to_get_RedirectedDirectory_nextFileInfo = pNode->RedirectedDataInfo.bFlag_dont_need_to_get_nextFileInfo;
		bFlag_Dont_need_to_get_OrignalDirectory_nextFileInfo	= pNode->OrignalDataInfo.bFlag_dont_need_to_get_nextFileInfo ;
		FileInformation_new = NULL ;

		//  [过滤伊始]
		if ( bFlag_Dont_need_to_get_RedirectedDirectory_nextFileInfo )
		{
			if ( bFlag_Dont_need_to_get_OrignalDirectory_nextFileInfo )
			{
				// 如果重定向目录 & 原始目录 都有文件存在,在此判断
				BOOL bIs_not_Equal = RtlCompareUnicodeString( &pNode->OrignalDataInfo.uniFileName, &pNode->RedirectedDataInfo.uniFileName, TRUE );

				//  
				// 若查询到"重定向目录"中的"文件名长度" 小于 "原目录"中的当前查询到的"文件名长度",则用原目录中的文件信息
				// 否则一律用重定向目录中的文件信息. 这里仅是一个按照文件名显示先后的问题!
				//  
				if ( bIs_not_Equal < 0 ) { goto _Dont_need_to_get_RedirectedDirectory_nextFileInfo_ ; }

				FileInformation_new = pNode->RedirectedDataInfo.FileInfo ;

				// 置0表示需要查询重定向目录中下个文件的文件信息; 置1表明下个文件的信息已查询到,不需要继续往后查询
				pNode->RedirectedDataInfo.bFlag_dont_need_to_get_nextFileInfo = FALSE ;

				// 此句判断很关键,若重定向目录 & 原始目录的文件名相同,则不管那么多,原始目录需要跳过当前文件,跳到下个文件区查询
				if ( FALSE == bIs_not_Equal ) { goto _need_to_get_OrignalDirectory_nextFileInfo_ ; }
				
				goto _next_ ;
			}

			//  
			// 到这条IF语句的情况是:原始目录中的文件已遍历完,没有文件可查询了.
			// 在此判断重定向目录中是否还有文件存在.若存在,则依次拷贝当前重定向文件
			// 信息到总的FileInformation中,让其在沙箱中运行的explorer.exe显示出来
			//  
			if ( bFlag_Dont_need_to_get_RedirectedDirectory_nextFileInfo )
			{
				FileInformation_new = pNode->RedirectedDataInfo.FileInfo ;
				pNode->RedirectedDataInfo.bFlag_dont_need_to_get_nextFileInfo = FALSE ;

				goto _next_ ;
			}
		}

		if ( bFlag_Dont_need_to_get_OrignalDirectory_nextFileInfo )
		{
_Dont_need_to_get_RedirectedDirectory_nextFileInfo_:
			FileInformation_new = pNode->OrignalDataInfo.FileInfo ;

_need_to_get_OrignalDirectory_nextFileInfo_:
			// 置0表示需要查询原始目录下个文件的文件信息; 置1表明下个文件的信息已查询到,不需要继续往后查询
			pNode->OrignalDataInfo.bFlag_dont_need_to_get_nextFileInfo = FALSE ;
		}

_next_:
		//  
		// 1.2.1 [这里过滤操作的结果]: 不显示在沙箱中已被删除的文件,而其实在真实系统中是存在的文件
		//  

		if ( FileInformation_new == pNode->RedirectedDataInfo.FileInfo && IsDirtyKeyEx((PVOID)&FileInformation_new->FileBothDirInfo.CreationTime) )
		{
			goto _while_next_ ;
		}

		if ( NULL == FileInformation_new )
		{
			status = ((FileInformationCur != FileInformation) - 1) & STATUS_NO_MORE_FILES ;
			*(DWORD *)FileInformationDummy = 0 ;
			goto _END_ ;
		}

		// 1.2.2.(1) 用修改过的信息填充参数三FileInformation内存块

		// 得到已查询到的下个文件信息所需要的内存块大小
		next_file_needed_size = StructSize - 2 ;
		if ( FileInformationCur != FileInformation )
		{
			next_file_needed_size += FileInformation_new->FileBothDirInfo.FileNameLength ; 
		}

		// 检查是否有足够的空间存放已查询到的下个文件的文件信息
		if ( FileInformationCur + next_file_needed_size - FileInformation > Length ) { break; }

		pName = Copy_NtQueryDirectoryFile_FileInformation( FileInformationCur, (FILE_INFORMATION_CLASS)FileInformationClass, FileInformation_new );

		// 1.2.2.(2) 向参数三FileInformation这个Buffer中填充查询到的文件短名
		FileNameLength = FileInformation_new->FileBothDirInfo.FileNameLength ;// 已查询到的下个文件的文件短名
		CurrentTotal_FileInformation_Size = FileInformationCur
			+ FileInformation_new->FileBothDirInfo.FileNameLength
			- FileInformation
			+ StructSize
			- 2 ;

		// 检查"当前已经填充过的FileInformation的总大小"是否已超出参数四@Length规定长度上限.若超出,则只拷贝一部分的文件短名.确保不会出现堆栈溢出的情况!
		if ( CurrentTotal_FileInformation_Size > Length )
		{
			FileNameLength = Length + FileNameLength - CurrentTotal_FileInformation_Size ;
		}

		if ( pName )
			memcpy( pName, FileInformation_new->FileName, FileNameLength );

		if ( FileNameLength < FileInformation_new->FileBothDirInfo.FileNameLength )
		{
			// 处理对于文件短名没有拷贝完全的情况.
			pNode->OrignalDataInfo.bFlag_dont_need_to_get_nextFileInfo	  = bFlag_Dont_need_to_get_OrignalDirectory_nextFileInfo ;
			pNode->RedirectedDataInfo.bFlag_dont_need_to_get_nextFileInfo = bFlag_Dont_need_to_get_RedirectedDirectory_nextFileInfo ;
			
			*(int *)FileInformationDummy = 0 ;
			status = STATUS_BUFFER_OVERFLOW ;
			goto _END_ ;
		}

		// 再次进行Buffer越界检查
		FileInformationDummy = FileInformationCur ;
		FileInformationNext = *(int *)FileInformationCur + FileInformationCur ;
		FileInformationCur += *(int *)FileInformationCur ;
		
		if ( ReturnSingleEntry || StructSize + FileInformationNext > Length + FileInformation )
		{
			*(int *)FileInformationDummy = 0 ;
			goto _END_ ;
		}

_while_next_ :
		// Redirected目录查询 往后面移动一个
		status = Handler_NtQueryDirectoryFile_FileInformation( &pNode->RedirectedDataInfo, SearchName );
		if ( ! NT_SUCCESS(status) ) { goto _END_ ; }

	} // end-of-while

	pNode->OrignalDataInfo.bFlag_dont_need_to_get_nextFileInfo    = bFlag_Dont_need_to_get_OrignalDirectory_nextFileInfo ;
	pNode->RedirectedDataInfo.bFlag_dont_need_to_get_nextFileInfo = bFlag_Dont_need_to_get_RedirectedDirectory_nextFileInfo ;
	
	*(int *)FileInformationDummy = 0;
	if ( FileInformationCur == FileInformation ) { return STATUS_BUFFER_OVERFLOW ; }

_END_:
	IoStatusBlock->Information = FileInformationCur - FileInformation ;
	IoStatusBlock->Status = status;
	return status;
}


NTSTATUS 
Handler_NtQueryDirectoryFile_FileInformation (
	IN LPDIR_HANDLE_INFO_LITTLE DataInfo ,
	IN PUNICODE_STRING SearchName
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG FileInformationLength = 0 ;
	USHORT FileNameLength = 0 ;
	IO_STATUS_BLOCK IostatusBlock ;
	FILE_INFORMATION_CLASS FileInformationClass ;
	LPFILE_COM_DIR_INFORMATION FileInformation = NULL ;
	LPDIR_HANDLE_INFO_LITTLEEX NodeNew = NULL, NodeCur = NULL ;


	if ( DataInfo->bFlag_dont_need_to_get_nextFileInfo || FALSE == DataInfo->bFlag2 )
	{
		return STATUS_SUCCESS ;
	}

	while ( TRUE )
	{
		// 1. 若FileInformation不存在,则分配内存
		if ( NULL == DataInfo->FileInfo )
		{
			DataInfo->FileInfoLength = FileInformationLength = DataInfo->FileInfoLength + 0x100 ;
			DataInfo->FileInfo = (LPFILE_COM_DIR_INFORMATION) kmalloc( FileInformationLength );
		}

		if ( FALSE == DataInfo->FuckedFlag )
		{
			FileInformationClass = FileIdBothDirectoryInformation ;
			if ( DataInfo->bFlag_Is_FileBothDirectoryInformation ) { FileInformationClass = FileBothDirectoryInformation ; }

			status = g_NtQueryDirectoryFile_addr (
				DataInfo->hFile,
				0,
				0,
				0,
				&IostatusBlock,
				DataInfo->FileInfo,
				DataInfo->FileInfoLength,
				FileInformationClass,
				TRUE, // TRUE == ReturnSingleEntry.表明不是查询所有的文件信息,而是仅查询一个文件
				SearchName,
				DataInfo->bFlag_RestartScan
				);

			if ( NT_SUCCESS(status) && DataInfo->bFlag_Is_FileBothDirectoryInformation )
			{
				FileInformation = DataInfo->FileInfo ;
				memmove (
					FileInformation->FileName,
					FileInformation->FileBothDirInfo.FileName,
					FileInformation->FileBothDirInfo.FileNameLength
					);

				FileInformation->Reserved1 = 0 ;
				FileInformation->Reserved2 = 0 ;

			}

			if ( STATUS_BUFFER_OVERFLOW == status ) { goto _while_next_ ; }
			break ;
		}

		NodeCur = (LPDIR_HANDLE_INFO_LITTLEEX) DataInfo->ListEntry.Flink ;
		if ( NULL == NodeCur ) 
		{
			status = STATUS_NO_MORE_FILES ;
			break ;
		}

		if ( DataInfo->FileInfoLength >= NodeCur->StructLength )
		{
			memcpy( DataInfo->FileInfo, &NodeCur->FileDirInfo, NodeCur->StructLength );
			RemoveEntryListEx( &DataInfo->ListEntry, &NodeCur->ListEntry );
			
			status = STATUS_SUCCESS ;
			break ;
		}

_while_next_:
		kfree( DataInfo->FileInfo );
		DataInfo->FileInfo = NULL ;

	} // end-of-while

	DataInfo->bFlag_RestartScan = FALSE ;
	
	if ( ! NT_SUCCESS(status) )
	{
		DataInfo->bFlag2 = FALSE ;
		if ( STATUS_NO_MORE_FILES == status || STATUS_NO_SUCH_FILE == status ) { status = STATUS_SUCCESS ; }		
	}
	else
	{
		if ( DataInfo->FileNameLength < DataInfo->FileInfo->FileBothDirInfo.FileNameLength + 2 )
		{
			if ( DataInfo->wszFileName ) { kfree( DataInfo->wszFileName ); }

			DataInfo->FileNameLength = DataInfo->FileInfo->FileBothDirInfo.FileNameLength + 2 ;
			DataInfo->wszFileName = (LPWSTR) kmalloc( DataInfo->FileNameLength );
		}

		if ( DataInfo->wszFileName )
		{
			memcpy( DataInfo->wszFileName, DataInfo->FileInfo->FileName, DataInfo->FileInfo->FileBothDirInfo.FileNameLength );

			DataInfo->wszFileName[DataInfo->FileInfo->FileBothDirInfo.FileNameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

			FileNameLength = LOWORD( DataInfo->FileInfo->FileBothDirInfo.FileNameLength );

			DataInfo->uniFileName.Length		= FileNameLength ;
			DataInfo->uniFileName.MaximumLength = FileNameLength + 2 ;

			DataInfo->uniFileName.Buffer = DataInfo->wszFileName ;
			DataInfo->bFlag_dont_need_to_get_nextFileInfo = TRUE ;
		}
	}

	return status;
}


LPWSTR 
Copy_NtQueryDirectoryFile_FileInformation (
	IN OUT int FileInformationOld,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN LPFILE_COM_DIR_INFORMATION FileInformationNew
	)
{
	LPWSTR pName = NULL, ShortName = NULL ;

	switch ( FileInformationClass )
	{
		case FileDirectoryInformation :
		{
			PFILE_DIRECTORY_INFORMATION FileDirInfo = (PFILE_DIRECTORY_INFORMATION) FileInformationOld ;

			FileDirInfo->NextEntryOffset = FileInformationNew->FileBothDirInfo.FileNameLength + 0x40 ;// 跳过本BLOCK块,将NextEntryOffset指向下个BLOCK
			FileDirInfo->FileIndex = FileInformationNew->FileBothDirInfo.FileIndex ;
			FileDirInfo->CreationTime.LowPart = FileInformationNew->FileBothDirInfo.CreationTime.LowPart ;
			FileDirInfo->CreationTime.HighPart = FileInformationNew->FileBothDirInfo.CreationTime.HighPart ;
			FileDirInfo->LastAccessTime.LowPart = FileInformationNew->FileBothDirInfo.LastAccessTime.LowPart ;
			FileDirInfo->LastAccessTime.HighPart = FileInformationNew->FileBothDirInfo.LastAccessTime.HighPart ;
			FileDirInfo->LastWriteTime.LowPart = FileInformationNew->FileBothDirInfo.LastWriteTime.LowPart ;
			FileDirInfo->LastWriteTime.HighPart = FileInformationNew->FileBothDirInfo.LastWriteTime.HighPart ;
			FileDirInfo->ChangeTime.LowPart = FileInformationNew->FileBothDirInfo.ChangeTime.LowPart ;
			FileDirInfo->ChangeTime.HighPart = FileInformationNew->FileBothDirInfo.ChangeTime.HighPart ;
			FileDirInfo->EndOfFile.LowPart = FileInformationNew->FileBothDirInfo.EndOfFile.LowPart ;
			FileDirInfo->EndOfFile.HighPart = FileInformationNew->FileBothDirInfo.EndOfFile.HighPart ;
			FileDirInfo->AllocationSize.LowPart = FileInformationNew->FileBothDirInfo.AllocationSize.LowPart ;
			FileDirInfo->AllocationSize.HighPart = FileInformationNew->FileBothDirInfo.AllocationSize.HighPart ;
			FileDirInfo->FileAttributes = FileInformationNew->FileBothDirInfo.FileAttributes ;
			FileDirInfo->FileNameLength = FileInformationNew->FileBothDirInfo.FileNameLength ;
			pName = FileDirInfo->FileName ;
			break;
		}

		case FileFullDirectoryInformation :
		{
			PFILE_FULL_DIR_INFORMATION FileFullDirInfo = (PFILE_FULL_DIR_INFORMATION) FileInformationOld ;

			FileFullDirInfo->NextEntryOffset = FileInformationNew->FileBothDirInfo.FileNameLength + 0x44 ;// 跳过本BLOCK块,将NextEntryOffset指向下个BLOCK
			FileFullDirInfo->FileIndex = FileInformationNew->FileBothDirInfo.FileIndex;
			FileFullDirInfo->CreationTime.LowPart = FileInformationNew->FileBothDirInfo.CreationTime.LowPart;
			FileFullDirInfo->CreationTime.HighPart = FileInformationNew->FileBothDirInfo.CreationTime.HighPart;
			FileFullDirInfo->LastAccessTime.LowPart = FileInformationNew->FileBothDirInfo.LastAccessTime.LowPart;
			FileFullDirInfo->LastAccessTime.HighPart = FileInformationNew->FileBothDirInfo.LastAccessTime.HighPart;
			FileFullDirInfo->LastWriteTime.LowPart = FileInformationNew->FileBothDirInfo.LastWriteTime.LowPart;
			FileFullDirInfo->LastWriteTime.HighPart = FileInformationNew->FileBothDirInfo.LastWriteTime.HighPart;
			FileFullDirInfo->ChangeTime.LowPart = FileInformationNew->FileBothDirInfo.ChangeTime.LowPart;
			FileFullDirInfo->ChangeTime.HighPart = FileInformationNew->FileBothDirInfo.ChangeTime.HighPart;
			FileFullDirInfo->EndOfFile.LowPart = FileInformationNew->FileBothDirInfo.EndOfFile.LowPart;
			FileFullDirInfo->EndOfFile.HighPart = FileInformationNew->FileBothDirInfo.EndOfFile.HighPart;
			FileFullDirInfo->AllocationSize.LowPart = FileInformationNew->FileBothDirInfo.AllocationSize.LowPart;
			FileFullDirInfo->AllocationSize.HighPart = FileInformationNew->FileBothDirInfo.AllocationSize.HighPart;
			FileFullDirInfo->FileAttributes = FileInformationNew->FileBothDirInfo.FileAttributes;
			FileFullDirInfo->FileNameLength = FileInformationNew->FileBothDirInfo.FileNameLength;
			FileFullDirInfo->EaSize = FileInformationNew->FileBothDirInfo.EaSize;
			pName = FileFullDirInfo->FileName ;
			break;
		}

		case FileBothDirectoryInformation :
		{
			PFILE_BOTH_DIR_INFORMATION FileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION) FileInformationOld ;

			FileBothDirInfo->NextEntryOffset = FileInformationNew->FileBothDirInfo.FileNameLength + 0x5E ;// 跳过本BLOCK块,将NextEntryOffset指向下个BLOCK
			FileBothDirInfo->FileIndex = FileInformationNew->FileBothDirInfo.FileIndex;
			FileBothDirInfo->CreationTime.LowPart = FileInformationNew->FileBothDirInfo.CreationTime.LowPart;
			FileBothDirInfo->CreationTime.HighPart = FileInformationNew->FileBothDirInfo.CreationTime.HighPart;
			FileBothDirInfo->LastAccessTime.LowPart = FileInformationNew->FileBothDirInfo.LastAccessTime.LowPart;
			FileBothDirInfo->LastAccessTime.HighPart = FileInformationNew->FileBothDirInfo.LastAccessTime.HighPart;
			FileBothDirInfo->LastWriteTime.LowPart = FileInformationNew->FileBothDirInfo.LastWriteTime.LowPart;
			FileBothDirInfo->LastWriteTime.HighPart = FileInformationNew->FileBothDirInfo.LastWriteTime.HighPart;
			FileBothDirInfo->ChangeTime.LowPart = FileInformationNew->FileBothDirInfo.ChangeTime.LowPart;
			FileBothDirInfo->ChangeTime.HighPart = FileInformationNew->FileBothDirInfo.ChangeTime.HighPart;
			FileBothDirInfo->EndOfFile.LowPart = FileInformationNew->FileBothDirInfo.EndOfFile.LowPart;
			FileBothDirInfo->EndOfFile.HighPart = FileInformationNew->FileBothDirInfo.EndOfFile.HighPart;
			FileBothDirInfo->AllocationSize.LowPart = FileInformationNew->FileBothDirInfo.AllocationSize.LowPart;
			FileBothDirInfo->AllocationSize.HighPart = FileInformationNew->FileBothDirInfo.AllocationSize.HighPart;
			FileBothDirInfo->FileAttributes = FileInformationNew->FileBothDirInfo.FileAttributes;
			FileBothDirInfo->FileNameLength = FileInformationNew->FileBothDirInfo.FileNameLength;
			FileBothDirInfo->EaSize = FileInformationNew->FileBothDirInfo.EaSize;
			FileBothDirInfo->ShortNameLength = FileInformationNew->FileBothDirInfo.ShortNameLength;

			memcpy( FileBothDirInfo->ShortName, FileInformationNew->FileBothDirInfo.ShortName, 0x18 );
			
// 			ShortName = FileInformationNew->FileBothDirInfo.ShortName;
// 			*(DWORD *)&FileBothDirInfo->ShortName[0] = *(DWORD *)ShortName;
// 			*(DWORD *)&FileBothDirInfo->ShortName[2] = *((DWORD *)ShortName + 1);
// 			*(DWORD *)&FileBothDirInfo->ShortName[4] = *((DWORD *)ShortName + 2);
// 			*(DWORD *)&FileBothDirInfo->ShortName[6] = *((DWORD *)ShortName + 3);
// 			*(DWORD *)&FileBothDirInfo->ShortName[8] = *((DWORD *)ShortName + 4);
// 			*(DWORD *)&FileBothDirInfo->ShortName[10] = *((DWORD *)ShortName + 5);
			
			pName = FileBothDirInfo->FileName ;
			break;
		}

		case FileNamesInformation :
		{
			PFILE_NAMES_INFORMATION FileNameInfo = (PFILE_NAMES_INFORMATION) FileInformationOld ;

			FileNameInfo->NextEntryOffset = FileInformationNew->FileBothDirInfo.FileNameLength + 0xC ;
			FileNameInfo->FileIndex = FileInformationNew->FileBothDirInfo.FileIndex;
			FileNameInfo->FileNameLength = FileInformationNew->FileBothDirInfo.FileNameLength;
			pName = FileNameInfo->FileName ;
			break;
		}

		case FileIdBothDirectoryInformation :
		{
			PFILE_ID_BOTH_DIR_INFORMATION FileIdBothDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION) FileInformationOld ;

			FileIdBothDirInfo->NextEntryOffset = FileInformationNew->FileBothDirInfo.FileNameLength + 0x68;
			FileIdBothDirInfo->FileIndex = FileInformationNew->FileBothDirInfo.FileIndex;
			FileIdBothDirInfo->CreationTime.LowPart = FileInformationNew->FileBothDirInfo.CreationTime.LowPart;
			FileIdBothDirInfo->CreationTime.HighPart = FileInformationNew->FileBothDirInfo.CreationTime.HighPart;
			FileIdBothDirInfo->LastAccessTime.LowPart = FileInformationNew->FileBothDirInfo.LastAccessTime.LowPart;
			FileIdBothDirInfo->LastAccessTime.HighPart = FileInformationNew->FileBothDirInfo.LastAccessTime.HighPart;
			FileIdBothDirInfo->LastWriteTime.LowPart = FileInformationNew->FileBothDirInfo.LastWriteTime.LowPart;
			FileIdBothDirInfo->LastWriteTime.HighPart = FileInformationNew->FileBothDirInfo.LastWriteTime.HighPart;
			FileIdBothDirInfo->ChangeTime.LowPart = FileInformationNew->FileBothDirInfo.ChangeTime.LowPart;
			FileIdBothDirInfo->ChangeTime.HighPart = FileInformationNew->FileBothDirInfo.ChangeTime.HighPart;
			FileIdBothDirInfo->EndOfFile.LowPart = FileInformationNew->FileBothDirInfo.EndOfFile.LowPart;
			FileIdBothDirInfo->EndOfFile.HighPart = FileInformationNew->FileBothDirInfo.EndOfFile.HighPart;
			FileIdBothDirInfo->AllocationSize.LowPart = FileInformationNew->FileBothDirInfo.AllocationSize.LowPart;
			FileIdBothDirInfo->AllocationSize.HighPart = FileInformationNew->FileBothDirInfo.AllocationSize.HighPart;
			FileIdBothDirInfo->FileAttributes = FileInformationNew->FileBothDirInfo.FileAttributes;
			FileIdBothDirInfo->FileNameLength = FileInformationNew->FileBothDirInfo.FileNameLength;
			FileIdBothDirInfo->EaSize = FileInformationNew->FileBothDirInfo.EaSize;
			FileIdBothDirInfo->ShortNameLength = FileInformationNew->FileBothDirInfo.ShortNameLength;
			
			memcpy( FileIdBothDirInfo->ShortName, FileInformationNew->FileBothDirInfo.ShortName, 0x18 );

// 			*(DWORD *)&FileIdBothDirInfo->ShortName[0] = *(DWORD *)&FileInformationNew->FileBothDirInfo.ShortName[0];
// 			*(DWORD *)&FileIdBothDirInfo->ShortName[2] = *(DWORD *)&FileInformationNew->FileBothDirInfo.ShortName[2];
// 			*(DWORD *)&FileIdBothDirInfo->ShortName[4] = *(DWORD *)&FileInformationNew->FileBothDirInfo.ShortName[4];
// 			*(DWORD *)&FileIdBothDirInfo->ShortName[6] = *(DWORD *)&FileInformationNew->FileBothDirInfo.ShortName[6];
// 			*(DWORD *)&FileIdBothDirInfo->ShortName[8] = *(DWORD *)&FileInformationNew->FileBothDirInfo.ShortName[8];
// 			*(DWORD *)&FileIdBothDirInfo->ShortName[10] = *(DWORD *)&FileInformationNew->FileBothDirInfo.ShortName[10];
			
			FileIdBothDirInfo->FileId.LowPart = FileInformationNew->Reserved1 ;
			FileIdBothDirInfo->FileId.HighPart = FileInformationNew->Reserved2 ;
			pName = FileIdBothDirInfo->FileName ;
			break;
		}

		case FileIdFullDirectoryInformation :
		{
			PFILE_ID_FULL_DIR_INFORMATION FileIdFullDirInfo = (PFILE_ID_FULL_DIR_INFORMATION) FileInformationOld;

			FileIdFullDirInfo->NextEntryOffset = FileInformationNew->FileBothDirInfo.FileNameLength + 0x50;
			FileIdFullDirInfo->FileIndex = FileInformationNew->FileBothDirInfo.FileIndex;
			FileIdFullDirInfo->CreationTime.LowPart = FileInformationNew->FileBothDirInfo.CreationTime.LowPart;
			FileIdFullDirInfo->CreationTime.HighPart = FileInformationNew->FileBothDirInfo.CreationTime.HighPart;
			FileIdFullDirInfo->LastAccessTime.LowPart = FileInformationNew->FileBothDirInfo.LastAccessTime.LowPart;
			FileIdFullDirInfo->LastAccessTime.HighPart = FileInformationNew->FileBothDirInfo.LastAccessTime.HighPart;
			FileIdFullDirInfo->LastWriteTime.LowPart = FileInformationNew->FileBothDirInfo.LastWriteTime.LowPart;
			FileIdFullDirInfo->LastWriteTime.HighPart = FileInformationNew->FileBothDirInfo.LastWriteTime.HighPart;
			FileIdFullDirInfo->ChangeTime.LowPart = FileInformationNew->FileBothDirInfo.ChangeTime.LowPart;
			FileIdFullDirInfo->ChangeTime.HighPart = FileInformationNew->FileBothDirInfo.ChangeTime.HighPart;
			FileIdFullDirInfo->EndOfFile.LowPart = FileInformationNew->FileBothDirInfo.EndOfFile.LowPart;
			FileIdFullDirInfo->EndOfFile.HighPart = FileInformationNew->FileBothDirInfo.EndOfFile.HighPart;
			FileIdFullDirInfo->AllocationSize.LowPart = FileInformationNew->FileBothDirInfo.AllocationSize.LowPart;
			FileIdFullDirInfo->AllocationSize.HighPart = FileInformationNew->FileBothDirInfo.AllocationSize.HighPart;
			FileIdFullDirInfo->FileAttributes = FileInformationNew->FileBothDirInfo.FileAttributes;
			FileIdFullDirInfo->FileNameLength = FileInformationNew->FileBothDirInfo.FileNameLength;
			FileIdFullDirInfo->EaSize = FileInformationNew->FileBothDirInfo.EaSize;

			*(DWORD *)(FileInformationOld + 0x48) = FileInformationNew->Reserved1 ;
			*(DWORD *)(FileInformationOld + 0x4C) = FileInformationNew->Reserved2 ;
			pName = (LPWSTR)(FileInformationOld + 0x50);
			break;
		}

		default:
			pName = NULL ;
			break;
	}
	
	return pName ;
}


NTSTATUS 
NtSetInformationFile_FilePipeInformation_Filter (
	IN UCHAR Index,
	IN ULONG Length,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN PVOID FileInformation,
	IN int FileInformationClass
	)
{
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPRPC_IN_FilePipeInformation pInBuffer = NULL ;
	LPRPC_OUT_FilePipeInformation pOutBuffer = NULL ;

	hFile = (HANDLE)g_FileHandle_Arrays[ Index - 1 ];
	if ( NULL == hFile ) { return STATUS_INVALID_HANDLE; }

	if ( FileCompletionInformation == FileInformationClass ) { return STATUS_SUCCESS; }

	pInBuffer = (LPRPC_IN_FilePipeInformation) kmalloc( Length + 0x14 );
	if ( NULL == pInBuffer ) { return STATUS_INSUFFICIENT_RESOURCES; }

	pInBuffer->RpcHeader.DataLength = Length + 0x14 ;
	pInBuffer->hFile = hFile ;
	pInBuffer->FileInfoLength = Length ;
	pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_NtSetInformationFile_ ;

	memcpy( pInBuffer->FileInfo, FileInformation, Length );

	pOutBuffer = (LPRPC_OUT_FilePipeInformation) PB_CallServer( pInBuffer );
	kfree( pInBuffer );

	if ( NULL == pOutBuffer ) { return STATUS_INSUFFICIENT_RESOURCES; }

	status = pOutBuffer->RpcHeader.u.Status ;
	if ( pOutBuffer->RpcHeader.ReturnLength > 8 )
	{
		IoStatusBlock->Status = (NTSTATUS)pOutBuffer->Pointer ;
		IoStatusBlock->Information = pOutBuffer->Information ;
	}

	PB_FreeReply( pOutBuffer );
	return status ;
}


NTSTATUS 
NtSetInformationFile_FileRenameInformation_Filter (
	IN HANDLE FileHandle,
	IN PFILE_RENAME_INFORMATION FileRenameInfo
	)
{
	WCHAR OldData ;
	ULONG Length = 0 ;
	OBJECT_ATTRIBUTES ObjAtr = {} ;
	UNICODE_STRING uniObjectName = {} ;
	IO_STATUS_BLOCK IoStatusBlock = {} ;
	FILE_NETWORK_OPEN_INFORMATION FullAttributes = {} ;
	NTSTATUS status = STATUS_SUCCESS, statusDummy = STATUS_SUCCESS ; 
	BOOL bIsHandlerSelfFilePath = FALSE, bDoWork = FALSE ;
	PFILE_RENAME_INFORMATION FileRenameInfoNew = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPWSTR OrignalPathDummy = NULL, RedirectedPathDummy = NULL ;
	LPWSTR RenamedOrignalPath = NULL, RenamedRedirectedPath = NULL ;
	HANDLE hRedirectedFile = NULL, hRootDirectory = NULL, hRedirectedFile_Orignal = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, g_SecurityDescriptor );
	RtlInitUnicodeString( &uniObjectName, NULL );

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetFilePath ( &uniObjectName, FileHandle, &OrignalPath, &RedirectedPath, &bIsHandlerSelfFilePath );
	if ( ! NT_SUCCESS(status) ) { return status; }

	//  
	// 这里以 写 & 删除权限打开沙箱外的原始文件 "\Device\HarddiskVolume1\555.txt",企图
	// 获取其句柄,结果被fake_NtCreateFile重定向后去掉了这些权限,并且返回的句柄属于:
	// "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\555.txt"
	//

	RtlInitUnicodeString( &uniObjectName, OrignalPath );

	++ pNode->sFileLock.nLockNtDeleteFile;

	status = ZwCreateFile (
		&hRedirectedFile,
		FILE_GENERIC_WRITE | DELETE, // /*0x130116*/,
		&ObjAtr,
		&IoStatusBlock,
		0,
		0,
		7,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		0,
		0
		);

	-- pNode->sFileLock.nLockNtDeleteFile;

	if ( STATUS_SHARING_VIOLATION == status )
	{
		status = ZwCreateFile( &hRedirectedFile, 0x110000, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x20, 0, 0 );
		if ( STATUS_SHARING_VIOLATION == status )
		{
			hRedirectedFile = FileHandle ;
			status = STATUS_SUCCESS ;
		}
	}

	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	RtlInitUnicodeString( &uniObjectName, NULL );

	status = GetFilePath (
		&uniObjectName,
		hRedirectedFile,	// 原始文件被重定向后的句柄
		&OrignalPath,       // "\Device\HarddiskVolume1\555.txt"
		&RedirectedPath,    // "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\555.txt"
		&bIsHandlerSelfFilePath
		);

	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	if ( bIsHandlerSelfFilePath )
	{
		ULONG OrignalPathLength = 2 * wcslen( OrignalPath ) + 2 ;
		OrignalPathDummy = (LPWSTR) kmalloc( OrignalPathLength );
		memcpy( OrignalPathDummy, OrignalPath, OrignalPathLength );

		ULONG RedirectedPathLength = 2 * wcslen( RedirectedPath ) + 2 ;
		RedirectedPathDummy = (LPWSTR) kmalloc( RedirectedPathLength );
		memcpy( RedirectedPathDummy, RedirectedPath, RedirectedPathLength );
	}

	uniObjectName.Length = LOWORD( FileRenameInfo->FileNameLength );
	uniObjectName.MaximumLength = uniObjectName.Length ;
	uniObjectName.Buffer = FileRenameInfo->FileName ;

	status = GetFilePath (
		&uniObjectName,
		FileRenameInfo->RootDir, // 新文件的句柄
		&OrignalPath,            // 新文件原始路径: "\Device\HarddiskVolume1\imadus.txt"
		&RedirectedPath,         // 新文件被重定向后的路径: "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\imadus.txt"
		&bIsHandlerSelfFilePath
		);

	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	ULONG RenamedOrignalPathLength = 2 * wcslen( OrignalPath ) + 2 ;
	RenamedOrignalPath = (LPWSTR) kmalloc( RenamedOrignalPathLength );
	memcpy( RenamedOrignalPath, OrignalPath, RenamedOrignalPathLength );

	ULONG RenamedRedirectedPathLength = 2 * wcslen( RedirectedPath ) + 2 ;
	RenamedRedirectedPath = (LPWSTR) kmalloc( RenamedRedirectedPathLength );
	memcpy( RenamedRedirectedPath, RedirectedPath, RenamedRedirectedPathLength );

	LPWSTR RenamedOrignalPathShort = wcsrchr( RenamedOrignalPath, '\\' );

	if ( wcschr( RenamedOrignalPathShort, ':' ) )
	{
		status = STATUS_INVALID_PARAMETER ;
		goto _over_ ;
	}

	++ RenamedOrignalPathShort ;
	*(RenamedOrignalPathShort - 1) = 0 ;

	status = PB_RenameFile (
		hRedirectedFile,		 // 本来要重命名后的文件全路径是"C:\imadus.txt"
		RenamedOrignalPath,      // "\Device\HarddiskVolume1"
		RenamedOrignalPathShort, // "imadus.txt"
		FileRenameInfo->Replace
		);
	
	*(RenamedOrignalPathShort - 1) = '\\' ;

	if ( STATUS_BAD_INITIAL_PC != status )
	{
		if ( status < 0 ) { goto _over_ ; }
		goto _next_ ;
	}

	//
	// 打开 "重命名后的原始文件"的父目录
	//

	OldData = *RenamedOrignalPathShort ;
	*RenamedOrignalPathShort = 0 ; // 截断前为: "\Device\HarddiskVolume1\imadus.txt"

	RtlInitUnicodeString( &uniObjectName, RenamedOrignalPath ); // 截断后为: "\Device\HarddiskVolume1\"

	++ pNode->sFileLock.nLockNtDeleteFile; ;

	status = ZwCreateFile( &hRootDirectory, FILE_GENERIC_WRITE, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN, 0x21, 0, 0 );
	if ( STATUS_ACCESS_DENIED == status )
	{
		status = ZwCreateFile( &hRootDirectory, FILE_GENERIC_READ, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN, 0x21, 0, 0 );
	}

	-- pNode->sFileLock.nLockNtDeleteFile; ;
	*RenamedOrignalPathShort = OldData ;

	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }
	
	Length = 2 * wcslen(RenamedOrignalPathShort) + 0x10 ;
	FileRenameInfoNew = (PFILE_RENAME_INFORMATION) kmalloc( Length );

	FileRenameInfoNew->Replace = FileRenameInfo->Replace ;
	FileRenameInfoNew->RootDir = hRootDirectory ; // eg: \Sandbox\AV\DefaultBox\drive\C {HarddiskVolume1}
	FileRenameInfoNew->FileNameLength = 2 * wcslen( RenamedOrignalPathShort );

	memcpy( FileRenameInfoNew->FileName, RenamedOrignalPathShort, FileRenameInfoNew->FileNameLength ); // "imadus.txt"

	RtlInitUnicodeString( &uniObjectName, RenamedRedirectedPath );  // "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\imadus.txt"
	
	if ( FileRenameInfoNew->Replace )
	{
_delete_file_:
		g_NtDeleteFile_addr( &ObjAtr );

_Rename_file_:
		status = g_NtSetInformationFile_addr (
			hRedirectedFile,       // \Sandbox\AV\DefaultBox\drive\C\555.txt {HarddiskVolume1}
			&IoStatusBlock,
			FileRenameInfoNew,     // FileRenameInfoNew->RootDir --> \Sandbox\AV\DefaultBox\drive\C {HarddiskVolume1}
			Length,
			FileRenameInformation
			);

		if ( STATUS_SHARING_VIOLATION == status && hRedirectedFile != FileHandle )
		{
			ZwClose( hRedirectedFile );
			hRedirectedFile = FileHandle ;
			status = g_NtSetInformationFile_addr( FileHandle, &IoStatusBlock, FileRenameInfoNew, Length, FileRenameInformation );
		}

		if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

		ZwClose( hRootDirectory );
		hRootDirectory = NULL ;

		RtlInitUnicodeString( &uniObjectName, RenamedRedirectedPath );

		status = g_NtCreateFile_addr( &hRootDirectory, FILE_GENERIC_READ, &ObjAtr, &IoStatusBlock, 0, 0, 7, 1, 0x20, 0, 0 );

		if ( status < 0 )
		{
			status = STATUS_SUCCESS ;
		}
		else
		{
			RemoveDirtyTag( hRootDirectory, RenamedRedirectedPath ); 
			ZwClose( hRootDirectory );
			hRootDirectory = NULL ;
		}

		BOOL bInsert = IsWinsxsFile( RenamedOrignalPath );
		InsertFileHandle( FileHandle, RenamedOrignalPath, bInsert ); // FileHandle --> \555.txt {HarddiskVolume1}

_next_:
		if ( NULL == OrignalPathDummy || NULL == RedirectedPathDummy ) { goto _over_ ; }

		RtlInitUnicodeString( &uniObjectName, OrignalPathDummy );  // "\Device\HarddiskVolume1\555.txt"
		statusDummy = g_NtQueryFullAttributesFile_addr( &ObjAtr, &FullAttributes );
		if ( ! NT_SUCCESS(statusDummy) ) { goto _over_ ; }

		RtlInitUnicodeString( &uniObjectName, RedirectedPathDummy ); // "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\555.txt"
		statusDummy = g_NtCreateFile_addr( &hRedirectedFile_Orignal,FILE_GENERIC_READ,&ObjAtr,&IoStatusBlock,0,0,7, FILE_CREATE, 0x60,0,0 );
		if ( ! NT_SUCCESS(statusDummy) ) { goto _over_ ; }

		MarkAsDirtyFile( hRedirectedFile_Orignal, RedirectedPathDummy );
		ZwClose( hRedirectedFile_Orignal );
	}
	else
	{
		bDoWork = FALSE ;
		statusDummy = g_NtQueryFullAttributesFile_addr( &ObjAtr, &FullAttributes );

		if ( statusDummy < 0 )
		{
			// 被重定向到沙箱内的新文件暂时还不存在,故 status == 0xc0000034
			RtlInitUnicodeString( &uniObjectName, RenamedOrignalPath );  // "\Device\HarddiskVolume1\imadus.txt"
			statusDummy= g_NtQueryFullAttributesFile_addr( &ObjAtr, &FullAttributes );
			
			if ( statusDummy < 0 )        
			{
				// 正常情况下,重命名时候,新的原始文件应该同样不存在, 依然有status == 0xc0000034
				RtlInitUnicodeString( &uniObjectName, RenamedRedirectedPath ); // "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\imadus.txt"
				bDoWork = TRUE ;
			}
		}
		else
		{
			if ( IsDirtyKeyEx( (PVOID)&FullAttributes.CreationTime ) )
			{
				FileRenameInfoNew->Replace = TRUE ;
				bDoWork = TRUE ;
			}
		}

		if ( bDoWork )
		{
			if ( FALSE == FileRenameInfoNew->Replace ) { goto _Rename_file_ ; }
			goto _delete_file_;
		}

		status = STATUS_OBJECT_NAME_COLLISION ;		
	}

_over_:
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	if ( hRedirectedFile && hRedirectedFile != FileHandle ) { ZwClose( hRedirectedFile ); }
	if ( hRootDirectory ) { ZwClose( hRootDirectory ); }
	if ( OrignalPathDummy ) { kfree( OrignalPathDummy ); }
	if ( RedirectedPathDummy ) { kfree( RedirectedPathDummy ); }
	if ( RenamedOrignalPath ) { kfree( RenamedOrignalPath ); }
	if ( RenamedRedirectedPath ) { kfree( RenamedRedirectedPath ); }
	if ( FileRenameInfoNew ) { kfree( FileRenameInfoNew ); }
	
	return status;
}


NTSTATUS 
CloseLittleHandle (
	IN UCHAR Index
	)
{
	HANDLE hFile = NULL, *ptr = NULL ;
	RPC_IN_HANDLE pInBuffer ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPRPC_OUT_HEADER pOutBuffer = NULL ;

	ptr = (HANDLE*) &g_FileHandle_Arrays[ Index - 1 ];
	hFile = *ptr ;
	if ( NULL == hFile ) { return STATUS_INVALID_HANDLE; }

	pInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_HANDLE ) ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_CloseLittleHandle_ ;
	pInBuffer.hFile = hFile ;

	pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return STATUS_INVALID_HANDLE; }

	status = pOutBuffer->u.Status ;
	if ( status >= 0 ) { *ptr = NULL ; }

	PB_FreeReply( pOutBuffer );
	return status ;
}


void 
NtCloseFilter (
	IN HANDLE hFile,
	OUT PVOID *RpcBuffer
	)
{
	int n = 0 ;
	ULONG pNodeHead = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	HANDLE CurrentHandle = NULL ;
	LPWSTR OrignalPath, RedirectedPath ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	LPRPC_IN_NtCloseFilter pInBuffer = NULL ;
	LPHANDLE_INFO pNode = NULL, HandleInfo = NULL ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;
	ULONG RedirectedPathLength = 0, OrignalPathLength = 0 ;
	BOOL bFound = TRUE, bIsHandlerSelfFilePath = FALSE, bIsRecoverFolder = FALSE, bIsWinsxsFile = FALSE, bInsertNode = FALSE ;
	LPPROCESS_GLOBALMEMORY_NODE pBigNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	// 1. 查找@hFile是否在数组中
	if ( NULL == hFile || NULL == g_FileHandle_Arrays ) { return ; }

	EnterCriticalSection( &g_HandlesLock );

	if ( IsArrayEnd( g_Redirectd_FileHandle_Array ) )
	{ 
		LeaveCriticalSection( &g_HandlesLock );
		return ;
	}

	pNodeHead = g_Redirectd_FileHandle_Array ;
	CurrentHandle = *(HANDLE *) pNodeHead ;

	while ( CurrentHandle != hFile )
	{
		++ n ;
		pNodeHead += 4 ;
		CurrentHandle = *(HANDLE *) pNodeHead ;
		
		if ( (HANDLE)0xFFFFFFFF == CurrentHandle ) 
		{ 
			// 已遍历至数组末尾,仍未找到对应的句柄
			bFound = FALSE ; 
			break ;
		}
	}

	// 在数组中找到对应的FileHandle. 清除当前句柄
	if ( bFound ) { *(DWORD *)(g_Redirectd_FileHandle_Array + 4 * n) = 0 ; }
	LeaveCriticalSection( &g_HandlesLock );

	// 只关注在数组中的handle
	if ( FALSE == bFound ) { return ; }
	
	// 2. 查询句柄对应的文件原始 & 重定向路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pBigNode);

	RtlInitUnicodeString( &uniObjectName, NULL );
	status = GetFilePath ( &uniObjectName, hFile, &OrignalPath, &RedirectedPath, &bIsHandlerSelfFilePath );
	if ( ! NT_SUCCESS(status) ) { return; }

	if ( FALSE == bIsHandlerSelfFilePath ) { goto _over_ ; }

	bIsRecoverFolder = IsRecoverFolder( OrignalPath );
	bIsWinsxsFile	 = IsWinsxsFile( OrignalPath );

	// 3. 只关注 "可恢复目录" 或者 "Winsxs下的目录"
	if ( FALSE == bIsRecoverFolder && FALSE == bIsWinsxsFile ) { goto _over_ ; }

	// 获取文件大小信息
	status = g_NtQueryInformationFile_addr( hFile, &IoStatusBlock, &FileInfo, sizeof(FILE_NETWORK_OPEN_INFORMATION), FileNetworkOpenInformation );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; } 

	if ( 0 == FileInfo.EndOfFile.HighPart && 0 == FileInfo.EndOfFile.LowPart ) { goto _over_ ; }

	// 3.1 处理 Winsxs下的目录,需要通过RPC解决
	if ( bIsWinsxsFile )
	{
		RedirectedPathLength = 2 * wcslen( RedirectedPath );
		pInBuffer = (LPRPC_IN_NtCloseFilter) kmalloc( RedirectedPathLength + 0x12 );

		pInBuffer->RpcHeader.DataLength = RedirectedPathLength + 0x12 ;
		pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_NtCloseWinsxs_ ;
		pInBuffer->RedirectedPathLength = RedirectedPathLength ;

		wcscpy( pInBuffer->RedirectedPath, RedirectedPath );
		*RpcBuffer = (PVOID) pInBuffer ;
	}

	// 3.2 处理 可恢复目录; 将当前信息更新至链表中
	if ( bIsHandlerSelfFilePath )
	{
		EnterCriticalSection( &g_HandlesLock );
		
		bInsertNode = FALSE ;
		OrignalPathLength = wcslen( OrignalPath );

		pNode = (LPHANDLE_INFO) g_pNodeHead_Handles.Flink ;
		if ( NULL == pNode ) { bInsertNode = TRUE ; }

		if ( pNode )
		{
			while ( pNode->OrignalPathLength != OrignalPathLength || wcsicmp( pNode->OrignalPath, OrignalPath ) )
			{
				pNode = (LPHANDLE_INFO)pNode->ListEntry.Flink ;
				if ( NULL == pNode ) { bInsertNode = TRUE ; break; }
			}
		}

		if ( bInsertNode )
		{
			HandleInfo = (LPHANDLE_INFO) kmalloc( 2 * OrignalPathLength + 0x16 );
			
			HandleInfo->TickCount = GetTickCount() ;
			HandleInfo->OrignalPathLength = OrignalPathLength ;
			
			wcscpy( HandleInfo->OrignalPath, OrignalPath );
			
			InsertListA( &g_pNodeHead_Handles, NULL, &HandleInfo->ListEntry );
		}

		LeaveCriticalSection(&g_HandlesLock);
	}

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pBigNode);
	return;
}


NTSTATUS
NtFsControlFileFilter (
	IN HANDLE FileHandle,
	IN PREPARSE_DATA_BUFFER InputBuffer
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath, RedirectedPath ;
	UNICODE_STRING uniObjectName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	USHORT SubstituteNameOffset = 0, SubstituteNameLength = 0 ;
	ULONG RedirectedReparsePathOffset = 0, DataLength = 0 ;
	LPWSTR szRedirectedPath = NULL, szRedirectedReparsePath = NULL ;
	LPRPC_IN_NtFsControlFileFilter pInBuffer = NULL ;
	LPRPC_OUT_HEADER pOutBuffer = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( NULL == InputBuffer || IO_REPARSE_TAG_MOUNT_POINT != InputBuffer->ReparseTag ) { return STATUS_BAD_INITIAL_PC; }

	// 1. 只处理 IO_REPARSE_TAG_MOUNT_POINT 的请求
	SubstituteNameOffset = InputBuffer->MountPointReparseBuffer.SubstituteNameOffset ;
	SubstituteNameLength = InputBuffer->MountPointReparseBuffer.SubstituteNameLength ;

	// 2. 查询句柄对应的文件原始&重定向路径
	RtlInitUnicodeString( &uniObjectName, NULL );
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetFilePath ( &uniObjectName, FileHandle, &OrignalPath, &RedirectedPath, NULL );
	if ( ! NT_SUCCESS(status) ) { goto _over_ ; }

	// 3. 过黑白名单
	WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );

	if ( bIsWhite ) { status = STATUS_BAD_INITIAL_PC; goto _over_ ; }
	if ( bIsBlack ) { status = STATUS_ACCESS_DENIED ; goto _over_ ; }

	// 4. 处理灰名单
	szRedirectedPath = (LPWSTR) kmalloc( 2 * wcslen(RedirectedPath) + 8 );
	wcscpy( szRedirectedPath, RedirectedPath );

	// 4.1 获取要reparse的文件原始&重定向路径
	uniObjectName.MaximumLength = uniObjectName.Length = SubstituteNameLength ;
	uniObjectName.Buffer = (WCHAR *)( (char *)InputBuffer->MountPointReparseBuffer.PathBuffer + SubstituteNameOffset );

	status = GetFilePath( &uniObjectName, NULL, &OrignalPath, &RedirectedPath, NULL );
	if ( ! NT_SUCCESS(status) ) 
	{ 
		kfree( szRedirectedPath );
		goto _over_ ;
	}

	szRedirectedReparsePath = (LPWSTR) kmalloc( 2 * wcslen(RedirectedPath) + 8 );
	wcscpy( szRedirectedReparsePath, RedirectedPath );

	// 4.2 rpc通信
	RedirectedReparsePathOffset = 2 * wcslen( szRedirectedPath ) + 0x20 ;
	DataLength = RedirectedReparsePathOffset + 2 * wcslen( szRedirectedReparsePath ) + 8 ;
	pInBuffer = (LPRPC_IN_NtFsControlFileFilter) kmalloc( DataLength );

	pInBuffer->RpcHeader.DataLength = DataLength ;
	pInBuffer->RpcHeader.Flag = 0x1903 ;

	wcscpy( pInBuffer->Buffer, szRedirectedPath );
	wcscpy( (wchar_t *)((char *)pInBuffer + RedirectedReparsePathOffset), szRedirectedReparsePath );

	pInBuffer->RedirectedReparsePathOffset = RedirectedReparsePathOffset ;
	pInBuffer->RedirectedPathLength = 2 * wcslen( szRedirectedPath );
	pInBuffer->RedirectedReparsePathLength = 2 * wcslen( szRedirectedReparsePath );

	pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( pInBuffer );
	kfree( pInBuffer );
	if ( pOutBuffer )
	{
		status = pOutBuffer->u.Status ;
		PB_FreeReply( pOutBuffer );
	}
	else
	{
		status = STATUS_ACCESS_DENIED ;
	}

	kfree( szRedirectedPath );
	kfree( szRedirectedReparsePath );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////