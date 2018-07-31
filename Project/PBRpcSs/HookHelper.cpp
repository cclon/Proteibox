/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/11 [11:1:2012 - 13:01]
* MODULE : \PBRpcSs\HookHelper.cpp
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

#include "stdafx.h"
#include "..\Base\mhoolib\mhook.h"
#include "HookHelper.h"

//////////////////////////////////////////////////////////////////////////

HMODULE g_hModule_KernelBase = NULL;
LPMODULEINFO __ProcModulesInfo = NULL;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



BOOL HookOne( LPHOOKINFOLittleRpcss Info )
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 10:35]

Routine Description:
  对单个函数节点进行挂钩  
    
Arguments:
  Info - 结构体指针,包含了函数的原始地址/过滤地址 等信息 
 
--*/
{
	if ( NULL == Info ) { return FALSE ; }
	if ( Info->bHooked ) { return TRUE ; }
	if ( NULL == Info->OrignalAddress || NULL == Info->FakeAddress ) { return TRUE ; }

	return Mhook_SetHook( (PVOID*)&Info->OrignalAddress, Info->FakeAddress );
}


VOID GethModule()
{
	OSVERSIONINFOW VersionInfo = {0};

	RtlZeroMemory( &VersionInfo, sizeof(VersionInfo) );
	VersionInfo.dwOSVersionInfoSize = 0x114;
	GetVersionExW(&VersionInfo);

	if ( NULL == __ProcModulesInfo )
	{
		__ProcModulesInfo = (LPMODULEINFO) kmalloc( sizeof(__ProcModulesInfo) );
		if ( NULL == __ProcModulesInfo )
		{
			MYTRACE( "error! | GethModule() - kmalloc(); |  \n" );
			ExitProcess(0xFFFFFFFF);
		}
	}

	if ( VersionInfo.dwMajorVersion > 6 || VersionInfo.dwMajorVersion == 6 && VersionInfo.dwMinorVersion >= 1 )
	{
		if ( !(g_hModule_KernelBase = (HMODULE) GetModuleHandle("KernelBase.dll")) )
			g_hModule_KernelBase = (HMODULE) LoadLibrary( "KernelBase.dll" );
	}
	else
	{
// 		if ( !(g_hModule_Kernel32 = (HMODULE) GetModuleHandle("kernel32.dll")) )
// 			g_hModule_Kernel32 = (HMODULE) LoadLibrary( "kernel32.dll" );
	}

	__ProcModulesInfo->hModuleArrays[Ntdll_TAG]			= GetModuleHandle( "ntdll.dll"		 );
	__ProcModulesInfo->hModuleArrays[Kernel32_TAG]		= GetModuleHandle( "kernel32.dll"   );
	__ProcModulesInfo->hModuleArrays[KernelBase_TAG]	= GetModuleHandle( "KernelBase.dll" );
	__ProcModulesInfo->hModuleArrays[ADVAPI32_TAG]		= GetModuleHandle( "advapi32.dll"	 );
	__ProcModulesInfo->hModuleArrays[WS2_32_TAG]		= GetModuleHandle( "ws2_32.dll"	 );

	return;
}


PVOID kmalloc ( ULONG length )
{
	PVOID buffer = NULL;

	//	buffer = malloc( length );
	buffer = new PVOID[ length ] ;
	if ( NULL == buffer )
	{
		ExitProcess( 0xFFFFFFFF );
	}

	memset( buffer, 0, length * sizeof(PVOID) );
	return (PVOID)buffer ;
}


VOID kfree ( PVOID ptr )
{
	if ( ptr && TRUE == MmIsAddressValid(ptr, 1) )
	{
		delete [] ptr ;
		ptr = NULL ;
	}

	return ;
}


BOOL
MmIsAddressValid(
	IN PVOID ptr,
	IN ULONG length
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/27 [27:10:2010 - 10:46]

Routine Description:
  验证指定范围内,内存数据的合法性
    
Arguments:
  ptr - 待验证的指针
  length - 地址长度

Return Value:
  TRUE / FALSE
    
--*/
{
	if ( NULL == ptr ) { return FALSE ; }

	if ( IsBadReadPtr( (const void *)ptr, length ) ) { return FALSE ; }

	return TRUE ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////