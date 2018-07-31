#pragma once

#include "PBAdvapi32dllData.h"

//////////////////////////////////////////////////////////////////////////


// fake_CryptProtectData
typedef struct _RPC_IN_CryptProtectData_ 
{
/*0x000*/ ULONG TotalLength ;
/*0x004*/ ULONG Flag;
/*0x008*/ DWORD dwFlags;
/*0x00C*/ DWORD pDataIn_cbData;
/*0x010*/ DWORD pOptionalEntropy_cbData;
/*0x014*/ DWORD DataDescrLength;
/*0x018*/ WCHAR Data[1];

} RPC_IN_CryptProtectData, *LPRPC_IN_CryptProtectData ;


typedef struct _RPC_OUT_CryptProtectData_  // size >= 0x14
{
/*0x000*/ ULONG Reserved1 ;
/*0x004*/ ULONG err;
/*0x008*/ ULONG Reserved2 ;
/*0x00C*/ DWORD pDataOut_cbData;
/*0x010*/ PBYTE pDataOut_pbData;

} RPC_OUT_CryptProtectData, *LPRPC_OUT_CryptProtectData ;


// fake_CryptUnprotectData
typedef struct _RPC_IN_CryptUnprotectData_ // size >= 0x18
{
/*0x000*/ ULONG TotalLength ;
/*0x004*/ ULONG Flag;
/*0x008*/ DWORD dwFlags;
/*0x00C*/ DWORD pDataIn_cbData;
/*0x010*/ DWORD pOptionalEntropy_cbData;
/*0x014*/ WCHAR Data[1];

} RPC_IN_CryptUnprotectData, *LPRPC_IN_CryptUnprotectData ;


typedef struct _RPC_OUT_CryptUnprotectData_  // size >= 0x14
{
/*0x000*/ ULONG Reserved ;
/*0x004*/ ULONG err;
/*0x008*/ ULONG pDataOut_cbData ;
/*0x00C*/ DWORD DataDescrLength;
/*0x010*/ PBYTE pDataOut_pbData;

} RPC_OUT_CryptUnprotectData, *LPRPC_OUT_CryptUnprotectData ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL WINAPI Hook_pharase0_advapi32dll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase1_crypt32dll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase2_hnetcfgdll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase3_ws2_32dll( IN HMODULE hModule );

///////////////////////////////   END OF FILE   ///////////////////////////////

