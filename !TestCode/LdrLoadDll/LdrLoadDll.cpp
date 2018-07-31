// LdrLoadDll.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>


//////////////////////////////////////////////////////////////////////////

VOID TestLdrLoadDll ();

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	system( "title=Test for Sandboxie:LdrLoadDll()" );
	
	while ( bOK )
	{
		printf (
			"ÇëÑ¡Ôñ²Ù×÷: \n"
			"0. ESC \n"
			"1. call LoadLibrary()						\n"
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
			TestLdrLoadDll() ;
			break;
			
		default:
			break;
		}
	}
	
	getchar() ;
	return 0;
}



VOID TestLdrLoadDll ()
{
	HMODULE hModule = NULL ;
			 
	hModule = LoadLibrary( "ntdll.dll" );
	if ( NULL == hModule )
	{
		printf( "LoadLibrary() failed \n" );
		return ;
	}
	
	PVOID addr = GetProcAddress( hModule, "LdrLoadDll" );
	printf( "LdrLoadDll addr: 0x%08lx \n\n", addr );

	FreeLibrary( hModule );
	return ;
}