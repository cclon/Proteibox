/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/30 [30:12:2010 - 15:41]
* MODULE : e:\Data\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDLL\PBLoadData.cpp
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
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBFilesData.h"
#include "PBDynamicData.h"
#include "PBUser32dll/PBUser32dll.h"
#include "PBComData.h"
#include "PBLoadData.h"

#pragma warning(disable : 4995) 

//////////////////////////////////////////////////////////////////////////


LPWSTR g_C_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs = NULL ;
LPWSTR g_Device_HarddiskVolume1_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs = NULL ;

LARGE_INTEGER g_lpBuffer_winsxs_LastUpdateTime ;

LIST_ENTRY_EX g_Node_GetModuleFileName ;
CRITICAL_SECTION g_lock_GetModuleFileName ;

DWORD g_OldProtect = 0 ;
BYTE g_AddressOfEntryPoint_OrigData[ 5 ] ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID 
Walk_c_windows_winsxs_Total (
	IN LPWSTR lpFileName,
	IN LPWSTR lpSearchPath,
	IN int bFlag
	)
{
	ULONG Length = 0, FilePosition = 0, WriteSize = 0 ;
	BOOL bOpenFile = FALSE, bIsHandlerSelfFilePath = FALSE ;
	HANDLE hFile = NULL ;
	LPWSTR lpNtName = NULL, lpBuffer = NULL, Path = NULL, ptr = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;

	lpNtName = Handler_winsxs_lastupdatetime( FALSE );
	if ( NULL == lpNtName ) { return; }

	lpBuffer = (LPWSTR) kmalloc( 0x400 );
	if ( 0 == bFlag )
	{
		if ( wcslen(lpFileName) >= 0x1C2 ) { goto _END_ ; }

		wcscpy( lpBuffer + 4, lpFileName );
		bOpenFile = TRUE ;
	}
	else
	{
		Length = SearchPathW( lpSearchPath, lpFileName, 0, 0x1F4, lpBuffer + 4, &lpFileName );
		if ( Length && Length < 0x1C2 ) { bOpenFile = TRUE ; }
	}

	if ( FALSE == bOpenFile ) { goto _END_ ; }

	// 打开该路径对应的文件,得到句柄
	wcscpy( lpFileName, L"\\??\\" );
	RtlInitUnicodeString( &uniObjectName, lpBuffer );
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile( &hFile, FILE_GENERIC_READ, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN, 0x60, 0, 0 );
	if ( ! NT_SUCCESS(status) ) { goto _END_ ; }

	// 查询也许是重定位后的句柄对应的文件全路径
	Path = (LPWSTR) kmalloc( 0x4000 );
	status = PB_GetHandlePath( hFile, Path, &bIsHandlerSelfFilePath );

	if ( NT_SUCCESS(status) && bIsHandlerSelfFilePath && wcslen(Path) <= 0x1C2 ) 
	{ 
		wcscpy( lpBuffer, Path ); 
	}
	else
	{
		*lpBuffer = 0 ;
	}

	kfree( Path );

	if ( *lpBuffer )
	{
		// 查询最终得到的文件的属性信息
		ptr = &lpBuffer[ wcslen(lpBuffer) ];
		wcscpy( ptr, L".manifest" );

		RtlInitUnicodeString( &uniObjectName, lpBuffer );

		if ( ZwQueryFullAttributesFile(&ObjAtr, &FileInfo) < 0
			|| g_lpBuffer_winsxs_LastUpdateTime.HighPart > FileInfo.LastWriteTime.HighPart
			|| g_lpBuffer_winsxs_LastUpdateTime.HighPart == FileInfo.LastWriteTime.HighPart
			&& g_lpBuffer_winsxs_LastUpdateTime.LowPart > FileInfo.LastWriteTime.LowPart
			)
		{
			// 若文件信息不匹配,则重新操作之
			*ptr = 0 ;

			Walk_c_windows_winsxs_Total_dep( hFile, bFlag, lpBuffer, lpSearchPath, lpNtName, &FilePosition, &WriteSize );
		}
	}

	ZwClose( hFile );

	if ( g_QueryActCtxSettingsW_addr && 0 == bFlag && FilePosition && WriteSize )
	{
		Handler_Write_manifest( lpBuffer, (PFILE_POSITION_INFORMATION)&FilePosition, WriteSize );
	}

_END_ :
	kfree( lpBuffer );
	return;
}


