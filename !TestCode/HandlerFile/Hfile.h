/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/12/28 [28:12:2009 - 17:13]
* MODULE : D:\Program\R0\Coding\…≥œ‰\SandBox\Code\HandlerFile\Hfile.h
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include<string.h>

#include "ntdll.h"
#pragma comment(lib, "ntdll.lib")


//////////////////////////////////////////////////////////////////////////

NTSTATUS
HFCreateFileEx(
	IN char* szFileName,
	IN DWORD dwCreationDisposition
	) ;


NTSTATUS
HFCreateFile(
	IN char* szFileName,
	IN DWORD dwDesiredAccess,
	IN DWORD dwCreationDisposition,
	IN DWORD dwFlagsAndAttributes,
	OUT PVOID *FileHandle
	) ;

NTSTATUS
HFDeleteFile(
	IN char* szFileName
	) ;

NTSTATUS
HFWriteFile(
	IN char* szFileName,
	IN char* pBuffer
	) ;

NTSTATUS
HFReadFile(
	IN char* szFileName,
	OUT char* pBuffer
	) ;

NTSTATUS
HFRenameFile(
	IN char* szFileName,
	IN char* szFileNameNew
	) ;

//////////////////////////////////////////////////////////////////////////

VOID
TestWriteFile(
	) ;

VOID
TestRenameFile(
	) ;

VOID
TestReadFile(
	) ;

VOID
TestDeleteFile(
	) ;

///////////////////////////////   END OF FILE   ///////////////////////////////
