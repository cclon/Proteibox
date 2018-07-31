#include "StdAfx.h"
#include "MemoryManager.h"

//////////////////////////////////////////////////////////////////////////

ULONG __FuckTag = 'adus';


//////////////////////////////////////////////////////////////////////////

CMemoryManager::CMemoryManager(void)
{
	m_dwTlsIndex = TLS_OUT_OF_INDEXES ;
}

CMemoryManager::~CMemoryManager(void)
{
}


LPVOID CMemoryManager::ExAllocatePool( IN LPMMNODETLS pTLSData, IN ULONG Size )
{
	LPVOID pData = NULL ;

	if ( Size <= 0 ) { return NULL; }

	if ( Size <= 0xC000 )
		pData = ExAllocatePoolL( pTLSData, (Size + 0x7F) >> 7 );
	else
		pData = ExAllocatePoolB( pTLSData, Size );

	return pData;
}



LPVOID CMemoryManager::ExAllocatePoolL( IN LPMMNODETLS pTLSData, IN ULONG Size )
{
	LPBYTE ptr = NULL ;
	LPVOID ret = NULL ;
	ULONG data = 0, nIndex = 0, IndexFucked = 0, SizeDummy = Size ;
	LPMMNODEL pNodeLA = NULL, pNodeLANext = NULL, pFlink = NULL, pBlink = NULL, pOld = NULL, pTemp = NULL ;

	if ( NULL == pTLSData ) { return NULL; }

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data + 1) );

	if ( 0 == data ) { EnterCriticalSection(&pTLSData->cs); }

	pNodeLA = (LPMMNODEL) pTLSData->ListEntryExA.Flink ;
	if ( pNodeLA )
	{
		// 枚举每个内存节点pNodeL
		do
		{
			pNodeLANext = pNodeLA->Flink ;
			if ( pNodeLA->Tag != pTLSData->Tag )
			{
				OutputDebugStringW( L"pool abend POOL_GET_CELLS_EYECATCHER_MISMATCH\n" );
				__asm { int 3 }
			}
	
			if ( (pNodeLA->Size & 0xFFFF) < Size ) { goto _WHILE_NEXT_; }

			nIndex = MMGetIndex( pNodeLA, Size );
			if ( nIndex != 0xFFFFFFFF ) { goto _GOON_; }

			pNodeLA->Size = Size - 1;
			if ( (pNodeLA->Size & 0xFFFF) >= 4 ) { goto _WHILE_NEXT_; }

			//
			pBlink = pNodeLA->Blink ;
			pFlink = pNodeLA->Flink ;
			-- pTLSData->ListEntryExA.TotalCounts ;

			if ( pBlink )
			{
				if ( pFlink )
				{
					pBlink->Flink = pFlink ;
					pFlink->Blink = pBlink ;
				}
				else
				{
					pTLSData->ListEntryExA.Blink = (LIST_ENTRY *)pBlink ;
					pBlink->Flink = NULL ;
				}
			}
			else
			{
				if ( pFlink )
				{
					pTLSData->ListEntryExA.Flink = (LIST_ENTRY *)pFlink ;
					pFlink->Blink = NULL ;
				}
				else
				{
					pTLSData->ListEntryExA.Blink = NULL ;
					pTLSData->ListEntryExA.Flink = NULL ;
				}
			}

			++ pTLSData->ListEntryExB.TotalCounts ;

			pOld = (LPMMNODEL)pTLSData->ListEntryExB.Flink ;
			pTLSData->ListEntryExB.Flink = (LIST_ENTRY *)pNodeLA ;
			
			pNodeLA->Blink = NULL ;
			pNodeLA->Flink = pOld ;
			
			if ( pOld ) {
				pOld->Blink = pNodeLA ;
			} else {
				pTLSData->ListEntryExB.Blink = (LIST_ENTRY *)pNodeLA ;
			}

_WHILE_NEXT_:
			pNodeLA = pNodeLANext ;
		}
		while ( pNodeLANext );
	}

	pNodeLA = MMAllocateNode( pTLSData, pTLSData->Tag );
	if ( pNodeLA )
	{
		nIndex = 0;
		SizeDummy = Size;
_GOON_:
		pNodeLA->Size -= SizeDummy ;
		if ( pNodeLA->Size < 4 )
		{
			RemoveEntryListEx( &pTLSData->ListEntryExA, (LIST_ENTRY *)pNodeLA );

			++ pTLSData->ListEntryExB.TotalCounts ;

			pTemp = (LPMMNODEL) pTLSData->ListEntryExB.Flink ;
			pTLSData->ListEntryExB.Flink = (LIST_ENTRY *)pNodeLA ;
			
			pNodeLA->Blink = NULL ;
			pNodeLA->Flink = pTemp ;
			if ( pTemp )
				pTemp->Blink = pNodeLA ;
			else
				pTLSData->ListEntryExB.Blink = (LIST_ENTRY *)pNodeLA ;
		}

		IndexFucked = 1 << (nIndex & 7);
		ptr = &pNodeLA->MemoryInUseFlag[ nIndex >> 3 ];
		while ( SizeDummy )
		{
			if ( IndexFucked != 1 || SizeDummy <= 8 )
			{
				*ptr |= IndexFucked;
				IndexFucked = (unsigned __int8)(2 * IndexFucked) | (2 * IndexFucked >> 8);
				ptr += IndexFucked & 1;
				--SizeDummy;
			}
			else
			{
				*ptr++ = -1;
				SizeDummy -= 8;
			}
		}

		ret = (LPVOID)((char *)pNodeLA + 128 * (nIndex + 2));
	}

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	if ( 1 == data ) { LeaveCriticalSection( &pTLSData->cs ); }
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data - 1) );
	
	return ret;
}



