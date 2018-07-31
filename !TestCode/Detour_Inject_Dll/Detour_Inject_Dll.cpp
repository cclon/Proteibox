// Detour_Inject_Dll.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////
//
//  Test DetourCreateProcessWithDll function (withdll.cpp).
//
//  Microsoft Research Detours Package, Version 2.1.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include <stdio.h>
#include <windows.h>
#include <detours.h>

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "detoured.lib")

#define arrayof(x)      (sizeof(x)/sizeof(x[0]))

//////////////////////////////////////////////////////////////////////////////
//
void PrintUsage(void)
{
    printf("Usage:\n"
           "    withdll.exe [options] [command line]\n"
           "Options:\n"
           "    /d:file.dll   : Start the process with file.dll.\n"
           "    /p:path       : Specify path to detoured.dll.\n"
           "    /?            : This help screen.\n");
}

//////////////////////////////////////////////////////////////////////////////
//
//  This code verifies that the named DLL has been configured correctly
//  to be imported into the target process.  DLLs must export a function with
//  ordinal #1 so that the import table touch-up magic works.
//
static BOOL CALLBACK ExportCallback(PVOID pContext,
                                    ULONG nOrdinal,
                                    PCHAR pszSymbol,
                                    PVOID pbTarget)
{
    (void)pContext;
    (void)pbTarget;
    (void)pszSymbol;

    if (nOrdinal == 1) {
        *((BOOL *)pContext) = TRUE;
    }
    return TRUE;
}

BOOL DoesDllExportOrdinal1(PCHAR pszDllPath)
{
    HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (hDll == NULL) {
        printf("withdll.exe: LoadLibraryEx(%s) failed with error %d.\n",
               pszDllPath,
               GetLastError());
        return FALSE;
    }

    BOOL validFlag = FALSE;
    DetourEnumerateExports(hDll, &validFlag, ExportCallback);
    FreeLibrary(hDll);
    return validFlag;
}

//////////////////////////////////////////////////////////////////////// main.
//
int CDECL main(int argc, char **argv)
{
    BOOLEAN fNeedHelp = FALSE;
    PCHAR pszDllPath = NULL;
    PCHAR pszDetouredDllPath = NULL;

    int arg = 1;
    for (; arg < argc && (argv[arg][0] == '-' || argv[arg][0] == '/'); arg++) {

        CHAR *argn = argv[arg] + 1;
        CHAR *argp = argn;
        while (*argp && *argp != ':')
            argp++;
        if (*argp == ':')
            *argp++ = '\0';

        switch (argn[0]) {
          case 'd':                                     // Set DLL Name
          case 'D':
            pszDllPath = argp;
            break;

          case 'p':                                     // Path to Detoured.dll
          case 'P':
            pszDetouredDllPath = argp;
            break;

          case '?':                                     // Help
            fNeedHelp = TRUE;
            break;

          default:
            fNeedHelp = TRUE;
            printf("withdll.exe: Bad argument: %s\n", argv[arg]);
            break;
        }
    }

    if (arg >= argc) {
        fNeedHelp = TRUE;
    }

    if (pszDllPath == NULL) {
        fNeedHelp = TRUE;
    }

    if (fNeedHelp) {
        PrintUsage();
        return 9001;
    }

    /////////////////////////////////////////////////////////// Validate DLLs.
    //
    CHAR szDllPath[1024];
    CHAR szDetouredDllPath[1024];
    PCHAR pszFilePart = NULL;

    if (!GetFullPathName(pszDllPath, arrayof(szDllPath), szDllPath, &pszFilePart)) {
        printf("withdll.exe: Error: %s is not a valid path name..\n",
               pszDllPath);
        return 9002;
    }
    if (!DoesDllExportOrdinal1(pszDllPath)) {
        printf("withdll.exe: Error: %s does not export function with ordinal #1.\n",
               pszDllPath);
        return 9003;
    }

    if (pszDetouredDllPath != NULL) {
        if (!GetFullPathName(pszDetouredDllPath,
                             arrayof(szDetouredDllPath),
                             szDetouredDllPath,
                             &pszFilePart)) {
            printf("withdll.exe: Error: %s is not a valid path name.\n",
                   pszDetouredDllPath);
            return 9004;
        }
        if (!DoesDllExportOrdinal1(pszDetouredDllPath)) {
            printf("withdll.exe: Error: %s does not export function with ordinal #1.\n",
                   pszDetouredDllPath);
            return 9005;
        }
    }
    else {
        HMODULE hDetouredDll = DetourGetDetouredMarker();
        GetModuleFileName(hDetouredDll,
                          szDetouredDllPath, arrayof(szDetouredDllPath));
#if 0
        if (!SearchPath(NULL, "detoured.dll", NULL,
                        arrayof(szDetouredDllPath),
                        szDetouredDllPath,
                        &pszFilePart)) {
            printf("withdll.exe: Couldn't find Detoured.DLL.\n");
            return 9006;
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    CHAR szCommand[2048];
    CHAR szExe[1024];
    CHAR szFullExe[1024] = "\0";
    PCHAR pszFileExe = NULL;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    szCommand[0] = L'\0';

#ifdef _CRT_INSECURE_DEPRECATE
    strcpy_s(szExe, sizeof(szExe), argv[arg]);
#else
    strcpy(szExe, argv[arg]);
#endif
    for (; arg < argc; arg++) {
        if (strchr(argv[arg], ' ') != NULL || strchr(argv[arg], '\t') != NULL) {
#ifdef _CRT_INSECURE_DEPRECATE
            strcat_s(szCommand, sizeof(szCommand), "\"");
            strcat_s(szCommand, sizeof(szCommand), argv[arg]);
            strcat_s(szCommand, sizeof(szCommand), "\"");
#else
            strcat(szCommand, "\"");
            strcat(szCommand, argv[arg]);
            strcat(szCommand, "\"");
#endif
        }
        else {
#ifdef _CRT_INSECURE_DEPRECATE
            strcat_s(szCommand, sizeof(szCommand), argv[arg]);
#else
            strcat(szCommand, argv[arg]);
#endif
        }

        if (arg + 1 < argc) {
#ifdef _CRT_INSECURE_DEPRECATE
            strcat_s(szCommand, sizeof(szCommand), " ");
#else
            strcat(szCommand, " ");
#endif
        }
    }
    printf("withdll.exe: Starting: `%s'\n", szCommand);
    printf("withdll.exe:   with `%s'\n\n", szDllPath);
    printf("withdll.exe:   marked by `%s'\n\n", szDetouredDllPath);
    fflush(stdout);

    DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

    SetLastError(0);
    SearchPath(NULL, szExe, ".exe", arrayof(szFullExe), szFullExe, &pszFileExe);
    if (!DetourCreateProcessWithDll(szFullExe[0] ? szFullExe : NULL, szCommand,
                                    NULL, NULL, TRUE, dwFlags, NULL, NULL,
                                    &si, &pi, szDetouredDllPath, szDllPath, NULL)) {
        printf("withdll.exe: DetourCreateProcessWithDll failed: %d\n", GetLastError());
        ExitProcess(9007);
    }

    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) {
        printf("withdll.exe: GetExitCodeProcess failed: %d\n", GetLastError());
        return 9008;
    }

    return dwResult;
}
//
///////////////////////////////////////////////////////////////// End of File.
