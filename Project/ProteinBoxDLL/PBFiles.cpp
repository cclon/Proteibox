/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/11/09 [9:11:2010 - 18:41]
* MODULE : SandBox\Code\Project\ProteinBoxDLL\PBFiles.cpp
* 
* Description:
*      
*   文件重定向模块                       
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
#include "PBRegsData.h"
#include "PBFilesData.h"
#include "ProteinBoxDLL.h"
#include "PBFiles.h"

#pragma warning(disable : 4995 )

//////////////////////////////////////////////////////////////////////////

CRITICAL_SECTION g_Lock_RtlGetCurrentDirectory ;

LPWSTR g_CurrentDirectory_Orignal	 = NULL ;
LPWSTR g_CurrentDirectory_Redirected = NULL ;

static const LPWSTR g_Houzhui_Arrays[ ] = // 保存禁止操作的文件后缀名
{ 
	L".url",
	L".avi",
	L".wmv",
	L".mpg",
	L".mp3",
	L".mp4",
	L".wmdb",
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


ULONG WINAPI
fake_NtCreateFile (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
	)
{
	ULONG nCounts = 0 ;
	ULONG Is_OrignalFile_exist, Flag, TMP ;
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtrDummy, *ObjAtr ;
	UNICODE_STRING ObjAtrDummy_ObjectName ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, HouZhuiPtr = NULL ;
	LPWSTR OrignalPathShortDummy = NULL, RedirectedPathShortDummy = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPWSTR OrignalPathShort = NULL, RedirectedPathShort = NULL ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNotEndOfFile = FALSE ;
	BOOL bRedirected_File_is_exist = FALSE, bXX1 = FALSE, bXX2 = FALSE  ;
	BOOL bHas_FILE_DELETE_ON_CLOSE = FALSE ;
	_NtCreateFile_ OrignalAddress = (_NtCreateFile_) GetFileFunc(NtCreateFile) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
	{
		return OrignalAddress( FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
			FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength );
	}

	if ( pNode->sFileLock.nLockNtCreateFile )
	{
		return OrignalAddress( FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
			FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength );
	}

	pNode->sFileLock.nLockNtCreateFile = 1;
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	while ( TRUE )
	{
		IoStatusBlock->Information = 5 ;
		IoStatusBlock->Status = 0 ;

		ObjAtrDummy.Length = 0x18 ;
		ObjAtrDummy.RootDirectory = NULL ;
		if ( ObjectAttributes )
			ObjAtrDummy.Attributes = ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE ;
		else
			ObjAtrDummy.Attributes = 0 ;
		
		ObjAtrDummy.ObjectName = &ObjAtrDummy_ObjectName;
		ObjAtrDummy.SecurityDescriptor = g_SecurityDescriptor ;
		ObjAtrDummy.SecurityQualityOfService = NULL ;
		CreateOptions &= 0xFFFFBFFF ; // 过滤掉 FILE_OPEN_FOR_BACKUP_INTENT 权限

		// 1. 得到原始 & 重定向 路径
		status = GetFilePath (
			ObjectAttributes->ObjectName,     
			ObjectAttributes->RootDirectory, 
			&OrignalPath,		// "\Device\HarddiskVolume1\555.txt"
			&RedirectedPath,	// "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\C\555.txt"
			0
			);

		// 2.处理 管道 | 油槽 通信
		if ( ! NT_SUCCESS(status) )
		{
			ULONG  NameType = 0 ;
			LPWSTR PipeName = NULL ;

			if ( STATUS_BAD_INITIAL_PC != status ) { goto _over_ ; }

			NameType = GetNameType( OrignalPath, &PipeName );
			if ( NameType )
			{
				status = Handler_PipeMailslot_Communication (
					CreateOptions ,
					&ObjAtrDummy,
					FileHandle,
					DesiredAccess,
					ObjectAttributes->SecurityDescriptor,
					IoStatusBlock,
					ShareAccess,
					CreateDisposition,
					OrignalPath,
					NameType,
					PipeName
					);
			}
			else
			{
				status = OrignalAddress ( FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
					FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength );

				if ( STATUS_ACCESS_DENIED == status )
				{
					status = OrignalAddress ( FileHandle, DesiredAccess & 0x801200A9, ObjectAttributes, IoStatusBlock, 
						AllocationSize, FileAttributes, ShareAccess, 1, CreateOptions & 0xFFFFEFFF, EaBuffer, EaLength);
				}
			}

			goto _over_ ;
		}

		// 3.判断黑白名单
		WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );

		if ( bIsBlack ) { status = STATUS_ACCESS_DENIED; goto _over_ ; }

		if ( bIsWhite ) 
		{
			RtlInitUnicodeString( &ObjAtrDummy_ObjectName, OrignalPath );
			ObjAtrDummy.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;
			
			status = OrignalAddress( FileHandle, DesiredAccess, &ObjAtrDummy, IoStatusBlock, AllocationSize,
				FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);

			if ( STATUS_ACCESS_DENIED == status && 0x2000000 == DesiredAccess )
			{
				status = OrignalAddress( FileHandle, 0x120089, &ObjAtrDummy, IoStatusBlock, AllocationSize,
					FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);				
			}

			goto _over_ ;
		}

		// 4. 灰名单,权限过滤(这是第一次过滤,主要是参数合法性校验)
		status = NtCreateFileFilter( 0xFFFFFFFF, CreateDisposition, DesiredAccess, CreateOptions );
		if ( !NT_SUCCESS(status) ) { goto _over_ ; }

		// 4.1 修正文件名 [去掉"原始","重定向"文件名后的冒号":"]
		OrignalPathShortDummy = OrignalPathShort = wcsrchr( OrignalPath, '\\' );
		if ( OrignalPathShort )
		{
			// 去掉文件短名 冒号":"之后的内容,比如文件短名为:"555:111.txt",则修正后的值为:"555"
			OrignalPathShortDummy = ptr1 = wcschr( OrignalPathShort, ':' ); 
			if ( ptr1 ) { *ptr1 = 0 ; }
		}

		RedirectedPathShortDummy = RedirectedPathShort = wcsrchr( RedirectedPath, '\\' );
		if ( RedirectedPathShort )
		{
			RedirectedPathShortDummy = ptr2 = wcschr( RedirectedPathShort, ':' );
			if ( ptr2 ) { *ptr2 = 0 ; }
		}

		// 4.2 校验各层重定位目录的合法性
		if ( IsDirtyDirectory( RedirectedPath ) )
		{
			status = STATUS_OBJECT_PATH_NOT_FOUND;
			goto _over_;
		}

		// 4.3 校验重定向后文件是否已存在
		bNotEndOfFile = FALSE ;
		Is_OrignalFile_exist = 0x3F ;

		RtlInitUnicodeString( &ObjAtrDummy_ObjectName, RedirectedPath );
		status = QueryAttributesFile( &ObjAtrDummy, &Flag, &bNotEndOfFile );

		if ( NT_SUCCESS(status) )
		{
			// 4.3.1 查询重定向文件信息成功
			bXX1 = TRUE ;
			bRedirected_File_is_exist = TRUE ;

			if ( CreateOptions & FILE_DELETE_ON_CLOSE )
			{
				// 判断原始文件是否存在
				RtlInitUnicodeString( &ObjAtrDummy_ObjectName, OrignalPath );
				status = QueryAttributesFile( &ObjAtrDummy, &TMP, NULL );

				Is_OrignalFile_exist = status < 0 ? 'N' : 'Y' ;
				status = STATUS_SUCCESS ;
			}
		}
		else
		{
			// 4.3.2 查询重定向文件信息失败
			if ( STATUS_OBJECT_NAME_NOT_FOUND != status && STATUS_OBJECT_PATH_NOT_FOUND != status ) { goto _over_ ; }
			
			// 4.3.2.1 若重定向后的文件还未生成,则查询原始文件的属性
			bRedirected_File_is_exist = FALSE ;
			bXX1 = status == STATUS_OBJECT_NAME_NOT_FOUND ;

			RtlInitUnicodeString( &ObjAtrDummy_ObjectName, OrignalPath );

			// 这次查询原始文件很关键,只要原始文件存在,则Flag至少包含0x1(对应目录) 或者 0x40 (对应文件)
			status = QueryAttributesFile( &ObjAtrDummy, &Flag, NULL );
			if ( status >= 0 || STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status )
			{
				if ( CreateOptions & FILE_DELETE_ON_CLOSE ) 
					Is_OrignalFile_exist = (((status < 0) - 1) & 0xB) + 'N';

				bXX2 = status != STATUS_OBJECT_PATH_NOT_FOUND ;
				status = STATUS_SUCCESS ;
			}

			// 要操作的原始文件存在,说明不是创建新文件,而是要打开已存在的旧文件
			if ( Flag )
			{
				// (a) 过滤后缀名
				HouZhuiPtr = wcsrchr(OrignalPath, '.');
				if ( HouZhuiPtr )
				{
					for (int Index = 0; Index < ARRAYSIZEOF(g_Houzhui_Arrays); Index++ )
					{
						if ( 0 == wcsnicmp( HouZhuiPtr, g_Houzhui_Arrays[Index], sizeof(g_Houzhui_Arrays[Index]) ) )
						{
							// 
							// 加上标志位 SBFILE_ATTRIBUTE_READONLY_SYSTEM后,表明要严格禁止对文件名后缀 是"url,avi,wmv,mpg,mp3,mp4,wmdb"的越界操作.
							// 也就是无法以写权限打开这些后缀的文件,无法删除,重命名,以写方式打开. 但对于不是这些后缀的文件,就没有这么多的限制了.
							// 沙箱会对以上操作进行重新定位,而不是直接拒绝.
							//

							Flag |= SBFILE_ATTRIBUTE_READONLY_SYSTEM ;
							break ;
						}
					}
				}

				// (b) 过滤掉危险权限
				if ( FILE_OPEN == CreateDisposition || FILE_OPEN_IF == CreateDisposition )
				{
					if ( (FILE_WRITE_ATTRIBUTES == (DesiredAccess & 0x7FEDFF56)) && (0 == pNode->sFileLock.nLockNtDeleteFile) )
					{
						ClearFlag( DesiredAccess, FILE_WRITE_ATTRIBUTES );
					}

					if ( (DELETE == (DesiredAccess & 0x7FEDFF56)) && !(CreateOptions & FILE_DELETE_ON_CLOSE) )
					{
						ClearFlag( DesiredAccess, DELETE );
					}

					if ( !(DesiredAccess & 0x7FEDFF56) )
					{

						//  
						// 屏蔽掉Delete & FILE_WRITE_ATTRIBUTES 后,若没有除了以下权限外的其他权限则进入该IF语句内部!
						//   0x8000 0000 GENERIC_READ
						//   0x0010 0000 SYNCHRONIZE
						//   0x0002 0000 READ_CONTROL 
						//   0x0000 0080 FILE_READ_ATTRIBUTES
						//   0x0000 0020 FILE_EXECUTE
						//   0x0000 0008 FILE_READ_EA
						//   0x0000 0001 FILE_READ_DATA 
						//

						if ( PB_IsWow64() )
						{
							ObjAtr = ObjectAttributes ;
						}
						else
						{
							ObjAtr = &ObjAtrDummy ;

							if ( OrignalPathShortDummy ) { *OrignalPathShortDummy = ':' ; }
							RtlInitUnicodeString( &ObjAtrDummy_ObjectName, OrignalPath );
						}

						if ( CreateDisposition == FILE_OPEN_IF ) { CreateDisposition = FILE_OPEN; }

						status = g_NtCreateFile_addr (
							FileHandle,
							DesiredAccess,// 已被重新擦拭过的权限,少了写 & 删
							ObjAtr,
							IoStatusBlock,
							AllocationSize,
							FileAttributes,
							ShareAccess,
							CreateDisposition,
							CreateOptions,
							EaBuffer,
							EaLength
							);

						if ( FALSE == g_bIs_Inside_PBCrypto_exe || STATUS_SHARING_VIOLATION != status )
						{
							goto _over_ ;
						}

						DesiredAccess |= 0x120116 ;
						status = STATUS_SUCCESS ;
					}
				}
			}

		}

		if ( !NT_SUCCESS(status) ) { goto _over_ ; }

		// 4.4 再次进行文件操作权限过滤 (这是第二次过滤,对可能越界的权限进行过滤)
		status = NtCreateFileFilter( Flag, CreateDisposition, DesiredAccess, CreateOptions );

		if ( STATUS_FILE_IS_A_DIRECTORY == status && OrignalPathShortDummy && RedirectedPathShortDummy )
		{
			if ( FILE_OPEN == CreateDisposition || FILE_OPEN_IF == CreateDisposition )
			{
				if ( (CreateOptions & FILE_NON_DIRECTORY_FILE) && (Flag & SBFILE_DIRECTORY) )
				{
					status = STATUS_SUCCESS;
				}
			}
		}

		if ( !NT_SUCCESS(status) ) { goto _over_ ; }

		if ( CreateOptions & FILE_DELETE_ON_CLOSE )
		{
			if ( OrignalPathShortDummy || RedirectedPathShortDummy )
			{
				status = STATUS_ACCESS_DENIED ;
				goto _over_;
			}

			ClearFlag( CreateOptions, FILE_DELETE_ON_CLOSE );
			bHas_FILE_DELETE_ON_CLOSE = TRUE ;
		}
		else
		{
			bHas_FILE_DELETE_ON_CLOSE = FALSE ;
		}

		// 4.5 考虑在沙箱中运行的是explorer.exe的情况
		if (   g_bIs_Inside_Explorer_exe
			&& 0 == pNode->sFileLock.nLockNtDeleteFile
			&& FALSE == bHas_FILE_DELETE_ON_CLOSE	// 没有FILE_DELETE_ON_CLOSE权限
			&& FALSE == bRedirected_File_is_exist	// 重定向后的文件不存在
			&& Flag									// 要操作的原始文件存在
			&& FILE_OPEN == CreateDisposition		// 以此权限打开文件(夹)
			)   
		{
			RtlInitUnicodeString( &ObjAtrDummy_ObjectName, OrignalPath ); // 放行之
			status = g_NtCreateFile_addr (
				FileHandle,
				0x120089 ,
				&ObjAtrDummy,
				IoStatusBlock,
				AllocationSize,
				FileAttributes,
				ShareAccess,
				1,
				CreateOptions,
				EaBuffer,
				EaLength
				);
			
			goto _over_;
		}

		if ( bXX1 ) { break ; }

		if ( FALSE == bXX2 ) 
		{
			status = STATUS_OBJECT_PATH_NOT_FOUND;
			goto _over_;
		}

		if ( nCounts <= 4 )
		{
			++ nCounts ;
			if ( CallHandlerDevicesEx( OrignalPath ) ) { continue ; }
		}

		// 4.6 递归创建重定位后文件的上层目录,把没有的目录都创建好
		status = CreateRedirectedDirectorys( OrignalPath, RedirectedPath );
		if ( !NT_SUCCESS(status) ) { goto _over_ ; }

		break ;
	} // end-of-while


	// 5. 原始文件存在,重定向文件不存在的情况
	if ( FALSE == bRedirected_File_is_exist && Flag )
	{
		BOOL bResult = TRUE, bTmp = FALSE ;

		if ( FILE_OPEN == CreateDisposition || FILE_OPEN_IF == CreateDisposition || OrignalPathShortDummy )
		{
			bTmp = TRUE ;
			
			if ( !(Flag & SBFILE_NON_DIRECTORY) || bHas_FILE_DELETE_ON_CLOSE && (DesiredAccess & 0x7FEDFF56) == DELETE )
			{
				bTmp = FALSE ;// 若操作的是目录,或有FILE_COMPLETE_IF_OPLOCKED | DELETE 权限,就不拷贝数据!
			}			
		}
		else if ( FILE_OVERWRITE == CreateDisposition || FILE_OVERWRITE_IF == CreateDisposition )
		{
			bTmp = FALSE ;	
		}
		else
		{
			bResult = FALSE ;
		}

		if ( bResult )
		{
			status = Copy_OrignalFileData_to_RedirectedFileData ( bTmp, OrignalPath, RedirectedPath );
		}

		if ( !NT_SUCCESS(status) )
		{
			if ( STATUS_BAD_INITIAL_PC == status )
			{
				ClearFlag( CreateOptions, FILE_DELETE_ON_CLOSE );

				if ( OrignalPathShortDummy ) { *OrignalPathShortDummy = ':' ; }
				RtlInitUnicodeString( &ObjAtrDummy_ObjectName, OrignalPath );

				status = g_NtCreateFile_addr (
					FileHandle,
					DesiredAccess & 0x801200A9,
					&ObjAtrDummy,
					IoStatusBlock,
					AllocationSize,
					FileAttributes,
					ShareAccess,
					CreateDisposition,
					CreateOptions,
					EaBuffer,
					EaLength
					);
			}

			goto _over_;
		}
	}

	// 6. 修正权限后,调用原始函数操作重定位文件(夹)
	BOOL bIsDirectory = FALSE ;

	if ( bRedirected_File_is_exist && (Flag & SBFILE_MarkedAsDirty) && CreateDisposition != FILE_OPEN )
	{
		// 重定向文件已存在,且打开权限不为FILE_OPEN,则先删除重定向文件
		RtlInitUnicodeString( &ObjAtrDummy_ObjectName, RedirectedPath );
		status = g_NtDeleteFile_addr( &ObjAtrDummy );
		if ( !NT_SUCCESS(status) ) { goto _over_ ; }

		Flag = 0;
		if ( (CreateOptions & FILE_DIRECTORY_FILE) && (NULL == RedirectedPathShortDummy) )
		{
			bIsDirectory = TRUE ;
		}
	}

	if ( DesiredAccess & 0x52000106 )
	{
		if ( CreateOptions & FILE_DIRECTORY_FILE
			|| Flag & SBFILE_DIRECTORY && NULL == OrignalPathShortDummy && NULL == RedirectedPathShortDummy 
			)
		{
			DesiredAccess = DesiredAccess & 0xADFFFEF9 | 0x120089 ;
		} 
		else 
		{
			CreateOptions |= FILE_NON_DIRECTORY_FILE ;
		}
	}

	if ( RedirectedPathShortDummy ) { *RedirectedPathShortDummy = ':' ; }
	RtlInitUnicodeString( &ObjAtrDummy_ObjectName, RedirectedPath ); 

	if ( bHas_FILE_DELETE_ON_CLOSE && Is_OrignalFile_exist == 'N' )
	{
		CreateOptions |= FILE_DELETE_ON_CLOSE ;
		DesiredAccess |= DELETE ;
	}

	PHANDLE hRedirectedFile = FileHandle ;
	ACCESS_MASK DesiredAccess_New = DesiredAccess | FILE_READ_ATTRIBUTES;
	
	status = g_NtCreateFile_addr (
		FileHandle,
		DesiredAccess_New,
		&ObjAtrDummy,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength
		);

	if ( bHas_FILE_DELETE_ON_CLOSE && 'N' == Is_OrignalFile_exist )
	{
		if ( status < 0 )
		{
			ClearFlag( CreateOptions, FILE_DELETE_ON_CLOSE );
			ClearFlag( DesiredAccess, DELETE );
			DesiredAccess |= FILE_READ_ATTRIBUTES ;
			DesiredAccess_New = DesiredAccess ;
			
			status = g_NtCreateFile_addr (
				FileHandle,
				DesiredAccess,
				&ObjAtrDummy,
				IoStatusBlock,
				AllocationSize,
				FileAttributes,
				ShareAccess,
				CreateDisposition,
				CreateOptions,
				EaBuffer,
				EaLength
				);
		}
	}
	
	if ( !NT_SUCCESS(status) ) { goto _over_ ; }

	if ( bHas_FILE_DELETE_ON_CLOSE )
	{
		if ( RedirectedPathShortDummy )
		{
			status = STATUS_ACCESS_DENIED ;
		}
		else
		{
			status = MarkAsDirtyFile( *FileHandle, RedirectedPath );
			hRedirectedFile = FileHandle ;
		}

		if ( NT_SUCCESS(status) ) { goto _over_ ; }
	}
	else
	{
		BOOL InsertOK = FALSE ;

		status = RemoveDirtyTag( *FileHandle, RedirectedPath );
		if ( NT_SUCCESS(status) )
		{
			if ( CreateOptions & FILE_DIRECTORY_FILE )
			{
				if ( bIsDirectory )
				{
					pNode->sFileLock.nLockNtCreateFile = 0 ;
					MarkAsDirtyFileEx( OrignalPath );
				}
			}
			else
			{
				BOOL bInsert = IsWinsxsFile( OrignalPath );
				if ( bInsert || 0 == Flag || bNotEndOfFile || Flag & SBFILE_MarkedAsDirty )
				{
					InsertOK = InsertFileHandle( *FileHandle, OrignalPath, bInsert );
					hRedirectedFile = FileHandle ;
				}
			}

			if ( FALSE == InsertOK )
			{
				if ( 0 == Flag || Flag & SBFILE_MarkedAsDirty )
				{
					if ( Handler_RedirectedFile( OrignalPath, RedirectedPath, *hRedirectedFile ) )
					{
						status = g_NtCreateFile_addr (
							hRedirectedFile,
							DesiredAccess_New,
							&ObjAtrDummy,
							IoStatusBlock,
							AllocationSize,
							FileAttributes,
							ShareAccess,
							1,
							CreateOptions,
							EaBuffer,
							EaLength
							);
						
						if ( NT_SUCCESS(status) ) { goto _over_; }
						*hRedirectedFile = NULL ;
					}
				}
			}

			if ( NT_SUCCESS(status) ) { goto _over_; }
		}
	}

	// 7. 善后工作
	if ( *hRedirectedFile )
	{
		ZwClose( *hRedirectedFile );
		*hRedirectedFile = NULL ;
	}

	IoStatusBlock->Information = 0 ;

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	pNode->sFileLock.nLockNtCreateFile = 0 ;
	if ( status < 0 ) { IoStatusBlock->Status = status ; }
	return status ;
}


