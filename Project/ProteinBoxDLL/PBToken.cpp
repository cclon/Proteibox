/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/12/27 [27:12:2010 - 16:11]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBToken.cpp
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
#include "MemoryManager.h"
#include "PBTokenData.h"
#include "PBToken.h"


//////////////////////////////////////////////////////////////////////////

_NtQueryInformationToken_	g_NtQueryInformationToken_addr = NULL ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    )
{
	ACCESS_MASK DesiredAccessNew = 0 ;
	ACCESS_MASK DesiredAccessDummy = 0;
	NTSTATUS status = STATUS_SUCCESS ; 
	DWORD _DeclineAccess = PROCESS_DUP_HANDLE | 0x2000000 ;
	_NtOpenProcess_ OrignalAddress = (_NtOpenProcess_) GetTokenFunc(NtOpenProcess) ;

	DesiredAccessDummy = DesiredAccess;
	status = OrignalAddress( ProcessHandle, DesiredAccess, ObjectAttributes, ClientId );
	if ( STATUS_ACCESS_DENIED == status )
	{
		if ( DesiredAccess & _DeclineAccess ) { DesiredAccessDummy = DesiredAccess | _DeclineAccess ; }

		DesiredAccessNew = DesiredAccessDummy & 0x120410;
		if ( DesiredAccessNew )
			status = OrignalAddress( ProcessHandle, DesiredAccessNew, ObjectAttributes, ClientId );
	}

	return status;
}



ULONG WINAPI
fake_NtOpenThread (
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    )
{
	ACCESS_MASK DesiredAccessNew = 0 ;
	ACCESS_MASK DesiredAccessDummy = 0;
	NTSTATUS status = STATUS_SUCCESS ; 
	_NtOpenThread_ OrignalAddress = (_NtOpenThread_) GetTokenFunc(NtOpenThread) ;

	DesiredAccessDummy = DesiredAccess;
	status = OrignalAddress( ThreadHandle, DesiredAccess, ObjectAttributes, ClientId );
	if ( STATUS_ACCESS_DENIED == status )
	{
		if ( DesiredAccess & 0x2000000 ) { DesiredAccessDummy = DesiredAccess | 0x120040 ; }

		DesiredAccessNew = DesiredAccessDummy & 0x120048;
		if ( DesiredAccessNew )
			status = OrignalAddress( ThreadHandle, DesiredAccessNew, ObjectAttributes, ClientId );
	}

	return status;
}



ULONG WINAPI
fake_NtDuplicateObject (
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options
    )
{
	ULONG_PTR OptionsNew = 0; 
	ULONG_PTR OptionsDummy = Options ;
	HANDLE FuckedHandle = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	_NtDuplicateObject_ OrignalAddress = (_NtDuplicateObject_) GetTokenFunc(NtDuplicateObject) ;

	ClearFlag( OptionsDummy, DUPLICATE_CLOSE_SOURCE );
	status = OrignalAddress (
		SourceProcessHandle,
		SourceHandle,
		TargetProcessHandle,
		TargetHandle,
		DesiredAccess,
		HandleAttributes,
		OptionsDummy
		);

	if ( status >= 0 )
	{
		if ( Options & DUPLICATE_CLOSE_SOURCE )
		{
			OrignalAddress( SourceProcessHandle, SourceHandle, NULL, NULL, DesiredAccess, HandleAttributes, DUPLICATE_CLOSE_SOURCE );
		}

		if ( INVALID_HANDLE_VALUE == SourceProcessHandle && INVALID_HANDLE_VALUE == TargetProcessHandle )
		{
			NtDuplicateObjectRecorder( SourceHandle, *TargetHandle );
		}

		return status;
	}

	if ( STATUS_ACCESS_DENIED != status ) { return status; }

	FuckedHandle = SourceProcessHandle ;

	if ( INVALID_HANDLE_VALUE == TargetProcessHandle )
	{
		if ( SourceProcessHandle == TargetProcessHandle ) { FuckedHandle = SourceHandle; }
	}
	else
	{
		if ( INVALID_HANDLE_VALUE != SourceProcessHandle ) { return status; }

		FuckedHandle = TargetProcessHandle ;
		OptionsNew = Options | 0x80000 ;
	}

	if ( NULL == FuckedHandle ) { return status; }

	if ( HandleAttributes & OBJ_INHERIT ) { OptionsNew |= 0x40000 ; }

	return PB_DuplicateObject( TargetHandle, FuckedHandle, SourceHandle, DesiredAccess, OptionsNew );
}



ULONG WINAPI
fake_NtQuerySecurityObject (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    )
{
	BOOL bIsWhite = FALSE ;
	HANDLE HandleNew = NULL ;
	ULONG_PTR InfoLength = 0x100 ;
	NTSTATUS status = STATUS_SUCCESS ;
	_NtQuerySecurityObject_ OrignalAddress = (_NtQuerySecurityObject_) GetTokenFunc(NtQuerySecurityObject) ;

	WCHAR Buffer[ 256 ] = {};
	POBJECT_TYPE_INFORMATION pObjectTypeInfo = (POBJECT_TYPE_INFORMATION)Buffer;
	if ( ZwQueryObject( Handle, ObjectTypeInformation, pObjectTypeInfo, 256, &InfoLength ) >= 0 )
	{
		if ( 0 == wcsicmp(pObjectTypeInfo->Name.Buffer, L"File") )
		{
			HandleNew = (HANDLE) SecurityObjectFileFilter( Handle, &bIsWhite );	
		}
		else if ( 0 == wcsicmp(pObjectTypeInfo->Name.Buffer, L"Key") )
		{
			HandleNew = (HANDLE) SecurityObjectKeyFilter( Handle, &bIsWhite );
		}
	} 

	if ( bIsWhite )
	{
		if ( HandleNew ) { ZwClose( HandleNew ); }
		status = OrignalAddress( Handle, SecurityInformation, SecurityDescriptor, Length, LengthNeeded );
	}
	else
	{
		if ( NULL == HandleNew ) { HandleNew = Handle; }

		status = OrignalAddress( HandleNew, SecurityInformation, SecurityDescriptor, Length, LengthNeeded );
		if ( STATUS_ACCESS_DENIED == status && SecurityInformation & 0x50000008 )
		{
			status = OrignalAddress( HandleNew, SecurityInformation & 0xAFFFFFF7, SecurityDescriptor, Length, LengthNeeded );
		}

		if ( HandleNew != Handle ) { ZwClose( HandleNew ); }
	}

	return status;
}