LPVOID CMemoryManager::ExAllocatePoolB( IN LPMMNODETLS pTLSData, IN ULONG Size )
{
	ULONG SizeDummy = 0, data = 0 ;
	LPMMNODETLS pTLSDataCur = NULL ;
	LPMMNODEL pNodeL = NULL, pNodeFuck = NULL ;


	SizeDummy = (Size + 0x10017) & 0xFFFF0000 ;
	pTLSDataCur = (LPMMNODETLS) VirtualAlloc( 0, SizeDummy, 0x3000, PAGE_READWRITE );
	if ( NULL == pTLSDataCur ) { return NULL; }

	pNodeL = (LPMMNODEL)((char *)pTLSDataCur + SizeDummy - 0x18);

	pNodeL->Tag				= pTLSData->Tag	;
	pNodeL->Size			= SizeDummy		;
	pNodeL->pTLSDataGlobal	= pTLSData		;
	pNodeL->pTLSDataCur		= pTLSDataCur	;

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data + 1) );
	if ( 0 == data ) { EnterCriticalSection( &pTLSData->cs ); }

	++ pTLSData->ListEntryExFuck.TotalCounts ;

	pNodeFuck = (LPMMNODEL) pTLSData->ListEntryExFuck.Flink ;
	pTLSData->ListEntryExFuck.Flink = (LIST_ENTRY *)pNodeL ;
	
	pNodeL->Blink = NULL ;
	pNodeL->Flink = pNodeFuck ;
	
	if ( pNodeFuck )
		pNodeFuck->Blink = pNodeL ;
	else
		pTLSData->ListEntryExFuck.Blink = (LIST_ENTRY *)pNodeL ;

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	if ( 1 == data ) { LeaveCriticalSection( &pTLSData->cs ); }
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data - 1) );

	return pTLSDataCur ;
}



ULONG CMemoryManager::MMGetIndex( IN LPMMNODEL pNodeL, IN ULONG Size )
{
	LPBYTE ptr = NULL ;
	ULONG n = 1, Data = 0, Temp = 0 ; 
	ULONG nIndex = 0, nIndexDummy = 0 ;

	ptr = pNodeL->MemoryInUseFlag ;

	do
	{
		Data = n & *ptr;
		n = (unsigned __int8)(2 * n) | (2 * n >> 8);
		ptr += n & 1;
		nIndexDummy = nIndex + 1;

		if ( Data )
		{
			while ( nIndexDummy < 0x1FE )
			{
				if ( n != 1 || *ptr != 0xFF )
				{
					if ( !((BYTE)n & *ptr) )
						break;

					n = (unsigned __int8)(2 * n) | (2 * n >> 8);
					++nIndexDummy;
					ptr += n & 1;
				}
				else
				{
					nIndexDummy += 8;
					++ptr;
				}
			}
		}
		else
		{
			if ( nIndexDummy < 0x1FE )
			{
				Temp = 1;
				do
				{
					if ( Temp >= Size ) { break; }
		
					if ( n != 1 || *ptr )
					{
						if ( (BYTE)n & *ptr ) { break; }

						n = (unsigned __int8)(2 * n) | (2 * n >> 8);
						++nIndexDummy;
						++Temp;
						ptr += n & 1;
					}
					else
					{
						nIndexDummy += 8;
						Temp += 8;
						++ptr;
					}
				}
				while ( nIndexDummy < 0x1FE );
			}

			if ( nIndexDummy - nIndex >= Size ) { return nIndex; }
		}

		nIndex = nIndexDummy;
	}
	while ( nIndexDummy < 0x1FE );

	return 0xFFFFFFFF;
}



