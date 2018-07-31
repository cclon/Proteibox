#pragma once

#include <ntimage.h>

//////////////////////////////////////////////////////////////////////////

// 函数调用成功是否予以提示
#define __SDT_TRACE__ 0
#define SDTTrace if (__SDT_TRACE__) dprintf


// 定义内存结构体
#define LPMPHEAD PMAPPED_PE_HEAND
#define LPMPNODE PMAPPED_PE

#define MPHEAD MAPPED_PE_HEAND
#define MPNODE MAPPED_PE


// 定义操作 MappedPE 节点的函数
#define kgetaddrMP( _Tag )			MPFindNode  ( (PVOID)g_ListHead__MappedPE, _Tag )
#define kfreeMP()					MPDistroyAll( (PVOID)g_ListHead__MappedPE		)

// 得到指定(Shadow) SSDT 函数地址
#define CONCAT(x, y)		x ## y
#define CONCAT3(x, y, z)	x ## y ## z

#define ZwXXaddr( l )	CONCAT3( g_, l, _addr )
#define ZwTag( l )		CONCAT( Tag_, l )
#define ZwXXDefine( l ) CONCAT3( _, l, _ )

#define  kgetaddrSDT( Name )	Get_sdtfunc_addr( (PULONG)&ZwXXaddr(Name), (ULONG)ZwTag(Name) )


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


#pragma pack(push, 1)
typedef struct _ServiceDescriptorTableEntry 
{
	PVOID   *ServiceTable ;
	PULONG  CounterTable ;
	ULONG   TableSize ;
	PUCHAR  ArgumentTable ;        

} ServiceDescriptorTableEntry, *PServiceDescriptorTableEntry;
#pragma pack(pop)

__declspec(dllimport) ServiceDescriptorTableEntry KeServiceDescriptorTable ;


//
// ZwMapViewOfSection映射相关DLL到内存的结构体
//

typedef struct _MAPPED_PE_ // size - 0x64
{
/*0x000*/ struct _MAPPED_PE_* pFlink ; // 上个结点
/*0x004*/ struct _MAPPED_PE_* pBlink ; // 下个结点
/*0x008*/ WCHAR	wszModuleName[ 0x20 ] ;
/*0x048*/ HANDLE	hFile ; 
/*0x04C*/ HANDLE	SectionHandle ;
/*0x050*/ ULONG	ImageBase ;
/*0x054*/ ULONG	SizeOfImage ;
/*0x058*/ ULONG	MappedAddr ;
/*0x05C*/ PIMAGE_NT_HEADERS pinh ;
/*0x060*/ PIMAGE_EXPORT_DIRECTORY pEATAddr ;

} MAPPED_PE, *PMAPPED_PE ; 


typedef struct _MAPPED_PE_HEAND_ 
{
/*0x000*/ int			nTotalCounts ;
/*0x004*/ PERESOURCE	QueueLockList	; // 链表的自旋锁
/*0x008*/ MAPPED_PE	ListHead ;

} MAPPED_PE_HEAND, *PMAPPED_PE_HEAND ;


typedef struct _FAKE_FUNC_INFO_ 
{
	int		RetAddr   ; // 保存返回地址
	LONG	RefCounts ; 

} FAKE_FUNC_INFO, *LPFAKE_FUNC_INFO ;


//
// SSDT & Shadow SSDT函数相关结构体
//