ULONG WINAPI
fake_NtOpenFile (
	OUT PHANDLE FileHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG ShareAccess,
	IN ULONG OpenOptions
	)
{
	return fake_NtCreateFile( FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
		0, 0, ShareAccess, 1, OpenOptions, 0, 0 );
}


ULONG WINAPI
fake_NtQueryAttributesFile (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
	)
{
	NTSTATUS status ;
	FILE_NETWORK_OPEN_INFORMATION FileInfo ;
	_NtQueryAttributesFile_ OrignalAddress = (_NtQueryAttributesFile_) GetFileFunc(NtQueryAttributesFile) ;

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( ObjectAttributes, FileInformation );

	status = ZwQueryFullAttributesFile( ObjectAttributes, &FileInfo );
	if ( status < 0 ) { return status; }
	
	*(QWORD *)&FileInformation->CreationTime	= *(QWORD *)&FileInfo.CreationTime ;
	*(QWORD *)&FileInformation->LastAccessTime	= *(QWORD *)&FileInfo.LastAccessTime.LowPart ;
	*(QWORD *)&FileInformation->LastWriteTime	= *(QWORD *)&FileInfo.LastWriteTime.LowPart ;
	*(QWORD *)&FileInformation->ChangeTime		= *(QWORD *)&FileInfo.ChangeTime.LowPart ;
	FileInformation->FileAttributes				= FileInfo.FileAttributes ;

	return status ;
}


