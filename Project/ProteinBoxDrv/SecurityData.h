#pragma once

//////////////////////////////////////////////////////////////////////////

//
// 定义内存结构体
//

#define LPSDHEAD LPREGISTRY_USERSID_INFO_HEAND
#define LPSDNODE LPREGISTRY_USERSID_INFO

#define SDHEAD REGISTRY_USERSID_INFO_HEAND
#define SDNODE REGISTRY_USERSID_INFO

//
// 定义操作内存的函数
//

#define kgetnodeSD( _Tag )			SDFindNode( (PVOID)g_ListHead__RegistryUserSID, _Tag	)
#define kbuildnodeSD( _Tag )		SDBuildNode( (PVOID)g_ListHead__RegistryUserSID, _Tag	)
#define kfreeSD()					SDDistroyAll( (PVOID)g_ListHead__RegistryUserSID		)



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef struct _REGISTRY_USERSID_INFO_ {

/*0x000 */ struct _REGISTRY_USERSID_INFO_* pFlink ; // 上个结点
/*0x004 */ struct _REGISTRY_USERSID_INFO_* pBlink ; // 下个结点

/*0x008 */ ULONG StructSize ;  // 该结构体的总大小
/*0x00C */ ULONG Length_RegitryUserSID ; // 该结构保存的字符串1的长度
/*0x010 */ ULONG Length_CurrentUserName ; // 该结构保存的字符串2的长度
/*0x014 */ WCHAR RegitryUserSID[ MAX_PATH ] ; // 该结构保存的字符串1的地址
/*0x018 */ WCHAR CurrentUserName[ MAX_PATH ] ; // 该结构保存的字符串2的地址, eg:"AV"

} REGISTRY_USERSID_INFO, *LPREGISTRY_USERSID_INFO ;


//
// SID链表的总节点
//

typedef struct _REGISTRY_USERSID_INFO_HEAND_ 
{
/*0x000 */ int			nTotalCounts ;
/*0x004 */ PERESOURCE	QueueLockList	; // 链表的自旋锁
/*0x008 */ REGISTRY_USERSID_INFO ListHead ;

} REGISTRY_USERSID_INFO_HEAND, *LPREGISTRY_USERSID_INFO_HEAND ;

extern LPREGISTRY_USERSID_INFO_HEAND g_ListHead__RegistryUserSID ;
extern BOOL g_SecurityDataManager_Inited_ok ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

BOOL
InitSecurityData (
	);

BOOL 
SDCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
SDDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
SDAllocateNode(
	OUT PVOID _pCurrenList_
	);

PVOID
SDBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	);

PVOID
SDBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	);

PVOID
SDFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR RegisterUserID
	);

BOOL
SDDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	);

VOID
SDDistroyAll (
	IN PVOID _TotalHead
	);

VOID
SDWalkNodes (
	IN PVOID _TotalHead
	);



///////////////////////////////   END OF FILE   ///////////////////////////////