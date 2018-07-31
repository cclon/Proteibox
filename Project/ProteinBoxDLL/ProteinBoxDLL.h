#pragma once

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////


extern PIMAGE_IMPORT_DESCRIPTOR g_pImportTable_mscoree_dll ;
extern PIMAGE_IMPORT_DESCRIPTOR g_pImportTable_msvcm80_dll ;
extern PIMAGE_IMPORT_DESCRIPTOR g_pImportTable_msvcm90_dll ;

extern PB_BOX_INFO g_BoxInfo ;

extern ULONG g_CurrentPID_runInSandbox ;

#define SUBKEY_PROTEINBOX		_T("SOFTWARE\\Proteinbox\\Config")
#define SUBVALUE_ROOTFOLDER		_T("RootFolder")

//////////////////////////////////////////////////////////////////////////

class CProteinBoxDLL 
{
public:
	CProteinBoxDLL(void);
	virtual ~CProteinBoxDLL();

public:
	BOOL DoWork( HMODULE hModule );
	VOID StopWork( BOOL Flag );
	BOOL CheckGetHandle();
	BOOL IoControl( DWORD nCode, PVOID pInBuffer, DWORD nInCount, PVOID pOutBuffer, DWORD nOutCount);
	NTSTATUS IoControlEx( DWORD dwIoControlCode, PVOID lpInBuffer, DWORD nInBufferSize, PVOID lpOutBuffer, DWORD nOutBufferSize);

protected:
	BOOL IsInvalidProc();	
	VOID FixupModifiedPE();
	NTSTATUS GetInjectSaveArea( PVOID buffer );
	BOOL InitializeSid();
	BOOL __InitializeSid ( PSID, PSID_IDENTIFIER_AUTHORITY,BYTE, ULONG );

private:
	HANDLE m_hFile_ProteinboxDrv ; 
};

extern CProteinBoxDLL g_ProteinBoxDLL ;

///////////////////////////////   END OF FILE   ///////////////////////////////