ULONG WINAPI
fake_NtQueryFullAttributesFile (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
	)
{
	LPWSTR szName = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	_NtQueryFullAttributesFile_ OrignalAddress = (_NtQueryFullAttributesFile_) GetFileFunc(NtQueryFullAttributesFile) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( ObjectAttributes, FileInformation );

	// 1. 当explorer.exe在沙箱中运行,拒绝掉查询 "\??\X:\autorun.inf" 文件
	if ( g_bIs_Inside_Explorer_exe )
	{
		if ( ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Length == 0x24 )
		{
			szName = ObjectAttributes->ObjectName->Buffer ;
			if ( szName && 0 == wcsicmp(szName + 7, L"autorun.inf") )
			{
				return STATUS_OBJECT_NAME_NOT_FOUND;
			}
		}
	}

	// 2. 得到文件原始 & 重定向 路径
	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	
	status = GetFilePath( ObjectAttributes->ObjectName, ObjectAttributes->RootDirectory, &OrignalPath, &RedirectedPath, NULL );
	if( ! NT_SUCCESS( status ) ) 
	{ 
		if ( STATUS_BAD_INITIAL_PC == status )
		{
			wcscat( OrignalPath, L"\\" );
			RtlInitUnicodeString( &uniObjectName, OrignalPath );

			status = g_NtQueryFullAttributesFile_addr( &ObjAtr, FileInformation );
		}

		goto _over_;
	}

	// 3. 过黑白名单
	WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );

	if ( bIsBlack ) 
	{ 
		status = STATUS_ACCESS_DENIED; 
		goto _over_;
	}

	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		status = g_NtQueryFullAttributesFile_addr( &ObjAtr, FileInformation );
		goto _over_;
	}

	// 4. 处理灰名单

	// 4.1 检查重定向目录的合法性
	if ( IsDirtyDirectory( RedirectedPath ) ) 
	{ 
		status = STATUS_OBJECT_PATH_NOT_FOUND; 
		goto _over_;
	}

	// 4.2 重定向目录合法,则把对原始文件的查询重定向当前目录进行查询
	RtlInitUnicodeString( &uniObjectName, RedirectedPath );
	status = g_NtQueryFullAttributesFile_addr( &ObjAtr, FileInformation );
	
	if ( ! NT_SUCCESS(status) )
	{
		// 4.3 处理重定向文件不存在的情况
		if ( STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status )
		{
			// 重定向文件不存在则查询原始文件
			RtlInitUnicodeString( &uniObjectName, OrignalPath );
			status = g_NtQueryFullAttributesFile_addr( &ObjAtr, FileInformation );
		}
	}
	else
	{
		// 4.4 重定向文件存在,再进行一次校验
		if ( IsDirtyKeyEx( (PVOID)&FileInformation->CreationTime ) ) { status = STATUS_OBJECT_NAME_NOT_FOUND; }
	}

