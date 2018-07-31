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

#define STATUS_BUFFER_OVERFLOW				((NTSTATUS)0x80000005L)
#define STATUS_CANCELLED					((NTSTATUS)0xC0000120L)
#define STATUS_INSUFFICIENT_RESOURCES		((NTSTATUS)0xC000009AL) 


#define FILE_DIRECTORY_FILE          0x00000001
#define FILE_NON_DIRECTORY_FILE		 0x00000040
#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020
#define FILE_OPEN_NO_RECALL          0x00400000


#define FILE_OPEN 0x00000001
#define FILE_OPEN_IF 0x00000003
#define FILE_OVERWRITE_IF 0x00000005

#define FILE_COMPLETE_IF_OPLOCKED 0x00000100
#define FILE_OPEN_FOR_BACKUP_INTENT 0x00004000

typedef struct _LIST_ENTRY_EX_ 
{
	struct _LIST_ENTRY *Flink;
	struct _LIST_ENTRY *Blink;

	ULONG TotalCounts ;

} LIST_ENTRY_EX, *LPLIST_ENTRY_EX ;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

LPLIST_ENTRY_EX 
ClearStruct( 
	IN LPLIST_ENTRY_EX pNode
	);

PLIST_ENTRY
RemoveEntryListEx (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pCurrentNode
	);

BOOL
MmIsAddressValid(
	IN PVOID ptr,
	IN ULONG length
	);

PVOID kmalloc ( ULONG length );

VOID kfree ( PVOID ptr );

void kfreeEx( PVOID pBuffer );

///////////////////////////////   END OF FILE   ///////////////////////////////
