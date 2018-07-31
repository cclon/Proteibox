#include "stdafx.h"
#include "ProteinBoxDLL.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

BOOL g_bDoWork_OK = FALSE ;
BOOL g_bIs_Inside_iexplore_exe = FALSE ;
BOOL g_bIs_Inside_Explorer_exe = FALSE ;
BOOL g_bIs_Inside_PBRpcSs_exe = FALSE ;
BOOL g_bIs_Inside_PBCrypto_exe = FALSE ;
BOOL g_bSID_is_S_1_5_18	= FALSE ;	

LPPROCESSNODE __ProcessNode = NULL ; // 进程总结构体,里面包含了各种信息
const LPWSTR g_PBRpcSs_exe = L"PBRpcSs.exe" ;
const LPWSTR g_PBDcomLaunch_exe = L"PBDcomLaunch.exe" ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+															  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL GetProcessToken( PUNICODE_STRING SidName )
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/13 [13:9:2010 - 15:41]

Routine Description:
  调用者负责释放内存    
    
Arguments:
  SidName - 保存获取的SID

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	DWORD Length = 0 ;
	PSID TokUser = NULL ;
	HANDLE hProcessToken = NULL ;
	BYTE Buffer[256] = { 0 };

	if ( NULL == SidName ) { return FALSE ; }

	if ( FALSE == OpenProcessToken( (HANDLE)0xFFFFFFFF, TOKEN_QUERY, &hProcessToken ))
    {
		MYTRACE( L"error! | GetProcessToken() - OpenProcessToken(); | ErrorCode=(0x%x)\n ", GetLastError() );
		return FALSE;
    }
	
	if ( FALSE == GetTokenInformation( hProcessToken, TokenUser, (PVOID)Buffer, sizeof(Buffer), &Length) )
    {
		MYTRACE( L"error! | GetProcessToken() - GetTokenInformation(); | ErrorCode=(0x%x)\n ", GetLastError() );
		CloseHandle( hProcessToken );
		return FALSE;
    }
	
	TokUser = ((PTOKEN_USER)Buffer)->User.Sid ;
	status = RtlConvertSidToUnicodeString( SidName, TokUser, TRUE );
	if ( !NT_SUCCESS(status) )
    {
		MYTRACE( L"error! | GetProcessToken() - RtlConvertSidToUnicodeString(); | ErrorCode=(0x%x)\n ", GetLastError() );
		CloseHandle( hProcessToken );
		return FALSE;
    }

	CloseHandle( hProcessToken );
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



void kfreeExp( PVOID pBuffer )
{
	DWORD ErrorCode = 0 ;

	ErrorCode = GetLastError();
	kfree( pBuffer );
	SetLastError( ErrorCode );

	return;
}




VOID 
WhiteOrBlack (
	IN int Flag,
	IN LPCWSTR szPath,
	OUT BOOL* bIsWhite,
	OUT BOOL* bIsBlack
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/26 [26:9:2010 - 11:00]

Routine Description:
  判定文件/对象/注册表等的黑白. 驱动中保存着当前进程的配置信息,
  将字符串抛给R0,让其返回结果.
    
Arguments:
  Flag - 标志位
  szPath - 要查找的字符串全路径(文件名/对象名/注册表键值 etc.)
  bIsWhite - 标记@szPath 是否在白名单中
  bIsBlack - 标记@szPath 是否在黑名单中
    
--*/
{
	IOCTL_PROTEINBOX_BUFFER buffer ;
	BOOL __bIsWhite = FALSE, __bIsBlack = FALSE ;

	if ( NULL == szPath || NULL == bIsWhite || NULL == bIsBlack || FALSE == IsWBFlagOK(Flag) )
	{
		MYTRACE( L"error! | WhiteOrBlack() - IsWBFlagOK(); | 参数不合法 \n" );
		return ;
	}

	// 和驱动交互
	buffer.Head.Flag = FLAG_Ioctl_WhiteOrBlack ;
	buffer.WhiteOrBlackBuffer.Flag = Flag ;
	buffer.WhiteOrBlackBuffer.szPath = (LPCWSTR) szPath ;
	buffer.WhiteOrBlackBuffer.PathLength = (wcslen(szPath) + 1) * sizeof(WCHAR) ;
	buffer.WhiteOrBlackBuffer.bIsWhite = &__bIsWhite ;
	buffer.WhiteOrBlackBuffer.bIsBlack = &__bIsBlack ;

	BOOL bRet = g_ProteinBoxDLL.IoControl ( 
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER) 
		);

	*bIsWhite	= __bIsWhite ;
	*bIsBlack	= __bIsBlack ;

	return ;
}



