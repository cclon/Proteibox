#include "stdafx.h"

#include "outputdebug.h"
#include <strsafe.h>


#ifdef _MYDEBUG
// void _cdecl MyAtlTraceW(LPCWSTR lpszFormat, ...)
// {  
//     va_list args;
//     va_start(args, lpszFormat);
//     
//     WCHAR szBuffer[512]; 
//     WCHAR szOutText[512+20];
//     
//     HRESULT hr = StringCbVPrintfW(szBuffer, sizeof(szBuffer), lpszFormat, args);
//     ATLASSERT(SUCCEEDED(hr)); //Output truncated as it was > sizeof(szBuffer)
//     
//     StringCbCopyW(szOutText, sizeof(szOutText), _MYDEBUGFLAG_W);
//     StringCbCatW(szOutText, sizeof(szOutText), szBuffer);
// 
// 	int nLen = wcslen(szOutText);
// 	if(nLen && szOutText[nLen-1] != '\n')
// 		StringCbCatW(szOutText, sizeof(szOutText), L"\n");
//     
//     OutputDebugStringW(szOutText);
//     va_end(args);
// }

void _cdecl MyAtlTraceA(LPCSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    
    CHAR szBuffer[512];
    CHAR szOutText[512+20];
    
    HRESULT hr = StringCbVPrintfA(szBuffer, sizeof(szBuffer), lpszFormat, args);
 //   ATLASSERT(SUCCEEDED(hr)); //Output truncated as it was > sizeof(szBuffer)
    
    StringCbCopyA(szOutText, sizeof(szOutText), _MYDEBUGFLAG_A);
    StringCbCatA(szOutText, sizeof(szOutText), szBuffer);
    
    OutputDebugStringA(szBuffer);
    va_end(args);
}


#endif