ULONG WINAPI
fake_NtSetSecurityObject (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
	BOOL bIsWhite = FALSE ;
	HANDLE HandleNew = NULL ;
	ULONG_PTR InfoLength = 0x100 ;
	NTSTATUS status = STATUS_SUCCESS ;
	_NtSetSecurityObject_ OrignalAddress = (_NtSetSecurityObject_) GetTokenFunc(NtSetSecurityObject) ;

	WCHAR Buffer[ 256 ] = {};
	POBJECT_TYPE_INFORMATION pObjectTypeInfo = (POBJECT_TYPE_INFORMATION)Buffer;
	if ( ZwQueryObject( Handle, ObjectTypeInformation, pObjectTypeInfo, 256, &InfoLength ) >= 0 )
	{
		if ( 0 == wcsicmp(pObjectTypeInfo->Name.Buffer, L"File") )
		{
			HandleNew = (HANDLE) SecurityObjectFileFilter( Handle, &bIsWhite );	
		}
		else if ( 0 == wcsicmp(pObjectTypeInfo->Name.Buffer, L"Key") )
		{
			HandleNew = (HANDLE) SecurityObjectKeyFilter( Handle, &bIsWhite );
		}
	} 

	if ( bIsWhite )
	{
		if ( HandleNew ) { ZwClose( HandleNew ); }
		status = OrignalAddress( Handle, SecurityInformation, SecurityDescriptor );
	}
	else
	{
		status = STATUS_SUCCESS;
	}

	return status;
}



ULONG WINAPI
fake_NtSetInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN PVOID TokenInformation,
    IN ULONG TokenInformationLength
    )
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtSetInformationToken_ OrignalAddress = (_NtSetInformationToken_) GetTokenFunc(NtSetInformationToken) ;

	status = OrignalAddress( TokenHandle, TokenInformationClass, TokenInformation, TokenInformationLength );
	if ( status < 0 )
	{
		if ( TokenSessionId == TokenInformationClass || TokenInformationClass == 0x19 )
		{
			status = STATUS_SUCCESS ;
		}
	}

	return status;
}


ULONG WINAPI
fake_NtAdjustPrivilegesToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    )
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtAdjustPrivilegesToken_ OrignalAddress = (_NtAdjustPrivilegesToken_) GetTokenFunc(NtAdjustPrivilegesToken) ;

	status = OrignalAddress( TokenHandle, DisableAllPrivileges, NewState, BufferLength, PreviousState, ReturnLength );
	if ( STATUS_NOT_ALL_ASSIGNED == status ) { status = STATUS_SUCCESS; }

	return status;
}


//////////////////////////////////////////////////////////////////////////

ULONG WINAPI
fake_RtlQueryElevationFlags (
	OUT int Flags
	)
{
	int bRet = 0 ;
	_RtlQueryElevationFlags_ OrignalAddress = (_RtlQueryElevationFlags_) GetTokenExFunc(RtlQueryElevationFlags) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( pNode->sProcessLock.nLockCreateProcessAsUser && FALSE == g_bFlag_Is_in_SynTP_exe )
	{
		bRet = OrignalAddress( Flags );
	}
	else
	{
		*(DWORD *)Flags = 0 ;
		bRet = 0;
	}

	return bRet ;
}



ULONG WINAPI
fake_NtQueryInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    )
{
	NTSTATUS status = STATUS_SUCCESS ;
	_NtQueryInformationToken_ OrignalAddress = (_NtQueryInformationToken_) GetTokenExFunc(NtQueryInformationToken) ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	status = OrignalAddress( TokenHandle, TokenInformationClass, TokenInformation, TokenInformationLength, ReturnLength );
	if ( ! NT_SUCCESS(status) ) { return status; }

	if ( pNode->sProcessLock.nLockCreateProcessAsUser && FALSE == g_bFlag_Is_in_SynTP_exe ) { return status; }

	if ( TokenInformationClass == 0x12 )
	{
		*(DWORD *) TokenInformation = 2 ;
	}
	else
	{
		if ( TokenInformationClass == 0x19 && TokenInformationLength >= 0x14 )
		{
			*(DWORD *)TokenInformation = (int)TokenInformation + 8;
			*((DWORD *)TokenInformation + 1) = 0x60 ;
			*((DWORD *)TokenInformation + 2) = 0x101 ;
			*((DWORD *)TokenInformation + 3) = 0x10000000 ;
			*((DWORD *)TokenInformation + 4) = 0x3000 ;
			
			if ( ReturnLength ) { *ReturnLength = 0x14 ; }
		}
	}

	return status;
}

///////////////////////////////   END OF FILE   ///////////////////////////////