// RegLoadKey.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

void EnableToken( LPCTSTR lpName );
LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile);
LONG MyRegSaveKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile);

int main(int argc, char* argv[])
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	system( "title=Test for RegLoadKey" );
	
	while ( bOK )
	{
		printf (
			"??????2¨´¡Á¡Â: \n"
			"0. ESC \n"
			"1. ¡À¡ê¡ä?¡Á¡é2¨¢¡À¨ªHKEY_LOCAL_MACHINE\\SOFTWARE\\Adobe¦Ì?C:\\sudami.hiv???t	\n"
			"2. ?¨®??C:\\sudami.hiv???t¦Ì?HKEY_LOCAL_MACHINE\\SOFTWARE\\Google		\n"
			"-------------------------------------------\n"
			);
		
		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			ExitProcess( -1 );
			break;
			
		case 1:
			MyRegSaveKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\Adobe", "C:\\sudami.hiv" );
			break;
			
		case 2:
			MyRegLoadKey( HKEY_CURRENT_USER, "SOFTWARE\\Google", "C:\\sudami.hiv" );
			break;
			
		default:
			break;
		}
	}
	
//	getchar() ;
//	system( "pause" );
	return 0;
}

//////////////////////////////////////////////////////////////////////////

LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile)
{
	LONG err= ERROR_FILE_NOT_FOUND;
	
	EnableToken( SE_BACKUP_NAME );
	EnableToken( SE_RESTORE_NAME );
	
	RegUnLoadKey( hKey, lpSubKey );
	
    err = RegLoadKey(hKey, lpSubKey, lpFile);
	printf("MyRegLoadKey(); err=%d \n",  err);
    return err;
}


LONG MyRegSaveKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile)
{
	LONG r;
	HKEY hReg;
	
	r = RegOpenKey( hKey, lpSubKey, &hReg );
	if( ERROR_SUCCESS != r )
	{
		printf("err! | MyRegSaveKey() - RegOpenKey(); | ");
		return 1;
	}
	
	EnableToken( SE_BACKUP_NAME );
	r = RegSaveKey( hReg, lpFile, NULL );
	if( ERROR_SUCCESS != r )
	{
		if( ERROR_ALREADY_EXISTS == r)
		{
			printf("ko! | MyRegSaveKey() - RegSaveKey(); | File Exists! \n");
			return 1;
		}
		
		RegCloseKey(hReg);
		printf("err! | MyRegSaveKey() - RegSaveKey(); | \n");
		return 1;
	}
	
	RegCloseKey(hReg);
	printf("ok! | MyRegSaveKey(); | Success \n");
	return 0;
}


void EnableToken( LPCTSTR lpName )
{
	HANDLE hToken;
	TOKEN_PRIVILEGES NewToken;
	NewToken.PrivilegeCount=1;
	NewToken.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	
	LookupPrivilegeValue(NULL,lpName,&NewToken.Privileges[0].Luid);
	OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken);
	
	if(AdjustTokenPrivileges(hToken,FALSE,&NewToken,NULL,NULL,NULL)==0)
	{
		printf("EnableToken Error!");
	}
	
	CloseHandle(hToken);
}


