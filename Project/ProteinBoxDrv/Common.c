/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/23 [23:5:2010 - 1:55]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\Common.c
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "Version.h"

//////////////////////////////////////////////////////////////////////////

ULONG g_ImageFileName_Offset = 0 ;



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
GetEProcessByName_QueryInfo (
  IN  WCHAR* processname, 
  OUT PVOID* proc
  )
/*++

逆向: sudami  08/02/28

参数:
  processname - 进程名
  proc - [OUT] 进程的EPROCESS

功能:
  原流程 --> 通过ZwQuerySystemInformation查询,得到匹配的进程ID.调用
  PsLookupProcessByProcessId得到该进程的EPROCESS.

  现流程 --> 通过ZwQuerySystemInformation查询,得到匹配的进程ID.调用
  ZwOpenProcess得到其句柄, 通过ObReferenceObjectByHandle得到EPROCESS

--*/
{
	NTSTATUS		status ;
	ULONG			info_size = PAGE_SIZE ;
	ULONG			result_size, length ;
	ULONG			ProcessId = 0 ;
	PVOID			Object ;
	HANDLE			hProcess ;
	CLIENT_ID		ClientId ;
	OBJECT_ATTRIBUTES  ObjectAttributes ;
	PSYSTEM_PROCESS_INFORMATION info, curr ;
	
	*proc = NULL ;
	
	//
	// 不断的分配内存,直到调用成功
	//

	while ( TRUE ) 
	{
		info = kmalloc ( info_size );
		if ( NULL == info ) { return STATUS_NO_MEMORY ; }
		
		status = ZwQuerySystemInformation( SystemProcessInformation, info, info_size, &result_size );  
		if( NT_SUCCESS(status) ) { break ; }
		
		if( status != STATUS_INFO_LENGTH_MISMATCH ) { return STATUS_NO_MEMORY ; }
			
		kfree( (PVOID)info ) ;
		info = NULL ;
		info_size += PAGE_SIZE ; 
	}
    
	length = wcslen(processname);
	curr = info;
	
	do 
	{
		if(    ( length * sizeof (WCHAR) == curr->ImageName.Length )
			&& ( 0 == _wcsicmp( processname, curr->ImageName.Buffer ) ) 
			)
		{
			ProcessId = (ULONG)curr->UniqueProcessId ;
			break;
		}
		
		if( curr->NextEntryOffset ) {
			(PBYTE)curr  += (curr->NextEntryOffset);
		}
		
	} while( curr->NextEntryOffset ); 
	
	kfree( (PVOID)info ) ;	
	
	if ( 0 == ProcessId ) { return STATUS_NOT_FOUND ; }
	
	InitializeObjectAttributes( &ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );
	
	ClientId.UniqueProcess = (HANDLE) ProcessId ;
	ClientId.UniqueThread  = 0 ;
	
	// 通过ID得到句柄
	status = ZwOpenProcess( &hProcess, PROCESS_ALL_ACCESS, &ObjectAttributes, &ClientId );
	if ( !NT_SUCCESS(status) ) { return status ; }
	
	// 通过句柄得到EPROCESS
	status = ObReferenceObjectByHandle( hProcess, PROCESS_ALL_ACCESS, NULL, KernelMode, &Object, NULL );
	
	ZwClose ( hProcess );
	if (!NT_SUCCESS(status)){ return status ; }
	
	ObDereferenceObject( Object ) ;
	*proc = Object ;
	return STATUS_SUCCESS ;
}



VOID 
NtPath2DosPathA (
	IN LPCSTR szNtPath,
	OUT LPSTR szDosPath,
	IN DWORD cchSize
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 15:11]

Routine Description:
  将nt路径转换为Dos路径;即 "\Device\HarddiskVolume1\xx" --> "c:\xx"
  可参考WIN2k_Src中的 BasepGetComputerNameFromNtPath
    
Arguments:
  szNtPath - [IN] 待转换的字符串; eg:"\Device\HarddiskVolume1\xx"
  szDosPath - [OUT] 保存转换后的字符串; eg:"c:\xx"
  cchSize - [IN] 指定@szDosPath的最大值

--*/
{
	NTSTATUS		status = STATUS_UNSUCCESSFUL  ;
	WCHAR			wszDosPath[ MAX_PATH ] = L""  ;
	UNICODE_STRING	uni_szNtPath,  uni_szDosPath  ;
	ANSI_STRING		ansi_szNtPath, ansi_szDosPath ;
	
	//
	// 1. 将待转换的NT路径对应的字符串 转换为UNI格式,新申请内存
	//

	RtlInitAnsiString( &ansi_szNtPath, szNtPath );
	
	status = RtlAnsiStringToUnicodeString( &uni_szNtPath, &ansi_szNtPath, TRUE );
	if ( !NT_SUCCESS(status) ) 
	{
		dprintf( "error! | NtPath2DosPathA() - RtlAnsiStringToUnicodeString(); 1 \n" );
		return ;
	}
	
	//
	// 2. 调用之
	//

	NtPath2DosPathW( uni_szNtPath.Buffer, wszDosPath, cchSize );

	//
	// 3. 将得到的结果转换为ANSI
	//

	RtlInitUnicodeString( &uni_szDosPath, wszDosPath );
	
	status = RtlUnicodeStringToAnsiString(
		&ansi_szDosPath,
		&uni_szDosPath,
		TRUE
		);
	
	if ( !NT_SUCCESS(status) ) 
	{
		dprintf( "error! | NtPath2DosPathA() - RtlUnicodeStringToAnsiString(); 2 \n" );
		RtlFreeUnicodeString( &uni_szNtPath ); // 释放内存
		return ;
	}

	strncpy( szDosPath, ansi_szDosPath.Buffer, cchSize );
	
	//
	// 4. 收尾工作
	//

	RtlFreeAnsiString	( &ansi_szDosPath	);  // 释放内存
	RtlFreeUnicodeString( &uni_szNtPath		);  // 释放内存

	return ;
}



VOID 
NtPath2DosPathW (
	IN LPCWSTR szNtPath,
	OUT LPWSTR szDosPath,
	IN DWORD cchSize
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 15:11]

Routine Description:
  将nt路径转换为Dos路径;即 "\Device\HarddiskVolume1\xx" --> "c:\xx"
  可参考WIN2k_Src中的 BasepGetComputerNameFromNtPath
    
Arguments:
  szNtPath - [IN] 待转换的字符串; eg: L"\Device\HarddiskVolume1\xx"
  szDosPath - [OUT] 保存转换后的字符串; eg: L"c:\xx"
  cchSize - [IN] 指定@szDosPath的最大值

