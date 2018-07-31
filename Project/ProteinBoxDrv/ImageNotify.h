#pragma once

//////////////////////////////////////////////////////////////////////////

extern NOTIY_INFO g_ImageNotify_Info ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

void
ImageNotifyRoutine (
  IN PUNICODE_STRING  FullImageName,
  IN HANDLE  ProcessId,
  IN PIMAGE_INFO  ImageInfo
  );

BOOL 
CheckImageNotifyState (
	);

BOOL
Is_special_process (
	IN PUNICODE_STRING  FullImageName
	);

///////////////////////////////   END OF FILE   ///////////////////////////////