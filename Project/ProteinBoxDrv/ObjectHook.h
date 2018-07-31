#pragma once

//////////////////////////////////////////////////////////////////////////

// 对链表中所有指定要hook的对象体指针进行Patch
#define OBHook()		HookObjectAll(	 (PVOID)g_ListHead__ObjectData )
#define OBUnhook()		UnhookObjectAll( (PVOID)g_ListHead__ObjectData )


typedef struct _ADDR_FOR_CaptureStackBackTrace_ 
{
/*0x000 */ ULONG ObOpenObjectByPointerWithTag_addr_vista;
/*0x004 */ ULONG ObOpenObjectByPointer_addr_vista ;
/*0x008 */ ULONG KeFindConfigurationEntry_addr ;
/*0x00C */ ULONG PsCreateSystemThread_addr ;

} ADDR_FOR_CaptureStackBackTrace, *LPADDR_FOR_CaptureStackBackTrace ;

extern LPADDR_FOR_CaptureStackBackTrace g_CaptureStackBackTrace_needed_data ;


//
// Object Hook 相关结构体
//

typedef struct _IOCTL_HOOKOBJECT_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // hook开关
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKOBJECT_BUFFER, *LPIOCTL_HOOKOBJECT_BUFFER ;


typedef PVOID (WINAPI* _ObQueryNameInfo_) ( IN PVOID Object );
extern _ObQueryNameInfo_ g_ObQueryNameInfo_addr ;

typedef PVOID (WINAPI* _ObGetObjectType_) ( IN PVOID Object );
extern _ObGetObjectType_ g_ObGetObjectType_addr ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

NTSTATUS
Ioctl_HookObject (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);

BOOL
HandlerObject (
	);

BOOL
Prepare_for_ObjectHook (
	);

BOOL
Init_objectXX_address(
	);

BOOL
HookObjectOne (
	IN PVOID pInBufferS,
	IN PVOID pInBufferB
	);

BOOL
HookObjectAll (
	IN PVOID _TotalHead
	);

VOID
UnhookObjectAll(
	IN PVOID _TotalHead
	);

///////////////////////////////   END OF FILE   ///////////////////////////////