_over_:
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtQueryInformationFile (
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
	)
{
	UNICODE_STRING uniObjectName ;
	ULONG OrignalPathLength, FileNameLength, size ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	PFILE_NAME_INFORMATION FileInfo = (PFILE_NAME_INFORMATION) FileInformation ;
	_NtQueryInformationFile_ OrignalAddress = (_NtQueryInformationFile_) GetFileFunc(NtQueryInformationFile) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass );

	// 1. 只处理类型为FileNameInformation的情况,替换一下Name即可
	if ( FileNameInformation != FileInformationClass )
		return OrignalAddress( FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass );

	if ( Length < 0x10 ) { return STATUS_INFO_LENGTH_MISMATCH; }

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	RtlInitUnicodeString( &uniObjectName, NULL );
	status = GetFilePath( &uniObjectName, FileHandle, &OrignalPath, &RedirectedPath, NULL );
	if ( STATUS_BAD_INITIAL_PC == status ) { status = STATUS_SUCCESS; }

	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	PB_TranslateNtToDosPath( OrignalPath );

	OrignalPathLength = wcslen( OrignalPath );
	if ( OrignalPathLength >= 2 && ':' == OrignalPath[1] )
	{
		{
			OrignalPath[ OrignalPathLength ] = '\\';
			OrignalPath[ OrignalPathLength + 1 ] = 0;
			OrignalPath += 2 ;
			-- OrignalPathLength ;
		}
	}

	size = Length - 4 ;
	FileInfo->FileNameLength = FileNameLength = 2 * OrignalPathLength ;

	if ( size <= FileNameLength )
	{
		status = STATUS_BUFFER_OVERFLOW ;
	}
	else
	{
		status = STATUS_SUCCESS;
		size = FileNameLength;
	}

	memcpy( FileInfo->FileName, OrignalPath, size );
	
	IoStatusBlock->Status = status ;
	IoStatusBlock->Information = size + 4 ;

