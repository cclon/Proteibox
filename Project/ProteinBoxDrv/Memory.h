#pragma once

//////////////////////////////////////////////////////////////////////////


//
// 定义内存结构体
//

#define LPMMHEAD PMEMORY_HEAD_INFO
#define LPMMNODE PMEMORY_INFO

#define MMHEAD MEMORY_HEAD_INFO
#define MMNODE MEMORY_INFO

//
// 定义操作内存的函数
//

#define kmallocMM( _nSize, _Tag )	MMBuildNode( (PVOID)g_ListHead__MemoryManager, _nSize, _Tag	)
#define kfreeMM()					MMDistroyAll( (PVOID)g_ListHead__MemoryManager				)
#define kgetaddrMM( _Tag )			MMFindNode  ( (PVOID)g_ListHead__MemoryManager, _Tag		)

//
// others
//

extern BOOL g_MemoryManager_Inited_ok ;


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
// 内存节点
//

typedef struct _MEMORY_INFO_ 
{
/*0x000 */ LIST_ENTRY MemoryListEntry ; // 指向下个节点

/*0x004 */ ULONG nSize		; // 内存大小
/*0x008 */ ULONG pAddress	; // 内存地址

/*0x00C */ ULONG Tag		; // 内存块标记

} MEMORY_INFO, *PMEMORY_INFO ;


//
// 内存管理的总节点
//

typedef struct _MEMORY_HEAD_INFO_ 
{
/*0x000 */ PERESOURCE	QueueLockList	; // 链表的自旋锁
/*0x004 */ ULONG		nTotalCounts    ; // 子节点总数
/*0x004 */ MEMORY_INFO  ListHead		;

} MEMORY_HEAD_INFO, *PMEMORY_HEAD_INFO ;

extern PMEMORY_HEAD_INFO g_ListHead__MemoryManager ;


//
// 内存块对应的TAG类型
//

//
// 这些Tag标记的内存会被频繁用到,一般保存的是链表之类的数据结构; 不可供临时内存使用
//

typedef enum _MEMORY_TAG_ 
{
	MTAG___TokenInformation_DefaultDacl_New = 1,
	MTAG___SecurityDescriptor = 2 ,
	MTAG___NewDefaultDacl = 3 ,
	MTAG___Privilege_BlackList = 4 ,
	MTAG___SeAliasAdminsSid = 5 ,
	MTAG___SeAliasPowerUsersSid = 6 ,
	MTAG___SeAuthenticatedUsersSid = 7 ,
	MTAG___SeWorldSid = 8 ,
	MTAG___Context_InfoTip = 9 ,
	MTAG___ADDR_FOR_CaptureStackBackTrace = 10 ,
	MTAG___IOCTL_HANDLERCONF_GLOBAL = 11 ,
	MTAG___InjectDllPath = 12 ,

};


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
InitMemoryManager (
	);

BOOL 
MMCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
MMDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
MMAllocateNode(
	OUT PVOID _pCurrenList_
	);

ULONG
MMBuildNode (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG Tag
	);

PVOID
MMBuildNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG pAddress,
	IN ULONG Tag
	);

ULONG
MMFindNode (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	);

PVOID
MMFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	);

BOOL
MMDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	);

VOID
MMDistroyAll (
	IN PVOID _TotalHead
	);

VOID
MMWalkNodes (
	IN PVOID _TotalHead
	);


///////////////////////////////   END OF FILE   ///////////////////////////////