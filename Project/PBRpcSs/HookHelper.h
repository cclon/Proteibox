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


#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE (3L)
#define STATUS_PRIVILEGE_NOT_HELD ((NTSTATUS)0xC0000061L)

#define ARRAYSIZEOF(x) sizeof (x) / sizeof (x[0])

#define MAXLENGTH 0x30

typedef enum _DLLTAG_
{
	Ntdll_TAG = 0,
	Kernel32_TAG,
	KernelBase_TAG,
	ADVAPI32_TAG,
	WS2_32_TAG,

	Nothing_TAG = 0x10,
};

typedef struct _HOOKINFOLittle_rpcss_ 
{
	ULONG_PTR DllTag ;	// 函数所属的模块标记
	ULONG_PTR Tag ; // 标记当前节点单元
	char FunctionName[ MAXLENGTH ]; // 函数名
	char FunctionNameEx[ MAXLENGTH ]; // WIN7函数名,如果为空，表明和上面的函数名一致
	BOOL bHooked ;	// 是否被Hook
	PVOID OrignalAddress ;	// 原始地址
	PVOID FakeAddress ;		// 过滤函数地址
} HOOKINFOLittleRpcss, *LPHOOKINFOLittleRpcss ;


typedef struct _MODULEINFO_ 
{
/*0x000*/ HMODULE hModuleArrays[10];

} MODULEINFO, *LPMODULEINFO ;

extern LPMODULEINFO __ProcModulesInfo;
extern HMODULE g_hModule_KernelBase;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL HookOne( LPHOOKINFOLittleRpcss Info );
VOID GethModule();
PVOID kmalloc ( ULONG length );
VOID kfree ( PVOID ptr );

BOOL
MmIsAddressValid(
	IN PVOID ptr,
	IN ULONG length
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