LPWSTR 
Handler_GetModuleFileNameW_Total (
	IN int Flag,
	IN PVOID hModule,
	OUT PULONG pNameLength,
	OUT BOOL* bIsLocked
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/12/30 [30:12:2010 - 15:45]

Routine Description:
  主要是操作结构体struct _GetModuleFileName_INFO_ ;处理和GetModuleFileNameW / LdrLoadDll / LdrUnloadDll 相关API 
  新建/删除/查找和hModule对应的结点
    
Arguments:
  Flag - 分三种情况: 0x0:新建结点; 0x1:删除指定结点; 0x2:查找与hModule对应的结点
  hModule - 模块基址
  pNameLength - 保存模块名长度
  bIsLocked - 标记是否调用了EnterCriticalSection();

Return Value:
  lpModulePath,模块全路径

--*/
{
	ULONG length = 0, NameLength = 0 ;
	HANDLE hFile = NULL ;
	HMODULE hModuleDummy = NULL ;
	LPWSTR lpFileName = NULL, lpModulePath_ptr = NULL ;
	LPGetModuleFileName_INFO pCurrentNode = NULL, pNextNode = NULL, pNodeNew = NULL ;

	*bIsLocked = FALSE ;
	hModuleDummy = (HMODULE)hModule ;
	if ( NULL == hModuleDummy ) { hModuleDummy = GetModuleHandleW(NULL); }

	// Case 1: 新建结点
	if ( _GetModuleFileName_Flag_Add_ == Flag )
	{
		// 1.1 调用原始函数得到hModule对应的文件全路径
		lpFileName = (LPWSTR) kmalloc( 0xFFF );
		
		length = g_GetModuleFileNameW_addr( hModuleDummy, lpFileName, 0xFFF );
		if ( length > 0xFFF ) { length = 0xFFF ; }
		lpFileName[ length ] = 0 ;

		if ( 0 == length )
		{
			kfree( lpFileName );
			return NULL ;
		}

		// 1.2 打开文件,获得对应的句柄
		hFile = CreateFileW( lpFileName, GENERIC_READ, 7, 0, OPEN_EXISTING, 0, 0 ); 
		if ( INVALID_HANDLE_VALUE == hFile )
		{
			kfree( lpFileName );
			return NULL ;
		}

		// 1.3 经过2次转换,获得最后的文件全路径(可能经过重定向)
		if ( PB_GetHandlePath(hFile, lpFileName, NULL) >= 0 && PB_TranslateNtToDosPath(lpFileName) ) 
		{
			EnterCriticalSection( &g_lock_GetModuleFileName );
			*bIsLocked = TRUE ;

			// 1.4 将得到的最终文件全路径,新建一个结点,保存至链表中
			pCurrentNode = (LPGetModuleFileName_INFO) g_Node_GetModuleFileName.Flink ;

			// 1.4.1 若有旧的,先删除掉旧的结点
			if ( pCurrentNode )
			{	
				do
				{
					pNextNode = pCurrentNode->pFlink ;
					if ( hModule == pCurrentNode->hModule )
					{
						RemoveEntryListEx( &g_Node_GetModuleFileName, (PLIST_ENTRY)pCurrentNode );
						kfree( pCurrentNode );
					}

					pCurrentNode = pNextNode ;
				}
				while ( pNextNode );
			}

			// 1.4.2 建立新结点
			NameLength = 2 * wcslen(lpFileName) + 2 ;
			pNodeNew = (LPGetModuleFileName_INFO) kmalloc( NameLength + sizeof(GetModuleFileName_INFO) );
			
			pNodeNew->hModule = hModule ;
			pNodeNew->NameLength = (NameLength >> 1) - 1 ;
			memcpy( pNodeNew->lpModulePath, lpFileName, NameLength );

			InsertListB( &g_Node_GetModuleFileName, NULL, (PLIST_ENTRY)pNodeNew );

			lpModulePath_ptr = pNodeNew->lpModulePath ;
			if ( pNameLength ) { *pNameLength = pNodeNew->NameLength ; }
		}

		CloseHandle( hFile );
		kfree( lpFileName );
		return lpModulePath_ptr ;
	}

	EnterCriticalSection( &g_lock_GetModuleFileName );
	*bIsLocked = TRUE;
	
	pCurrentNode = (LPGetModuleFileName_INFO) g_Node_GetModuleFileName.Flink ;
	if ( NULL == pCurrentNode ) { return NULL ; }

	// 查找 @hModule 对应的结点,未找到则返回失败
	while ( pCurrentNode->hModule != hModuleDummy )
	{
		pCurrentNode = pCurrentNode->pFlink ;
		if ( NULL == pCurrentNode ) { return NULL ; }
	}

	// 若找到@hModule对应的结点,有2种选择,如下:
	if ( _GetModuleFileName_Flag_Del_ == Flag )
	{
		// Case 2: 删除指定结点
		RemoveEntryListEx( &g_Node_GetModuleFileName, (PLIST_ENTRY)pCurrentNode );
		kfree( pCurrentNode );
		return NULL ;
	}
	else if ( _GetModuleFileName_Flag_Find_ == Flag )
	{
		// Case 3: 返回hModule对应的结点中保存的文件全路径
		if ( pNameLength ) { *pNameLength = pCurrentNode->NameLength ; }
		return pCurrentNode->lpModulePath ;
	}

	return NULL ;
}


LPWSTR Handler_winsxs_lastupdatetime (IN BOOL bTranlateToDos)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/12/31 [31:12:2010 - 15:38]

Routine Description:
  读取C:\WINDOWS\WinSxS\LastUpdateTime文件的8字节内容
    
Arguments:
  bTranlateToDos - 是否需要将路径转换成DOS格式,即"\Device\HarddiskVolume1"-->"c:"

Return Value:
  成功返回"C:\Sandbox\AV\DefaultBox\drive\C\WINDOWS\winsxs" 或 "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\WINDOWS\winsxs"
  失败返回 NULL

--*/
{
	WCHAR Buffer[ MAX_PATH ] = L"" ;
	HANDLE hFile1 = NULL, hFile2 = NULL ;
	BOOL bIsHandlerSelfFilePath = FALSE ;
	ULONG_PTR length = 0, NumberOfBytesRead = 0 ;
	LPWSTR lpDosName = NULL, lpNtName = NULL, Path = NULL, lpName = NULL ;

	// 1. 获取winsxs对应的dos / nt 全路径
	lpDosName = g_C_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs ;
	if ( NULL == lpDosName )
	{
		RtlZeroMemory( Buffer, MAX_PATH );
		GetSystemWindowsDirectoryW( Buffer, MAX_PATH );

		if ( NULL == Buffer ) { wcscpy( Buffer, L"C:\\WINDOWS" ); }

		if ( '\\' == Buffer[ wcslen(Buffer) ] ) { Buffer[ wcslen(Buffer) ] = 0 ; }

		length = g_BoxInfo.FileRootPathLength + 2 * wcslen(Buffer) + 0x80 ;

		lpNtName  = (LPWSTR) kmalloc( length );
		lpDosName = (LPWSTR) kmalloc( length );

		swprintf( lpNtName, L"%s\\drive\\%c%s\\winsxs", g_BoxInfo.FileRootPath, Buffer[0], Buffer+2 );
		wcscpy( lpDosName, lpNtName );  // "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\WINDOWS\winsxs"

		if ( FALSE == PB_TranslateNtToDosPath(lpDosName) )
		{
			kfree( lpDosName );
			kfree( lpNtName );
			return NULL ;
		}

		g_C_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs = lpDosName ;
		g_Device_HarddiskVolume1_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs = lpNtName ;
	}

	// 2.读文件中的8字节内容
	if ( g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok == '?' )
	{
		hFile1 = CreateFileW( lpDosName, GENERIC_READ, 7, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0 );
		if ( hFile1 != (HANDLE)INVALID_HANDLE_VALUE )
		{
			Path = (LPWSTR) kmalloc( 0x1000 );
			PB_GetHandlePath( hFile1, Path, &bIsHandlerSelfFilePath );
			CloseHandle( hFile1 );

			if ( bIsHandlerSelfFilePath )
			{
				wcscpy( Path, g_C_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs );
				wcscat( Path, L"\\LastUpdateTime" );

				hFile2 = CreateFileW( Path, GENERIC_READ, 7, 0, OPEN_EXISTING, 0, 0 );
				if ( hFile2 != (HANDLE)INVALID_HANDLE_VALUE )
				{
					ReadFile( hFile2, &g_lpBuffer_winsxs_LastUpdateTime, 8, &NumberOfBytesRead, NULL );
					CloseHandle( hFile2 );
				}

				kfree( Path );
			}
		}

		if ( NumberOfBytesRead != 8 )
		{
			g_lpBuffer_winsxs_LastUpdateTime.HighPart = 0 ;
			g_lpBuffer_winsxs_LastUpdateTime.LowPart  = 0 ;
		}

		if ( FALSE == bIsHandlerSelfFilePath )
		{
			g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok = 'n';
			return NULL;
		}

		lpDosName = g_C_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs ;
		g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok = 'y';
	}
	else
	{
		if ( g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok != 'y' ) { return NULL; }
	}

	lpName = lpDosName;
	if ( FALSE == bTranlateToDos ) { lpName = g_Device_HarddiskVolume1_Sandbox_AV_DefaultBox_drive_C_WINDOWS_winsxs ; }

	return lpName ;
}


PVOID 
Handler_Read_manifest (
	IN LPWSTR lpFileName,
	IN ULONG ReadSize
	)
{
	PVOID pBuffer =  NULL ;
	LPWSTR ptr = NULL ;

	ptr = &lpFileName[ wcslen(lpFileName) ];
	wcscpy( ptr, L".manifest" );
	
	pBuffer = MyReadFile( lpFileName );
	*ptr = 0;

	if ( pBuffer && ParseManifest(pBuffer, ReadSize) > ReadSize )
	{
		kfree( pBuffer );
		return NULL;
	}

	return pBuffer;
}


void 
Handler_Write_manifest (
	IN LPWSTR lpFileName,
	IN PFILE_POSITION_INFORMATION FilePosition,
	IN ULONG WriteSize
	)
{
	LPWSTR ptr = NULL ;
	HANDLE hFile = NULL ;
	PVOID pbuffer = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;

	ptr = &lpFileName[ wcslen(lpFileName) ];
	wcscpy( ptr, L".manifest" );

	pbuffer = MyReadFile( lpFileName );
	*ptr = 0 ;

	if ( NULL == pbuffer ) { return ; }
	
	if ( ParseManifest(pbuffer, WriteSize) <= WriteSize )
	{
		RtlInitUnicodeString( &uniObjectName, lpFileName );
		InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		if ( ZwCreateFile( &hFile, 0x12019F, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN, 0x60, 0, 0 ) >= 0 )
		{
			if ( ZwSetInformationFile( hFile, &IoStatusBlock, FilePosition, 8, FilePositionInformation ) >= 0 )
				ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, pbuffer, WriteSize, 0, 0 );

			ZwClose( hFile );
		}
	}
	
	kfree( pbuffer );	
	return ;
}


