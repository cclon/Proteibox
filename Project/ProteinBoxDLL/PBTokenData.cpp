/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/27 [27:12:2010 - 17:12]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBTokenData.cpp
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
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBFilesData.h"
#include "MemoryManager.h"
#include "PBRegsData.h"
#include "PBTokenData.h"

#pragma warning(disable : 4995) 

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


VOID 
NtDuplicateObjectRecorder (
	IN HANDLE SourceHandle,
	IN HANDLE TargetHandle 
	)
{
	int n = 0 ;
	ULONG pNodeHead = 0 ;
	HANDLE CurrentHandle = NULL ;

	if ( NULL == g_Redirectd_FileHandle_Array ) { return; }

	EnterCriticalSection( &g_HandlesLock );

	if ( IsArrayEnd( g_Redirectd_FileHandle_Array ) )
	{ 
		LeaveCriticalSection( &g_HandlesLock );
		return ;
	}

	pNodeHead = g_Redirectd_FileHandle_Array ;
	CurrentHandle = *(HANDLE *) pNodeHead ;

	while ( CurrentHandle != SourceHandle )
	{
		pNodeHead += 4 ;
		CurrentHandle = *(HANDLE *) pNodeHead ;

		// 已遍历至数组末尾,仍未找到对应的句柄
		if ( (HANDLE)0xFFFFFFFF == CurrentHandle ) { goto _over_ ; } 
	}

	if ( NULL == TargetHandle ) { goto _over_ ; }

	pNodeHead = g_Redirectd_FileHandle_Array ;
	while ( CurrentHandle != TargetHandle )
	{
		pNodeHead += 4 ;
		CurrentHandle = *(HANDLE *) pNodeHead ;

		if ( CurrentHandle != (HANDLE)0xFFFFFFFF ) { continue; }

		if ( *(HANDLE *)g_Redirectd_FileHandle_Array != (HANDLE)0xFFFFFFFF )
		{
			pNodeHead = g_Redirectd_FileHandle_Array ;
			CurrentHandle = *(HANDLE *) pNodeHead ;

			while ( CurrentHandle )
			{
				++ n ;
				pNodeHead += 4 ;
				CurrentHandle = *(HANDLE *) pNodeHead ;

				// 已遍历至数组末尾,仍未找到对应的句柄
				if ( (HANDLE)0xFFFFFFFF == CurrentHandle ) { goto _over_ ; } 
			}

			*(HANDLE *)( g_Redirectd_FileHandle_Array + 4 * n ) = TargetHandle ;
		}

		break;
	}

_over_ :
	LeaveCriticalSection( &g_HandlesLock );
	return ;
}


HANDLE 
SecurityObjectKeyFilter (
	IN HANDLE KeyHandle,
	OUT BOOL *bFlag
	)
{
	HANDLE hReg = NULL ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;

	if ( bFlag ) { *bFlag = FALSE; }

	RtlInitUnicodeString( &KeyName, NULL );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | SecurityObjectKeyFilter() - GetRegPath(); | status=0x%08lx \n", status );
		return NULL ;
	}

	WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, OrignalPath, &bIsWhite, &bIsBlack );
	if ( bIsWhite )
	{
		if ( bFlag ) { *bFlag = TRUE; }
	}
	else
	{
		// 处理灰名单
		if ( FALSE == bIsBlack )
		{
			RtlInitUnicodeString( &KeyName, OrignalPath );
			if ( g_NtOpenKey_addr( &hReg, KEY_READ, &ObjAtr) < 0 ) { hReg = NULL ; }
		}
	}

	kfree( OrignalPath );
	kfree( RedirectedPath );
	return hReg ;
}


HANDLE 
SecurityObjectFileFilter (
	IN HANDLE FileHandle,
	OUT BOOL *bFlag
	)
{
	HANDLE hFile = NULL ;
	UNICODE_STRING uniObjectName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	IO_STATUS_BLOCK IostatusBlock ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE ;
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( bFlag ) { *bFlag = FALSE; }

	CMemoryManager::GetInstance().AntiDebuggerEnter(pNode);

	RtlInitUnicodeString( &uniObjectName, NULL );
	InitializeObjectAttributes( &ObjAtr, &uniObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = GetFilePath( &uniObjectName, FileHandle, &OrignalPath, &RedirectedPath, NULL );

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | SecurityObjectFileFilter() - GetFilePath(); | status=0x%08lx \n", status );
		hFile = NULL;
		goto _over_ ;
	}

	WhiteOrBlackEx( OrignalPath, &bIsWhite, &bIsBlack );
	if ( bIsWhite )
	{
		if ( bFlag ) { *bFlag = TRUE; }
	}
	else
	{
		// 处理灰名单
		if ( FALSE == bIsBlack )
		{
			RtlInitUnicodeString( &uniObjectName, OrignalPath );
			if ( g_NtCreateFile_addr(&hFile, FILE_GENERIC_READ, &ObjAtr, &IostatusBlock, 0, 0, 7, 1, 0x20, 0, 0) < 0 ) { hFile = NULL ; }
		}
	}

_over_:
	CMemoryManager::GetInstance().AntiDebuggerLeave(pNode);
	return hFile ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////