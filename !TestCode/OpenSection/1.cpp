/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/12/07 [7:12:2009 - 14:43]
* MODULE : D:\PROGRAM\R0\CODING\沙箱\SANDBOX\CODE\OpenSection\1.cpp
* 
* Description:
*   这段代码测试沙箱中的过滤函数Object_OpenProcedure_Filter_dep()
*   是访问全局Section --> "\\KnownDlls\\*"下的东西,禁止以下权限的访问:
*	  #define DELETE                0x00010000
*     #define SECTION_EXTEND_SIZE   0x0010   
*
*   测试结果 - Sandboxie过滤正常,即在沙箱中运行本程序,会得到以下结果 :
*
* NtOpenSection("SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRIT"),OK!
* NtOpenSection("DELETE | SECTION_EXTEND_SIZE"),Failed! c0000022
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/


#include <windows.h>
#include <stdlib.h>
#include<string.h>

#include "ntdll.h"
#pragma comment(lib, "ntdll.lib")


//////////////////////////////////////////////////////////////////////////



#define OBJ_CASE_INSENSITIVE    0x00000040L
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)


#define MsgEx(lpContext) MessageBox( NULL, lpContext, "", MB_OK )
#define Msg(lpContext) printf( "%s", lpContext )

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

NTSTATUS
TestOpenSection(
	DWORD dwDesiredAccess,
	LPCWSTR lpName
	)
{
	OBJECT_ATTRIBUTES Obja;
	UNICODE_STRING ObjectName;
	NTSTATUS Status;
	HANDLE Object;
	
	if ( NULL == lpName ) { return STATUS_UNSUCCESSFUL ; }
	
	RtlInitUnicodeString( &ObjectName,lpName );
	
	InitializeObjectAttributes(
		&Obja,
		&ObjectName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

	Status = ZwOpenSection(
		&Object,
		dwDesiredAccess,
		&Obja
		);

	if ( NT_SUCCESS(Status) ) 
	{
		ZwClose( Object );
	}

	return Status ;
}



NTSTATUS
Test(
	DWORD dwDesiredAccess
	)
{
	LPCWSTR lpName = L"\\KnownDlls\\kernel32.dll" ;

	return TestOpenSection( dwDesiredAccess, lpName ) ;
}

VOID main()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	getchar() ;
	status = TestOpenSection( SECTION_MAP_READ, L"\\RPC Control\\ntsvcs" ) ;
	if ( NT_SUCCESS(status) )
	{
		Msg( "NtOpenSection(\"SECTION_MAP_READ\", RPC Control\\ntsvcs),OK! \n\n" ) ;
	}
	else
	{
		Msg( "NtOpenSection(\"SECTION_MAP_READ\", RPC Control\\ntsvcs),ERR! \n\n" ) ;
	}

	getchar() ;
	return;



	status = Test( SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE ) ;
	if ( NT_SUCCESS(status) )
	{
		Msg( "NtOpenSection(\"SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRIT\"),OK! \n\n" ) ;
	} 
	
	status = Test( DELETE | SECTION_EXTEND_SIZE ) ;
	if ( NT_SUCCESS(status) ) {
		Msg( "NtOpenSection(\"DELETE | SECTION_EXTEND_SIZE\"),OK! \n\n" ) ;
	} else {
		printf( "NtOpenSection(\"DELETE | SECTION_EXTEND_SIZE\"),Failed! %08lx \n", status );
	}

	getchar() ;

	return ;
}