#pragma once

//////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

BOOL Hook_pharase8_setupapidll_dep1( IN HMODULE hModule );
BOOL Hook_pharase8_setupapidll_dep2( HMODULE hModule, char Flag );
BOOL WINAPI Hook_pharase8_setupapidll( IN HMODULE hModule );
BOOL WINAPI Hook_pharase9_cfgmgr32dll( IN HMODULE hModule );

///////////////////////////////   END OF FILE   ///////////////////////////////
