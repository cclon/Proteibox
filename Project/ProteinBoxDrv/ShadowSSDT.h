#pragma once

#include <ntimage.h>


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
// Inline Hook Shadow SSDT相关结构体
//

typedef struct _IOCTL_HOOKSHADOW_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // Shadow ssdt inline hook的开关
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKSHADOW_BUFFER, *LPIOCTL_HOOKSHADOW_BUFFER ;


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
Ioctl_HookShadow (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);

BOOL
HookShadowSSDT (
	);

VOID
UnhookShadowSSDT (
	);

VOID
UnhookShadowSSDTEx (
	);

BOOL
HandlerEnableWindow (
	);

ULONG
HandlerEnableWindowEx (
	IN int Tag
	);

VOID
HandlerSystemParametersInfoW (
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
