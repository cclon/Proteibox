#pragma once

#include "RPCHandler.h"


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


// PBIPStoreGetNode()
typedef struct _RPC_IN_PBIPStoreGetNode_	// size - 0x18
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  IID	iid_data ;

} RPC_IN_PBIPStoreGetNode, *LPRPC_IN_PBIPStoreGetNode ;


typedef struct _RPC_OUT_PBIPStoreGetNode_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Reserved ;
/*0x00C */  ULONG NameLength ;
/*0x010 */  WCHAR szDisplayName[1] ;

} RPC_OUT_PBIPStoreGetNode, *LPRPC_OUT_PBIPStoreGetNode ;


// PBIPStoreGetChildNode()
typedef struct _RPC_IN_PBIPStoreGetChildNode_	// size - 0x28
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  IID	iid_data ;
/*0x018 */  IID	iid_data_sun ;

} RPC_IN_PBIPStoreGetChildNode, *LPRPC_IN_PBIPStoreGetChildNode ;


typedef struct _RPC_OUT_PBIPStoreGetChildNode_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  ULONG Reserved ;
/*0x00C */  ULONG NameLength ;
/*0x010 */  WCHAR szDisplayName[1] ;

} RPC_OUT_PBIPStoreGetChildNode, *LPRPC_OUT_PBIPStoreGetChildNode ;


// PBIPStoreGetGrandchildNode()
typedef struct _RPC_IN_PBIPStoreGetGrandchildNode_	// size - 0x030
{ 
/*0x000 */ RPC_IN_HEADER RpcHeader ;
/*0x008 */ IID		iid_data ; 
/*0x018 */ IID		iid_data_sun ; 
/*0x028 */ ULONG	Length ;
/*0x02C */ WCHAR    szItemName[1] ;

} RPC_IN_PBIPStoreGetGrandchildNode, *LPRPC_IN_PBIPStoreGetGrandchildNode ;


typedef struct _RPC_OUT_PBIPStoreGetGrandchildNode_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  DWORD	cbData ;
/*0x00C */  WCHAR Data[1] ;

} RPC_OUT_PBIPStoreGetGrandchildNode, *LPRPC_OUT_PBIPStoreGetGrandchildNode ;


// PBIPStoreEnumSubtypes()
typedef struct _RPC_IN_PBIPStoreEnumSubtypes_	// size - 0x020
{ 
/*0x000 */ RPC_IN_HEADER RpcHeader ;
/*0x008 */ int Key ; 
/*0x00C */ GUID ClsidType ; 
/*0x01C */ BOOL bEnumSubtypes ; 

} RPC_IN_PBIPStoreEnumSubtypes, *LPRPC_IN_PBIPStoreEnumSubtypes ;


typedef struct _RPC_OUT_PBIPStoreEnumSubtypes_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  int	nCounts ;
/*0x00C */  LPCLSID Data[1] ;

} RPC_OUT_PBIPStoreEnumSubtypes, *LPRPC_OUT_PBIPStoreEnumSubtypes ;


// PBIPStoreEnumItems
typedef struct _RPC_IN_PBIPStoreEnumItems_	// size - 0x02C
{ 
/*0x000 */ RPC_IN_HEADER RpcHeader ;
/*0x008 */ int Key ; 
/*0x00C */ GUID ClsidType ; 
/*0x01C */ GUID ClsidStubType ; 

} RPC_IN_PBIPStoreEnumItems, *LPRPC_IN_PBIPStoreEnumItems ;


typedef struct _RPC_OUT_PBIPStoreEnumItems_	// size - 0x18
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  int	nCounts ;
/*0x00C */  WCHAR Data[1] ;

} RPC_OUT_PBIPStoreEnumItems, *LPRPC_OUT_PBIPStoreEnumItems ;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcPStore : public CRPCHandler
{
public:

	CRpcPStore(void);
	~CRpcPStore(void);

	VOID HandlerPStore( PVOID pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
		ProcPStore(
		IN PVOID pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

protected:
	static int WINAPI PStoreThread( LPVOID param );

protected:
	LPPOSTPROCRPCINFO RpcPStoreGetNode( PVOID pInfo, PVOID _pRpcInBuffer);
	LPPOSTPROCRPCINFO RpcPStoreGetChildNode( PVOID pInfo, PVOID _pRpcInBuffer);
	LPPOSTPROCRPCINFO RpcPStoreGetGrandchildNode( PVOID pInfo, PVOID _pRpcInBuffer);
	LPPOSTPROCRPCINFO RpcPStoreEnumSubtypes( PVOID pInfo, PVOID _pRpcInBuffer);
	LPPOSTPROCRPCINFO RpcPStoreEnumItems( PVOID pInfo, PVOID _pRpcInBuffer);


private:

	
};


extern CRpcPStore g_CRpcPStore ;


///////////////////////////////   END OF FILE   ///////////////////////////////