VOID 
WhiteOrBlackEx (
	IN LPWSTR szPath,
	OUT BOOL* bIsWhite,
	OUT BOOL* bIsBlack
	)
{
	int length = 0 ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, Name = NULL ;

	// 校验参数合法性
	if ( NULL == szPath || NULL == bIsWhite || NULL == bIsBlack ) { return ; }

	WhiteOrBlack( WhiteOrBlack_Flag_XFilePath, szPath, bIsWhite, bIsBlack );

	if ( *bIsWhite || *bIsBlack ) { return ; }

	// 处理灰名单
	if ( 0 == wcsnicmp(szPath, L"\\device\\lanmanredirector\\", 0x19) )
	{
		length = 0x19 ;
	}
	else if ( 0 == wcsnicmp(szPath, L"\\device\\mup\\;lanmanredirector\\", 0x1E) )
	{
		length = 0x1E ;
	}
	else
	{
		return ;
	}

	// 处理文件名是 "\\device\\lanmanredirector\\"或者 "\\device\\mup\\;lanmanredirector\\" 的情况
	ptr1 = &szPath[ length ] ;

	if ( ';' == szPath[length] )
		ptr2 = wcschr( ptr1, '\\' );
	else
		ptr2 = ptr1 - 1 ;

	if ( NULL == ptr2 || NULL == *ptr2 || NULL == ptr2[1] ) { return ; }

	ptr3 = ptr2 + 1 ;
	length = wcslen( ptr3 );
	Name = (LPWSTR) kmalloc( length + 0x30 );
	memcpy( Name, L"\\device\\mup\\", 0x18 );
	memcpy( Name+0x18, ptr3, (length + 1) * sizeof(WCHAR) );
//	wsprintf( Name, L"%ws%ws", L"\\device\\mup\\", ptr3 );

	WhiteOrBlack( WhiteOrBlack_Flag_XFilePath, Name, bIsWhite, bIsBlack );

	kfree( Name );
	return ;
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



PVOID
InsertListA (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pNodeCurrent,
	IN PLIST_ENTRY pNodeNew
	)
{
	PVOID Result = (PVOID) pNodeHead ;
	PLIST_ENTRY pFlink, pBlink ;

	if ( NULL == pNodeHead || NULL == pNodeNew ) { return NULL ; }

	++ pNodeHead->TotalCounts ;
	pBlink = pNodeHead->Blink ;

	if ( pNodeCurrent != pBlink && pNodeCurrent )
	{       
		// 将 @NewNode 插入到@pCurrentNode之前
		Result = pNodeNew ;
		pFlink = pNodeCurrent->Flink ;
		
		pNodeCurrent->Flink->Blink	= pNodeNew ;
		pNodeNew->Flink				= pFlink ;
		pNodeCurrent->Flink			= pNodeNew ;
		pNodeNew->Blink				= pNodeCurrent ;
	}
	else
	{                                             
		// 将 @NewNode 插入表头
		pNodeHead->Blink	= pNodeNew ;
		pNodeNew->Blink		= pBlink ;
		pNodeNew->Flink		= 0 ;

		if ( pBlink )
			pBlink->Flink = pNodeNew ;
		else
			pNodeHead->Flink = pNodeNew ;
	}

	return Result ;
}



PVOID 
InsertListB (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pNodeCurrent,
	IN PLIST_ENTRY pNodeNew
	)
{
	PVOID Result = (PVOID) pNodeHead ;
	PLIST_ENTRY pFlink, pBlink ;

	if ( NULL == pNodeHead || NULL == pNodeNew ) { return NULL ; }

	++ pNodeHead->TotalCounts ;
	pFlink = pNodeHead->Flink ;

	if ( pNodeCurrent != pFlink && pNodeCurrent )
	{
		Result = pNodeNew;
		pBlink = pNodeCurrent->Blink ;
		
		pBlink->Flink		= pNodeNew ;
		pNodeNew->Blink		= pBlink ;
		pNodeCurrent->Blink = pNodeNew ;
		pNodeNew->Flink		= pNodeCurrent ;
	}
	else
	{
		pNodeHead->Flink	= pNodeNew ;
		pNodeNew->Blink		= 0 ;
		pNodeNew->Flink		= pFlink ;

		if ( pFlink )
			pFlink->Blink = pNodeNew ;
		else
			pNodeHead->Blink = pNodeNew ;
	}

	return Result ;
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



LONG
NTAPI
RtlCompareUnicodeString (
   IN PCUNICODE_STRING s1,
   IN PCUNICODE_STRING s2,
   IN BOOLEAN  CaseInsensitive
   )
{
   unsigned int len;
   LONG ret = 0;
   LPCWSTR p1, p2;

   len = min(s1->Length, s2->Length) / sizeof(WCHAR);
   p1 = s1->Buffer;
   p2 = s2->Buffer;

   if (CaseInsensitive)
   {
     while (!ret && len--) ret = RtlUpcaseUnicodeChar(*p1++) - RtlUpcaseUnicodeChar(*p2++);
   }
   else
   {
     while (!ret && len--) ret = *p1++ - *p2++;
   }

   if (!ret) ret = s1->Length - s2->Length;
   return ret;
}



WCHAR NTAPI
RtlUpcaseUnicodeChar (
	IN WCHAR Source
	)
{
	USHORT Offset;

	if (Source < 'a')
		return Source;

	if (Source <= 'z')
		return (Source - ('a' - 'A'));

	Offset = 0;

	return Source + (SHORT)Offset;
}



int 
RtlCompareUnicodeStringDummy (
	IN LPWSTR szNameA, 
	IN LPWSTR szNameB,
	IN USHORT Length
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/12/10 [10:12:2010 - 10:50]

Routine Description:
  比较2个宽字符串

Return Value:
  0表示相等
    
--*/
{
	UNICODE_STRING uniBufferA, uniBufferB ;

	uniBufferA.MaximumLength = uniBufferA.Length = 2 * Length ;
	uniBufferB.MaximumLength = uniBufferB.Length = 2 * Length ;

	uniBufferA.Buffer = szNameA ;
	uniBufferB.Buffer = szNameB ;

	return RtlCompareUnicodeString( &uniBufferA, &uniBufferB, TRUE );
}



PIMAGE_NT_HEADERS
SRtlImageNtHeader (
	IN PVOID BaseAddress
	)
{
	PIMAGE_NT_HEADERS NtHeader = NULL ;
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)BaseAddress;

	if (DosHeader && DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)BaseAddress + DosHeader->e_lfanew);
		if (NtHeader->Signature == IMAGE_NT_SIGNATURE)
			return NtHeader;
	}

	return NULL;
}



LPVOID HeapAllocEx( IN DWORD Size )
{
	return HeapAlloc( GetProcessHeap(), 4, Size );
}


///////////////////////////////   END OF FILE   ///////////////////////////////