LPMMNODEL CMemoryManager::MMAllocateNode( IN LPMMNODETLS pTLSData, IN ULONG Tag )
{
	LPMMNODEL pNode = NULL, pOld = NULL ;

	pNode = (LPMMNODEL) VirtualAlloc( 0, 0x10000, 0x3000, PAGE_READWRITE );
	if ( NULL == pNode ) { return NULL; }
	
	pNode->Tag				= Tag ;
	pNode->Size				= 0x1FE ;
	pNode->pTLSDataGlobal	= NULL ;
	
	if ( pTLSData )
	{
		memcpy( pNode->MemoryInUseFlag, pTLSData->MemoryInUseFlag, sizeof(pNode->MemoryInUseFlag) );

		pNode->pTLSDataCur = pTLSData ;
		pOld = (LPMMNODEL)pTLSData->ListEntryExA.Flink ;
		++ pTLSData->ListEntryExA.TotalCounts ;
		pTLSData->ListEntryExA.Flink = (LIST_ENTRY *)pNode ;

		pNode->Blink = NULL ;
		pNode->Flink = pOld ;
		if ( pOld )
			pOld->Blink = pNode ;
		else
			pTLSData->ListEntryExA.Blink = (LIST_ENTRY *)pNode ;
	}

	return pNode;
}



LPMMNODETLS CMemoryManager::MMCreateTLS()
{
	return _MMCreateTLS( _sanbox_owner_tag_ );
}



LPMMNODETLS CMemoryManager::_MMCreateTLS( IN ULONG Tag )
{
	BOOL bFlag = FALSE ;
	LPBYTE MemoryInUseFlag = NULL ;
	LPMMNODEL pNodeL = NULL, pBlink = NULL ;
	LPMMNODETLS pTLSData = NULL ;

	if ( m_dwTlsIndex != TLS_OUT_OF_INDEXES || (m_dwTlsIndex = TlsAlloc(), m_dwTlsIndex != TLS_OUT_OF_INDEXES) )
	{
		bFlag = TRUE ;
	}

	if ( FALSE == bFlag ) { return NULL; }

	// 1. 为TLSData分配内存
	pNodeL = (LPMMNODEL) VirtualAlloc( 0, 0x10000, 0x3000, PAGE_READWRITE );
	if ( NULL == pNodeL ) { return NULL; }

	MemoryInUseFlag = pNodeL->MemoryInUseFlag;

	pNodeL->Tag				= Tag	;
	pNodeL->Size			= 0x1FE ;
	pNodeL->pTLSDataGlobal	= NULL	;
	
	memset( pNodeL->MemoryInUseFlag, 0, sizeof(pNodeL->MemoryInUseFlag) );

	// 2. 初始化内存使用标志区域
	ULONG nCouts = 0x1FE ;
	ULONG n = 0x3F ;
	ULONG MemoryInitialValue = 0x40 ;

	do
	{
		if ( MemoryInitialValue == 1 )
		{
			MemoryInUseFlag[n] = 0xFF ;
			nCouts += 8;
		}
		else
		{
			MemoryInUseFlag[n] |= MemoryInitialValue;
			++nCouts;
		}

		n = nCouts >> 3;
		MemoryInitialValue = 1 << (nCouts & 7);
	}
	while ( nCouts >> 3 < 0x80 );

	// 3. 填充TLSData
	pTLSData = &pNodeL->TLSDataCur ;
	pNodeL->pTLSDataCur = pTLSData ;
	pTLSData->Tag = Tag ;

	InitializeCriticalSectionAndSpinCount( &pTLSData->cs, 0x3E8 );
	memcpy( pTLSData->MemoryInUseFlag, MemoryInUseFlag, sizeof(pTLSData->MemoryInUseFlag) );

	pTLSData->ListEntryExA.Flink = NULL ;
	pTLSData->ListEntryExA.Blink = NULL ;
	pTLSData->ListEntryExA.TotalCounts = 0 ;
	pBlink = (LPMMNODEL) pTLSData->ListEntryExA.Flink ;
	pTLSData->ListEntryExA.TotalCounts = 1 ;
	pTLSData->ListEntryExA.Flink = (LIST_ENTRY *)pNodeL ;
	
	pNodeL->Blink = NULL ;
	pNodeL->Flink = pBlink;
	if ( pBlink )
		pBlink->Blink = pNodeL ;
	else
		pTLSData->ListEntryExA.Blink = (LIST_ENTRY *)pNodeL ;

	pTLSData->ListEntryExB.Flink = NULL ;
	pTLSData->ListEntryExB.Blink = NULL ;
	pTLSData->ListEntryExB.TotalCounts = 0;

	pTLSData->ListEntryExFuck.Flink = NULL ;
	pTLSData->ListEntryExFuck.Blink = NULL ;
	pTLSData->ListEntryExFuck.TotalCounts = 0;

	*MemoryInUseFlag |= 3 ;

	return pTLSData;
}



