#pragma once

//////////////////////////////////////////////////////////////////////////

//
// 定义OD结构体
//

#define LPODHEAD LPOBJECT_DATA_INFO_HEAND
#define LPODNODE LPOBJECT_DATA_INFO

#define ODHEAD OBJECT_DATA_INFO_HEAND
#define ODNODE OBJECT_DATA_INFO

//
// 定义操作OD的函数
//

#define kgetnodeOD( _Tag )			ODFindNode( (PVOID)g_ListHead__ObjectData, _Tag	)
#define kbuildnodeOD( _Tag )		ODBuildNode( (PVOID)g_ListHead__ObjectData, _Tag	)
#define kfreeOD()					ODDistroyAll( (PVOID)g_ListHead__ObjectData		)


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


#define ObjecTypeLength 0x20	// 对象类型名的长度

#define OBJECT_INDEX_LowerLimit			0	 // Object Index 的下限	
#define OBJECT_INDEX_UperLimit			100  // Object Index 的上限.可以定义大一些,以后可以扩充


// 当前Index是否属于Object单元内
#define IS_OBJECT_INDEX( l ) ( l > OBJECT_INDEX_LowerLimit && l <= OBJECT_INDEX_UperLimit )

// 定义待Hook的数量
#define MaxObjectCounts			LAST_INDEX_NEVER_USERD - 1


enum _OBJECT_INDEX_ 
{
	TOKEN_INDEX = OBJECT_INDEX_LowerLimit + 1,
	PROCESS_INDEX,
	THREAD_INDEX,
	EVENT_INDEX,
	MUTANT_INDEX,
	SEMAPHORE_INDEX,
	SECTION_INDEX,
	LPCPORT_INDEX,

	FILE_INDEX,
	DEVICE_INDEX,
	KEY_INDEX,

	LAST_INDEX_NEVER_USERD,
};


typedef struct _OBJECT_INIT_ 
{
/*0x000 */ ULONG ObjectIndex ;
/*0x004 */ WCHAR szObjecType[ ObjecTypeLength ] ; // 对象类型名,eg:"Process"
/*0x008 */ ULONG FakeAddrOpen ;		// 代理函数地址
/*0x00C */ ULONG FakeAddrParse ;	// 代理函数地址
/*0x010 */ ULONG TypeInfoOffset ;
/*0x014 */ ULONG ProcedureOffsetOpen ;
/*0x018 */ ULONG ProcedureOffsetParse ;

} OBJECT_INIT, *LPOBJECT_INIT ;


typedef struct _OBHOOKDATA_LITTLE_ 
{
/*0x000 */ ULONG FakeAddr ;		// 代理函数地址
/*0x004 */ ULONG OrignalAddr ;	// 原始Object指针

} OBHOOKDATA_LITTLE, *LPOBHOOKDATA_LITTLE ;


typedef struct _OBHOOKDATA_ 
{
/*0x000 */ ULONG ObjectIndex ;
/*0x004 */ OBHOOKDATA_LITTLE Open ;
/*0x008 */ OBHOOKDATA_LITTLE Parse ;

} OBHOOKDATA, *LPOBHOOKDATA ;


//
// 定义Oject Hook相关的大结构
//

typedef struct _OBJECT_DATA_LITTLE_ // size - 0x18
{
/*0x000 */ BOOL bCare ; // 是否关注当前节点

/*0x004 */ ULONG FakeAddr ;		// 代理函数地址
/*0x008 */ ULONG OrignalAddr ;	// 原始Object指针
/*0x01C */ ULONG UnhookPoint ;	// 卸载ObjectHook时要修改的内存地址
/*0x010 */ BOOL	 bHooked ;		// 是否成功Hook	
/*0x018 */ ULONG ProcedureOffset ; // 定位Object指针时的偏移,依赖于操作系统版本

/*
  以XP为例,这里的 TypeInfoOffset为+0x060; ProcedureOffset为 +0x03c
  nt!_OBJECT_TYPE -r // 这样看更明晰
    +0x060 TypeInfo         : _OBJECT_TYPE_INITIALIZER ; 
      +0x03c ParseProcedure   : (null) 
*/

} OBJECT_DATA_LITTLE, *LPOBJECT_DATA_LITTLE ;


typedef struct _OBJECT_DATA_INFO_ 
{
/*0x000 */ struct _OBJECT_DATA_INFO_* pFlink ; // 上个结点
/*0x004 */ struct _OBJECT_DATA_INFO_* pBlink ; // 下个结点

/*0x008 */ ULONG ObjectIndex ;
/*0x00C */ WCHAR szObjecType[ ObjecTypeLength ] ; // 对象类型名,eg:"Process"

/*0x010 */ ULONG TypeInfoOffset ; // 定位_OBJECT_TYPE_INITIALIZER指针时的偏移,依赖于操作系统版本

/*0x014 */ OBJECT_DATA_LITTLE Open ;
/*0x02C */ OBJECT_DATA_LITTLE Parse ;

} OBJECT_DATA_INFO, *LPOBJECT_DATA_INFO ;


//
// OD链表的总节点
//

typedef struct _OBJECT_DATA_INFO_HEAND_ 
{
/*0x000 */ int			nTotalCounts ;
/*0x004 */ PERESOURCE	QueueLockList	; // 链表的资源锁
/*0x008 */ OBJECT_DATA_INFO ListHead ;

} OBJECT_DATA_INFO_HEAND, *LPOBJECT_DATA_INFO_HEAND ;

extern LPOBJECT_DATA_INFO_HEAND g_ListHead__ObjectData ;
extern BOOL g_ObjectData_Inited_ok ;

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
InitObjectData (
	);

BOOL 
ODCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
ODDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
ODAllocateNode(
	OUT PVOID _pCurrenList_
	);

PVOID
ODBuildNode (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	);

PVOID
ODBuildNodeEx (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	);

PVOID
ODFindNode (
	IN PVOID _TotalHead ,
	IN ULONG ObjectIndex
	);

BOOL
ODDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG ObjectIndex
	);

VOID
ODDistroyAll (
	IN PVOID _TotalHead
	);

VOID
ODWalkNodes (
	IN PVOID _TotalHead
	);



///////////////////////////////   END OF FILE   ///////////////////////////////