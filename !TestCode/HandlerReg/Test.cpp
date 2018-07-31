/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/01/06 [6:1:2010 - 11:30]
* MODULE : D:\Program\R0\Coding\…≥œ‰\SandBox\Code\HandlerReg\Test.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "Test.h"


//////////////////////////////////////////////////////////////////////////


WCHAR g_wszRegPath[MAX_PATH]  = L"\\Registry\\USER\\Sandbox_AV_DefaultBox\\machine" ;
WCHAR g_wszRegValue[MAX_PATH] = L"sudami" ;

//////////////////////////////////////////////////////////////////////////


VOID
TestCreateKey (
	)
{
	NTSTATUS Status;
	HANDLE KeyHandle;
	
	Status = HRCreateKey( g_wszRegPath, &KeyHandle ) ;
	if ( !NT_SUCCESS(Status) )
	{
		printf( "HRCreateKey() \"%ws\" | error | 0x%08lx \n", g_wszRegPath, Status ) ;
	} 
	else 
	{
		ZwClose( KeyHandle );
		printf( "HRCreateKey() \"%ws\" | OK \n", g_wszRegPath ) ;
	}
	
	return ;
}



VOID
TestOpenKey (
	)
{
	NTSTATUS Status;
	HANDLE KeyHandle;
	
	Status = HROpenKey( g_wszRegPath, &KeyHandle ) ;
	if ( !NT_SUCCESS(Status) )
	{
		printf( "HROpenKey() \"%ws\" | error | 0x%08lx \n", g_wszRegPath, Status ) ;
	} 
	else 
	{
		ZwClose( KeyHandle );
		printf( "HROpenKey() \"%ws\" | OK \n", g_wszRegPath ) ;
	}
	
	return ;
}



VOID
TestSetValueKey (
	)
{
	NTSTATUS Status;
	WCHAR data[MAX_PATH] = L"22222" ;
	
	Status = HRSetValueKey( g_wszRegPath, g_wszRegValue, (PCWSTR)data, REG_SZ ) ;
	if ( !NT_SUCCESS(Status) )
	{
		printf( "HRSetValueKey() \"%ws\" | error | 0x%08lx \n", g_wszRegPath, Status ) ;
	} 
	else 
	{
		printf( "HRSetValueKey() \"%ws\" | OK \n", g_wszRegPath ) ;
	}
	
	return ;

}



///////////////////////////////   END OF FILE   ///////////////////////////////
