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


#define _WhiteOrBlack_Flag_LowerLimit_	0x60
#define _WhiteOrBlack_Flag_UperLimit_	0x70

#define IsWBFlagOK( l )	  ( l >= _WhiteOrBlack_Flag_LowerLimit_ && l <= _WhiteOrBlack_Flag_UperLimit_ ) 

enum _WhiteOrBlack_Flag_ 
{
	WhiteOrBlack_Flag_XFilePath = _WhiteOrBlack_Flag_LowerLimit_ ,
	WhiteOrBlack_Flag_XRegKey,
	WhiteOrBlack_Flag_XIpcPath,
	WhiteOrBlack_Flag_XClassName,
};


typedef struct _IOCTL_WHITEORBLACK_BUFFER_ // size - 0x24
{
	ULONG Flag ;	 // [IN] 标记要匹配的链表(File/Reg/IPC ...)
	LPCWSTR szPath ; // [IN] 待检测的字符串指针
	ULONG PathLength ; // [IN] 待检测的字符串长度

	BOOL* bIsWhite ; // [OUT] 是否为白
	BOOL* bIsBlack ; // [OUT] 是否为黑

} IOCTL_WHITEORBLACK_BUFFER, *LPIOCTL_WHITEORBLACK_BUFFER ;

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
Ioctl_WhiteOrBlack (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);



///////////////////////////////   END OF FILE   ///////////////////////////////