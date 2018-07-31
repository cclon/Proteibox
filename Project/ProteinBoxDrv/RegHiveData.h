#pragma once

//////////////////////////////////////////////////////////////////////////

//
// 定义内存结构体
//

#define LPRHHEAD LPREGHIVE_NODE_INFO_HEAND
#define LPRHNODE LPREGHIVE_NODE_INFO

#define RHHEAD REGHIVE_NODE_INFO_HEAND
#define RHNODE REGHIVE_NODE_INFO

//
// 定义操作内存的函数
//

#define kgetnodeRH( Tag1, Tag2, Tag3 )	RHFindNode( (PVOID)g_ListHead__RegHive, Tag1, Tag2, Tag3 )
#define kbuildnodeRH( _Tag )			RHBuildNode( (PVOID)g_ListHead__RegHive, _Tag	)
#define kfreeRH()						RHDistroyAll( (PVOID)g_ListHead__RegHive		)


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

//
// size - 0x01C
//

typedef struct _REGHIVE_NODE_INFO_ {

/*0x000 */ struct _REGHIVE_NODE_INFO_* pFlink ;	// 上个节点
/*0x004 */ struct _REGHIVE_NODE_INFO_* pBlink ;	// 下个节点

/*0x008 */ WCHAR HiveFilePath[ MAX_PATH ] ;  // eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\RegHive"
/*0x00C */ WCHAR HiveRegPath[ MAX_PATH ] ;   // eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
/*0x010 */ ULONG PorcessRefCounts ; // 当前沙箱中进程计数,因为启动一个进程,就会调用一次Ioctl_StartProcess.其中会操作当前沙箱的Hive文件.这里便是其引用计数
/*0x014 */ BOOL bNeedToDistroy ; // 置TRUE,表明当前RegHive已无进程占有,可以卸载掉!
/*0x018 */ BOOL ProcessesLock ; // 用于多个进程间的同步操作!即一个沙箱中可能会有2+个进程同时在启动时操作Hive,所以必须同步,这里为1表示被占有,处于锁定状态!

} REGHIVE_NODE_INFO, *LPREGHIVE_NODE_INFO ;


//
// RH 链表的总节点
//

typedef struct _REGHIVE_NODE_INFO_HEAND_ 
{
	/*0x000 */ int			nTotalCounts ;
	/*0x004 */ PERESOURCE	QueueLockList	; // 链表的资源锁
	/*0x008 */ REGHIVE_NODE_INFO ListHead ;

} REGHIVE_NODE_INFO_HEAND, *LPREGHIVE_NODE_INFO_HEAND ;

extern LPREGHIVE_NODE_INFO_HEAND g_ListHead__RegHive ;
extern BOOL g_RegHive_Inited_ok ;


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
InitRegHive (
	);

BOOL 
RHCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
RHDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
RHAllocateNode(
	OUT PVOID _pCurrenList_
	);

PVOID
RHBuildNode (
	IN PVOID _TotalHead,
	IN PVOID _ProcessNode
	);

PVOID
RHBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath
	);

PVOID
RHFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath,
	OUT BOOL* bResult
	);

BOOL
RHDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	);

VOID
RHDistroyAll (
	IN PVOID _TotalHead
	);

VOID
RHWalkNodes (
	IN PVOID _TotalHead
	);

//////////////////////////////////////////////////////////////////////////

BOOL
CheckRegHive (
	IN PVOID _pNode,
	IN PVOID _ProcessNode
	);

BOOL
CmLoadKey(
	IN PVOID _ProcessNode,
	IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	IN POBJECT_ATTRIBUTES FileObjectAttributes
	);

///////////////////////////////   END OF FILE   ///////////////////////////////