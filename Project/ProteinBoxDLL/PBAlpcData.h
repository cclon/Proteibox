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
GetAlpcPath (
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT LPWSTR *lppOrignalPath,
	OUT LPWSTR *lppRedirectedPath
	);

NTSTATUS 
GetAlpcPathEx (
	IN PUNICODE_STRING ObjectName,
	IN HANDLE RootDirectory,
	OUT LPWSTR *lppOrignalPath,
	OUT LPWSTR *lppRedirectedPath,
	OUT BYTE *out_bFlag_Is_Sandbox_Object
	);

NTSTATUS 
CreateDirOrLinks (
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	);

BOOL
PB_StartCOM_Ex (
	IN LPWSTR szName
	);

HANDLE 
GetEventHandleSBIE (
	IN LPWSTR szName,
	OUT BOOL* bFlagCreateEvent
	);

HMODULE GetPBDllHandle();

VOID HandlerAlpcList();

VOID CreateObjectLink();

NTSTATUS MyRpcImpersonateClient();

///////////////////////////////   END OF FILE   ///////////////////////////////