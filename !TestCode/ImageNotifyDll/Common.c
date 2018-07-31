/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/12 [12:5:2010 - 14:25]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\!TestCode\ImageNotifyDll\Common.c
* 
* Description:
*      
*   公共函数                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <stdio.h>
#include <ntddk.h>

#include "struct.h"
#include "Common.h"


//////////////////////////////////////////////////////////////////////////


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
	NTSTATUS		status = STATUS_UNSUCCESSFUL ;
	WCHAR			wszDosPath[ MAX_PATH ] = L"" ;
	UNICODE_STRING	uni_szNtPath, uni_szDosPath ;
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

	wcsncpy( szDosPath, szNtPath, cchSize );
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

	InitializeObjectAttributes( 
		&ObjectAttr,
		&FileName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);
	
	status = ZwCreateFile(
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
	
	status = ZwWriteFile(
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






///////////////////////////////   END OF FILE   ///////////////////////////////