ULONG 
ParseManifest (
	IN PVOID pBuffer,
	IN ULONG WriteSize
	)
{
	PCHAR ptr; // edi@1
	CHAR *ptr1; // edi@3
	PCHAR ptr2; // esi@3
	CHAR Data2; // al@5
	PCHAR ptr3; // esi@7
	PCHAR ptr4; // edi@10
	CHAR Data5; // al@12
	char Data6; // al@14
	PCHAR ptr5; // esi@14
	CHAR Data7; // al@18
	PCHAR ptr6; // esi@18
	PCHAR ptr7; // esi@19
	CHAR Data8; // al@20
	PCHAR v17; // esi@20
	CHAR Data4; // al@23
	ULONG Length; // edx@24
	ULONG Size; // ebx@25
	PCHAR ptr8; // esi@25
	CHAR Data3; // al@34
	PCHAR ptr3Dummy; // [sp+Ch] [bp-4h]@7

	ptr = (PCHAR)pBuffer;
	if ( *ptr )
	{
		do
		{
			if ( *ptr == '>' )
			{
				ptr2 = ptr1 = ptr + 1 ;

				if ( *ptr2 )
				{
					while ( isspace(*ptr2) )
					{
						Data2 = (ptr2++)[1];
						if ( !Data2 )
						{
							*ptr1 = 0;
							break;
						}
					}
				}

				if ( !*ptr2 )
				{
					*ptr1 = 0;
					break;
				}

				if ( ptr2 > ptr1 ) { memmove( ptr1, ptr2, strlen(ptr2) + 1 ); }

				ptr = ptr1 - 1;
			}

			Data3 = (ptr++)[1];
		}
		while ( Data3 );
	}

	ptr3 = ptr3Dummy = (PCHAR)pBuffer;
	if ( *ptr3 )
	{
		while ( TRUE )
		{
			if ( *ptr3 != '<' || strnicmp(ptr3 + 1, "assemblyIdentity", 0x10) )
				goto LABEL_23;

			ptr4 = ptr3;
			if ( *ptr3 )
				break;
LABEL_13:
			if ( *ptr4 == '>' )
				goto LABEL_14;
LABEL_23:
			Data4 = (ptr3++)[1];
			ptr3Dummy = ptr3;
			if ( !Data4 )
				goto _over_;
		}

		while ( *ptr4 != '>' )
		{
			Data5 = (ptr4++)[1];
			if ( !Data5 )
				goto LABEL_13;
		}

LABEL_14:
		ptr5 = ptr4 + 1;
		for ( Data6 = ptr4[1]; Data6; Data6 = (ptr5++)[1] )
		{
			if ( !isspace(Data6) )
				break;
		}

		if ( *ptr5 == '<' )
		{
			Data7 = ptr5[1];
			ptr6 = ptr5 + 1;
			if ( Data7 == '/' )
			{
				ptr7 = ptr6 + 1;
				if ( !strnicmp(ptr7, "assemblyIdentity", 0x10) )
				{
					Data8 = ptr7[16];
					v17 = ptr7 + 16;
					if ( Data8 == '>' )
					{
						*ptr4 = '/';
						ptr4[1] = '>';
						memmove(ptr4 + 2, v17 + 1, strlen(v17 + 1) + 1);
					}
				}
			}
		}

		ptr3 = ptr3Dummy;
		goto LABEL_23;
	}

_over_:
	Length = strlen((const char *)pBuffer);
	if ( Length < WriteSize )
	{
		ptr8 = (char *)pBuffer + Length;
		Size = WriteSize - Length;
		memset( ptr8, '    ', Size );
		
		Length = WriteSize;
		ptr8[Size] = 0;
	}

	return Length;
}