VOID CMemoryManager::FreeTLSData( IN LPMMNODETLS pTLSData )
{
	LPMMNODEL pNodeCur = NULL, pNodeNext = NULL ;
	LPMMNODEL pNodeLACur = NULL, pNodeLANext = NULL ;
	LPMMNODEL pNodeLBCur = NULL, pNodeLBNext = NULL, pTemp = NULL ;

	// 1.
	pNodeCur = (LPMMNODEL) pTLSData->ListEntryExFuck.Flink ;
	if ( pNodeCur )
	{
		do
		{
			pNodeNext = pNodeCur->Flink ;
			if ( !VirtualFree(pNodeCur->pTLSDataCur, 0, MEM_RELEASE) ) { goto _error_; }

			pNodeCur = pNodeNext;
		}
		while ( pNodeNext );
	}

	// 2.
	pNodeLACur = (LPMMNODEL) pTLSData->ListEntryExA.Flink ;
	if ( pNodeLACur )
	{
		pTemp = (LPMMNODEL)((ULONG)pTLSData & 0xFFFF0000);
		do
		{
			pNodeLANext = pNodeLACur->Flink ;
			if ( pTemp != pNodeLACur )
			{
				if ( !VirtualFree(pNodeLACur, 0, MEM_RELEASE) ) { goto _error_; }
			}

			pNodeLACur = pNodeLANext ;
		}
		while ( pNodeLANext );
	}

	// 3.
	pNodeLBCur = (LPMMNODEL) pTLSData->ListEntryExB.Flink ;
	if ( pNodeLBCur )
	{
		pTemp = (LPMMNODEL)((ULONG)pTLSData & 0xFFFF0000);
		do
		{
			pNodeLBNext = pNodeLBCur->Flink;
			if ( pTemp != pNodeLBCur )
			{
				if ( !VirtualFree(pNodeLBCur, 0, MEM_RELEASE) ) { goto _error_; }
			}

			pNodeLBCur = pNodeLBNext ;
		}
		while ( pNodeLBNext );
	}

	if ( !VirtualFree((LPVOID)pTemp, 0, MEM_RELEASE) ) { goto _error_; }
	return;

_error_:
	RaiseException( 0xC0000005, 0xC0000025, 0, 0 );
	ExitProcess( 0xFFFFFFFF );
}



VOID CMemoryManager::ExFreePool( IN int lpAddress, IN int Size)
{
	if ( lpAddress && Size )
	{
		if ( lpAddress & 0xFFFF )
			ExFreePoolL( lpAddress, (Size + 0x7F) >> 7 );
		else
			ExFreePoolB( lpAddress, Size );
	}
	else
	{
		OutputDebugStringW(L"pool abend POOL_FREE_NULL_PTR_OR_ZERO_SIZE\n");
		__asm { int 3 }
	}

	return;
}



