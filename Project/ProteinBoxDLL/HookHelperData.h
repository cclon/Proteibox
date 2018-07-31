#pragma once

//////////////////////////////////////////////////////////////////////////

extern HANDLE g_KeyHandle_PBAutoExec ;

//////////////////////////////////////////////////////////////////////////
char Handler_RegKey_PBAutoExec( IN char data );
BOOL Handler_RegKey_RegLink();
VOID Handler_RegKey_DisableDCOM();
VOID Handler_RegKey_NukeOnDelete_Recycle();
VOID Handler_RegKey_BrowseNewProcess();
VOID Handler_RegKey_Elimination();
VOID Handler_RegKey_clsid_1();
VOID Handler_RegKey_clsid_2();
VOID HandlerSelfAutoExec();
HANDLE Stub_NtCreateKey_Special( HKEY hRootKey, LPWSTR szKeyName, int *pFlag );
VOID HandlerIEEmbedding();
VOID fake_entrypoint();

///////////////////////////////   END OF FILE   ///////////////////////////////