VOID 
Walk_c_windows_winsxs_Total_dep (
	IN HANDLE hFile,
	IN int bFlag,
	IN LPWSTR szPath,
	IN LPWSTR lpSearchPath,
	IN LPWSTR szWinsxsPath,
	OUT PULONG FilePosition,
	OUT PULONG WriteSize
	)
{
	int FuckedBuffer = 0 ;
	LPRPC_OUT_WINSXS RpcBuffer = NULL ;
	HANDLE hMappedHandle = NULL ;
	PIMAGE_DOS_HEADER FileData = NULL ;

	hMappedHandle = CreateFileMappingW( hFile, 0, SEC_IMAGE | PAGE_READONLY, 0, 0, 0 ); 
	if ( NULL == hMappedHandle ) { return; }

	FileData = (PIMAGE_DOS_HEADER) MapViewOfFileEx( hMappedHandle, FILE_MAP_READ, 0, 0, 0, 0 );
	CloseHandle( hMappedHandle );
	if ( NULL == FileData ) { return; }

	RpcBuffer = (LPRPC_OUT_WINSXS) Walk_c_windows_winsxs_Total_dep_phase1( (PCHAR)FileData, szPath, szWinsxsPath, (LPWSTR*)&FuckedBuffer, WriteSize );
	if ( RpcBuffer )
	{
		if ( FuckedBuffer ) { *FilePosition = Walk_c_windows_winsxs_Total_dep_phase2(FuckedBuffer, FileData); }

		Walk_c_windows_winsxs_Total_dep_phase3( RpcBuffer, lpSearchPath, bFlag );
		kfree( RpcBuffer );
	}

	Walk_c_windows_winsxs_Total_dep_phase4( (int)FileData, lpSearchPath, bFlag );
	
	UnmapViewOfFile( FileData );
	return;
}


