/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/09 [9:7:2011 - 0:42]
* MODULE : \PBStart\Common.cpp
* 
* Description:
*
*   共有函数模块
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////


PVOID kmalloc( ULONG length )
{
	PVOID buffer = NULL;
	
	//	buffer = malloc( length );
	buffer = new PVOID[ length ] ;
	if ( NULL == buffer )
	{
		ExitProcess( 0xFFFFFFFF );
	}
	
	memset( buffer, 0, length );
	return buffer ;
}



VOID kfree( PVOID ptr )
{
	if ( ptr && TRUE == MmIsAddressValid(ptr, 1) )
	{
		delete [] ptr ;
		ptr = NULL ;
	}
	
	return ;
}



BOOL
MmIsAddressValid (
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
