#pragma once

//////////////////////////////////////////////////////////////////////////



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
// 灰名单结构体
//

typedef struct _GRAYLIST_INFO_ {

/*0x000 */ LIST_ENTRY ListEntry ;

/*+0x008*/ ULONG NameLength ;	// 字符串长度
/*+0x00C*/ LPWSTR wszName ;	// 字符串内容的指针. eg: c:\Test.exe

} GRAYLIST_INFO, *LPGRAYLIST_INFO ;


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
GLAllocateNode(
	OUT PVOID* pNode
	);

PVOID
GLBuildNode (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead,
	IN LPWSTR wszName,
	IN int NameLength
	);

BOOL
GLCopyNode (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead,
	IN PVOID KeyHead
	);

PVOID
GLFindNode (
	IN PLIST_ENTRY ListHead,
	IN PERESOURCE QueueLockList,
	IN LPWSTR wszName
	);

BOOL
GLFindNodeEx (
	IN PLIST_ENTRY ListHead,
	IN PERESOURCE QueueLockList,
	IN LPWSTR Key,
	OUT LPWSTR Name
	);

VOID
GLDistroyAll (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead
	);

VOID
GLWalkNodes (
	IN PERESOURCE QueueLockList,
	IN PLIST_ENTRY ListHead
	);

BOOL
BuildGrayList_for_IopParse (
	IN PVOID ProcessNode
	);

BOOL
BuildGrayList_for_OpenProcedure (
	IN PVOID ProcessNode
	);

BOOL
BuildGrayList_for_Wnd (
	IN PVOID ProcessNode
	);

BOOL
BuildGrayList_for_RegKey (
	IN PVOID ProcessNode
	);

BOOL
IsKnownSID (
	IN PVOID ProcessNode
	);

///////////////////////////////   END OF FILE   ///////////////////////////////