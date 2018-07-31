// GetModuleFileNameW.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

//////////////////////////////////////////////////////////////////////////

#define DPRINT DbgPrint("(%s:%d) ",__FILE__,__LINE__); DbgPrint
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

//////////////////////////////////////////////////////////////////////////



ULONG
DbgPrint(char *Fmt, ...)
{
	va_list Args;
	
	va_start(Args, Fmt);
	vfprintf(stderr, Fmt, Args);
	va_end(Args);
	
	return 0;
}


int main(int argc, char* argv[])
{
	CHAR lpTest[MAX_PATH];

	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	system( "title=Test for Sandboxie:GetModuleFileName()" );

	while ( bOK )
	{
		printf (
			"ÇëÑ¡Ôñ²Ù×÷: \n"
			"0. ESC \n"
			"1. call GetModuleFileName()				\n"
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
			RtlZeroMemory( lpTest, MAX_PATH );
			GetModuleFileName(NULL, lpTest, MAX_PATH);
			printf("%s \n\n", lpTest);
			break;

		default:
			break;
		}
	}

	getchar() ;
	return 0;
}