_over_:
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtQueryDirectoryFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
	)
{
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR ptr1 = NULL, ptr2 = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bLocked = FALSE ;
	_NtQueryDirectoryFile_ OrignalAddress = (_NtQueryDirectoryFile_) GetFileFunc(NtQueryDirectoryFile) ;
	LPPROCESS_GLOBALMEMORY_NODE pBigNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
	{
		return OrignalAddress( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, 
			Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
	}

	CMemoryManager::GetInstance().AntiDebuggerEnter(pBigNode);

	RtlInitUnicodeString( &uniObjectName, NULL );
	status = GetFilePath( &uniObjectName, FileHandle, &OrignalPath, &RedirectedPath, NULL );
	if ( STATUS_BAD_INITIAL_PC == status && RedirectedPath ) 
	{
		ptr1 = &OrignalPath[ wcslen(OrignalPath) ];
		*ptr1 = '\\' ;
		ptr1[1] = 0  ;
		status = STATUS_SUCCESS ;
	}

	if ( NT_SUCCESS(status) ) 
	{
		WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );
		
		if ( bIsBlack ) { status = STATUS_ACCESS_DENIED; }
		if ( bIsWhite ) { status = STATUS_BAD_INITIAL_PC; }
	}

	if ( !NT_SUCCESS(status) ) 
	{
		if ( status == STATUS_BAD_INITIAL_PC )
		{
			status = OrignalAddress( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, 
				Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
		}

		goto _over_ ;
	}

	if ( FALSE == ReturnSingleEntry ) { RestartScan = FALSE ; }

	LPDIR_HANDLE_INFO pNode = NULL ;
	status = GetDirNode( FileHandle, OrignalPath, RedirectedPath, RestartScan, FileName, &pNode );

	if ( !NT_SUCCESS(status) ) 
	{
		if ( status == STATUS_BAD_INITIAL_PC )
		{
			status = OrignalAddress( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, 
				Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan );
		}

		goto _over_ ;
	}

	bLocked = TRUE ;
	status = NtQueryDirectoryFileFilter (
		pNode,
		IoStatusBlock,
		(int)FileInformation,
		Length,
		FileInformationClass,
		ReturnSingleEntry
		);

	if ( pNode->bFlag2 )
	{
		if ( STATUS_NO_MORE_FILES == status ) { status = STATUS_NO_SUCH_FILE; }
		
		pNode->bFlag2 = 0 ;
	}

	LeaveCriticalSection( &g_Lock_DirHandle );
	bLocked = FALSE;
	if ( Event ) { SetEvent( Event ); }

	if ( ApcRoutine )
	{
		LPAPCINFO APC = (LPAPCINFO) kmalloc( sizeof(APCINFO) );
		
		APC->ApcRoutine		= (PIO_APC_ROUTINE) ApcRoutine ;
		APC->ApcContext		= ApcContext ;
		APC->IoStatusBlock	= IoStatusBlock ;

		QueueUserAPC( (PAPCFUNC)pfnAPC, GetCurrentThread(), (ULONG_PTR)APC );
	}

_over_ :
	if ( bLocked )
		LeaveCriticalSection(&g_Lock_DirHandle);

	CMemoryManager::GetInstance().AntiDebuggerLeave(pBigNode);
	return status;
}


ULONG WINAPI
fake_NtSetInformationFile (
    IN HANDLE FileHandle,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	DWORD dwErrCode = GetLastError();
	BOOL bFlag = TRUE, bCallOrig = FALSE ;
	_NtSetInformationFile_ OrignalAddress = (_NtSetInformationFile_) GetFileFunc(NtSetInformationFile) ;

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass );

	if ( FileBasicInformation == FileInformationClass )
	{
		if ( Length < 0x28 )
		{
			status = STATUS_INFO_LENGTH_MISMATCH ;
		}
		else
		{
			status = MarkFileTime( FileInformation, FileHandle, NULL );
		}
	}
	else if ( FileDispositionInformation == FileInformationClass )
	{
		InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, FileHandle, NULL );

		RtlInitUnicodeString( &uniObjectName, L"" );
		ZwDeleteFile( &ObjAtr );
	}
	else if ( FileRenameInformation == FileInformationClass )
	{
		status = NtSetInformationFile_FileRenameInformation_Filter( FileHandle, (PFILE_RENAME_INFORMATION)FileInformation );
	}
	else if ( FilePipeInformation == FileInformationClass || FileCompletionInformation == FileInformationClass )
	{
		bFlag = FALSE ;

		if ( 0xFFFFFF00 == ((ULONG)FileHandle & 0xFFFFFF00) )
		{
			status = NtSetInformationFile_FilePipeInformation_Filter( (UCHAR)FileHandle & 0xFF, Length, IoStatusBlock, FileInformation, FileInformationClass );
		}
		else
		{
			bCallOrig = TRUE ;
		}
	}
	else
	{
		bFlag = FALSE ;
		bCallOrig = TRUE ;
	}

	if ( bCallOrig ) 
	{ 
		status = OrignalAddress( FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass );
	}
	
	if ( bFlag )
	{
		IoStatusBlock->Status = status ;
		IoStatusBlock->Information = 0 ;
	}

	SetLastError(dwErrCode);
	return status;
}


ULONG WINAPI
fake_NtDeleteFile (
    IN POBJECT_ATTRIBUTES ObjectAttributes
	)
{
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	IO_STATUS_BLOCK IoStatusBlock ;
	_NtDeleteFile_ OrignalAddress = (_NtDeleteFile_) GetFileFunc(NtDeleteFile) ;

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( ObjectAttributes );

	status = ZwCreateFile( &hFile , DELETE, ObjectAttributes, &IoStatusBlock, 0, 0, 7, FILE_OPEN, FILE_DELETE_ON_CLOSE, 0, 0 );
	if ( status >= 0 ) { ZwClose( hFile ); }

	return status ;
}


ULONG WINAPI
fake_NtClose (
	IN HANDLE  Handle
	)
{
	BOOL bFlag = FALSE ;
	WCHAR Buffer[ 256 ] = {};
	ULONG InfoLength = 0x80 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	POBJECT_TYPE_INFORMATION pObjectInfo = NULL ;
	PVOID pInRpcBuffer = NULL, pOutBuffer = NULL ;
	LPDIR_HANDLE_INFO pNodeCurrent = NULL, pNodeNext = NULL ;
	_NtClose_ OrignalAddress = (_NtClose_) GetFileFunc(NtClose) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( Handle );

	if ( pNode->sFileLock.nLockNtClose || INVALID_HANDLE_VALUE == Handle || (HANDLE)0xFFFFFFFE == Handle )
		return g_NtClose_addr( Handle );

	pNode->sFileLock.nLockNtClose = 1;

	if ( ((ULONG)Handle & 0xFFFFFF00) == 0xFFFFFF00 )
	{
		// 对于只有2位的句柄,调用该函数通过RPC方式关闭
		CloseLittleHandle( (UCHAR)Handle & 0xFF );

		pNode->sFileLock.nLockNtClose = 0 ;
		return STATUS_SUCCESS ;
	}

	pObjectInfo = (POBJECT_TYPE_INFORMATION) Buffer ;
	status = ZwQueryObject( Handle, ObjectTypeInformation, pObjectInfo, sizeof( Buffer ), &InfoLength );

	if ( status >= 0 && *pObjectInfo->Name.Buffer == 'F' && 0 == wcsicmp(pObjectInfo->Name.Buffer, L"File") ) { bFlag = TRUE ; }

	// 非文件类型句柄,直接调原始函数关闭掉
	if ( FALSE == bFlag ) 
	{ 
		status = g_NtClose_addr( Handle );
		pNode->sFileLock.nLockNtClose = 0 ;
		return status ;
	}

	// 处理文件类型的句柄
	EnterCriticalSection( &g_Lock_DirHandle );

	pNodeCurrent = (LPDIR_HANDLE_INFO) g_pNodeHead_DirHandle.Flink ;
	if ( pNodeCurrent )
	{
		do
		{
			pNodeNext = (LPDIR_HANDLE_INFO) pNodeCurrent->ListEntry.Flink;
			if ( Handle == pNodeCurrent->FileHandle )
			{
				RemoveEntryListEx( &g_pNodeHead_DirHandle, &pNodeCurrent->ListEntry );
				FreeDirNode( pNodeCurrent );
			}

			pNodeCurrent = pNodeNext ;
		}
		while ( pNodeNext );
	}

	LeaveCriticalSection(&g_Lock_DirHandle);

	NtCloseFilter( Handle, &pInRpcBuffer );

	status = g_NtClose_addr( Handle );

	if ( pInRpcBuffer )
	{
		pOutBuffer = PB_CallServer( pInRpcBuffer );
		kfree( pInRpcBuffer );
		if ( pOutBuffer ) { PB_FreeReply( pOutBuffer ); }
	}

	pNode->sFileLock.nLockNtClose = 0;
	return status ;
}