VOID CMemoryManager::ExFreePoolL( IN int lpAddress, IN int Size)
{
	PBYTE ptr = NULL ;
	ULONG n = 0, Index = 0, data = 0 ;
	LPMMNODETLS pTLSData = NULL ;
	LPMMNODEL pNodeL = NULL, pFlink = NULL, pBlink = NULL ;

	if ( 0 == lpAddress || IsBadReadPtr((PVOID)lpAddress, 4) ) { return; }

	Index = (lpAddress & 0xFFFF - 0x100) >> 7 ;
	pNodeL = (LPMMNODEL)(lpAddress & 0xFFFF0000);
	pTLSData = pNodeL->pTLSDataCur ;
	ptr = &pNodeL->MemoryInUseFlag[ Index >> 3 ];
	n = 1 << (Index & 7);	

	if ( pNodeL->Tag != pTLSData->Tag )
	{
		OutputDebugStringW( L"pool abend POOL_GET_CELLS_EYECATCHER_MISMATCH\n" );
		__asm { int 3 }
	}

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data + 1) );
	
	if ( NULL == data ) { EnterCriticalSection( &pTLSData->cs ); }

	if ( pNodeL->Size < 4 )
	{
		while ( FALSE )
		{
			if ( Size + pNodeL->Size < 4 ) { break; }

			RemoveEntryListEx( &pTLSData->ListEntryExB, (LIST_ENTRY *)pNodeL );

			++ pTLSData->ListEntryExA.TotalCounts ;

			pFlink = (LPMMNODEL)pTLSData->ListEntryExA.Flink ;
			pTLSData->ListEntryExA.Flink = (LIST_ENTRY *)pNodeL ;
			
			pNodeL->Blink = NULL ;
			pNodeL->Flink = pFlink ;

			if ( pFlink )
				pFlink->Blink = pNodeL ;
			else
				pTLSData->ListEntryExA.Blink = (LIST_ENTRY *)pNodeL ;
		}
	}

	pNodeL->Size += Size ;

	while ( Size )
	{
		if ( n != 1 || Size <= 8 )
		{
			*ptr &= ~(BYTE)n;
			n = (unsigned __int8)(2 * n) | (2 * n >> 8);
			ptr += n & 1;
			--Size;
		}
		else
		{
			*ptr++ = 0;
			Size -= 8;
		}
	}

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	if ( 1 == data ) { LeaveCriticalSection( &pTLSData->cs ); }
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data - 1) );
	return;
}



VOID CMemoryManager::ExFreePoolB( IN int lpAddress, IN int Size)
{
	ULONG SizeDummy = 0, data = 0 ;
	LPMMNODETLS pTLSData = NULL ;
	LPMMNODEL pNodeL = NULL, pFlink = NULL, pBlink = NULL ;

	SizeDummy = (Size + 0x10017) & 0xFFFF0000 ;
	pTLSData = *(LPMMNODETLS *)(SizeDummy + lpAddress - 0xC) ;
	pNodeL = (LPMMNODEL)(SizeDummy + lpAddress - 0x18) ;

	if ( pNodeL->Tag != pTLSData->Tag )
	{
		OutputDebugStringW( L"pool abend POOL_GET_CELLS_EYECATCHER_MISMATCH\n" );
		__asm { int 3 }
	}

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data + 1) );

	if ( NULL == data ) { EnterCriticalSection( &pTLSData->cs ); }

	pBlink = pNodeL->Blink ;
	pFlink = pNodeL->Flink ;
	
	-- pTLSData->ListEntryExFuck.TotalCounts ;

	if ( NULL == pBlink )
	{
		if ( NULL == pFlink )
		{
			pTLSData->ListEntryExFuck.Blink = NULL ;
			pTLSData->ListEntryExFuck.Flink = NULL ;
		} 
		else
		{
			pTLSData->ListEntryExFuck.Flink = (LIST_ENTRY *)pFlink ;
			pFlink->Blink = NULL ;
		}
	}
	else
	{
		if ( NULL == pFlink )
		{
			pTLSData->ListEntryExFuck.Blink = (LIST_ENTRY *)pBlink ;
			pBlink->Flink = NULL ;
		}
		else
		{
			pBlink->Flink = pFlink ;
			pFlink->Blink = pBlink ;
		}
	}

	data = (ULONG) TlsGetValue( m_dwTlsIndex );
	if ( 1 == data ) { LeaveCriticalSection( &pTLSData->cs ); }
	TlsSetValue( m_dwTlsIndex, (LPVOID)(data - 1) );

	if ( FALSE == VirtualFree((LPVOID)lpAddress, 0, 0x8000) )
	{
		RaiseException( STATUS_ACCESS_VIOLATION, STATUS_NONCONTINUABLE_EXCEPTION, 0, 0 );
		ExitProcess( 0xFFFFFFFF );
	}

	return;
}