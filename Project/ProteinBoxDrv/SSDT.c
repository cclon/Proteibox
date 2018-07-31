/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/19 [19:5:2010 - 10:41]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\SSDT.c
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"
#include "SdtData.h"
#include "SSDT.h"

//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
HandlerSSDT(
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 16:46]

Routine Description:
  获取部分SSDT函数地址

--*/
{
	BOOL bRet = FALSE ;
	int	 i	  =	0, TotalCounts = 0  ;
	ULONG	MappedFuncAddr	= 0		;
	LPWSTR	wszModuleName	= NULL	;
	LPSTR	szFunctionName	= NULL	;
	LPSSDT_SSSDT_FUNC pArray = NULL ;

	//
	// 1.初始化
	//

	bRet = InitSdtData();
	if ( FALSE == bRet )
	{
		dprintf( "error! | HandlerSSDT() - InitSdtData | 初始化MP结构数据失败 \n" );
		return FALSE ;
	}

	//
	// 2.1 获取SSDT地址; 这些函数地址都是Zw*的地址,用于调用,不用于Hook
	//

	if (   FALSE == g_SdtData.bInited 
		|| NULL == g_SdtData.pSdtArray || 0 == g_SdtData.TotalCounts
		|| NULL == g_SdtData.SpecArray || 0 == g_SdtData.SpecCounts
		)
	{
		dprintf( "error! | HandlerSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return FALSE ;
	}

	pArray = g_SdtData.pSdtArray ;
	TotalCounts = g_SdtData.TotalCounts ;

	for( i=0; i<TotalCounts; i++ )
	{
		// 确保遍历的是SSDT数组部分
		if ( FALSE == IS_SSDT_TAG( pArray[i].Tag ) ) 
		{
			// 已经到了Shadow SSDT数组中了,记录此时的序号(Index),供后期 Handler_ShadowSSDT()函数使用
			g_SdtData.ShadowArrayIndex = i ;
			g_SdtData.ShadowSSDTCounts = TotalCounts - i ;
			break ;
		}

		if ( pArray[ i ].SpecailVersion && pArray[ i ].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			// 表明当前函数不在当前平台上,无需关注
			continue ;
		}

		// 获取每个SSDT单元的函数地址,保存于单元内
		wszModuleName	= pArray[ i ].wszModuleName	 ;
		szFunctionName	= pArray[ i ].szFunctionName ;

		pArray[ i ].MappedFuncAddr = MappedFuncAddr = GetProcAddress( wszModuleName, szFunctionName, TRUE );
		if ( 0 == MappedFuncAddr )
		{
			dprintf( "error! | HandlerSSDT() - GetProcAddress(); | can't get mapped addr: \"%s\" \n", szFunctionName );
			continue ;
		}

		bRet = Get_sdt_function_addr( (PVOID)&pArray[i], _AddressFindMethod_SSDT_, _IndexCheckMethod_SSDT_ );
		if ( FALSE == bRet )
		{
			dprintf( "error! | HandlerSSDT() - Get_ssdt_function_addr(); \n" );
			continue ;
		}
	}

//	SDTWalkNodes( _SDTWalkNodes_SSDT_ ) ;

	//
	// 2.2 获取特殊的ssdt函数地址; 这些函数地址都是Nt*的地址,用于Hook,不用于调用
	//

	pArray = g_SdtData.SpecArray ;
	TotalCounts = g_SdtData.SpecCounts ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( pArray[ i ].SpecailVersion && pArray[ i ].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			// 表明当前函数不在当前平台上,无需关注
			continue ;
		}

		// 获取每个SSDT单元的函数地址,保存于单元内
		wszModuleName	= pArray[ i ].wszModuleName	 ;
		szFunctionName	= pArray[ i ].szFunctionName ;

		pArray[ i ].MappedFuncAddr = MappedFuncAddr = GetProcAddress( wszModuleName, szFunctionName, TRUE );
		if ( 0 == MappedFuncAddr )
		{
			dprintf( "error! | HandlerSSDT() - GetProcAddress(); | can't get mapped addr: \"%s\" \n", szFunctionName );
			continue ;
		}

		bRet = Get_sdt_function_addr( (PVOID)&pArray[i], _AddressFindMethod_Shadow_, _IndexCheckMethod_SSDT_ );
		if ( FALSE == bRet )
		{
			dprintf( "error! | HandlerSSDT() - Get_ssdt_function_addr(); \n" );
			continue ;
		}
	}

	//
	// 3. 开启对RPC函数的Hook
	//

	HookSSDT ();

	dprintf( "ok! | HandlerSSDT(); \n" );
	return TRUE ;
}



VOID HookSSDT ()
{
	PatchSSDT( TRUE );
}



VOID UnhookSSDT ()
{
	PatchSSDT( FALSE );
}



VOID
PatchSSDT (
	IN BOOL bFlag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 18:06]

Routine Description:
  对指定的N个ssdt函数进行Inline(Hook/Unhook)   
    
Arguments:
  bFlag - TRUE为开启InlineHook; FALSE为关闭InlineHook

--*/
{
	ULONG TotalCounts = 0, i= 0 ;
	LPSSDT_SSSDT_FUNC pArray = NULL ;

	if ( FALSE == g_SdtData.bInited || NULL == g_SdtData.SpecArray || 0 == g_SdtData.SpecCounts )
	{
		dprintf( "error! | PatchSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return ;
	}

	pArray = g_SdtData.SpecArray ;
	TotalCounts = g_SdtData.SpecCounts ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( pArray[i].SpecailVersion && pArray[i].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			continue ; // 表明当前函数不在当前平台上,无需关注
		}

		PatchSDTFunc( (PVOID)&pArray[i], bFlag );
	}

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////