ULONG WINAPI
fake_NtCreateNamedPipeFile (
    OUT PHANDLE NamedPipeFileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN ULONG WriteModeMessage,
    IN ULONG ReadModeMessage,
    IN ULONG NonBlocking,
    IN ULONG MaxInstances,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PLARGE_INTEGER DefaultTimeOut
	)
{
	int NameType = 0, OrignalPathLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	PUNICODE_STRING pUniObjectName = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL, pNewName = NULL ;
	_NtCreateNamedPipeFile_ OrignalAddress = (_NtCreateNamedPipeFile_) GetFileFunc(NtCreateNamedPipeFile) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
	{
		return OrignalAddress( NamedPipeFileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, CreateDisposition,
			CreateOptions, WriteModeMessage, ReadModeMessage, NonBlocking, MaxInstances, InBufferSize, OutBufferSize, DefaultTimeOut );
	}

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetFilePath (
		ObjectAttributes->ObjectName,
		ObjectAttributes->RootDirectory,
		&OrignalPath,
		&RedirectedPath,
		NULL
		);

	if ( STATUS_BAD_INITIAL_PC != status ) { status = STATUS_INVALID_PARAMETER; goto _over_ ; }

	if ( NULL == ObjectAttributes->RootDirectory
		|| (pUniObjectName = ObjectAttributes->ObjectName) != 0 && pUniObjectName->Length )
	{
_Goon_ :
		InitializeObjectAttributes( &ObjAtr, &uniObjectName, ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE, NULL, g_SecurityDescriptor );

		WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );
		
		if ( bIsBlack ) { status = STATUS_ACCESS_DENIED; goto _over_ ; }
		if ( bIsWhite )
		{
			RtlInitUnicodeString( &uniObjectName, OrignalPath );
			ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

			status = OrignalAddress( NamedPipeFileHandle, DesiredAccess, &ObjAtr, IoStatusBlock, ShareAccess, CreateDisposition,
				CreateOptions, WriteModeMessage, ReadModeMessage, NonBlocking, MaxInstances, InBufferSize, OutBufferSize, DefaultTimeOut );

			goto _over_ ;
		}

		NameType = GetNameType( OrignalPath, NULL );

		pNewName = Get_Redirected_PipeMailslot_Name( OrignalPath, NameType );
		if ( NULL == pNewName ) { status = STATUS_INVALID_PARAMETER ; goto _over_ ; }

		RtlInitUnicodeString( &uniObjectName, pNewName );
		status = OrignalAddress( NamedPipeFileHandle, DesiredAccess, &ObjAtr, IoStatusBlock, ShareAccess, CreateDisposition,
			CreateOptions, WriteModeMessage, ReadModeMessage, NonBlocking, MaxInstances, InBufferSize, OutBufferSize, DefaultTimeOut );

		kfree( pNewName );
		goto _over_ ;
	}

	OrignalPathLength = wcslen( OrignalPath );

	if (   (OrignalPathLength != 0x11 && OrignalPathLength != 0x12)
		|| wcsnicmp( OrignalPath, L"\\device\\namedpipe\\", 0x11 )
		)
	{
		goto _Goon_ ;
	}

	status = OrignalAddress( NamedPipeFileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, CreateDisposition,
		CreateOptions, WriteModeMessage, ReadModeMessage, NonBlocking, MaxInstances, InBufferSize, OutBufferSize, DefaultTimeOut );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtCreateMailslotFile (
    OUT PHANDLE MailSlotFileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG MaxMessageSize,
    IN PLARGE_INTEGER TimeOut
	)
{
	int NameType = 0, OrignalPathLength = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	PUNICODE_STRING pUniObjectName = NULL ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL, pNewName = NULL ;
	_NtCreateMailslotFile_ OrignalAddress = (_NtCreateMailslotFile_) GetFileFunc(NtCreateMailslotFile) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( MailSlotFileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, FileAttributes, ShareAccess, MaxMessageSize, TimeOut );

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	status = GetFilePath (
		ObjectAttributes->ObjectName,
		ObjectAttributes->RootDirectory,
		&OrignalPath,
		&RedirectedPath,
		NULL
		);

	if ( STATUS_BAD_INITIAL_PC != status ) { return STATUS_INVALID_PARAMETER; }

	InitializeObjectAttributes( &ObjAtr, &uniObjectName, ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE, NULL, g_SecurityDescriptor );

	WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );

	if ( bIsBlack ) { status = STATUS_ACCESS_DENIED; goto _over_ ; }
	if ( bIsWhite )
	{
		RtlInitUnicodeString( &uniObjectName, OrignalPath );
		ObjAtr.SecurityDescriptor = ObjectAttributes->SecurityDescriptor ;

		status = OrignalAddress( MailSlotFileHandle, DesiredAccess, &ObjAtr, IoStatusBlock, FileAttributes, ShareAccess, MaxMessageSize, TimeOut );
		goto _over_ ;
	}

	NameType = GetNameType( OrignalPath, NULL );

	pNewName = Get_Redirected_PipeMailslot_Name( OrignalPath, NameType );
	if ( NULL == pNewName ) { return STATUS_INVALID_PARAMETER ; }

	RtlInitUnicodeString( &uniObjectName, pNewName );
	status = OrignalAddress( MailSlotFileHandle, DesiredAccess, &ObjAtr, IoStatusBlock, FileAttributes, ShareAccess, MaxMessageSize, TimeOut );

	kfree( pNewName );

_over_ :
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return status;
}


ULONG WINAPI
fake_NtReadFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL,
    IN PVOID UserApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
	)
{
	RPC_IN_NtReadFile pInBuffer ;
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPRPC_OUT_NtReadFile pOutBuffer = NULL ;
	_NtReadFile_ OrignalAddress = (_NtReadFile_) GetFileFunc(NtReadFile) ;

	if ( (FALSE == g_FileHookInitOK) || (0xFFFFFF00 != ((ULONG)FileHandle & 0xFFFFFF00)) )
		return OrignalAddress( FileHandle, Event, UserApcRoutine, UserApcContext, IoStatusBlock, Buffer, BufferLength, ByteOffset, Key );

	hFile = (HANDLE)g_FileHandle_Arrays[ (UCHAR)FileHandle & 0xFF - 1 ];
	if ( NULL == hFile ) { return STATUS_INVALID_HANDLE; }

	pInBuffer.RpcHeader.DataLength = sizeof( RPC_IN_NtReadFile ) ;
	pInBuffer.RpcHeader.Flag = 0x1504 ;
	pInBuffer.hFile = hFile;
	pInBuffer.BufferLength = BufferLength ;

	pOutBuffer = (LPRPC_OUT_NtReadFile) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer ) { return STATUS_INSUFFICIENT_RESOURCES; }

	status = pOutBuffer->RpcHeader.u.Status ;
	if ( pOutBuffer->RpcHeader.ReturnLength > 8 )
	{
		IoStatusBlock->Pointer = pOutBuffer->IoStatusBlock_Pointer ;
		IoStatusBlock->Information = pOutBuffer->IoStatusBlock_Information ;
		memcpy( Buffer, pOutBuffer->Buffer, pOutBuffer->BufferLength );
	}

	PB_FreeReply( pOutBuffer );
	if ( Event ) { SetEvent( Event ); }
	
	return status ;
}


ULONG WINAPI
fake_NtWriteFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset,
    IN PULONG Key OPTIONAL
	)
{
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPRPC_IN_NtWriteFile pInBuffer = NULL ;
	LPRPC_OUT_NtWriteFile pOutBuffer = NULL ;
	_NtWriteFile_ OrignalAddress = (_NtWriteFile_) GetFileFunc(NtWriteFile) ;

	if ( (FALSE == g_FileHookInitOK) || (0xFFFFFF00 != ((ULONG)FileHandle & 0xFFFFFF00)) )
		return OrignalAddress( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, Buffer, Length, ByteOffset, Key );

	pInBuffer = (LPRPC_IN_NtWriteFile) kmalloc( Length + sizeof(RPC_IN_NtWriteFile) );
	if ( NULL == pInBuffer ) { return STATUS_INSUFFICIENT_RESOURCES; }

	hFile = (HANDLE)g_FileHandle_Arrays[ (UCHAR)FileHandle & 0xFF - 1 ];
	if ( NULL == hFile ) 
	{ 
		kfree( pInBuffer );
		return STATUS_INVALID_HANDLE;
	}

	pInBuffer->RpcHeader.DataLength = Length + sizeof(RPC_IN_NtWriteFile) ;
	pInBuffer->RpcHeader.Flag = _PBSRV_APINUM_NtWriteFile_ ;
	pInBuffer->hFile = hFile ;
	pInBuffer->Length = Length ;

	memcpy( pInBuffer->Buffer, Buffer, Length );

	pOutBuffer = (LPRPC_OUT_NtWriteFile) PB_CallServer( pInBuffer );
	kfree( pInBuffer );
	if ( NULL == pOutBuffer ) { return STATUS_INSUFFICIENT_RESOURCES; }

	status = pOutBuffer->RpcHeader.u.Status ;
	if ( pOutBuffer->RpcHeader.ReturnLength > 8 )
	{
		IoStatusBlock->Pointer = pOutBuffer->IoStatusBlock_Pointer ;
		IoStatusBlock->Information = pOutBuffer->IoStatusBlock_Information ;
	}

	PB_FreeReply( pOutBuffer );
	if ( Event ) { SetEvent( Event ); }
	return status ;
}


