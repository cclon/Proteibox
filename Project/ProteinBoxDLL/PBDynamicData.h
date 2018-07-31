#pragma once

#include "PBIPStore.h"

//////////////////////////////////////////////////////////////////////////

#ifndef GET_WORD
#define GET_WORD(ptr)  (*(WORD *)(ptr))
#endif
#ifndef GET_DWORD
#define GET_DWORD(ptr) (*(DWORD *)(ptr))
#endif

typedef STRING LSA_STRING, *PLSA_STRING;
typedef ULONG LSA_OPERATIONAL_MODE, *PLSA_OPERATIONAL_MODE;

extern BOOL g_bFlag_Hook_OpenWinClass_etc ;
extern ATOM g_Atom_SBIE_DropTarget ;
extern ATOM g_Atom_OleDropTargetInterface ;
extern ATOM g_Atom_OleDropTargetInterface_Sandbox ;
extern ATOM g_Atom_OleDropTargetMarshalHwnd ;
extern ATOM g_Atom_OleDropTargetMarshalHwnd_Sandbox ;
extern ATOM g_Atom_OleEndPointID ;
extern ATOM g_Atom_OleEndPointID_Sandbox ;
extern ATOM g_Atom_SBIE_WindowProcOldW ;
extern ATOM g_Atom_SBIE_WindowProcOldA ;
extern ATOM g_Atom_SBIE_WindowProcNewW ;
extern ATOM g_Atom_SBIE_WindowProcNewA ;

typedef LPVOID (WINAPI* _CoTaskMemAlloc_) (ULONG size);
extern _CoTaskMemAlloc_ g_CoTaskMemAlloc_addr ;

typedef NTSTATUS (WINAPI* _LsaConnectUntrusted_) ( PHANDLE );
extern _LsaConnectUntrusted_ g_LsaConnectUntrusted_addr ;

typedef NTSTATUS (WINAPI* _LsaRegisterLogonProcess_) ( PLSA_STRING,PHANDLE,PLSA_OPERATIONAL_MODE );
extern _LsaRegisterLogonProcess_ g_LsaRegisterLogonProcess_addr ;

typedef DWORD (WINAPI* _VerifyCatalogFile_) ( LPCTSTR );
extern _VerifyCatalogFile_ g_VerifyCatalogFile_addr ;

typedef ULONG (WINAPI* _SxsInstallW_)(int);
extern _SxsInstallW_ g_SxsInstallW_addr ;

typedef ULONG (WINAPI* _GdiAddFontResourceW_)(LPCWSTR, FLONG, DESIGNVECTOR *);
extern _GdiAddFontResourceW_ g_GdiAddFontResourceW_addr ;

typedef ULONG (WINAPI* _CreateScalableFontResourceW_)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR);
extern _CreateScalableFontResourceW_ g_CreateScalableFontResourceW_addr ;

typedef NTSTATUS (WINAPI* _NtQuerySystemInformation_)( SYSTEM_INFORMATION_CLASS,PVOID,ULONG,PULONG );
extern _NtQuerySystemInformation_ g_NtQuerySystemInformation_addr ;

typedef HANDLE (WINAPI* _CreateActCtxA_)(PCACTCTXA);
extern _CreateActCtxA_ g_CreateActCtxA_addr ;

typedef HANDLE (WINAPI* _CreateActCtxW_)(PCACTCTXW);
extern _CreateActCtxW_ g_CreateActCtxW_addr ;

//////////////////////////////////////////////////////////////////////////

BOOL WINAPI Hook_pharase7_secur32dll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase13_sfc_osdll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase16_wevtapidll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase17_sxsdll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase18_gdi32dll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase19_sechostdll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase20_MsgPlusLivedll( IN HMODULE hModule );

ULONG WINAPI fake_LsaRegisterLogonProcess( PLSA_STRING LsaLogonProcessName, PHANDLE Handle, PLSA_OPERATIONAL_MODE OperationalMode );
ULONG WINAPI fake_SfcIsFileProtected( int a1, int a2 );
ULONG WINAPI fake_SfcGetNextProtectedFile(int a1, int a2);
ULONG WINAPI fake_EvtIntAssertConfig(int a1, int a2, int a3);
ULONG WINAPI fake_SxsInstallW(int a1);
ULONG WINAPI fake_GdiAddFontResourceW( LPCWSTR pwszFileName, int dwFlags, DESIGNVECTOR *pdv );
ULONG WINAPI fake_CreateScalableFontResourceW(DWORD fdwHidden, LPCWSTR lpszFontRes, LPCWSTR lpszFontFile, LPCWSTR lpszCurrentPath);
ULONG WINAPI fake_SetLocaleInfo(int a1, int a2, int a3);
ULONG WINAPI fake_NtQuerySystemInformation (SYSTEM_INFORMATION_CLASS,PVOID, ULONG, PULONG);
ULONG WINAPI fake_NtCreateTransaction(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10);
ULONG WINAPI fake_NtOpenTransaction(int a1, int a2, int a3, int a4, int a5);
ULONG WINAPI fake_CreateActCtxA( PCACTCTXA pActCtx );
ULONG WINAPI fake_CreateActCtxW( PCACTCTXW pActCtx );
NTSTATUS Stub_NtOpenKey( POBJECT_ATTRIBUTES ObjAtr, PHANDLE pKeyHandle, ULONG DesireAccess );

///////////////////////////////   END OF FILE   ///////////////////////////////
