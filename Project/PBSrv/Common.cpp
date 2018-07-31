/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/26 [26:7:2011 - 18:20]
* MODULE : \PBSrv\Common.cpp
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



LPLIST_ENTRY_EX 
ClearStruct( 
	IN LPLIST_ENTRY_EX pNode
	)
{
	if ( NULL == pNode ) { return NULL; }

	pNode->Flink		= NULL ;
	pNode->Blink		= NULL ;
	pNode->TotalCounts	= 0	   ;

	return pNode;
}




PLIST_ENTRY
RemoveEntryListEx (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pCurrentNode
	)
{
	struct _LIST_ENTRY *pBlink ;	
	struct _LIST_ENTRY *pFlink ;	

	pBlink = pCurrentNode->Blink ;
	pFlink = pCurrentNode->Flink ;

	-- pNodeHead->TotalCounts;

	if ( pBlink )
	{
		if ( pFlink )
		{
			pBlink->Flink = pFlink ;
			pFlink->Blink = pBlink ;
		}
		else
		{
			pNodeHead->Blink = pBlink ;
			pBlink->Flink = NULL ;
		}
	}
	else
	{
		if ( pFlink )
		{
			pNodeHead->Flink = pFlink ;
			pFlink->Blink = NULL ;
		}
		else
		{
			pNodeHead->Blink = NULL ;
			pNodeHead->Flink = NULL ;
		}
	}

	return pBlink ;
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
	return buffer ;
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



void kfreeEx( PVOID pBuffer )
{
	DWORD ErrorCode = 0 ;

	ErrorCode = GetLastError();
	kfree( pBuffer );
	SetLastError( ErrorCode );

	return;
}



///////////////////////////////   END OF FILE   ///////////////////////////////