ULONG WINAPI
fake_NtFsControlFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferSize,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferSize
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	WCHAR OrignalPath[ MAX_PATH ] = L"" ;
	WCHAR RedirectedPath[ MAX_PATH ] = L"" ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	_NtFsControlFile_ OrignalAddress = (_NtFsControlFile_) GetFileFunc(NtFsControlFile) ;

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize );

	if ( ((ULONG)FileHandle & 0xFFFFFF00) == 0xFFFFFF00 )
	{
		if ( IoControlCode != FSCTL_PIPE_TRANSCEIVE ) { return STATUS_INVALID_PARAMETER; }

		if ( InputBuffer && InputBufferSize )
		{
			status = fake_NtWriteFile( FileHandle, 0, 0, 0, IoStatusBlock, InputBuffer, InputBufferSize, 0, 0 );
			if ( status < 0 ) { return status; }
		}
		else
		{
			status = IoControlCode;
		}

		if ( OutputBuffer && OutputBufferSize )
		{
			status = fake_NtReadFile( FileHandle, 0, 0, 0, IoStatusBlock, OutputBuffer, OutputBufferSize, 0, 0 );
			if ( status < 0 ) { return status; }
		}
	
		if ( Event ) { SetEvent( Event ); }	
	}
	else
	{
		BOOL bCallOrig = FALSE ;

		if ( FSCTL_SET_REPARSE_POINT == IoControlCode ) 
		{
			status = NtFsControlFileFilter( FileHandle, (PREPARSE_DATA_BUFFER)InputBuffer );
			if ( STATUS_BAD_INITIAL_PC == status ) { bCallOrig = TRUE ;  }
		}
		else
		{ 
			bCallOrig = TRUE ; 
		}
		
		if ( FALSE == bCallOrig )  { return status ; }
		
		return OrignalAddress( FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize );
	}

	return status ;
}


ULONG WINAPI
fake_NtQueryVolumeInformationFile (
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
	)
{
	HANDLE hFile = NULL ;
	LPWSTR szPath = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING uniObjectName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	BOOL bIsHandlerSelfFilePath = FALSE ;
	_NtQueryVolumeInformationFile_ OrignalAddress = (_NtQueryVolumeInformationFile_) GetFileFunc(NtQueryVolumeInformationFile) ;

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( FileHandle, IoStatusBlock, FsInformation, Length, FsInformationClass );

	hFile = FileHandle ;
	szPath = (LPWSTR) kmalloc( 0x2000 );
	status = PB_GetHandlePath( FileHandle, szPath, &bIsHandlerSelfFilePath );

	if ( bIsHandlerSelfFilePath && (status >= 0 || STATUS_BAD_INITIAL_PC == status) )
	{
		status = PB_GetHandlePath( FileHandle, szPath, NULL );
		
		if ( NT_SUCCESS(status) )
		{
			wcscat( szPath, L"\\" );
			RtlInitUnicodeString( &uniObjectName, szPath );

			InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

			status = g_NtCreateFile_addr( &hFile, 0x80100000, &ObjAtr, IoStatusBlock, 0, 0, 7, 1, 0x20, 0, 0 );
			if ( !NT_SUCCESS(status) ) { hFile = FileHandle ; }
		}
	}

	kfree( szPath );

	status = OrignalAddress( hFile, IoStatusBlock, FsInformation, Length, FsInformationClass );

	if ( hFile != FileHandle ) { g_NtClose_addr( hFile ); }
	return status ;
}


ULONG WINAPI
fake_WaitNamedPipeW (
	IN LPCWSTR lpNamedPipeName,
	IN DWORD nTimeOut
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG PipeNameLength = 0 ;
	LPWSTR lpNamedPipeNameDummy = NULL ;
	LPWSTR LpcRootPath = NULL, ptr = NULL, Buffer = NULL, i = NULL ;
	WCHAR Data1, Data2 ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	_WaitNamedPipeW_ OrignalAddress = (_WaitNamedPipeW_) GetFileFunc(WaitNamedPipeW) ;

	if ( FALSE == g_FileHookInitOK )
		return OrignalAddress( lpNamedPipeName, nTimeOut );

	lpNamedPipeNameDummy = (LPWSTR) lpNamedPipeName ;

	// 1. 只关注名字为"\\\\.\\pipe\\*"的管道
	if ( NULL == lpNamedPipeName ) { goto _CallOrig_ ; }

	PipeNameLength = wcslen( lpNamedPipeName );
	if ( PipeNameLength <= 9 ) { goto _CallOrig_ ; }

	if ( wcsnicmp( lpNamedPipeName, L"\\\\.\\pipe\\", 9 ) ) { goto _CallOrig_ ; }
	
	// 2. 和沙箱的LpcRootPath进行对比
	ptr = LpcRootPath = g_BoxInfo.LpcRootPath ;
	Data1 = *ptr ;

	if ( ! Data1 ) { goto _CallOrig_ ; }

	while ( TRUE )
	{
		Data2 = *(WCHAR*)( (char*)(lpNamedPipeName + 9) + ((char*)ptr - (char*)LpcRootPath) );
		
		if ( Data2 != Data1 )
		{
			if ( Data2 != '_' || Data1 != '\\' ) { break ; }
		}

		++ ptr ;
		Data1 = *ptr ;

		if ( ! Data1 ) { goto _CallOrig_ ; }
	}

	if ( ! *ptr ) { goto _CallOrig_ ; }

	// 3.1 申请内存,拼接填充新路径
	Buffer = (LPWSTR) kmalloc( 2 * (PipeNameLength + g_BoxInfo.LpcRootPathLength) + 0x40 );

	wcscpy( Buffer, L"\\device\\namedpipe\\" );
	wcscat( Buffer, lpNamedPipeName + 9 );

	// 3.2 过黑白名单
	WhiteOrBlackEx( Buffer, &bIsWhite, &bIsBlack );

	if ( bIsWhite || bIsBlack ) { goto _CallOrig_ ; }

	// 3.3 处理灰名单
	wcsncpy( Buffer, lpNamedPipeName, 9 );

	ptr = g_BoxInfo.LpcRootPath ;
	Data1 = *ptr ;

	for ( i = Buffer + 9; Data1; ++i )
	{
		if ( Data1 == '\\' )
			*i = '_' ;
		else
			*i = Data1 ;
		
		++ ptr ;
		Data1 = *ptr ;
	}

	wcscpy( i, lpNamedPipeName + 8 );
	lpNamedPipeNameDummy = Buffer ;

_CallOrig_ :
	status = OrignalAddress( lpNamedPipeNameDummy, nTimeOut );
	
	if ( Buffer ) { kfree( Buffer ) ; }
	return status;
}


