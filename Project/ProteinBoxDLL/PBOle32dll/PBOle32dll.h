#pragma once

#include "PBOle32dllData.h"

//////////////////////////////////////////////////////////////////////////

typedef struct tagOBJREF 
{
	unsigned long signature;
	unsigned long flags;
	GUID iid;
/*
	[switch_is(flags)] union {
		[case(OBJREF_STANDARD)] struct OR_STANDARD {
			STDOBJREF std;
			DUALSTRINGARRAY saResAddr;
		} u_standard;
		[case(OBJREF_HANDLER)] struct OR_HANDLER {
			STDOBJREF std;
			CLSID clsid;
			DUALSTRINGARRAY saResAddr;
		} u_handler;
		[case(OBJREF_CUSTOM)] struct OR_CUSTOM {
			CLSID clsid;
			unsigned long cbExtension;
			ULONG size;
			[size_is(size), ref] byte *pData;
		} u_custom;
	} u_objref;
*/
} OBJREF;


// fake_CoUnmarshalInterface
typedef struct _RPC_IN_CoUnmarshalInterface_	// size >= 0x5C
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ IID riid ;
/*0x018*/ ULONG objrefDummy_TotalLength ;// sizeof(OBJREF) + data[]的长度
/*0x01C*/ OBJREF objref;
/*0x034*/ OBJREF objrefDummy;
/*0x05C*/ BYTE Data[];
} RPC_IN_CoUnmarshalInterface, *LPRPC_IN_CoUnmarshalInterface ;


// fake_CoMarshalInterface
typedef struct _RPC_IN_CoMarshalInterface_	// size - 0x28
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ ULONG Flag;
/*0x00C*/ IID riid ;
/*0x01C*/ DWORD dwDestContext;
/*0x020*/ DWORD mshlFlags;
} RPC_IN_CoMarshalInterface, *LPRPC_IN_CoMarshalInterface ;

typedef struct _RPC_OUT_CoMarshalInterface_	// size - MappedAddress_Length + 0x14
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ ULONG MappedAddress_PID;
/*0x00C*/ ULONG MappedAddress_Length ;
/*0x010*/ IID riid ;
} RPC_OUT_CoMarshalInterface, *LPRPC_OUT_CoMarshalInterface ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL WINAPI Hook_pharase5_ole32dll( IN HMODULE hModule );

///////////////////////////////   END OF FILE   ///////////////////////////////