--*/
{
	ULONG ul					 = 0	 ;
	WCHAR DeviceName[ MAX_PATH ] = L""	 ;
	WCHAR VolumeName[ 3 ]		 = L"A:" ;
	const WCHAR LanmanName[]	 = L"\\Device\\LanmanRedirector" ; 
	const WCHAR MupName[]		 = L"\\Device\\Mup"				 ;
	const WCHAR MupName2[]		 = L"\\Device\\Mup\\;LanmanRedirector" ;
	const WCHAR MupName3[]		 = L"\\\\" ;
	const WCHAR UNCName[]		 = L"\\??\\UNC"					 ;

	if( 0 == _wcsnicmp( szNtPath, UNCName, ARRAYSIZEOF(UNCName)-1 ) )
	{
		wcscpy( szDosPath, L"\\" );
		wcsncat( szDosPath, szNtPath + (ARRAYSIZEOF(UNCName) - 1), cchSize - 1 );
		
		return ;
	} 
	else if( 0 == _wcsnicmp( szNtPath, L"\\??\\", 4 ) )
	{
		wcsncat( szDosPath, szNtPath + 4, cchSize );
		return ;
	}

	if( 0 == _wcsnicmp( szNtPath, L"\\systemroot\\", 12) )
	{
		
	}

	while( TRUE )
	{
		if ( QueryDosDeviceW( VolumeName , DeviceName, MAX_PATH ) )
		{
			if ( 0 == _wcsnicmp(szNtPath, DeviceName, wcslen(DeviceName)) )
			{
				ul = wcslen(DeviceName) * sizeof(WCHAR) ;

				wcscpy( szDosPath, VolumeName );
				wcsncat( szDosPath, (wchar_t*)((ULONG)szNtPath + ul), cchSize - ul );

				return ;
			}
		}

		if ( L'Z' == VolumeName[0] ) { break; }

		VolumeName[0] ++;
	}

	//
	// 处理UNC路径
	//

	if ( 0 == _wcsnicmp( szNtPath, LanmanName, (sizeof(LanmanName) - sizeof(WCHAR))/sizeof(WCHAR) ) )
	{
		wcscpy( szDosPath, L"\\" );
		wcsncat( szDosPath, (wchar_t*)((ULONG)szNtPath + sizeof(LanmanName) - sizeof(WCHAR)), cchSize - 1 );

		return ;
	}
	else if ( 0 == _wcsnicmp( szNtPath, MupName, (sizeof(MupName) - sizeof(WCHAR))/sizeof(WCHAR) ) )
	{
		wcscpy( szDosPath, L"\\" );
		wcsncat( szDosPath, (wchar_t*)((ULONG)szNtPath + sizeof(MupName) - sizeof(WCHAR)), cchSize - 1 );

		return ;
	}
	else if ( 0 == _wcsnicmp( szNtPath, MupName2, (sizeof(MupName2) - sizeof(WCHAR))/sizeof(WCHAR) ) )
	{
		wcscpy( szDosPath, L"\\" );
		wcsncat( szDosPath, (wchar_t*)((ULONG)szNtPath + sizeof(MupName2) - sizeof(WCHAR)), cchSize - 1 );

		return ;
	}
	else if ( 0 == _wcsnicmp( szNtPath, MupName3, (sizeof(MupName3) - sizeof(WCHAR))/sizeof(WCHAR) ) )
	{
		wcscpy( szDosPath, L"\\" );
		wcsncat( szDosPath, (wchar_t*)((ULONG)szNtPath + sizeof(MupName3) - sizeof(WCHAR)), cchSize - 1 );

		return ;
	}

	RtlCopyMemory( szDosPath, szNtPath, cchSize );
	return ;
	
	//
	// 还有其他很多格式没处理,将就着先写这么多吧  \(^o^)/
	//
}



DWORD
QueryDosDeviceW(
	IN LPCWSTR lpDeviceName,
	OUT LPWSTR lpTargetPath,
	IN DWORD ucchMax
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 14:58]

Routine Description:
  查询符号名对应的设备名,返回该字符串长度
    
Arguments:
  lpDeviceName - [IN] 待查询的符号名; eg: L"C:"
  lpTargetPath - [OUT] 保存符号名对应的设备名; eg: L"\\Device\\HarddiskVolume1\\"
  ucchMax - [IN] 指定@lpTargetPath的最大值

Return Value:
  @lpTargetPath的长度; 0 表示查询失败
    
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL	;
	OBJECT_ATTRIBUTES	ObjectAttributes	;
	UNICODE_STRING		UnicodeString		;
	HANDLE DirectoryHandle, DeviceHandle	;
	ULONG ReturnLength	= 0 ;
	ULONG Length		= 0 ;
	
	if ( NULL == lpDeviceName ) 
	{
		dprintf( "error! | QueryDosDeviceW(); Invalid Paramaters; \n" );
		return 0; 
	}

	//
	// Open the '\??' directory
	//

	RtlInitUnicodeString ( &UnicodeString, L"\\??" );
	
	InitializeObjectAttributes (
		&ObjectAttributes,
		&UnicodeString,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

	status = ZwOpenDirectoryObject( &DirectoryHandle,DIRECTORY_QUERY, &ObjectAttributes );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "ZwOpenDirectoryObject() failed (status 0x%08lx) \n", status );
		return 0 ;
	}

	//
	// Open the lpDeviceName link object
	//

	RtlInitUnicodeString ( &UnicodeString, (PWSTR)lpDeviceName );
	
	InitializeObjectAttributes (
		&ObjectAttributes,
		&UnicodeString,
		OBJ_CASE_INSENSITIVE,
		DirectoryHandle,
		NULL
		);

	status = ZwOpenSymbolicLinkObject (
		&DeviceHandle,
		SYMBOLIC_LINK_QUERY,
		&ObjectAttributes
		);

	if ( !NT_SUCCESS (status) )
	{
#if 0
		dprintf( "NtOpenSymbolicLinkObject() failed \"%ws\" (status %lx)\n", lpDeviceName, Status );
#endif
		ZwClose( DirectoryHandle );
		return 0 ;
	}

	//
	// Query link target
	//

	UnicodeString.Length		= 0 ;
	UnicodeString.MaximumLength = (USHORT) ucchMax * sizeof( WCHAR ) ;
	UnicodeString.Buffer		= lpTargetPath ;

	status = ZwQuerySymbolicLinkObject ( DeviceHandle, &UnicodeString, &ReturnLength );
	ZwClose( DeviceHandle );
	ZwClose( DirectoryHandle );

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "NtQuerySymbolicLinkObject() failed (status %lx) \n", status );
		return 0 ;
	}

	Length = ReturnLength / sizeof( WCHAR ) ;
	if ( Length < ucchMax )
	{
		lpTargetPath[ Length ] = UNICODE_NULL ; // Append null-charcter
		Length ++ ;
	}
	else
	{
		dprintf( "error! | QueryDosDeviceW(); Buffer is too small \n" );
		return 0 ;
	}

	return Length ;
}



