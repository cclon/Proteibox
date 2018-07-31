/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/20 [20:5:2010 - 15:42]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ShadowSSDT.c
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
#include "Common.h"
#include "SdtData.h"
#include "LDasm.h"
#include "DispatchIoctl.h"
#include "ShadowSSDTProc.h"
#include "ShadowSSDT.h"


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


NTSTATUS
Ioctl_HookShadow (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	BOOL bRet = TRUE ;
	LPIOCTL_HOOKSHADOW_BUFFER Buffer = NULL ;

	//
	// 1. 校验参数合法性
	//

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_HookShadow(); \n" );

	if ( NULL == pInBuffer )
	{
		dprintf( "error! | Ioctl_HookShadow(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	//
	// 2. 取出R3的Buffer内容,决定是否Hook
	//

	__try
	{
		Buffer = (LPIOCTL_HOOKSHADOW_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_HookShadow() - getIoctlBufferBody(); | Body地址不合法 \n" );
			__leave ;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_HookShadow() - __try __except(); \n" );
		return STATUS_INVALID_ADDRESS ;
	}

	//
	// 3. 根据标记操作Shadow函数
	//

	if ( Buffer->bHook )
	{
		// 开启Hook
		bRet = HookShadowSSDT() ;
		if ( FALSE == bRet )
		{
			dprintf( "error! | Ioctl_HookShadow() - HookShadowSSDT(); \n" );
			return STATUS_UNSUCCESSFUL ;
		}
	}
	else
	{
		// 停止Hook
		UnhookShadowSSDT() ;
	}

	return STATUS_SUCCESS ;
}



BOOL
HookShadowSSDT (
	)
{
	BOOL bRet = FALSE, bSkipOnce = FALSE ;
	NTSTATUS status	= STATUS_UNSUCCESSFUL ;
	PEPROCESS	proc		= NULL	;
	LPWSTR	 wszModuleName	= NULL	;
	LPMPNODE pResult		= NULL	;
	LPSTR	 szFunctionName	= NULL	;
	LPSSDT_SSSDT_FUNC pArray = NULL ;
	int	i = 0, j = 0 ;
	ULONG MappedFuncAddr = 0, Tag = 0 ;
	int ShadowArrayIndex = 0, TotalCounts = 0 ;

	//
	// 1.初始化
	//

	if ( KernelMode == ExGetPreviousMode()) { return FALSE ; }

	//
	// 2. 获取Shadow SSDT中指定函数地址
	//

	if ( FALSE == g_SdtData.bInited || NULL == g_SdtData.pSdtArray || 0 == g_SdtData.ShadowSSDTCounts )
	{
		dprintf( "error! | HookShadowSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return FALSE ;
	}

	pArray = g_SdtData.pSdtArray ;
	TotalCounts		 = g_SdtData.TotalCounts	  ;
	ShadowArrayIndex = g_SdtData.ShadowArrayIndex ;

	//
	// 3. Shadow表需要附着到GUI进程空间才可读取地址,因为当前是IOCTL通信中,故已在R3进程中
	//

	for( i = ShadowArrayIndex; i < TotalCounts; i++ )
	{
		// 3.1 确保遍历的是Shadow ssdt数组部分
		Tag = pArray[ i ].Tag ;

		if ( FALSE == IS_ShadowSSDT_TAG( Tag ) ) { continue ; }

		if ( pArray[ i ].SpecailVersion && pArray[ i ].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			continue ; // 表明当前函数不在当前平台上,无需关注
		}

		// 获取每个Shadow SSDT单元的函数地址,保存于单元内
		wszModuleName	= pArray[ i ].wszModuleName	 ;
		szFunctionName	= pArray[ i ].szFunctionName ;

		// 映射一份数据到内存,得到Mapped的函数地址
		pArray[ i ].MappedFuncAddr = MappedFuncAddr = GetProcAddress( wszModuleName, szFunctionName, TRUE );
		if ( 0 == MappedFuncAddr )
		{
			dprintf( "error! | HookShadowSSDT() - GetProcAddress(); | can't get mapped addr: \"0x%08lx\" \n", szFunctionName );
			continue ;
		}

		// 3.2 获取函数真实地址
		bRet = Get_sdt_function_addr( (PVOID)&pArray[i], _AddressFindMethod_Shadow_, _IndexCheckMethod_Shadow_ );
		if ( FALSE == bRet )
		{
			dprintf( "error! | Handler_SSDT() - Get_ssdt_function_addr(); \n" );
			continue ;
		}

		//
		// 3.3 因为多个user32!函数可以对应一个win32k!ntuser*函数,故查看数组中当前位置的Index在其上方是否有相同的,
		//    没有则表明是第一个,可以hook,有相同的,表明该ntuser*函数已被Hook过,不要进行重复Hook
		//

		bSkipOnce = FALSE ;

		for ( j=i-1; j>=0; j-- )
		{
			if ( FALSE == IS_ShadowSSDT_TAG( pArray[j].Tag ) ) { break ; }

			if (   pArray[i]._IndexU_.xxIndex == pArray[j]._IndexU_.xxIndex
				&& pArray[i].RealFuncAddr == pArray[j].RealFuncAddr
				&& pArray[j].bHooked 
				)
			{
				bSkipOnce = TRUE ;
				break ;
			}
		}

		if ( bSkipOnce ) { continue ; }

		// 3.4 Inline Hook
		if ( pArray[i]._AddrU_.fakeFuncAddr )
		{
			PatchSDTFunc( (PVOID)&pArray[i], TRUE );
		}
	}

	if ( i >= TotalCounts ) 
	{ 
		bRet = TRUE ;
	//	SDTWalkNodes( _SDTWalkNodes_Shadow_ ) ;
		dprintf( "ok! | HookShadowSSDT(); \n" );
	}

	// 4.1 获取EnableWindow对应的xpfnProc
	HandlerEnableWindow();

	// 4.2 处理SystemParametersInfoW索引号可能获取失败的情况
	HandlerSystemParametersInfoW() ;

	//
	// 5. 卸载掉我们映射到CSRSS.EXE进程空间的user32.dll内存,由于驱动卸载时才会统一卸载映射模块,关闭句柄,
	//    但此时是在SYSTEM领空,关闭文件句柄时蓝屏.故在Dettach之前,做清理操作
	//

	pResult = (LPMPNODE) kgetaddrMP( L"USER32" );
	if ( pResult )
	{
		if ( pResult->MappedAddr )
			ZwUnmapViewOfSection( (HANDLE)0xFFFFFFFF, (PVOID)pResult->MappedAddr );

		if ( pResult->SectionHandle )
			ZwCloseSafe( pResult->SectionHandle, TRUE );

		if ( pResult->hFile )
			ZwCloseSafe( pResult->hFile, TRUE );

		pResult->MappedAddr = 0 ; 
		pResult->SectionHandle = NULL ;
		pResult->hFile = NULL ;
	}

	return bRet ;
}



VOID
UnhookShadowSSDT (
	)
{
	UnhookShadowSSDTEx();
	return ;
}



VOID
UnhookShadowSSDTEx (
	)
{
	BOOL bNeedAttach = FALSE ;
	LPSSDT_SSSDT_FUNC pArray = NULL ;
	int i = 0, ShadowArrayIndex = 0, TotalCounts = 0 ;

	// 1. 校验参数合法性
	if ( FALSE == g_SdtData.bInited || NULL == g_SdtData.pSdtArray || 0 == g_SdtData.ShadowSSDTCounts )
	{
		dprintf( "error! | UnhookShadowSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return ;
	}

	if ( KernelMode == ExGetPreviousMode()) 
	{ 
		// 表明不是R3主动要求卸载,而是驱动中强行卸载,此时需要attach到GUI进程
		bNeedAttach = TRUE ;
	}

	if ( bNeedAttach )
	{
		PEPROCESS proc = NULL ;

		NTSTATUS status = GetEProcessByName_QueryInfo( L"csrss.exe", &proc );
		if( !NT_SUCCESS(status) )
		{
			dprintf( "error! | UnhookShadowSSDT() - GetEProcessByName_QueryInfo() | 获取csrss.exe进程对象失败, status=0x%08lx \n", status );
			return ;
		}

		KeAttachProcess ( proc ) ; 
	}

	pArray = g_SdtData.pSdtArray ;
	TotalCounts		 = g_SdtData.TotalCounts	  ;
	ShadowArrayIndex = g_SdtData.ShadowArrayIndex ;

	for( i = ShadowArrayIndex; i < TotalCounts; i++ )
	{
		if ( FALSE == IS_ShadowSSDT_TAG( pArray[ i ].Tag ) ) { continue ; }

		PatchSDTFunc( (PVOID)&pArray[i], FALSE ); // UnHook
	}

	if ( bNeedAttach ) { KeDetachProcess() ; }
	return ;
}



BOOL
HandlerEnableWindow (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/25 [25:8:2010 - 14:52]

Routine Description:
  获取EnableWindow对应的xpfnProc.    
    
--*/
{
	ULONG Index = 0 ;

	Index = HandlerEnableWindowEx( Tag_EnableWindow );
	if ( 0 == Index )
	{
		dprintf( "error! | HandlerEnableWindow() - HandlerEnableWindowEx(); | 0 == Index \n" );
		return FALSE ;
	}

	g_Win32k_apfnSimpleCall_Index_Info.SFI_XXXENABLEWINDOW = Index ;
	return TRUE ;
}


//////////////////////////////////////////////////////////////////////////

ULONG
HandlerEnableWindowEx (
	IN int Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/25 [25:8:2010 - 13:40]

Routine Description:
  得到Tag对应函数的下层调用函数在数组中的Index;
  eg: EnableWindow最终调用 win32k!xxxEnableWindow, 为其传递的Index=0x60(Vista以后为0x61)

  在User32!EnableWindow的开头0xC字节内查找:
  kd> u 77d1c4d4 
  USER32!EnableWindow:
  77d1c4d4 8bff            mov     edi,edi
  77d1c4d6 55              push    ebp
  77d1c4d7 8bec            mov     ebp,esp
  77d1c4d9 6a60            push    60h // 找的是这个值
  77d1c4db ff750c          push    dword ptr [ebp+0Ch]
  77d1c4de ff7508          push    dword ptr [ebp+8]
  77d1c4e1 e8daffffff      call    USER32!NtUserCallHwndParamLock (77d1c4c0)
  77d1c4e6 5d              pop     ebp
  77d1c4e7 c20800          ret     8

  BOOL EnableWindow( HWND hwnd, BOOL bEnable)
  {
      return (BOOL)NtUserCallHwndParamLock(hwnd, bEnable,SFI_XXXENABLEWINDOW);
  }

  很多函数都会调用NtUserCallHwndParamLock,区别在于参数三@xpfnProc,它是一个数组序号(Index),
  系统会根据该序号在数组中找到对应的函数调用之; apfnSimpleCall[xpfnProc](pwnd, dwParam);

  比如关注EnableWindow对应的序号,则在此查找之;

Arguments:
  Tag - 函数对应的Tag,用于查找数组中的指定函数

Return Value:
  数组序列号 | 0

--*/
{
	ULONG pAddr = 0, Len = 0, ret = 0 ;
	PUCHAR opcode = NULL ;
	LPSSDT_SSSDT_FUNC pNode = NULL ;

	pNode = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag );
	if ( NULL == pNode ) 
	{
		dprintf( "error! | HandlerEnableWindowEx() - Get_sdt_Array(); | NULL == pNode \n" );
		return 0 ;
	}

	pAddr = pNode->MappedFuncAddr ;
	if ( 0 == pAddr ) 
	{
		dprintf( "error! | HandlerEnableWindowEx(); | NULL == pNode->MappedFuncAddr \n" );
		return 0 ;
	}

	for( ; pAddr<pNode->MappedFuncAddr + 0xC; pAddr += Len )
	{
		Len = SizeOfCode( (PUCHAR)pAddr, &opcode ) ;
		if( !Len ) { Len++; continue ; }

		if (   ( 0x6A == *(PUCHAR)pAddr )
			&& ( *(PUCHAR)(pAddr+1) >= 0x50 && *(PUCHAR)(pAddr+1) <= 0x70 )
			&& ( 0xFF == *(PUCHAR)(pAddr+2) )
			) 
		{
			ret = *(PUCHAR)(opcode + 1) ;
			break ;
		}
	}

	return ret ;
}



static const BYTE g_RealSystemParametersInfoW_Hardcode [] = 
{
	/*
	第1组数据对应
	77d19cfd c9              leave
	77d19cfe c21000          ret     10h
	*/
	0x0,
	0x10,
	0xC2,
	0xC9,

	/*
	第2组数据对应
	77d19ceb e89fefffff  call  USER32!NtUserSystemParametersInfo (77d18c8f)
	*/
	0xE8,
	0xFF, // 结束匹配的标记
	0,
	0
};


VOID
HandlerSystemParametersInfoW (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/26 [26:8:2010 - 14:11]

Routine Description:
  若普通方式获取 SystemParametersInfoW索引号失败,在此通过特殊方式再次获取之:
  在SystemParametersInfoW之上是USER32!RealSystemParametersInfoW,其不远处会调用 
  call    USER32!NtUserSystemParametersInfo; 找到该地址,获取Index即可.

  USER32!RealSystemParametersInfoW+0x28c:
  77d19ce0 8b7d0c          mov     edi,dword ptr [ebp+0Ch]
  77d19ce3 ff7514          push    dword ptr [ebp+14h]
  77d19ce6 56              push    esi
  77d19ce7 57              push    edi
  77d19ce8 ff7508          push    dword ptr [ebp+8]
  77d19ceb e89fefffff      call    USER32!NtUserSystemParametersInfo (77d18c8f) // ******
  77d19cf0 837d0851        cmp     dword ptr [ebp+8],51h
  77d19cf4 0f839a000000    jae     USER32!RealSystemParametersInfoW+0x2a6 (77d19d94)
  77d19cfa 5f              pop     edi
  77d19cfb 5e              pop     esi
  77d19cfc 5b              pop     ebx
  77d19cfd c9              leave
  77d19cfe c21000          ret     10h
  77d19d01 90              nop
  77d19d02 90              nop
  77d19d03 90              nop
  77d19d04 90              nop
  77d19d05 90              nop
  USER32!SystemParametersInfoW:
  77d19d06 6a10            push    10h
    
  USER32!NtUserSystemParametersInfo:
  77d18c8f b82f120000      mov     eax,122Fh // 取出Index
  77d18c94 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
  77d18c99 ff12            call    dword ptr [edx]
  77d18c9b c21000          ret     10h

--*/
{
	ULONG Index = 0 ;
	BOOL bRet = TRUE ;
	LPSSDT_SSSDT_FUNC pNode = NULL ;

	// 1. 获取 SystemParametersInfoW 函数对应节点
	pNode = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag_SystemParametersInfoW );
	if ( NULL == pNode ) 
	{
		dprintf( "error! | HandlerSystemParametersInfoW() - Get_sdt_Array(); | NULL == pNode \n" );
		return ;
	}

	__try
	{
		PBYTE ptr, pHardcode ;
		int n = 0, addr = 0, NtUserSystemParametersInfo_addr = 0 ;

		// 2. 验证节点合法性
		if ( pNode->_IndexU_.xxIndex && pNode->RealFuncAddr && pNode->bHooked ) { return ; }

		addr = pNode->MappedFuncAddr ;
		if ( 0 == addr || FALSE == MmIsAddressValid( (PVOID)addr ) )
		{
			dprintf( "error! | HandlerSystemParametersInfoW();| pNode->MappedFuncAddr 地址不合法 \n" );
			return ;
		}

		// 3. 查找匹配之
		pHardcode = (PBYTE) g_RealSystemParametersInfoW_Hardcode ;

		while ( n++ < 0x30 )
		{
			ptr = (PBYTE) (addr - n) ;
			if ( *pHardcode != *ptr ) { continue ; }
			
			++ pHardcode ;
			if ( 0xFF != *pHardcode ) { continue ; } // 结束符标记0xFF
			
			NtUserSystemParametersInfo_addr = (int) ((int)ptr +  *(int *)(ptr + 1) + 5) ;
			ProbeForRead( (PVOID)NtUserSystemParametersInfo_addr, 0x10, 1 );

			ptr = (PCHAR) NtUserSystemParametersInfo_addr ;

			if (   0xB8 == *(BYTE *)ptr
				&& 0xBA == *(BYTE *)(ptr + 5)
				&& 0xC2 == *(BYTE *)(ptr + 0xC) 
				&& 0x10 == *(BYTE *)(ptr + 0xD) 
				)
			{
				Index = pNode->_IndexU_.ShadowIndex = *(ULONG *)(ptr + 1) ;
				break ;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | HandlerSystemParametersInfoW() - __try __except(); \n" );
	    return ;
	}

	// 4.1 验证索引号的合法性
	if ( 0 == Index || Index >= 0x2000 || (Index & 0xFFF) >= 0x600 || 0x1000 != (Index & 0xF000) )
	{ 
		dprintf( "error! | HandlerSystemParametersInfoW(); | 通过暴力方式搜索到的索引号不合法; Index = %d \n", Index );
		return ; 
	}

	// 4.2 已通过搜索方式找到Index,获取对应的函数真是地址
	pNode->RealFuncAddr = Get_sdt_function_addr_normal( Index, pNode->ArgumentNumbers );
	if ( 0 == pNode->RealFuncAddr )
	{
		dprintf( "error! | HandlerSystemParametersInfoW() - Get_sdt_function_addr_normal(); \n" );
		return ;
	}

	// 4.3 开启Hook
	if ( pNode->_AddrU_.fake_shadowFunc_addr )
	{
		PatchSDTFunc( (PVOID)pNode, TRUE );
	}

	return ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////