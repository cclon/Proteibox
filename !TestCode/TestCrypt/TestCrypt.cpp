// TestCrypt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <afxwin.h>
#include <atlbase.h>
#include <atldef.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinCrypt.h>
#pragma comment(lib, "crypt32.lib")

void TestCrypt();
void TestCrypt2();
void MyHandleError(char *s);

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	printf("Test Crypt\n");

	getchar();
	TestCrypt();
	TestCrypt2();

	getchar();
	return 0;
}

void TestCrypt()

{
	// String to crypt
	CString Str = _T( "Plain Text" );
	
	DATA_BLOB DataIn;
	DataIn.pbData = (BYTE *)(LPCTSTR)Str;
	DataIn.cbData = (Str.GetLength() + 1) * sizeof(TCHAR);
	
	// Data after protection
	DATA_BLOB DataOut;
	
	// Do the protection
	BOOL ok = ::CryptProtectData (
		&DataIn, 
		L"Some data",
		NULL,
		NULL,
		NULL,
		0,
		&DataOut
		);
		
	// Test decrypting
	DATA_BLOB DataDecrpyted;
	ok = ::CryptUnprotectData (
		&DataOut,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		&DataDecrpyted
		);
	
//	ATLASSERT(ok);
	// This works if string to protect included end-of-string \0 !
	CString verify = (LPCTSTR)DataDecrpyted.pbData;
	
	// Check...	
//	ATLVERIFY( verify == Str );
	
	// Cleanup
	::LocalFree( DataDecrpyted.pbData );
	DataDecrpyted.pbData = NULL;
	DataDecrpyted.cbData = 0;
	
	::LocalFree( DataOut.pbData );
	DataOut.pbData = NULL;
	DataOut.cbData = 0;
	
	return;
}


void TestCrypt2()
{
	DATA_BLOB DataIn;
	DATA_BLOB DataOut;
	DATA_BLOB DataVerify;
	BYTE *pbDataInput =(BYTE *)"Hello world of data protection.";
	DWORD cbDataInput = strlen((char *)pbDataInput)+1;
	DataIn.pbData = pbDataInput;    
	DataIn.cbData = cbDataInput;
	CRYPTPROTECT_PROMPTSTRUCT PromptStruct;
	LPWSTR pDescrOut = NULL;
	
	//  Begin processing.	
	printf("The data to be encrypted is: %s\n",pbDataInput);
	
	//  Initialize PromptStruct.
	ZeroMemory(&PromptStruct, sizeof(PromptStruct));
	PromptStruct.cbSize = sizeof(PromptStruct);
	PromptStruct.dwPromptFlags = CRYPTPROTECT_PROMPT_ON_PROTECT;
	PromptStruct.szPrompt = L"This is a user prompt.";
	
	//  Begin protect phase.
	if(CryptProtectData(
		&DataIn,
		L"This is the description string.", // A description string. 
		NULL,                               // Optional entropy
		NULL,                               // Reserved.
		&PromptStruct,                      // Pass a PromptStruct.
		0,
		&DataOut))
	{
		printf("The encryption phase worked. \n");
	}
	else
	{
		MyHandleError("Encryption error!");
	}

	//   Begin unprotect phase.	
	if (CryptUnprotectData(
        &DataOut,
        &pDescrOut,
        NULL,                 // Optional entropy
        NULL,                 // Reserved
        &PromptStruct,        // Optional PromptStruct
        0,
        &DataVerify))
	{
		printf("The decrypted data is: %s\n", DataVerify.pbData);
		printf("The description of the data was: %S\n",pDescrOut);
	}
	else
	{
		MyHandleError("Decryption error!");
	}

	//-------------------------------------------------------------------
	// At this point, memcmp could be used to compare DataIn.pbData and 
	// DataVerify.pbDate for equality. If the two functions worked
	// correctly, the two byte strings are identical. 
	
	//-------------------------------------------------------------------
	//  Clean up.
	
	LocalFree(pDescrOut);
	LocalFree(DataOut.pbData);
	LocalFree(DataVerify.pbData);
	return;
}


void MyHandleError(char *s)
{
    fprintf(stderr,"An error occurred in running the program. \n");
    fprintf(stderr,"%s\n",s);
    fprintf(stderr, "Error number %x.\n", GetLastError());
    fprintf(stderr, "Program terminating. \n");
    exit(1);
} // End of MyHandleError