PVOID 
Walk_c_windows_winsxs_Total_dep_phase1 (
	IN PCHAR FileData,
	IN LPWSTR szPath,
	IN LPWSTR szWinsxsPath,
	OUT LPWSTR *OutFuckedBuffer,
	OUT PULONG OutFuckedLength
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	LPRPC_OUT_WINSXS OutRpcBuffer = NULL ;
	LPRPC_IN_WINSXS InRpcBuffer = NULL ;
	LPWSTR NextBuffer = NULL, FuckedBuffer = NULL, ptr = NULL ;
	ULONG WinsxsPathLengthL = 0, PathLength = 0, DataLength = 0, FuckedBufferoffset = 0, NextBufferOffset = 0 ;
	int FuckedLength = 0, temp = 0 ;

	sub_7D2523A0( (PIMAGE_DOS_HEADER)FileData, &FuckedBuffer, &FuckedLength, &temp );

	if ( NULL == FuckedBuffer ) { return NULL ; }
	
	WinsxsPathLengthL	= wcslen( szWinsxsPath );
	NextBufferOffset	= 2 * WinsxsPathLengthL + 0x28 ;
	FuckedBufferoffset	= 2 * wcslen(szPath) + 8 + NextBufferOffset ;
	DataLength			= FuckedBufferoffset + FuckedLength + 4 ;

	InRpcBuffer = (LPRPC_IN_WINSXS) kmalloc( DataLength );

	InRpcBuffer->RpcHeader.DataLength = DataLength ;
	InRpcBuffer->RpcHeader.Flag = 0x1904 ;

	// 第一段字符串
	InRpcBuffer->WinsxsPathLength = 2 * WinsxsPathLengthL + 4;

	wcscpy( InRpcBuffer->Buffer, szWinsxsPath );

	InRpcBuffer->Buffer[ WinsxsPathLengthL     ] = '\\';
	InRpcBuffer->Buffer[ WinsxsPathLengthL + 1 ] = '*';
	InRpcBuffer->Buffer[ WinsxsPathLengthL + 2 ] = 0 ;

	// 第二段字符串
	InRpcBuffer->NextBufferOffset = NextBufferOffset;

	NextBuffer = (LPWSTR)( (PCHAR)InRpcBuffer + NextBufferOffset );	
	wcscpy( NextBuffer, szPath );
	*wcsrchr( NextBuffer, '\\' ) = 0 ;

	InRpcBuffer->PathLength = 2 *wcslen(NextBuffer);

	// 第三段字符串
	InRpcBuffer->FuckedBufferoffset = FuckedBufferoffset ;
	InRpcBuffer->FuckedLength = FuckedLength ;

	memcpy( (char *)InRpcBuffer + FuckedBufferoffset, FuckedBuffer, FuckedLength );
	*(BYTE *)( (char *)InRpcBuffer + FuckedBufferoffset + FuckedLength ) = 0;
	
	// RPC通信
	OutRpcBuffer = (LPRPC_OUT_WINSXS) PB_CallServer( (PVOID)InRpcBuffer );
	if ( OutRpcBuffer )
	{
		if ( STATUS_SUCCESS == OutRpcBuffer->RpcHeader.u.Status && OutRpcBuffer->WriteSize )
		{
			ptr = &szPath[ wcslen(szPath) ];
			wcscpy( ptr, L".manifest" );

			status = MyWriteFile( szPath, &OutRpcBuffer->WriteBuffer, OutRpcBuffer->WriteSize );
			if ( NT_SUCCESS(status) && temp )
			{
				swprintf( ptr, L".%d", temp);
				wcscat( ptr, L".manifest" );

				status = MyWriteFile( szPath, &OutRpcBuffer->WriteBuffer, OutRpcBuffer->WriteSize );
			}

			*(WORD *)ptr = 0;
			if ( g_QueryActCtxSettingsW_addr )
			{
				if ( OutRpcBuffer->WriteSize != FuckedLength || strnicmp((const char *)&OutRpcBuffer->WriteBuffer, FileData, FuckedLength) )
				{
					*OutFuckedBuffer = FuckedBuffer ;
					*OutFuckedLength = FuckedLength;
				}
			}
		}

		PB_FreeReply( OutRpcBuffer );
	}
	
	kfree( InRpcBuffer );
	return (PVOID)OutRpcBuffer;
}


ULONG 
Walk_c_windows_winsxs_Total_dep_phase2 (
	IN int FuckedBuffer,
	IN PIMAGE_DOS_HEADER PeAddr
	)
{
	PIMAGE_NT_HEADERS pNtHeader = NULL ;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL ;
	ULONG ret = 0, Index = 0, offset = 0, NumberOfSections = 0, temp = 0 ;
	
	if ( PeAddr->e_magic != 'MZ' && PeAddr->e_magic != 'ZM' ) { return 0 ; }
	
	pNtHeader = (PIMAGE_NT_HEADERS)( (PCHAR)PeAddr + PeAddr->e_lfanew );
	if ( pNtHeader->Signature != 'EP' ) { return 0 ; }
	
	NumberOfSections = pNtHeader->FileHeader.NumberOfSections;
	if ( 0 == NumberOfSections )  { return 0 ; }

	offset = (ULONG)FuckedBuffer - (ULONG)PeAddr;
	pSectionHeader = (PIMAGE_SECTION_HEADER)( (PCHAR)&pNtHeader->OptionalHeader + pNtHeader->FileHeader.SizeOfOptionalHeader );
	temp = (ULONG)( (PCHAR)&pNtHeader->OptionalHeader.SizeOfUninitializedData + pNtHeader->FileHeader.SizeOfOptionalHeader );

	while ( offset < *(DWORD *)temp || offset >= *(DWORD *)temp + *(DWORD *)(temp + 4) )
	{
		++ Index ;
		temp += 0x28 ;

		if ( Index >= NumberOfSections )  { return 0 ; }
	}

	ret = offset + pSectionHeader[ Index ].PointerToRawData - pSectionHeader[ Index ].VirtualAddress ;
	return ret;
}