ULONG WINAPI
fake_RtlGetCurrentDirectory_U (
    ULONG MaximumLength,
    PWSTR Buffer
	)
{
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR szCurrentDirectory = NULL, CurrentDirectoryRedirected = NULL ;
	ULONG ret, CurrentDirectoryLength, CurrentDirectoryRedirectedLength, CurrentDirectoryRedirectedLengthLittle ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bFlag = FALSE, bAlreadyRedirected = FALSE ;
	_RtlGetCurrentDirectory_U_ OrignalAddress = (_RtlGetCurrentDirectory_U_) GetFileFunc(RtlGetCurrentDirectory_U) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( FALSE == g_FileHookInitOK ) { return OrignalAddress( MaximumLength, Buffer ); }

	if ( pNode->sFileLock.nLockRtlGetCurrentDirectory ) 
		return OrignalAddress( MaximumLength, Buffer );

	// 1. 获取原始的当前目录的全路径
	szCurrentDirectory = (LPWSTR) kmalloc( 0x2000 );
	if ( OrignalAddress( 0x1FFE, szCurrentDirectory ) > 0x1FFE )
	{
		kfree( szCurrentDirectory );
		return OrignalAddress( MaximumLength, Buffer );
	}

	pNode->sFileLock.nLockRtlGetCurrentDirectory = 1 ;
	EnterCriticalSection( &g_Lock_RtlGetCurrentDirectory );

	if ( g_CurrentDirectory_Orignal && 0 == wcsicmp(szCurrentDirectory, g_CurrentDirectory_Orignal) )
	{
		
	}
	else
	{
		// 释放旧的备份路径
		if ( g_CurrentDirectory_Redirected ) { kfree( g_CurrentDirectory_Redirected ); }
		if ( g_CurrentDirectory_Orignal )    { kfree( g_CurrentDirectory_Orignal ); }

		g_CurrentDirectory_Redirected	= NULL ;
		g_CurrentDirectory_Orignal		= NULL ;

		// 备份新的原始路径
		CurrentDirectoryLength = wcslen( szCurrentDirectory ) + 1 ;
		g_CurrentDirectory_Orignal = (LPWSTR) kmalloc( 2 * CurrentDirectoryLength );
		memcpy( g_CurrentDirectory_Orignal, szCurrentDirectory, 2 * CurrentDirectoryLength );

		if ( CurrentDirectoryLength >= 2 && '\\' == szCurrentDirectory[CurrentDirectoryLength - 2] )
		{
			bFlag = TRUE;
		}

		hFile = CreateFileW( szCurrentDirectory, GENERIC_READ, 7, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0 );
		
		if ( INVALID_HANDLE_VALUE != hFile )
		{
			status = PB_GetHandlePath( hFile, szCurrentDirectory, NULL );
			if ( NT_SUCCESS(status) && PB_TranslateNtToDosPath( szCurrentDirectory )  )
			{
				bAlreadyRedirected = TRUE ;
			}

			CloseHandle( hFile );

			if ( bAlreadyRedirected )
			{
				CurrentDirectoryRedirectedLengthLittle = wcslen(szCurrentDirectory) + 1 ;

				if ( CurrentDirectoryRedirectedLengthLittle >= 2 && '\\' == szCurrentDirectory[CurrentDirectoryRedirectedLengthLittle - 2] )
				{
					bFlag = FALSE;
				}

				CurrentDirectoryRedirected = (LPWSTR) kmalloc( 2 * (CurrentDirectoryRedirectedLengthLittle + 2) );
				CurrentDirectoryRedirectedLength = 2 * CurrentDirectoryRedirectedLengthLittle ;

				memcpy( CurrentDirectoryRedirected, szCurrentDirectory, CurrentDirectoryRedirectedLength );

				g_CurrentDirectory_Redirected = CurrentDirectoryRedirected ;

				if ( bFlag )
				{
					*(DWORD *)( (char *)CurrentDirectoryRedirected + CurrentDirectoryRedirectedLength - 2 ) = '\\' ;
				}
			}
		}
	}

	if ( FALSE == bAlreadyRedirected ) { CurrentDirectoryRedirected = g_CurrentDirectory_Redirected ; }
		
	if ( CurrentDirectoryRedirected )
	{
		ret = 2 * wcslen( CurrentDirectoryRedirected ) + 2 ;
		if ( MaximumLength >= ret )
		{
			memcpy( Buffer, g_CurrentDirectory_Redirected, ret );
			ret -= 2 ;
		}

		LeaveCriticalSection( &g_Lock_RtlGetCurrentDirectory );
	}
	else
	{
		LeaveCriticalSection( &g_Lock_RtlGetCurrentDirectory );
		ret = OrignalAddress( MaximumLength, Buffer );
	}

	kfree( szCurrentDirectory );
	pNode->sFileLock.nLockRtlGetCurrentDirectory = 0 ;
	return ret ;
}


ULONG WINAPI
fake_RtlSetCurrentDirectory_U (
    IN PUNICODE_STRING name
	)
{
	int ret = 0 ;
	LPWSTR lpBuffer = NULL ;
	UNICODE_STRING uniName ;
	_RtlSetCurrentDirectory_U_ OrignalAddress = (_RtlSetCurrentDirectory_U_) GetFileFunc(RtlSetCurrentDirectory_U) ;

	if ( FALSE == g_FileHookInitOK ) { return OrignalAddress( name ); }

	ret = OrignalAddress( name );
	if ( ret < 0 ) { return ret; }

	lpBuffer = (LPWSTR) kmalloc( 0x2000);
	if ( fake_RtlGetCurrentDirectory_U( 0x1FFE, lpBuffer) <= 0x1FFE )
	{
		lpBuffer[4095] = 0 ;

		RtlInitUnicodeString( &uniName, lpBuffer );
		ret = OrignalAddress( &uniName );
	}

	return ret; 
}


ULONG WINAPI
fake_RtlGetFullPathName_U (
    IN PCWSTR FileName,
    IN ULONG Size,
    IN PWSTR Buffer,
    OUT PWSTR *ShortName
	)
{
	LPWSTR szFileName = (LPWSTR) FileName ; 
	_RtlGetFullPathName_U_ OrignalAddress = (_RtlGetFullPathName_U_) GetFileFunc(RtlGetFullPathName_U) ;

	if ( FALSE == g_FileHookInitOK ) { return OrignalAddress( FileName, Size, Buffer, ShortName ); }

	if ( g_CurrentDirectory_Redirected && *FileName == *g_CurrentDirectory_Redirected )
	{	
		if ( ':' == FileName[1] && '.' == FileName[2] && !FileName[3] )
		{
			szFileName = g_CurrentDirectory_Redirected ;
		}
	}

	return OrignalAddress( szFileName, Size, Buffer, ShortName );
}


ULONG WINAPI
fake_MoveFileWithProgressW (
	LPCWSTR			lpExistingFileName,
	LPCWSTR			lpNewFileName,
	LPPROGRESS_ROUTINE	lpProgressRoutine,
	LPVOID			lpData,
	DWORD			dwFlags
	)
{
	DWORD dwFlagsDummy = dwFlags ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR FuckedKey = L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\PBHideKnownDlls" ;
	_MoveFileWithProgressW_ OrignalAddress = (_MoveFileWithProgressW_) GetFileFunc(MoveFileWithProgressW) ;

	if ( FALSE == g_FileHookInitOK ) { return OrignalAddress( lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags ); }

	if ( dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT )
	{
		WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, FuckedKey, &bIsWhite, &bIsBlack );

		if ( FALSE == bIsWhite && FALSE == bIsBlack )
		{
			RtlCreateRegistryKey( 0, FuckedKey ); // 在灰名单中,创建该键值
		}

		ClearFlag( dwFlagsDummy, MOVEFILE_DELAY_UNTIL_REBOOT );
	}

	return OrignalAddress( lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlagsDummy | MOVEFILE_COPY_ALLOWED );
}

///////////////////////////////   END OF FILE   ///////////////////////////////