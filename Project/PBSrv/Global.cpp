#include "StdAfx.h"
#include "Global.h"

//////////////////////////////////////////////////////////////////////////

CGlobal g_CGlobal ;

//////////////////////////////////////////////////////////////////////////

CGlobal::CGlobal(void)
{
	m_TotalData = NULL ;
}

CGlobal::~CGlobal(void)
{
}


LPTOTOAL_DATA CGlobal::GetTotalData()
{
	if ( m_TotalData ) { return m_TotalData	; }

	m_TotalData = (LPTOTOAL_DATA) new TOTOAL_DATA ;
	if ( NULL == m_TotalData ) { return NULL; }

	RtlZeroMemory( m_TotalData, sizeof(TOTOAL_DATA) );

	InitializeCriticalSectionAndSpinCount( &m_TotalData->CriticalSection, 0x3E8 );
//	InitializeCriticalSection( &m_TotalData->CriticalSection );

	m_TotalData->TLSData = CMemoryManager::GetInstance().MMCreateTLS();
	if ( NULL == m_TotalData->TLSData ) { goto _error_; }

	m_TotalData->hEvent = (HANDLE) CreateEventW( NULL, FALSE, TRUE, _PB_Global_event_ );
	if ( NULL == m_TotalData->hEvent ) { goto _error_; }

	return m_TotalData;

_error_ :
	DestroyTotalNode( m_TotalData );

	delete m_TotalData;
	m_TotalData = NULL;
	return NULL;
}



VOID CGlobal::DestroyTotalNode( IN LPTOTOAL_DATA pNode )
{
	if ( NULL == pNode ) { return; }

	if ( pNode->hThread ) 
	{
		InterlockedExchange( (volatile LONG *)&pNode->hEvent, 0 );
		SetEvent( pNode->hEvent );
		
		if ( WaitForSingleObject(pNode->hThread, 5000) == WAIT_TIMEOUT )
			TerminateThread( pNode->hThread, 0 );
	}

	if ( pNode->PortHandle )
	{
		ZwClose( pNode->PortHandle );
		InterlockedExchange( (volatile LONG *)&pNode->PortHandle, 0 );

		DWORD nRet = WaitForMultipleObjects( 4, &pNode->HandlesArray[0], TRUE, 3000 );
		if ( nRet == WAIT_TIMEOUT )
		{
			int n = 0;
			do
			{
				TerminateThread( pNode->HandlesArray[n], 0 );
				++ n ;
			}
			while ( n < 4 );

			nRet = WaitForMultipleObjects( 4, &pNode->HandlesArray[n], TRUE, 3000 );
		}
	}

	if ( pNode->TLSData )
	{
		CMemoryManager::GetInstance().FreeTLSData( pNode->TLSData );
	}

	return;
}



VOID InsertList( LPLIST_ENTRY_EX pHead, PLIST_ENTRY pOld, PLIST_ENTRY pNew )
{
	PLIST_ENTRY pFlink = NULL, pBlink = NULL;

	++ pHead->TotalCounts ;
	pBlink = pHead->Blink ;
	
	if ( pOld != pBlink && pOld )
	{
		pFlink = pOld->Flink ;
		pOld->Flink->Blink = pNew ;
		pNew->Flink = pFlink ;
		pOld->Flink = pNew ;
		pNew->Blink = pOld ;
	}
	else
	{
		pHead->Blink = pNew ;
		pNew->Blink = pBlink ;
		pNew->Flink = NULL ;

		if ( pBlink )
			pBlink->Flink = pNew;
		else
			pHead->Flink = pNew;
	}

	return;
}


///////////////////////////////   END OF FILE   ///////////////////////////////