VOID 
Walk_c_windows_winsxs_Total_dep_phase3 (
	IN LPRPC_OUT_WINSXS RpcBuffer,
	IN LPWSTR lpSearchPath,
	IN int bFlag
	)
{
	PCHAR ptr = NULL, ptr1 = NULL, ptr2 = NULL, lpName = NULL ;
	int OldData1 = 0, OldData2 = 0, length = 0 ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	ptr = (PCHAR) RpcBuffer->WriteBuffer ;
	if ( NULL == RpcBuffer->WriteBuffer ) { return; }
	
	do
	{
		ptr1 = strchr( ptr, '<' );
		if ( NULL == ptr1 ) { break; }

		ptr = ptr1 + 1;
		if ( strnicmp(ptr, "assemblyIdentity", 0x10) ) { continue; }
		if ( 0 == isspace( ptr[16] ) ) { continue; }
		
		lpName = ptr + 17;
		ptr = ptr1 = strchr( lpName, '>' );
		if ( NULL == ptr ) { return; }
			
		ptr2 = NULL ;
		if ( lpName != ptr1 )
		{
			while ( *lpName != 'n' && *lpName != 'N' || strnicmp(lpName, "name=\"", 6) )
			{
				++ lpName ;
				if ( lpName == ptr ) { goto _GOON_ ; }
			}

			lpName += 6 ;
			ptr2 = strchr( lpName, '\"' );
			if ( ptr2 == lpName ) { ptr2 = NULL ; }
		}

_GOON_ :
		length = (int) ((char *)&RpcBuffer->WriteBuffer + RpcBuffer->WriteSize - (DWORD)ptr2);
		if ( length <= 8 ) { return; }

		if ( ptr2 )
		{
			OldData1 = *(DWORD *)ptr2;
			OldData2 = *((DWORD *)ptr2 + 1);
			
			memcpy( ptr2, ".dll", 4 );
			*((DWORD *)ptr2 + 1) = 0;

			RtlInitString( &ansiBuffer, lpName );
			if ( RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE ) >= 0 )
			{
				Walk_c_windows_winsxs_Total( uniBuffer.Buffer, lpSearchPath, bFlag + 1 );
				RtlFreeUnicodeString( &uniBuffer );
			}

			*(DWORD *)ptr2 = OldData1;
			*((DWORD *)ptr2 + 1) = OldData2 ;
		}


	}
	while ( *ptr );

	return ;
}


VOID 
Walk_c_windows_winsxs_Total_dep_phase4 (
	IN int peAddr,
	IN LPWSTR lpSearchPath,
	IN int Flag
	)
{
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;
	int ptr1, ptr2, e_lfanew, Numbers ; 
	
	e_lfanew = *(DWORD *)(peAddr + 0x3C);
	Numbers = *(DWORD *)(e_lfanew + peAddr + 0x74);
	ptr1 = e_lfanew + peAddr + 0x18;
	if ( Numbers <= 1 ) { return; }
	
	ptr1 = *(DWORD *)(ptr1 + 0x68);
	if ( ptr1 <= 0 ) { return; }
	
	ptr2 = ptr1 + peAddr + 0xC;
	if ( 0 == *(DWORD *)ptr2 ) { return; }
	
	do
	{
		ptr1 = *(DWORD *)(ptr2 + 4);
		if ( 0 == ptr1 ) { break; }

		RtlInitString( &ansiBuffer,(PCHAR)(peAddr + *(DWORD *)ptr2) );
		if ( RtlAnsiStringToUnicodeString(&uniBuffer, &ansiBuffer, TRUE) >= 0 )
		{
			Walk_c_windows_winsxs_Total( uniBuffer.Buffer, lpSearchPath, Flag + 1 );
			RtlFreeUnicodeString( &uniBuffer );
		}

		ptr1 = *(DWORD *)(ptr2 + 0x14);
		ptr2 += 0x14 ;
	}
	while ( ptr1 );

	return;
}


int 
sub_7D2523A0 (
	IN PIMAGE_DOS_HEADER PeAddr,
	OUT LPWSTR *OutFuckedBuffer,
	OUT int *OutFuckedLength,
	OUT int *a4
	)
{
	int ret; // eax@1
	int peAddr; // esi@1
	LONG e_lfanew; // eax@1
	unsigned int NumberOfRvaAndSizes; // ecx@1
	int v8; // edx@4
	int v9; // edx@4
	int VirtrualAddr; // ecx@4
	int v11; // eax@4
	char v12; // zf@4
	int v13; // eax@10
	int v14; // edx@10
	int v15; // edi@10
	int v16; // eax@11
	int v17; // edx@11
	int v18; // eax@12
	int v19; // edi@12
	int v20; // edi@13

	peAddr = (int)PeAddr;
	*OutFuckedBuffer = 0;
	*OutFuckedLength = 0;
	*a4 = 0;

	e_lfanew = PeAddr->e_lfanew;
	NumberOfRvaAndSizes = *(DWORD *)(e_lfanew + peAddr + 0x74);
	ret = e_lfanew + peAddr + 0x18;
	if ( NumberOfRvaAndSizes > 2 )
	{
		ret = *(DWORD *)(ret + 0x70);
		if ( ret )
		{
			if ( ret >= 0 )
			{
				v9 = *(WORD *)(ret + peAddr + 0xE);
				VirtrualAddr = ret + peAddr;
				v11 = *(WORD *)(ret + peAddr + 0xC);
				v12 = v11 + v9 == 0;
				v8 = v11 + v9;
				ret = VirtrualAddr + 0x10;
				if ( !v12 )
				{
					while ( *(DWORD *)ret != 0x18 || *(DWORD *)(ret + 4) >= 0 )
					{
						ret += 8;
						--v8;
						if ( !v8 )
							return ret;
					}

					if ( v8 )
					{
						v13 = *(DWORD *)(ret + 4) & 0x7FFFFFFF;
						v14 = *(WORD *)(v13 + VirtrualAddr + 0xE);
						v15 = *(WORD *)(v13 + VirtrualAddr + 0xC);
						ret = VirtrualAddr + v13;
						if ( v15 + v14 )
						{
							v17 = ret + 16;
							v16 = *(DWORD *)(ret + 0x14);
							if ( v16 < 0 )
							{
								v18 = v16 & 0x7FFFFFFF;
								v19 = *(WORD *)(v18 + VirtrualAddr + 14);
								ret = VirtrualAddr + v18;
								if ( !(*(WORD *)(ret + 12) + v19) )
									return ret;

								v20 = *(DWORD *)v17;
								if ( *(DWORD *)v17 >= 0 )
									*a4 = (unsigned __int16)v20;
								v17 = ret + 16;
							}

							ret = *(DWORD *)(v17 + 4);
							if ( ret >= 0 )
							{
								*OutFuckedBuffer = (LPWSTR)(peAddr + *(DWORD *)(ret + VirtrualAddr));
								ret = *(DWORD *)(ret + VirtrualAddr + 4);
								*OutFuckedLength = ret;
							}
						}
					}
				}
			}
		}
	}

	return ret;
}


