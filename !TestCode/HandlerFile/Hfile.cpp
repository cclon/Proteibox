/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/12/28 [28:12:2009 - 17:15]
* MODULE : D:\Program\R0\Coding\…≥œ‰\SandBox\Code\HandlerFile\Hfile.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "Hfile.h"

//////////////////////////////////////////////////////////////////////////

NTSTATUS
HFCreateFileEx(
	IN char* szFileName,
	IN DWORD dwCreationDisposition
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL ;
	
	status = HFCreateFile( 
		szFileName, 
		GENERIC_READ, // | GENERIC_WRITE, // GENERIC_ALL, 
		dwCreationDisposition, 
		FILE_ATTRIBUTE_NORMAL, 
		&hFile 
		);

	if ( NULL != hFile )
	{
		CloseHandle( hFile );
	}
	
	return status ;
}



NTSTATUS
HFCreateFile(
	IN char* szFileName,
	IN DWORD dwDesiredAccess,
	IN DWORD dwCreationDisposition,
	IN DWORD dwFlagsAndAttributes, // eg:FILE_ATTRIBUTE_NORMAL
	OUT PVOID *FileHandle
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile;
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	
	hFile = CreateFile (
		szFileName, 
		dwDesiredAccess, // GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL ,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, 
		NULL,
		dwCreationDisposition, // CREATE_ALWAYS,
		dwFlagsAndAttributes,  // FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY,
		NULL
		);
	
	if ( INVALID_HANDLE_VALUE == hFile )
	{
		printf( "HFCreateFile -- CreateFile -- Error creating '%d'\n", (ULONG)GetLastError() );
		return STATUS_UNSUCCESSFUL;
	}

	*FileHandle = hFile ;

	return status ;
}



NTSTATUS
HFDeleteFileEx(
	IN char* szFileName
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL ;
	
	status = HFCreateFile( szFileName, GENERIC_READ | GENERIC_WRITE, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, &hFile );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFDeleteFileEx() -- HFCreateFile -- Error, 0x%08lx \n", status ); 
		return status ;
	}

	if ( NULL != hFile )
	{
		CloseHandle( hFile );
	}
	
	return status ;
}



NTSTATUS
HFDeleteFile(
	IN char* szFileName
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	
	if ( FALSE == DeleteFile (szFileName) )
	{
		return status = STATUS_UNSUCCESSFUL ;
	}
	
	return status ;
}



NTSTATUS
HFWriteFile(
	IN char* szFileName,
	IN char* pBuffer
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL ;
	ULONG BytesWritten;
	ULONG Size;
	
	status = HFCreateFile( szFileName, GENERIC_WRITE, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, &hFile );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFWriteFile -- HFCreateFile, Error: %s, 0x%08lx \n", szFileName, status ); 
		return STATUS_UNSUCCESSFUL ;
	}

	Size = (ULONG)strlen(pBuffer) + 1;
	if ( FALSE == WriteFile(hFile, pBuffer, Size, (LPDWORD)&BytesWritten, NULL))
	{
		printf( "HFWriteFile -- WriteFile, Error: %s, 0x%08lx. Cannot write to file.\n", szFileName, status );
		status = STATUS_UNSUCCESSFUL ;
		goto _end_ ;
	}
	
_end_ :
	if ( NULL != hFile )
	{
		CloseHandle( hFile );
	}
	
	return status ;
}



NTSTATUS
HFReadFile(
	IN char* szFileName,
	OUT char* pBuffer
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hFile = NULL ;
	ULONG BytesRead ;
	
	status = HFCreateFile( szFileName, GENERIC_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, &hFile );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFReadFile -- HFCreateFile, Error: %s, 0x%08lx \n", szFileName, status ); 
		return status ;
	}

	DWORD filesize = GetFileSize( hFile,NULL );
	char* buffer=new char[filesize+1];

	if ( FALSE == ReadFile(hFile, (PVOID)buffer, filesize, &BytesRead, NULL) )
	{
		printf( "HFReadFile -- ReadFile, Error: %s, 0x%08lx \n", szFileName ); 
		status = STATUS_UNSUCCESSFUL ;
	}

	buffer[ filesize ] = 0 ;
		
	if ( NULL != hFile )
	{
		CloseHandle( hFile );
	}

	memcpy( pBuffer, buffer, filesize ) ;

	delete buffer;

	return status ;
}



NTSTATUS
HFRenameFile(
	IN char* szFileName,
	IN char* szFileNameNew
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
	
	MoveFile( szFileName, szFileNameNew );
	
	return status ;
}





///////////////////////////////   END OF FILE   ///////////////////////////////