NTSTATUS 
PutFile( 
	IN WCHAR* filename,
	IN CHAR* buffer, 
	IN ULONG buffersize
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2008/12/07 [7:12:2008 - 8:44]

Routine Description:
  释放内存中的数据,写入文件到指定路径
    
Arguments:

  filename - 文件全路径/文件名
  buffer - 文件内容的指针
  buffersize - 文件大小
    
--*/
{
	NTSTATUS			status;
	HANDLE				FileHandle;
	OBJECT_ATTRIBUTES	ObjectAttr;
	UNICODE_STRING		FileName;
	IO_STATUS_BLOCK		ioStatusBlock;
	
	RtlInitUnicodeString( &FileName, filename );

	InitializeObjectAttributes ( 
		&ObjectAttr,
		&FileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);
	
	status = ZwCreateFile (
		&FileHandle,
		FILE_ALL_ACCESS,
		&ObjectAttr,
		&ioStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OVERWRITE_IF,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | PutFile() - ZwCreateFile() failed (status %lx) \n", status );
		return status;
	}
	
	status = ZwWriteFile (
		FileHandle,
		NULL,
		NULL,
		NULL,
		&ioStatusBlock,
		buffer,
		buffersize,
		NULL,
		NULL );
	
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | PutFile() - ZwWriteFile() failed (status %lx) \n", status );
		ZwClose( FileHandle );
		return status;
	}
	
	ZwClose( FileHandle );
	return STATUS_SUCCESS ;
}



