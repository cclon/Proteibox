/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/21 [21:1:2012 - 12:46]
* MODULE : \Code\Project\ProteinBoxDLL\PBAdvapi32dll\PBAdvapi32dllData.cpp
* 
* Description:
*
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "../StdAfx.h"
#include "../common.h"
#include "../PBDynamicData.h"
#include "../PBComData.h"
#include "../PBFilesData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../MemoryManager.h"
#include "PBAdvapi32dllData.h"


//////////////////////////////////////////////////////////////////////////


HMODULE g_handle_advapi32dll = NULL ;

_CryptProtectData_ g_CryptProtectData_addr = NULL ;
_CryptUnprotectData_	g_CryptUnprotectData_addr = NULL ;
_RegOpenKeyExA_ g_RegOpenKeyExA_addr = NULL ;
_RegOpenKeyExW_ g_RegOpenKeyExW_addr = NULL ;
_RegCloseKey_ g_RegCloseKey_addr = NULL ;
_RegGetKeySecurity_ g_RegGetKeySecurity_addr = NULL ;
_RegSetKeySecurity_ g_RegSetKeySecurity_addr = NULL ;
_GetFileSecurityA_ g_GetFileSecurityA_addr = NULL ;
_GetFileSecurityW_ g_GetFileSecurityW_addr = NULL ;
_SetFileSecurityA_ g_SetFileSecurityA_addr = NULL ;
_SetFileSecurityW_ g_SetFileSecurityW_addr = NULL ;
_LookupAccountNameW_ g_LookupAccountNameW_addr = NULL ;
_LookupPrivilegeValueW_ g_LookupPrivilegeValueW_addr = NULL ;
_RegConnectRegistryA_ g_RegConnectRegistryA_addr = NULL ;
_RegConnectRegistryW_ g_RegConnectRegistryW_addr = NULL ;
_CryptVerifySignatureA_ g_CryptVerifySignatureA_addr = NULL ;
_CryptVerifySignatureW_ g_CryptVerifySignatureW_addr = NULL ;
_WSANSPIoctl_ g_WSANSPIoctl_addr = NULL ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


HMODULE Get_advapi32dll_Handle()
{
	HMODULE ret = NULL ;

	ret = g_handle_advapi32dll;
	if ( NULL == g_handle_advapi32dll )
	{
		ret = LoadLibraryW( L"advapi32.dll" );
		g_handle_advapi32dll = ret;
	}

	return ret;
}


///////////////////////////////   END OF FILE   ///////////////////////////////