#pragma once


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef enum _IOCTL_DISABLEFORCEPROCESS_FLAG_
{
	_FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForce_	 = 3, // 表示开启强制运行的功能
	_FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForce_	 = 5, // 表示关闭强制运行的功能
	_FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForceALL_ = 7, // 表示取消所有的强制运行
	_FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForceALL_  = 9, // 表示开启所有的强制运行

} IOCTL_DISABLEFORCEPROCESS_FLAG ;


typedef struct _IOCTL_DISABLEFORCEPROCESS_BUFFER_ // size - 0x24
{
/*0x000 */ LPWSTR szProcName ; // 待取消的进程全路径
/*0x008 */ int NameLength ; // 待取消的进程全路径长度
/*0x00C */ int Flag ; // [开关] 可动态的开启/关闭 强制运行的功能

} IOCTL_DISABLEFORCEPROCESS_BUFFER, *LPIOCTL_DISABLEFORCEPROCESS_BUFFER ;


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
Ioctl_DisableForceProcess (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);


///////////////////////////////   END OF FILE   ///////////////////////////////