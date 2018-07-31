#pragma once

#include <ntddk.h>


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef struct _IMAGENOTIY_INFO_ {

/*0x000 */ BOOL bNotifyState ;		// 是否注册成功
/*0x004 */ PVOID NotifyRoutine ;    // 回调函数地址

} IMAGENOTIY_INFO, *PIMAGENOTIY_INFO ;

extern IMAGENOTIY_INFO g_ImageNotify_Info ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


void
DoWork (
	);

void
ImageNotifyRoutine (
  IN PUNICODE_STRING  FullImageName,
  IN HANDLE  ProcessId,
  IN PIMAGE_INFO  ImageInfo
  );

BOOL 
CheckNotifyState (
	);

BOOL
Is_special_process (
	IN PUNICODE_STRING  FullImageName
	);

///////////////////////////////   END OF FILE   ///////////////////////////////