NTSTATUS 
MyWriteFile (
	IN LPWSTR lpFileName,
	IN PVOID pBuffer,
	IN ULONG Length
	)
{
	HANDLE hFile = NULL ; 
	NTSTATUS status = STATUS_SUCCESS ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;

	RtlInitUnicodeString(&uniObjectName, lpFileName);
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile( &hFile, FILE_GENERIC_WRITE, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN_IF, 0x60, 0, 0 );
	if ( status >= 0 )
	{
		status = ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, pBuffer, Length, 0, 0 );
		ZwClose( hFile );
	}

	return status;
}


PVOID MyReadFile (IN LPWSTR szFileName)
{
	ULONG ReadSize = 0 ;
	HANDLE hFile = NULL ; 
	PVOID pBuffer = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	IO_STATUS_BLOCK IoStatusBlock ;

	RtlInitUnicodeString( &uniObjectName, szFileName );
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateFile( &hFile, FILE_GENERIC_READ, &ObjAtr, &IoStatusBlock, 0, 0, 7, FILE_OPEN, 0x60, 0, 0 );
	if ( ! NT_SUCCESS(status) ) { return NULL; }

	status = ZwQueryInformationFile( hFile, &IoStatusBlock, &FileInfo, 0x38, FileNetworkOpenInformation );
	if ( ! NT_SUCCESS(status) ) { goto _END_ ; }

	ReadSize = FileInfo.EndOfFile.LowPart ;
	if ( FileInfo.EndOfFile.HighPart || 0 == ReadSize || ReadSize >= 0x100000 ) { goto _END_ ; }

	pBuffer = kmalloc( ReadSize + 4 );
	status = ZwReadFile( hFile, 0, 0, 0, &IoStatusBlock, pBuffer, ReadSize, 0, 0 );

	if ( status < 0 || IoStatusBlock.Information != ReadSize )
	{
		kfree( pBuffer );
		pBuffer = NULL ;
	}
	else
	{
		*(DWORD *)((char *)pBuffer + ReadSize) = 0;
	}

_END_ :
	ZwClose( hFile );
	return pBuffer ;
}


VOID Hook_AddressOfEntryPoint()
{
	HWND hWnd = NULL ;
	BOOL bFlagMs = FALSE ;
	ULONG_PTR AddressOfEntryPoint = 0, BaseAddr = 0 ;
	PCHAR ptr = NULL ;

	//
	// TODO{sudami}: 1. 查询配置文件中指定要注入DLL的全路径
	//


	
	// 2. 决定是否Hook DLL的入口点
	hWnd = GetConsoleWindow ();
	if ( g_pImportTable_mscoree_dll || g_pImportTable_msvcm80_dll || g_pImportTable_msvcm90_dll ) { bFlagMs = TRUE ; }

	if ( NULL == hWnd && FALSE == bFlagMs ) { return; }

	// 3. 取得入口点地址,开始Hook
	BaseAddr = (ULONG_PTR) GetModuleHandleW( NULL );
	AddressOfEntryPoint = (ULONG_PTR) SRtlImageNtHeader((PVOID)BaseAddr)->OptionalHeader.AddressOfEntryPoint + BaseAddr ;

	RtlZeroMemory( g_AddressOfEntryPoint_OrigData, 5 );
	memcpy( g_AddressOfEntryPoint_OrigData, (PVOID)AddressOfEntryPoint, 5 ); // barkup

	VirtualProtect( (LPVOID)AddressOfEntryPoint, 5, PAGE_EXECUTE_READWRITE, &g_OldProtect );
	ptr = (PCHAR) AddressOfEntryPoint ;

	*ptr = (CHAR) 0xE8 ;
	*(DWORD*) (ptr+1) = (DWORD) ( (DWORD)fake_AddressOfEntryPoint - AddressOfEntryPoint - 5 );

	return;
}


