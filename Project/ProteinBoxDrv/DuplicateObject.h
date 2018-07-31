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


typedef struct _IOCTL_DUPLICATEOBJECT_BUFFER_ // size - 0x14
{
	PHANDLE pTargetHandle ;
	HANDLE FuckedHandle ;
	HANDLE SourceHandle ;
	ACCESS_MASK DesiredAccess ;
	ULONG Options ;

} IOCTL_DUPLICATEOBJECT_BUFFER, *LPIOCTL_DUPLICATEOBJECT_BUFFER ;

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
Ioctl_DuplicateObject (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);

NTSTATUS 
Is_ObjectTypeName_Allowed (
	IN HANDLE Handle
	);

///////////////////////////////   END OF FILE   ///////////////////////////////