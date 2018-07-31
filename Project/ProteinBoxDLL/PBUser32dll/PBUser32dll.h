#pragma once

#include "PBUser32dllFunctions.h"
#include "PBUser32dllData.h"

//////////////////////////////////////////////////////////////////////////

#define EWX_SHUTDOWN_REBOOT_POWEROFF 0xB
#define WM_CAP_DRIVER_DISCONNECT  (WM_USER+  11)

#define WM_DDE_FIRST	    0x03E0
#define WM_DDE_INITIATE     (WM_DDE_FIRST)
#define WM_DDE_TERMINATE    (WM_DDE_FIRST+1)
#define WM_DDE_ADVISE	    (WM_DDE_FIRST+2)
#define WM_DDE_UNADVISE     (WM_DDE_FIRST+3)
#define WM_DDE_ACK	        (WM_DDE_FIRST+4)


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL WINAPI Hook_pharase12_user32dll( IN HMODULE hModule );


///////////////////////////////   END OF FILE   ///////////////////////////////
