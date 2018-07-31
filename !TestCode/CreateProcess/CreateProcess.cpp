// CreateProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////

static CHAR g_szProcName[ MAX_PATH ] = 
//"C:\\Program Files\\Tencent\\Foxmail\\Foxmail.exe" ;
//	"C:\\Program Files\\Mozilla Thunderbird\\thunderbird.exe" ;
	"D:\\1.exe" ;

//////////////////////////////////////////////////////////////////////////

BOOL
RunPE (
	IN PCHAR szPath,
	IN PCHAR szCmdLine
	)
{
	PROCESS_INFORMATION  pi;
    STARTUPINFO          si;
	
	if ( NULL == szPath )
	{
		return FALSE ;
	}
	
	memset( &pi, 0, sizeof(PROCESS_INFORMATION) );
    memset( &si, 0, sizeof(STARTUPINFO) );
    si.cb = sizeof( STARTUPINFO );
	
	if ( !CreateProcess( szPath, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) )
	{
		printf( "[失败] 启动进程\"%s\" \n", g_szProcName );
		return FALSE ;
	}
	
	printf( "[成功] 启动进程\"%s\" \n", g_szProcName );
	return TRUE ;
}



//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{	
	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	system( "title=Test for CreateProcess()" );
	
	while ( bOK )
	{
		printf (
			"请选择操作: \n"
			"0. ESC \n"
			"1. 创建进程 *.exe						\n"
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
			RunPE( g_szProcName, NULL );
			break;
				
		default:
			break;
		}
	}

	return 0;
}