typedef struct _SSDT_SSSDT_FUNC_ // size - 0x24
{
/*0x000*/ LPWSTR wszModuleName	;	// 模块名 eg:"NTDLL"
/*0x004*/ LPSTR	szFunctionName	;	// 函数名 eg: "ZwCreateToken"
/*0x008*/ ULONG	ArgumentNumbers ;	// (Shadow)SSDT函数的参数个数
/*0x00C*/
		union 
		{
			ULONG   xxIndex ;
			ULONG	SSDTIndex ;		// SSDT函数的索引号
			ULONG	ShadowIndex ;	// Shadow SSDT函数的索引号
		} _IndexU_ ;

/*0x010*/ ULONG SpecailVersion ; // 指明当前过滤函数对应的操作系统平台
// 比如SendNotifyMessageW在win2k下是NtUserSendNotifyMessage,其他平台则是NtUserMessageCall
// 那么该单元的值为__win2k时对应的fake函数是fake_NtUserSendNotifyMessage; 该值默认设为0

/*0x014*/
		union 
		{
			ULONG fakeFuncAddr ;
			ULONG fake_shadowFunc_addr ; // Shadow ssdt代理函数地址
			ULONG fake_ssdtFunc_addr ;   // ssdt代理函数地址
		} _AddrU_ ;

/*0x018*/ ULONG Tag ;				// 每个单元的序号,用于查找指定的函数
/*0x01C*/ ULONG HookType ; // 挂钩类型 (目前就2种: Header 5 Bytes Jmp / call Hook)
/*0x020*/ ULONG MappedFuncAddr	;	// 函数地址(后期通过GetProcAddress获得) 
/*0x024*/ ULONG RealFuncAddr	;	// 函数的真实地址
/*0x028*/ BOOL  bHooked ;			// 是否已经HOOK过
/*0x02C*/ FAKE_FUNC_INFO FakeFuncInfo ; // 保存过滤函数的返回值(地址) / 引用计数 等信息

} SSDT_SSSDT_FUNC, *LPSSDT_SSSDT_FUNC ;



typedef struct _SDT_DATA_ // size - 0x14
{
/*0x000*/ BOOL  bInited ;				 // 是否初始化成功
/*0x004*/ int	 TotalCounts ;			 // 数组的总个数
/*0x008*/ LPSSDT_SSSDT_FUNC pSdtArray ; // 数组的首地址

/*0x00C*/ int SpecCounts ;			 // 特殊数组的总个数
/*0x010*/ LPSSDT_SSSDT_FUNC SpecArray ; // 特殊数组的首地址

/*0x014*/ int	 ShadowArrayIndex ;		 // 总的sdt数组中,Shadows SSDT单元的起始序号(Index)
/*0x018*/ int	 ShadowSSDTCounts ;		 // 数组中Shadow SSDT个数

} SDT_DATA, *PSDT_DATA ;



#define SSDT_TAG_LowerLimit			0	 // ssdt 函数序列号的下限	
#define SSDT_TAG_UperLimit			100  // ssdt 函数序列号的上限.可以定义大一些,以后可以扩充

#define ShadowSSDT_TAG_LowerLimit	199	 // shadow ssdt 函数序列号的下限	


// 当前TAG是否属于ssdt函数单元内
#define IS_SSDT_TAG( l ) ( l > SSDT_TAG_LowerLimit && l <= SSDT_TAG_UperLimit )

// 当前TAG是否属于shadow ssdt函数单元内
#define IS_ShadowSSDT_TAG( l ) ( l > ShadowSSDT_TAG_LowerLimit )

// (shadow)ssdt 索引号是否非法; TRUE表示非法
#define IS_INVALID_INDEX( l ) ( l == 0xFFFFFFFF || l >= 0x2000 || (l & 0xFFF) >= 0x600 )


//
// 枚举类型,用于查找指定的函数
//


typedef enum _SPECIAL_SSDT_FUNC_Tag_ 
{
	TagSpec_NtRequestPort = 0,
	TagSpec_NtRequestWaitReplyPort = 1,
	TagSpec_NtAlpcSendWaitReceivePort = 2
};


typedef enum _SSDT_SSSDT_FUNC_Tag_ 
{
	Tag_ZwCreateToken = SSDT_TAG_LowerLimit + 1,// 1
	Tag_ZwSetInformationToken,
	Tag_ZwProtectVirtualMemory,
	Tag_ZwQueryInformationThread,

	// ******* ssdt & shadow ssdt Tag的分界线 *******

	Tag_GetForegroundWindow = ShadowSSDT_TAG_LowerLimit + 1, // 200
	Tag_IsHungAppWindow,
	Tag_GetClassNameW,
	Tag_SetWindowsHookExW,
	Tag_SetWinEventHook,
	Tag_PostMessageW,
	Tag_PostThreadMessageW,
	Tag_EnableWindow,
	Tag_DestroyWindow,
	Tag_SendInput,
	Tag_BlockInput,
	Tag_SetSysColors,
	Tag_SystemParametersInfoW,
	Tag_SendMessageTimeoutW,
	Tag_SendNotifyMessageW_win2k,
	Tag_SendNotifyMessageW,
	Tag_SendMessageCallbackW_win2k,
	Tag_SendMessageCallbackW,
};



