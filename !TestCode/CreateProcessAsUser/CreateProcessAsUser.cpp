// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <TLHELP32.H>

//////////////////////////////////////////////////////////////////////////

WCHAR g_szPath[ MAX_PATH ] = L"d:\\A.exe";

//////////////////////////////////////////////////////////////////////////

BOOL GetTokenByName(HANDLE &hToken,LPWSTR lpName)
{
	HANDLE         hProcessSnap = NULL;
    BOOL           bRet      = FALSE;
    PROCESSENTRY32 pe32      = {0};

	if(!lpName) { return FALSE; }
	
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return (FALSE);
	
    pe32.dwSize = sizeof(PROCESSENTRY32);
	
    if (Process32First(hProcessSnap, &pe32))
    {  
        do
        {
			if( 0 == wcsicmp( pe32.szExeFile, lpName ) )
			{
				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION,FALSE,pe32.th32ProcessID );
				bRet = OpenProcessToken( hProcess,TOKEN_ALL_ACCESS,&hToken );
				CloseHandle ( hProcessSnap );
				return bRet;
			}
        }
        while (Process32Next(hProcessSnap, &pe32));
        bRet = TRUE;
    }
    else
        bRet = FALSE;
	
    CloseHandle (hProcessSnap);
    return (bRet);
}

BOOL RunProcess(LPWSTR lpImage)
{
	if(!lpImage)
	{
		return FALSE;
	}
	HANDLE hToken;
	if(!GetTokenByName( hToken, L"EXPLORER.EXE") )
	{
		return FALSE;
	}
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb= sizeof(STARTUPINFO);
	si.lpDesktop = TEXT("winsta0\\default");
	
	BOOL bResult = CreateProcessAsUserW(hToken,lpImage,NULL,NULL,NULL,
		FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
	CloseHandle(hToken);
	if(bResult)
	{
		OutputDebugStringW( L"CreateProcessAsUser ok!\r\n" );
	}
	else
	{
		OutputDebugStringW( L"CreateProcessAsUser false!\r\n" );
	}

	return bResult;
}



BOOL 
StartProcessAsUser (
	IN LPWSTR lpApplicationName
	)
{
	HANDLE htoken;
    BOOL   fRet;
	STARTUPINFO StartupInfo ;
    PROCESS_INFORMATION ProcessInfo ;

	memset( &StartupInfo, 0, sizeof (StartupInfo) );
    memset( &ProcessInfo, 0, sizeof (ProcessInfo) );

	fRet = OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &htoken );
	if ( FALSE == fRet )
	{
		OutputDebugStringW( L"**** BREAKAWAY ***** A \n" );
		return FALSE;
    }

	fRet = CreateProcessAsUser( 
		htoken,
		lpApplicationName,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&StartupInfo,
        &ProcessInfo 
		);

	return fRet;
}



int main(int argc, char* argv[])
{
	BOOL bRet = FALSE;

	bRet = RunProcess( g_szPath );
	if ( bRet )
	{
		printf( "Create ok. \n" );
	}
	else
	{
		printf( "Create error! \n" );
	}

	getchar();
	return 0;
}

