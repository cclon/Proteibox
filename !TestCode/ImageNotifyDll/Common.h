#pragma once

#include <ntddk.h>


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

VOID 
NtPath2DosPathA (
	IN LPCSTR szNtPath,
	OUT LPSTR szDosPath,
	IN DWORD cchSize
	);

VOID 
NtPath2DosPathW (
	IN LPCWSTR szNtPath,
	OUT LPWSTR szDosPath,
	IN DWORD cchSize
	);

DWORD
QueryDosDeviceW(
	IN LPCWSTR lpDeviceName,
	OUT LPWSTR lpTargetPath,
	IN DWORD ucchMax
	);

NTSTATUS
PutFile(
	IN WCHAR* filename,
	IN CHAR* buffer,
	IN ULONG buffersize 
	) ;


///////////////////////////////   END OF FILE   ///////////////////////////////