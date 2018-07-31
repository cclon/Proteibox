#pragma once

#include <ntimage.h>


//////////////////////////////////////////////////////////////////////////

#define g_szDllPathW	L"\\??\\C:\\Test\\ProteinBoxDLL.dll"
#define g_szDllPathA	"\\??\\C:\\Test\\ProteinBoxDLL.dll"


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


//
// 新构造的PE结构
//

typedef struct _NEW_PE_IAT_INFO_before_ { // size - 0x68

/*0x000 */ ULONG Reserved1[ 4 ] ;

/*0x010 */ PIMAGE_DATA_DIRECTORY pidd ;
/*0x014 */ ULONG Reserved2 ;
/*0x018 */ ULONG IMAGE_DIRECTORY_ENTRY_IMPORT_VirtualAddress ;
/*0x01C */ ULONG IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_VirtualAddress ;
/*0x020 */ ULONG IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_Size ;
/*0x024 */ ULONG IMAGE_DIRECTORY_ENTRY_IAT_VirtualAddress ;
/*0x028 */ ULONG IMAGE_DIRECTORY_ENTRY_IAT_Size ;
/*0x02C */ ULONG IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_VirtualAddress ;
/*0x030 */ ULONG IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_Size ;
/*0x034 */ ULONG OldProtection ;
/*0x038 */ PIMAGE_IMPORT_DESCRIPTOR pImportTable_mscoree_dll ;
/*0x03C */ PIMAGE_IMPORT_DESCRIPTOR pImportTable_msvcm80_dll ;
/*0x040 */ PIMAGE_IMPORT_DESCRIPTOR pImportTable_msvcm90_dll ;
/*0x044 */ IMAGE_IMPORT_BY_NAME IIN1 ;
/*0x048 */ ULONG Reserved4[ 4 ] ;
/*0x058 */ IMAGE_THUNK_DATA32 itd_1 ;
/*0x05C */ IMAGE_THUNK_DATA32 itd_2 ;
/*0x060 */ ULONG Reserved5[ 2 ] ;

} NEW_PE_IAT_INFO_before, *PNEW_PE_IAT_INFO_before ;


typedef struct _NEW_PE_IAT_INFO_TOTAL_ {

/*0x000 */ NEW_PE_IAT_INFO_before Stub ;
/*0x068 */ IMAGE_IMPORT_DESCRIPTOR IATNew ;
/*0x07C */ IMAGE_IMPORT_DESCRIPTOR IATOld[ 1 ] ;

} NEW_PE_IAT_INFO_TOTAL , *PNEW_PE_IAT_INFO_TOTAL ;


//
// 关注的IDD列表
//

static const ULONG g_ImageDataDirectoryIndex_concerned_array[] = 
{
	IMAGE_DIRECTORY_ENTRY_EXPORT,		// 0,  Export Directory	
	IMAGE_DIRECTORY_ENTRY_IMPORT,		// 1,  Import Directory
	IMAGE_DIRECTORY_ENTRY_EXCEPTION,	// 3,  Exception Directory
	IMAGE_DIRECTORY_ENTRY_SECURITY,		// 4,  Security Directory
	IMAGE_DIRECTORY_ENTRY_BASERELOC,	// 5,  Base Relocation Table
	IMAGE_DIRECTORY_ENTRY_GLOBALPTR,    // 8,  RVA of GP
	IMAGE_DIRECTORY_ENTRY_TLS,          // 9,  TLS Directory
	IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,  // 10, Load Configuration Directory
	IMAGE_DIRECTORY_ENTRY_IAT,			// 12, Import Address Table

#define IDD_ARRAY_END -1

	IDD_ARRAY_END
} ;


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
InjectDll_by_reconstruct_pe (
	IN PVOID _pNode,
	IN ULONG PEAddr,
	IN LPSTR szDllPath
	) ;

PVOID 
InjectDll_by_reconstruct_pe_dep(
	IN BOOL bIsX64,
	IN PIMAGE_DOS_HEADER PEAddr,
	IN PIMAGE_SECTION_HEADER pSectionHeader,
	IN ULONG NumberOfSections,
	IN PIMAGE_DATA_DIRECTORY pImageDataDirectory,
	IN LPSTR szDllPath,
	OUT PULONG out_cbNew,
	OUT PVOID out_pbNewIid_offset
	) ;

ULONG 
FindNearBase (
	IN PIMAGE_SECTION_HEADER pSectionHeader,
	IN ULONG NumberOfSections,
	IN PIMAGE_DATA_DIRECTORY pImageDataDirectory,
	IN ULONG cbNew
	) ;

DWORD 
PadToDword( 
	IN DWORD dw
	);

DWORD
PadToDwordPtr( 
	IN DWORD dw
	);

BOOL Get_ZwProtectVirtualMemory_addr () ;

PVOID 
FindUnresolved(
	IN PVOID pFunc 
	);

BOOL 
CheckPattern( 
	IN unsigned char* pattern1,
	IN unsigned char* pattern2,
	IN size_t size
	);

BOOL
Make_address_writable(
	IN PVOID address,
	IN ULONG size,
	IN ULONG pageAccess,
	OUT ULONG* OldAccessProtection
	);


///////////////////////////////   END OF FILE   ///////////////////////////////