BOOL
OpenEvent (
	IN LPWSTR lpEventName,
	IN ACCESS_MASK DesiredAccess,
	OUT PHANDLE hEvent
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/30 [30:6:2010 - 13:51]

Routine Description:
  根据@lpEventName,打开指定的事件对象,获得相应句柄    
    
Arguments:
  lpEventName - 对象名
  DesiredAccess - 期望的权限
  hEvent - 保存返回的句柄
    
--*/
{
	UNICODE_STRING EventName ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if ( (NULL == lpEventName) || (NULL == hEvent) )
	{
		dprintf( "error! | OpenEvent(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	RtlInitUnicodeString( &EventName, lpEventName );
	InitializeObjectAttributes( &ObjectAttributes, &EventName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );

	status = ZwOpenEvent(
		hEvent,
		EVENT_MODIFY_STATE,
		&ObjectAttributes
		);

	if( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | OpenEvent(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	return TRUE ;
}



BOOL
SetEvent (
	IN LPWSTR lpEventName,
	IN ACCESS_MASK DesiredAccess
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/30 [30:6:2010 - 13:51]

Routine Description:
  根据@lpEventName,打开指定的事件对象,设置为受信状态
    
Arguments:
  lpEventName - 对象名
  DesiredAccess - 期望的权限
    
--*/
{
	BOOL bRet = FALSE ;
	HANDLE hEvent = NULL ;

	if ( NULL == lpEventName )
	{
		dprintf( "error! | SetEvent(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	bRet = OpenEvent( lpEventName, DesiredAccess, &hEvent );
	if( FALSE == bRet )
	{
		dprintf( "error! | SetEvent() - OpenEvent(); | \"%s\" \n", lpEventName );
		return FALSE ;
	}

	ZwSetEvent( hEvent, 0 );
	ZwClose( hEvent );

	return TRUE ;
}



BOOL
w2a (
	IN LPWSTR pInBuffer,
	OUT LPSTR pOutBuffer,
	IN ULONG MaxLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 11:11]

Routine Description:
  将unicode转换为  ansi
    
Arguments:
  pInBuffer - UNICODE
  pOutBuffer - ANSI
  MaxLength - 字符串的最大长度
    
--*/
{
	ANSI_STRING ArcString = {0} ;
	UNICODE_STRING FileName = {0} ;
	NTSTATUS Status = STATUS_SUCCESS ;

	// 1. 校验参数合法性
	if ( NULL == pInBuffer || NULL == pOutBuffer || MaxLength <= 0 )
	{
		dprintf( "error! | w2a(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}
	
	// wchar -> unicode
	RtlInitUnicodeString( &FileName, pInBuffer );

	// Convert it to Ansi
	ArcString.Buffer = pOutBuffer;
	ArcString.Length = 0;
	ArcString.MaximumLength = (USHORT)MaxLength;

	Status = RtlUnicodeStringToAnsiString( &ArcString, &FileName, FALSE );
	if (!NT_SUCCESS(Status)) 
	{
		dprintf( "error! | w2a() - RtlUnicodeStringToAnsiString(); |  \n" );
		return FALSE ;
	}

	pOutBuffer[ ArcString.Length ] = ANSI_NULL;
	return TRUE;
}


BOOL
a2w (
	IN LPSTR pInBuffer,
	OUT LPWSTR pOutBuffer,
	IN ULONG MaxLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 11:11]

Routine Description:
  将ansi转换为unicode  
    
Arguments:
  pInBuffer - ANSI
  pOutBuffer - UNICODE
  MaxLength - 字符串的最大长度
    
--*/
{
	NTSTATUS Status ;
	UNICODE_STRING FileName;
	ANSI_STRING AnsiString ;
	ULONG length ;

	// 1. 校验参数合法性
	if ( NULL == pInBuffer || NULL == pOutBuffer || MaxLength <= 0 )
	{
		dprintf( "error! | a2w(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// char -> wchar
	RtlInitAnsiString( &AnsiString, pInBuffer );

	Status = RtlAnsiStringToUnicodeString( &FileName, &AnsiString, TRUE );
	if (!NT_SUCCESS(Status)) 
	{
		dprintf( "error! | a2w() - RtlAnsiStringToUnicodeString(); |  \n" );
		return FALSE ;
	}

	length = MaxLength < FileName.Length / sizeof(WCHAR) ? MaxLength : FileName.Length ;
	RtlCopyMemory( pOutBuffer, FileName.Buffer, length );

	pOutBuffer[ length / sizeof(WCHAR) ] = UNICODE_NULL ;

	RtlFreeUnicodeString( &FileName );
	return TRUE ;
}


BOOL InitResource( IN PERESOURCE* rc )
{
	// 初始化资源锁
	*rc = (PERESOURCE) kmalloc( sizeof(ERESOURCE) );
	if ( NULL == *rc )
	{
		dprintf( "error! | InitResource(); 申请资源锁内存失败! \n" );
		return FALSE ;
	}

	ExInitializeResource( *rc );
	return TRUE ;
}

VOID EnterCrit( IN PERESOURCE rc ) 
{
	// 获取锁
	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite( rc, TRUE );
	return ;
}

VOID LeaveCrit( IN PERESOURCE rc ) 
{
	// 释放锁
	ExReleaseResource( rc );
	KeLeaveCriticalRegion();
	return ;
}



BOOL
MyRtlCompareUnicodeString (
	IN LPName_Info StringA, 
	IN LPWSTR StringB,
	IN ULONG MaximumLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/12/23 [23:12:2011 - 23:31]

Routine Description:
  比较2个unicode_string是否相同; 不管字符串多长,只比较@MaximumLength大小的长度.
    
Arguments:
  StringA - 字符串1 eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox" 
  StringB - 字符串2 eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\drive\D\sudami.txt"
  MaximumLength - 字符串最大长度

Return Value:
  1 -- 字符串相同; 0 -- 字符串不同; 
    
--*/
{
	WCHAR data = 0 ;
	ULONG Length = 0 ;
	LPWSTR ptr = NULL ;

	if ( StringA->NameInfo.Name.Length < MaximumLength - 2 ) { return FALSE; }

	ptr = StringA->NameInfo.Name.Buffer;
	Length = (MaximumLength - 2) / sizeof(WCHAR);	
	if ( _wcsnicmp(ptr, StringB, Length) ) { return FALSE; }

	data = ptr[ Length ];
	if ( data && '\\' != data ) { return FALSE; }

	return TRUE;
}



NTSTATUS 
QueryNameString (
	IN PVOID DeviceObject, 
	OUT PUNICODE_STRING pNameInfo,
	OUT ULONG *pNameLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/19 [19:7:2010 - 17:14]

Routine Description:
  调用ObQueryNameString得到文件对象的文件名信息. 由调用者释放申请的内存   
    
--*/
{
	ULONG ReturnLength = 0, Size = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	OBJECT_NAME_INFORMATION ObjectNameInfo = { 0 } ;
	POBJECT_NAME_INFORMATION pBuffer = NULL ;

	// 1. 校验参数合法性
	if ( NULL == DeviceObject || NULL == pNameInfo )
	{
		dprintf( "error! | SepQueryNameString(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	status = ObQueryNameString( DeviceObject, &ObjectNameInfo, 0x48, &ReturnLength );
	if ( STATUS_OBJECT_PATH_INVALID == status )
	{
		ReturnLength = 0;
		status = ObQueryNameString( DeviceObject, &ObjectNameInfo, 0x40, &ReturnLength );
	}

	if( NT_SUCCESS( status ) )
	{
		if ( 0 == ObjectNameInfo.Name.Length || NULL == ObjectNameInfo.Name.Buffer ) 
		{
			*pNameLength = _key_unknown_executable_image_for_QueryNameString_;
			return STATUS_SUCCESS;
		}

		Size = ObjectNameInfo.Name.Length + 0x2 ;
		pNameInfo->Buffer = (PWSTR) kmalloc( Size );
		if ( NULL ==  pNameInfo->Buffer ) { return STATUS_INSUFFICIENT_RESOURCES ; }
		RtlZeroMemory( pNameInfo->Buffer, Size );
		
		pNameInfo->Length		= ObjectNameInfo.Name.Length ;
		pNameInfo->MaximumLength	= ObjectNameInfo.Name.MaximumLength ;
		memcpy( pNameInfo->Buffer, ObjectNameInfo.Name.Buffer, ObjectNameInfo.Name.Length );

		*pNameLength = Size;
		return STATUS_SUCCESS ;
	}

	if ( STATUS_INFO_LENGTH_MISMATCH != status && STATUS_BUFFER_OVERFLOW != status ) { return status; }
	
	Size = 0 ;
	if ( ReturnLength )
	{
		ULONG ReturnLengthDummy = 0;
		Size = ReturnLength + 0x10 ;
		pBuffer = (POBJECT_NAME_INFORMATION) kmalloc( Size );
		if ( NULL == pBuffer ) { return STATUS_INSUFFICIENT_RESOURCES ; }
		RtlZeroMemory( pBuffer, Size );

		status = ObQueryNameString( DeviceObject, pBuffer, Size - 8, &ReturnLengthDummy );
		if( ! NT_SUCCESS( status ) )
		{
			kfree( pBuffer );
			return status ; 
		}
	}
	else
	{
		while ( TRUE )
		{
			if ( pBuffer ) { kfree( (PVOID)pBuffer ) ; }

			Size += 0x80 ;
			pBuffer = (POBJECT_NAME_INFORMATION) kmalloc( Size );
			if ( NULL == pBuffer ) { return STATUS_INSUFFICIENT_RESOURCES ; }
			RtlZeroMemory( pBuffer, Size );

			ReturnLength = 0;
			status = ObQueryNameString( DeviceObject, pBuffer, Size - 8, &ReturnLength );
			if( NT_SUCCESS( status ) ) { break ; }

			if ( STATUS_OBJECT_PATH_INVALID == status )
			{
				ReturnLength = 0 ;
				status = ObQueryNameString( DeviceObject, pBuffer, Size - 0x10, &ReturnLength );
			}

			if ( STATUS_INFO_LENGTH_MISMATCH != status && STATUS_BUFFER_OVERFLOW != status )
			{
				kfree( (PVOID)pBuffer ) ;
				return status ;
			}
		}
	}

	if ( 0 == pBuffer->Name.Length || NULL == pBuffer->Name.Buffer ) 
	{ 
		kfree( (PVOID)pBuffer ) ;
		*pNameLength = _key_unknown_executable_image_for_QueryNameString_;
		return STATUS_SUCCESS;
	}

	if ( g_Version_Info.IS___win7 )
	{
		if ( pBuffer->Name.Length >= 4 )
		{
			ULONG length = pBuffer->Name.Length ;
			LPWSTR ptr = pBuffer->Name.Buffer ;

			if ( L'\\' == ptr[0] && L'\\' == ptr[1] )
			{
				memcpy( ptr, ptr + 1, length );
				ptr[ length / sizeof(WCHAR) ] = UNICODE_NULL ;

				pBuffer->Name.Length		-= 2 ;
				pBuffer->Name.MaximumLength -= 2 ;
			}
		}
	}

	pNameInfo->Buffer = (PWSTR) kmalloc( pBuffer->Name.Length + 0x2 );
	if ( NULL ==  pNameInfo->Buffer ) { return STATUS_INSUFFICIENT_RESOURCES ; }
	RtlZeroMemory( pNameInfo->Buffer, pBuffer->Name.Length + 0x2 );

	pNameInfo->Length = pBuffer->Name.Length;
	pNameInfo->MaximumLength = pBuffer->Name.MaximumLength;
	memcpy( pNameInfo->Buffer, pBuffer->Name.Buffer, pBuffer->Name.Length );

	*pNameLength = Size;
	kfree(pBuffer);
	return STATUS_SUCCESS ;
}



PVOID
SepQueryNameString(
    IN PVOID Object,
	OUT BOOL* bHasNoName
    )
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/22 [22:7:2010 - 15:46]

Routine Description:
  通过对象调用ObQueryNameString获取对象名,成功则需要调用者释放内存
    
Arguments:
  Object - 待查询的对象体
  bHasNoName - 有一种情况,查询成功,但该对象本来就没有名字,此时该值置TRUE

Return Value:
  NULL | 查询得到的POBJECT_NAME_INFORMATION结构体指针
    
--*/
{
    NTSTATUS status;
    ULONG ReturnLength = 0;
    POBJECT_NAME_INFORMATION ObjectNameInfo = NULL;
    PUNICODE_STRING ObjectName = NULL;

	// 1. 校验参数合法性
	if ( NULL == Object || NULL == bHasNoName ) { return NULL ; }

	*bHasNoName = FALSE ;

    status = ObQueryNameString( Object, ObjectNameInfo, 0, &ReturnLength );
    if ( status == STATUS_INFO_LENGTH_MISMATCH ) 
	{
		if ( ReturnLength )
		{
			ObjectNameInfo = kmalloc( ReturnLength + sizeof( OBJECT_NAME_INFORMATION ) );
			if ( NULL == ObjectNameInfo ) { return NULL ; }

			status = ObQueryNameString( Object, ObjectNameInfo, ReturnLength, &ReturnLength );
			if ( ! NT_SUCCESS( status )) 
			{
				//	dprintf( "error! | SepQueryNameString() - ObQueryNameString(); | status=0x%08lx. \n", status );
				kfree( (PVOID)ObjectNameInfo );
				return NULL ;
			}
		}
		else
		{
			ULONG Size = 0 ;

			while ( 1 )
			{
				if ( ObjectNameInfo ) { kfree( (PVOID)ObjectNameInfo ) ; }

				ReturnLength = 0 ;
				Size += 0x80 ;
				ObjectNameInfo = (POBJECT_NAME_INFORMATION) kmalloc( Size );
				if ( NULL == ObjectNameInfo ) { return NULL ; }
				RtlZeroMemory( ObjectNameInfo, Size );

				status = ObQueryNameString( Object, ObjectNameInfo, Size - sizeof( OBJECT_NAME_INFORMATION ), &ReturnLength );
				if( NT_SUCCESS( status ) ) { break ; }

				if ( STATUS_OBJECT_PATH_INVALID == status )
				{
					ReturnLength = 0 ;
					status = ObQueryNameString( Object, ObjectNameInfo, Size - 0x10, &ReturnLength );
				}

				if ( STATUS_INFO_LENGTH_MISMATCH != status && STATUS_BUFFER_OVERFLOW != status )
				{
					kfree( (PVOID)ObjectNameInfo ) ;
					return NULL ;
				}
			}
		}

		if ( 0 == ObjectNameInfo->Name.Length || NULL == ObjectNameInfo->Name.Buffer ) 
		{
		//	dprintf( "error! | SepQueryNameString() - ObQueryNameString(); | 查询到的对象名为空 \n" );
			*bHasNoName = TRUE ;
			kfree( (PVOID)ObjectNameInfo );
			return NULL ;		
		}

		if ( g_Version_Info.IS___win7 && ObjectNameInfo->Name.Length >= 4 )
		{
			ULONG length = ObjectNameInfo->Name.Length ;
			LPWSTR ptr = ObjectNameInfo->Name.Buffer ;

			if ( L'\\' == ptr[0] && L'\\' == ptr[1] )
			{
				memcpy( ptr, ptr + 1, length );
				ptr[ length / sizeof(WCHAR) ] = UNICODE_NULL ;

				ObjectNameInfo->Name.Length			-= 2 ;
				ObjectNameInfo->Name.MaximumLength	-= 2 ;
			}
		}
        
        return (PVOID)ObjectNameInfo ;
    }

    return NULL ;
}



BOOL 
WildcardCmpA (
	IN const char *wild,
	IN const char *string
	) 
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/26 [26:7:2010 - 18:17]

Routine Description:
  字符串匹配,支持通配符    
    
Arguments:
  wild - 包含通配符(*)的白名单串,姑且可以这么理解
  string - 要匹配的Key

Return Value:
  BOOL
    
--*/
{
	const char *cp = NULL, *mp = NULL;

	while ( (*string) && (*wild != '*') )
	{
		if ((*wild != *string) && (*wild != '?')) 
		{
			return 0;
		}

		wild++;
		string++;
	}

	while (*string) 
	{
		if (*wild == '*')
		{
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string+1;

		} 
		else if ((*wild == *string) || (*wild == '?')) 
		{
			wild++ ;
			string++ ;
		} 
		else 
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
	{
		wild++;
	}

	return (BOOL) !*wild ;
}



BOOL
StringMatchA (
	IN PCHAR ParentString,
	IN PCHAR ChildrenString,
	IN BOOL bUseWildcards
	)
{
	BOOL bResult = TRUE ;

	// 1. 校验参数合法性	
	if ( NULL == ParentString || NULL == ChildrenString )
	{
		dprintf( "error! | StringMatchA(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. 不支持通配符则直接全字符串比较
	if ( FALSE == bUseWildcards )
	{
		if ( 0 == _stricmp( ParentString, ChildrenString ) ) { return TRUE ; }
		return FALSE ;
	}

	// 3. 支持通配符调用WildcardCmpA()函数进行匹配
	bResult = WildcardCmpA( ParentString, ChildrenString );
	return bResult ;
}



BOOL
StringMatchW (
	IN PWCHAR ParentString,
	IN PWCHAR ChildrenString,
	IN BOOL bUseWildcards
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/26 [26:7:2010 - 16:06]

Routine Description:
  字符串匹配,要求支持通配符.    
    
Arguments:
  ParentString - 
  ChildrenString - 要匹配的字符串
  bUseWildcards - 是否支持通配符

Return Value:
  BOOL
    
--*/
{
	BOOL bResult = TRUE ;
	NTSTATUS Status ;
	UNICODE_STRING UnicodeString;
	ANSI_STRING AnsiStringP, AnsiStringC ;

	// 1. 校验参数合法性	
	if ( NULL == ParentString || NULL == ChildrenString )
	{
		dprintf( "error! | StringMatchW(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. 不支持通配符则直接全字符串比较
	if ( FALSE == bUseWildcards )
	{
		if ( 0 == _wcsicmp( ParentString, ChildrenString ) ) { return TRUE ; }
		return FALSE ;
	}

	// 3. 支持通配符,则将2字符串转换为Ansi格式,再调用WildcardCmpA()函数进行匹配
	RtlInitUnicodeString( &UnicodeString, ParentString );

	Status = RtlUnicodeStringToAnsiString(
		&AnsiStringP,
		&UnicodeString,
		TRUE
		);

	if (!NT_SUCCESS(Status)) 
	{
		dprintf( "error! | StringMatchW() - RtlAnsiStringToUnicodeString(); ParentString: \"%ws\" \n", ParentString );
		return FALSE ;
	}


	RtlInitUnicodeString( &UnicodeString, ChildrenString );

	Status = RtlUnicodeStringToAnsiString(
		&AnsiStringC,
		&UnicodeString,
		TRUE
		);

	if (!NT_SUCCESS(Status)) 
	{
		dprintf( "error! | StringMatchW() - RtlAnsiStringToUnicodeString(); ChildrenString: \"%ws\" \n", ChildrenString );
		RtlFreeAnsiString( &AnsiStringP );
		return FALSE ;
	}

	bResult = WildcardCmpA( AnsiStringP.Buffer, AnsiStringC.Buffer );

	RtlFreeAnsiString( &AnsiStringP );
	RtlFreeAnsiString( &AnsiStringC );

	return bResult ;
}



BOOL
GetHandleInformation (
	IN HANDLE hObject,
	OUT LPDWORD lpdwFlags
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 14:00]

Routine Description:
  调用NtQueryObject获取@hObject对应的标志位  
    
Arguments:
  hObject - 待查询的句柄 
  lpdwFlags - 保存句柄对应的标志位

Return Value:
  BOOL
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_HANDLE_ATTRIBUTE_INFORMATION HandleInfo ;
	ULONG BytesWritten ;
	DWORD Flags = 0 ;

	status = ZwQueryObject (
		hObject,
		ObjectHandleFlagInformation,
		&HandleInfo,
		sizeof(OBJECT_HANDLE_ATTRIBUTE_INFORMATION),
		&BytesWritten
		);

	if ( ! NT_SUCCESS(status))
	{
		dprintf( "error! | GetHandleInformation() - ZwQueryObject(); | (status=0x%08lx) \n", status );
		return FALSE;
	}
	
	if ( HandleInfo.Inherit )
		Flags |= HANDLE_FLAG_INHERIT ;

	if ( HandleInfo.ProtectFromClose )
		Flags |= HANDLE_FLAG_PROTECT_FROM_CLOSE ;

	*lpdwFlags = Flags ;
	return TRUE;
}



BOOL
SetHandleInformation (
	IN HANDLE hObject,
	IN DWORD dwFlags
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 14:00]

Routine Description:
  调用ZwSetInformationObject设置@hObject对应的权限  
    
Arguments:
  hObject - 待设置的句柄 
  lpdwFlags - 要设置的权限 HANDLE_FLAG_INHERIT | HANDLE_FLAG_PROTECT_FROM_CLOSE

Return Value:
  BOOL
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_HANDLE_ATTRIBUTE_INFORMATION HandleInfo ;
	ULONG BytesWritten;
	NTSTATUS Status;

	status = ZwQueryObject (
		hObject,
		ObjectHandleFlagInformation,
		&HandleInfo,
		sizeof(OBJECT_HANDLE_ATTRIBUTE_INFORMATION),
		&BytesWritten
		);

	if ( ! NT_SUCCESS(status))
	{
		dprintf( "error! | SetHandleInformation() - ZwQueryObject(); | (status=0x%08lx) \n", status );
		return FALSE;
	}
	
	HandleInfo.Inherit = (dwFlags & HANDLE_FLAG_INHERIT) != 0 ;
	HandleInfo.ProtectFromClose = (dwFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE) != 0 ;

	status = ZwSetInformationObject (
		hObject,
		ObjectHandleFlagInformation,
		&HandleInfo,
		sizeof(OBJECT_HANDLE_ATTRIBUTE_INFORMATION)
		);
	
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | SetHandleInformation() - ZwSetInformationObject(); | (status=0x%08lx) \n", status );
		return FALSE;
	}

	return TRUE;
}



NTSTATUS
ZwCloseSafe (
    IN HANDLE Handle,
	IN BOOL Check
    )
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 14:00]

Routine Description:
  安全的关闭句柄; 即遇到带有 HANDLE_FLAG_PROTECT_FROM_CLOSE 权限的句柄,需去掉该权限再关闭之.
    
Arguments:
  Handle - 待关闭的句柄 
  Check - 

Return Value:
  BOOL
    
--*/
{
	BOOL bRet = TRUE ;
	DWORD dwFlags = 0 ;
	NTSTATUS status = STATUS_SUCCESS ; 

	if ( NULL == Handle ) { return STATUS_INVALID_PARAMETER ; }
	if ( FALSE == Check ) { return ZwClose( Handle ); }

	bRet = GetHandleInformation( Handle, &dwFlags );
	if ( FALSE == bRet )
	{
		dprintf( "error! | ZwCloseSafe() - GetHandleInformation(); |  \n" );
		return status ;
	}

	if ( 0 == (dwFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE) ) { return ZwClose( Handle ); }

	ClearFlag( dwFlags, HANDLE_FLAG_PROTECT_FROM_CLOSE );

	bRet = SetHandleInformation( Handle, dwFlags );
	if ( FALSE == bRet )
	{
		dprintf( "error! | ZwCloseSafe() - SetHandleInformation(); |  \n" );
		return status ;
	}

	return ZwClose( Handle );
}



void GetProcessNameOffset()
{

	PEPROCESS curproc;
	int i;
	curproc = PsGetCurrentProcess();

	for( i = 0; i < 3*PAGE_SIZE; i++ )
	{
		if( !strncmp( "System", (PCHAR) curproc + i, strlen("System") ))
		{
			g_ImageFileName_Offset = i;
		}
	}

	if ( g_Version_Info.IS___win7 )
	{
		g_ImageFileName_Offset = 0x16c ;
	}

	return ;
}



BOOL
GetProcessImageFileName (
	IN HANDLE ProcessId, 
	OUT PUNICODE_STRING* lppImageFileName,
	OUT LPWSTR* lppImageFileShortName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/23 [23:8:2010 - 15:25]

Routine Description:
  通过PID获取进程全路径,调用者负责释放内存
    
Arguments:
  ProcessId - 带查询的进程句柄
  lppImageFileName - 保存进程全路径
  lppImageFileShortName - 保存进程短名
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	HANDLE	hProcess ;
	CLIENT_ID ClientId ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	PUNICODE_STRING lpImageFileName = NULL ;

	// 1. 校验参数合法性
	if ( NULL == ProcessId )
	{
		dprintf( "error! | GetProcessImageFileName(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	InitializeObjectAttributes( &ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );

	ClientId.UniqueProcess = ProcessId ;
	ClientId.UniqueThread  = 0 ;

	status = ZwOpenProcess( &hProcess, PROCESS_QUERY_INFORMATION, &ObjectAttributes, &ClientId );
	if ( !NT_SUCCESS(status) ) { return FALSE ; }

	if ( g_Version_Info.IS___win2k )
	{
		// win2k下通过PID获得进程全路径
		bRet = GetProcessFullPathFromPeb( hProcess, &lpImageFileName );
		if ( FALSE == bRet )
		{
			dprintf( "error! | GetProcessImageFileName() - GetProcessFullPathFromPeb(); | \n" );
			goto _CLEARUP_ ;
		}
	}
	else
	{
		// 除了win2k,其他平台通过PID得到进程全路径
		ULONG ReturnLength = 0 ;

		// 2.1 获取内存块大小
		status = ZwQueryInformationProcess( hProcess, ProcessImageFileName, 0, 0, &ReturnLength );
		if ( STATUS_INFO_LENGTH_MISMATCH != status )
		{
			dprintf( "error! | GetProcessImageFileName() - ZwQueryInformationProcess(); | (status=0x%08lx) \n", status );
			goto _CLEARUP_ ;
		}

		if ( 0 == ReturnLength ) { 
			ReturnLength = MAX_PATH * sizeof(WCHAR) + sizeof(UNICODE_STRING) ;
		} else {
			ReturnLength += 0x8 ;
		}
		
		// 2.2 根据得到的大小申请内存,重新查询
		lpImageFileName = kmalloc( ReturnLength + 0x8 ) ;
		if ( NULL == lpImageFileName )
		{
			dprintf( "error! | GetProcessImageFileName() - kmalloc(); | 分配内存失败 (Length=%d) \n", ReturnLength );
			goto _CLEARUP_ ;
		}

		status = ZwQueryInformationProcess(
			hProcess , 
			ProcessImageFileName ,
			lpImageFileName , 
			ReturnLength,
			&ReturnLength
			);

		if ( !NT_SUCCESS(status) )
		{
			dprintf( "error! | GetProcessImageFileName() - ZwQueryInformationProcess(); | (status=0x%08lx) \n", status );
			kfree( (PVOID)lpImageFileName );
			goto _CLEARUP_ ;
		}

		if ( 0 == lpImageFileName->Length || NULL == lpImageFileName->Buffer )
		{
			dprintf( "error! | GetProcessImageFileName(); | 获取的字符串内容不合法 (ImageFileName->Length=%d) \n", lpImageFileName->Length );
			kfree( (PVOID)lpImageFileName );
			goto _CLEARUP_ ;
		}
	}

	// 2.1 将查询到的进程全路径加上结束符
	lpImageFileName->Buffer[ lpImageFileName->Length / sizeof(WCHAR) ] = UNICODE_NULL ;
	*lppImageFileName = lpImageFileName ;

	// 2.2 获取进程短名
	if ( lppImageFileShortName )
	{
		LPWSTR ptr = NULL ;

		*lppImageFileShortName = lpImageFileName->Buffer ;

		ptr = wcsrchr( lpImageFileName->Buffer, L'\\' );
		if ( ptr && *(WORD*)(ptr+1) ) { *lppImageFileShortName = ptr + 1 ; }
	}

	bRet = TRUE ;

_CLEARUP_ :
	ZwClose( hProcess );
	return bRet ;
}



BOOL 
GetProcessFullPathFromPeb (
	IN HANDLE ProcessHandle ,
	OUT PUNICODE_STRING *pFullName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/23 [23:8:2010 - 22:59]

Routine Description:
  只用于WIN2000, 取全路径,调用者负责释放内存 
    
Arguments:
  ProcessHandle - 待查询的进程句柄
  pFullName - 保存已查询到的进程全路径,需由调用者释放该内存块

--*/
{
	KAPC_STATE old_apc_state ;
	BOOL bRet = FALSE, bAttach = FALSE ; 
	NTSTATUS status = STATUS_SUCCESS ; 
	PVOID Eprocess = NULL, ProcessInformation = NULL ; 
	ULONG length = MAX_PATH * sizeof(WCHAR) + sizeof(UNICODE_STRING) ;

	// 1. 校验参数合法性
	if ( NULL == ProcessHandle )
	{
		dprintf( "error! | GetProcessFullPathFromPeb(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( 0 == g_Version_Info.IS___win2k ) { return FALSE ; }

	// 2. 申请内存,用于存放进程全路径
	ProcessInformation = kmalloc( length );
	if ( NULL == ProcessInformation )
	{
		dprintf( "error! | GetProcessFullPathFromPeb() - kmalloc(); | 申请内存失败 (length=%d) \n", length );
		return FALSE ;
	}

	// 3. Handle --> eprocess
	status = ObReferenceObjectByHandle (
		ProcessHandle , 
		GENERIC_READ , 
		*PsProcessType , 
		KernelMode , 
		&Eprocess , 
		0
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | GetProcessFullPathFromPeb() - ObReferenceObjectByHandle(); | status=0x%08lx \n", status );
		kfree( ProcessInformation );
		return FALSE ;
	}

	if ( Eprocess != PsGetCurrentProcess() )
	{
		KeStackAttachProcess( Eprocess , &old_apc_state );
		bAttach = TRUE ; 
	}

	// 4.1 调用常规方式获取(Section法)
	bRet = GetProcessFullPathFromPebEx( Eprocess, length, (PUNICODE_STRING)ProcessInformation );
	if ( FALSE == bRet )
	{
		// 4.2 调用非常规方式获取(Peb法)
		bRet = GetProcessFullPathFromPebExp( Eprocess, (PUNICODE_STRING)ProcessInformation );
		if ( FALSE == bRet )
		{
			kfree( ProcessInformation );
		}
	}

	if ( bAttach ) { KeUnstackDetachProcess( &old_apc_state ); }
	ObDereferenceObject( Eprocess );

	*pFullName = (PUNICODE_STRING)ProcessInformation ;
	return bRet ;
}



BOOL 
GetProcessFullPathFromPebEx (
	IN PVOID Eprocess ,
	IN ULONG BufferLength,
	OUT PUNICODE_STRING pFullName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/23 [23:8:2010 - 22:59]

Routine Description:
  只用于WIN2000, Section法取全路径 
    
Arguments:
  Eprocess - 待查询的进程
  BufferLength - 内存块大小
  pFullName - 调用者提供的内存块

--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG ReturnLength ;
	BOOL bRet = TRUE ;
	HANDLE		 SectionHandle	= NULL ;
	PFILE_OBJECT FileObject		= NULL ;
	PVOID SectionObject ;
	ULONG SegmentObject, BaseAddress ;

	// 1. 校验参数合法性
	if ( NULL == Eprocess || 0 == BufferLength || NULL == pFullName )
	{
		dprintf( "error! | GetProcessFullPathFromPebEx(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( 0 == g_Version_Info.IS___win2k ) { return FALSE ; }

	// 2.x 通过Section得到对应的进程全路径
	SectionHandle = *(HANDLE*)( (ULONG)Eprocess + 0x1AC ) ;

	status = ObReferenceObjectByHandle (
		SectionHandle , 
		GENERIC_READ ,
		*MmSectionObjectType,
		KernelMode , 
		&SectionObject , 
		NULL
		);

	if( ! NT_SUCCESS( status ) ) { return FALSE ; }
	
	SegmentObject = *(ULONG*)((ULONG)SectionObject + 0x14) ;
	if ( FALSE == MmIsAddressValid((PVOID)SegmentObject) ) { return FALSE ; }
	
	BaseAddress = *(ULONG*)SegmentObject ;
	if ( FALSE == MmIsAddressValid((PVOID)BaseAddress) ) { return FALSE ; }

	FileObject = (PFILE_OBJECT)( *(ULONG*)(BaseAddress + 0x24) ) ;
	if ( FALSE == MmIsAddressValid((PVOID)FileObject) || NULL == FileObject->DeviceObject ) { return FALSE ; }
	
	status = ObReferenceObjectByPointer( FileObject, 0, *IoFileObjectType, KernelMode );
	if( ! NT_SUCCESS( status ) ) { return FALSE ; }
	
	status = ObQueryNameString( FileObject, (PVOID)pFullName, BufferLength, &ReturnLength );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | GetProcessFullPathFromPebEx() - ObQueryNameString(); | status=0x8lx \n", status );
		bRet = FALSE ;
	}
	else 
	{
		bRet = TRUE ;
	}

	ObDereferenceObject( FileObject );
	ObDereferenceObject( SectionObject );

	return bRet ;
}



BOOL 
GetProcessFullPathFromPebExp (
	IN PVOID eprocess ,
	IN PUNICODE_STRING namepool
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/23 [23:8:2010 - 22:59]

Routine Description:
  (该代码摘自MJ的360SelfProtect.sys) 只用于WIN2000, 当Section法取全路径失败时，通过PEB取全路径   
    
Arguments:
  eprocess - 待查询的目标进程
  namepool - 调用者提供的内存块

--*/
{
	PVOID Peb ; 
	PVOID nameBuffer = (PVOID)((ULONG)namepool + sizeof(UNICODE_STRING));

	if ( 0 == g_Version_Info.IS___win2k ) { return FALSE ; }

	//检查Eprocess->Peb是否有效
	if (!MmIsAddressValid((PVOID)((ULONG)eprocess + 0x1b0))) { return FALSE ; }

	//取到Peb地址
	Peb = (PVOID)(*(ULONG*)((ULONG)eprocess + 0x1b0));
	if (!Peb) { return FALSE ; } //如果没有PEB（如SYSTEM进程）

	//下面都是操作用户态内存，需要Probe

	__try
	{
		PRTL_USER_PROCESS_PARAMETERS pUserParm ; 

		//检查Peb->pUserParm + 1个指针

		ProbeForRead(Peb , 0x14 , 1);

		//取pUserParameters

		pUserParm = (PRTL_USER_PROCESS_PARAMETERS)(*(ULONG*)((ULONG)Peb + 0x10));

		//检查User parameters

		ProbeForRead(pUserParm , sizeof(RTL_USER_PROCESS_PARAMETERS) , 1);

		//检查 ImagePathName的Buffer

		ProbeForRead(pUserParm->ImagePathName.Buffer , pUserParm->ImagePathName.Length , sizeof(WCHAR));

		//如果ImagePathName无长度或者超长
		if (pUserParm->ImagePathName.Length == 0 || pUserParm->ImagePathName.Length > ((MAX_PATH - 5) * sizeof(WCHAR)))
		{
			return FALSE ; 
		}

		if (*(WCHAR*)pUserParm->ImagePathName.Buffer == L'\\')
		{
			RtlCopyMemory(nameBuffer, pUserParm->ImagePathName.Buffer , pUserParm->ImagePathName.Length);
			*(WORD*)((ULONG)nameBuffer + pUserParm->ImagePathName.Length) = 0 ;

		}
		else
		{
			RtlCopyMemory(nameBuffer , L"\\??\\" , 4 * sizeof(WCHAR));
			RtlCopyMemory((PVOID)((ULONG)nameBuffer + 4 * sizeof(WCHAR)), pUserParm->ImagePathName.Buffer , pUserParm->ImagePathName.Length);
			*(WORD*)((ULONG)nameBuffer + pUserParm->ImagePathName.Length + 4 * sizeof(WCHAR)) = 0 ;
		}

		RtlInitUnicodeString(namepool , nameBuffer);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE; 
	}

	return TRUE ; 
}



PVOID
GetFileObject(
	IN PVOID Object
	)
{
	ULONG FileObject = 0 ;

	FileObject = 0;
	if ( g_Version_Info.IS___win2k )
	{
		FileObject = *(ULONG *)((ULONG)Object + 8);
	}
	else if ( g_Version_Info.IS___xp || g_Version_Info.IS___win2003 )
	{
		FileObject = *(ULONG *)Object;
	}
	else if ( g_Version_Info.IS___vista || g_Version_Info.IS___win7 )
	{
		FileObject = *(PULONG) ( *(PULONG)((ULONG)Object + 8) );
	}

	return (PVOID) FileObject ;
}



ULONG AddRef( LONG Ref )
{
	return InterlockedIncrement( &Ref );
}



ULONG DecRef( LONG Ref )
{
	return InterlockedDecrement( &Ref );
}



BOOL IsWrittenKernelAddr( IN PVOID StartAddr, IN ULONG_PTR length )
{
	if ( MM_HIGHEST_USER_ADDRESS <= StartAddr )
	{
		dprintf( "error! | IsWrittenKernelAddr(); | Virtual allocation above User Space\n" );
		return TRUE;
	}

	if (((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)StartAddr) < length)
	{
		dprintf( "error! | IsWrittenKernelAddr(); | Region size would overflow into kernel-memory\n" );
		return TRUE;
	}

	return FALSE;
}



BOOL GetSessionId( ULONG *pSessionId )
{
	ULONG ReturnLength = 4 ; 
	NTSTATUS status = STATUS_SUCCESS ;
	PROCESS_SESSION_INFORMATION SessionInformation;

	if ( NULL == pSessionId ) { return FALSE; }

	status = ZwQueryInformationProcess(
		(HANDLE)0xFFFFFFFF,
		ProcessSessionInformation,
		&SessionInformation,
		sizeof(SessionInformation),
		&ReturnLength
		);

	if( NT_SUCCESS(status) )
	{
		*pSessionId = SessionInformation.SessionId;
		return TRUE;
	}

	return FALSE ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////