typedef enum _sdt_show_info_ 
{
	_SDTWalkNodes_All_		= 0, // 0为打印全部;
	_SDTWalkNodes_SSDT_		= 1, // 1为打印SSDT数组
	_SDTWalkNodes_Shadow_	= 2, // 2为打印Shadow SSDT数组

} sdt_show_info ;



enum _AddressFindMethod_
{
	_AddressFindMethod_Shadow_	= 1,
	_AddressFindMethod_SSDT_	= 2,
};


enum _IndexCheckMethod_
{
	_IndexCheckMethod_Shadow_	= 1,
	_IndexCheckMethod_SSDT_		= 2,
};


#define IS_INVALID_METHOD( l ) ( l > 0 && l < 3 )


//////////////////////////////////////////////////////////////////////////

// others
extern BOOL g_MappedPE_Inited_ok ;

extern PMAPPED_PE_HEAND g_ListHead__MappedPE ; 

extern SDT_DATA g_SdtData ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
InitSdtData (
	);

ULONG
GetProcAddress (
	IN LPWSTR wszModuleName,
	IN LPSTR szFunctionName,
	IN BOOL bReloc
	);

PVOID
LoadPE (
	IN LPWSTR wszModuleName
	);

ULONG 
GetMappedFuncAddr (
	IN PVOID _pMappedInfo,
	IN LPSTR szFunctionName, 
	IN BOOL bReloc
	);

BOOL 
MPCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
MPDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
MPAllocateNode(
	OUT PVOID* _pCurrenList_
	);

PVOID
MPFindNode (
	IN PVOID  _TotalHead,
	IN LPWSTR wszModuleName
	);

PVOID
MPBuildNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszModuleName
	);

PVOID
MPBuildNodeEx (
	IN PVOID _TotalHead ,
	IN LPWSTR wszModuleName
	);

VOID
MPDistroyAll (
	IN PVOID _TotalHead
	);

VOID
MPWalkNodes (
	IN PVOID _TotalHead
	);

ULONG 
GetRealAddr (
	IN ULONG addr,
	IN PVOID _pMappedInfo
	);

BOOL
IsValidPE (
	ULONG PEAddr
	);



VOID
SDTWalkNodes (
	IN int Index
	);

BOOL
Get_sdt_function_addr (
	OUT PVOID _pNode,
	IN int AddressFindMethod,
	IN int IndexCheckMethod
	);

ULONG
Get_sdt_function_addr_normal (
	IN ULONG Index,
	IN ULONG ArgumentNumbers
	);

ULONG
Get_sdt_function_addr_force (
	IN ULONG Index
	);

ULONG
Get_sdt_function_addr_force_step1 (
	IN ULONG Index
	);

ULONG
Get_sdt_function_addr_force_step2 (
	IN ULONG Index
	);

BOOL
Get_sdtfunc_addr (
	IN PULONG addr,
	IN ULONG Tag
	);

ULONG
Get_KeServiceDescriptorTable_addr(
	) ;

PVOID
Get_sdt_Array (
	IN int Tag
	);

PVOID
Get_sdt_Array_spec (
	IN int Tag
	);

ULONG
Get_sdtfunc_addr_ex (
	IN int Tag
	);

ULONG
GetSDTIndex (
	IN ULONG pMappedAddr,
	IN BOOL bIsShadow
	);

int
GetSDTIndexEx (
	IN ULONG pMappedAddr
	);

int
GetSDTIndexExp (
	IN ULONG pMappedAddr,
	IN ULONG AddressArray,
	IN ULONG DeepCounts,
	IN int CurrentResult
	);

BOOL 
IsFunctionEnd (
	PBYTE CurrentEIP
	);

BOOL 
IsKiFastSystemCall (
	PBYTE CurrentEIP
	);

BOOL
PatchSDTFunc (
	IN PVOID _pNode,
	IN BOOL bHook
	);

///////////////////////////////   END OF FILE   ///////////////////////////////