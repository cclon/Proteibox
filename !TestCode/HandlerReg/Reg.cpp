/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/01/05 [5:1:2010 - 14:37]
* MODULE : D:\Program\R0\Coding\…≥œ‰\SandBox\Code\HandlerReg\Reg.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "Reg.h"


//////////////////////////////////////////////////////////////////////////


NTSTATUS
HRCreateKey(
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	)
{
	NTSTATUS Status;

	if ( NULL == wszRegPath )
	{
		return STATUS_INVALID_PARAMETER ;
	}

	Status = HRCreateKeyEx( KEY_READ | KEY_WRITE, wszRegPath, hKey );
	return Status ;
}



NTSTATUS
HRCreateKeyEx(
	IN ACCESS_MASK DesiredAccess,
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	)
{
	NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    ULONG Disposition;
    UNICODE_STRING KeyName;
	
	if ( NULL == wszRegPath )
	{
		return STATUS_INVALID_PARAMETER ;
	}

    RtlInitUnicodeString( &KeyName, wszRegPath ); // L"\\Registry\\Machine\\software\\0001"

    InitializeObjectAttributes(
		&ObjectAttributes,
		&KeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);
	
    Status = ZwCreateKey(
		&KeyHandle,
		DesiredAccess,
		&ObjectAttributes,
		0,
		NULL,
		0,
		&Disposition
		);

    if (!NT_SUCCESS(Status)) return Status;
	*hKey = KeyHandle ;

	return Status ;
}



NTSTATUS
HROpenKey(
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	)
{
	NTSTATUS Status;

	if ( NULL == wszRegPath )
	{
		return STATUS_INVALID_PARAMETER ;
	}

	Status = HROpenKeyEx( KEY_QUERY_VALUE, wszRegPath, hKey );
	return Status ;
}



NTSTATUS
HROpenKeyEx(
	IN ACCESS_MASK DesiredAccess,
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	) 
{
	NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle = NULL ;
    UNICODE_STRING KeyName  ;
	
	if ( NULL == wszRegPath )
	{
		return STATUS_INVALID_PARAMETER ;
	}
	
    RtlInitUnicodeString( &KeyName, wszRegPath ); // L"\\Registry\\Machine\\software\\0001"
	
    InitializeObjectAttributes(
		&ObjectAttributes,
		&KeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);
	
     Status = ZwOpenKey(
                 &KeyHandle,
                 DesiredAccess,
                 &ObjectAttributes
                 );
	
    if (!NT_SUCCESS(Status)) return Status;
	*hKey = KeyHandle ;
	
	return Status ;
}



NTSTATUS
HRSetValueKey(
	IN PCWSTR wszRegPath,
	IN PCWSTR wszRegValue,
	IN PCWSTR Buffer,
	IN ULONG Type
	) 
{
	NTSTATUS status = STATUS_SUCCESS ;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey = NULL;
    ULONG ValueLength;
	
    RtlInitUnicodeString( &KeyName, wszRegPath );

    InitializeObjectAttributes(&ObjectAttributes,
		&KeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);
	
    status = ZwOpenKey(&hKey, KEY_READ | KEY_WRITE, &ObjectAttributes);
    if ( !NT_SUCCESS( status ) )
    {
        return status ;
    }
	
    RtlInitUnicodeString( &ValueName, wszRegValue );
	
    ValueLength = (wcslen(Buffer) + 1) * sizeof(WCHAR);
	
    status = ZwSetValueKey(
		hKey,
		&ValueName,
		0,
		Type,
		(LPWSTR) Buffer,
		ValueLength
		);
	
    if ( NT_SUCCESS( status ) )
    {
        ZwFlushKey( hKey );
    }
	
    ZwClose(hKey);
	
	return status ;
}




///////////////////////////////   END OF FILE   ///////////////////////////////