VOID 
HandlerManagedCode (
	IN LPWSTR szDllName,
	IN PIMAGE_IMPORT_DESCRIPTOR piid
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/01/06 [6:1:2011 - 15:02]

Routine Description:
  加载指定的DLL,并填充它的IAT表,处理托管代码相关的DLL

--*/
{
	HMODULE hModule = NULL ;
	LPSTR lpProcName = NULL ;
	PIMAGE_IMPORT_BY_NAME pIIN = NULL ;
	ULONG_PTR ModuleBase = 0, FirstThunkAV = 0, Length = 0 ;
	DWORD flOldProtect, ptr_start = 0, ptr_end = 0, Tmp = 0 ;
	PIMAGE_THUNK_DATA pFirstThunk = NULL ;

	// 1. 加载该DLL
	hModule = (HMODULE) LoadLibraryW( szDllName );
	if ( NULL == hModule ) { return; }

	// 2. 定位到IAT的 FirstThunk 数组
	ModuleBase = (ULONG_PTR) GetModuleHandleW( NULL );

	FirstThunkAV = piid->FirstThunk ;
	if ( 0 == FirstThunkAV ) { return; }

	ptr_start = ptr_end = ModuleBase + FirstThunkAV ;
	pFirstThunk = (PIMAGE_THUNK_DATA) ptr_start ;

	Tmp = *(DWORD *) ptr_end ;
	if ( Tmp )
	{
		do
		{
			ptr_end += 4 ;
			Tmp = *(DWORD *) ptr_end ;
		}
		while ( Tmp );
	}

	// 3. 填充数组中指定函数名字对应的函数地址
	Length = (ULONG_PTR)(ptr_end - ptr_start) ;
	VirtualProtect( (LPVOID)pFirstThunk, Length, PAGE_READWRITE, &flOldProtect );

	while ( TRUE )
	{
		if ( 0 == pFirstThunk->u1.Ordinal ) { break; }

		pIIN = (PIMAGE_IMPORT_BY_NAME)(ModuleBase + (ULONG_PTR) pFirstThunk->u1.AddressOfData);

		lpProcName = (LPSTR)pIIN->Name ;
		if ( NULL == lpProcName ) { lpProcName = (LPSTR)pIIN->Hint ; }

		pFirstThunk->u1.Function = (DWORD) GetProcAddress( hModule, lpProcName );
		pFirstThunk ++ ;
	}

	VirtualProtect( (LPVOID)pFirstThunk, Length, flOldProtect, &flOldProtect );
	return;
}


static int ThreadProcOEP (IN LPAddressOfEntryPoint_THREAD_INFO pBuffer)
{
	{
		HMODULE hModule = Get_hModule_user32DLL();
		if ( NULL == hModule ) { return 0; }

		g_MsgWaitForMultipleObjects_addr = (_MsgWaitForMultipleObjects_) GetProcAddress( hModule, "MsgWaitForMultipleObjects" );
		if ( NULL == g_MsgWaitForMultipleObjects_addr ) { return 0; }

		g_PeekMessageW_addr = (_PeekMessageW_) GetProcAddress( hModule, "PeekMessageW" );
		if ( NULL == g_PeekMessageW_addr ) { return 0; }

		g_GetMessageW_addr = (_GetMessageW_) GetProcAddress( hModule, "GetMessageW" );
		if ( NULL == g_GetMessageW_addr ) { return 0; }
	}
	
	WNDCLASSW wndClass = {};

	RtlZeroMemory( &wndClass, sizeof(WNDCLASSW) );
	wndClass.lpfnWndProc = g_DefWindowProcW_addr ;
	wndClass.hInstance = __ProcessNode->DllBaseAddr.hModule_Self ;
	wndClass.lpszClassName = L"ProteinboxHelperWindowClass";
	LPWSTR lpClassName = (LPWSTR) g_RegisterClassW_addr(&wndClass);
	
	HWND MainWnd = (HWND) g_CreateWindowExW_addr (
		0,
		lpClassName,
		NULL,
		0xCF0000,
		1,1,1,1,
		0,0,0,0
		);

	if ( MainWnd )
	{
		MSG msg = {};

		HandlerWindowLong( MainWnd, 1 );
		SetEvent( pBuffer->hEvent );

		do
		{
			while ( g_PeekMessageW_addr(&msg, 0, 0, 0, 0) )
			{
				if ( g_GetMessageW_addr(&msg, 0, 0, 0) != 0xFFFFFFFF )
					g_DispatchMessageW_addr(&msg);
			}
		}
		while ( g_MsgWaitForMultipleObjects_addr(1, (HANDLE*)pBuffer, FALSE, INFINITE, 0x4FF) );
	}

	return 0 ;
}


VOID HandlerConsole()
{
	HANDLE hCallerThread = NULL ;
	LPWSTR lpConsoleTitle = NULL ;
	BOOL bRet = TRUE, bIsLocked = FALSE ;
	int i = 0, TotalCounts = 0 ;
	LPHOOKINFOLittle pArray = NULL ;
	LPAddressOfEntryPoint_THREAD_INFO pBuffer = NULL ;

	// 1. Inline Hook Console 相关的IAT函数
	TotalCounts = g_HookInfo.CONSOLE.ArrayCounts ;
	pArray		= g_HookInfo.CONSOLE.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | HandlerConsole() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return ;
		}
	}

	// 2.
	lpConsoleTitle = (LPWSTR) kmalloc( 0x200 );
	if ( GetConsoleTitleW( lpConsoleTitle, 0x1FE ) )
		SetConsoleTitleW( lpConsoleTitle );

	kfree( lpConsoleTitle );

	pBuffer = (LPAddressOfEntryPoint_THREAD_INFO) kmalloc( sizeof(AddressOfEntryPoint_THREAD_INFO) );

	pBuffer->hCallerThread = (HANDLE) OpenThread( 0x100000, FALSE, GetCurrentThreadId() );
	if ( NULL == pBuffer->hCallerThread ) { goto _END_ ; }

	pBuffer->hEvent = CreateEventW( 0, 0, 0, 0 );
	if ( NULL == pBuffer->hEvent ) { goto _END_ ; }
	
	pBuffer->hWorkThread = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)ThreadProcOEP, pBuffer, 0, 0 );
	if ( pBuffer->hWorkThread )
	{
		WaitForMultipleObjects( 3, &pBuffer->hCallerThread, FALSE, INFINITE );
		CloseHandle( pBuffer->hWorkThread );
	}

	CloseHandle( pBuffer->hEvent );

_END_ :
	kfree( pBuffer );
	return;
}

///////////////////////////////   END OF FILE   ///////////////////////////////