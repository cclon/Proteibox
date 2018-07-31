#pragma once

#include "Common.h"

//////////////////////////////////////////////////////////////////////////

//
// 定义内存结构体
//

#define LPPDHEAD LPPROCESS_NODE_INFO_HEAND
#define LPPDNODE LPPROCESS_NODE_INFO

#define PDHEAD PROCESS_NODE_INFO_HEAND
#define PDNODE PROCESS_NODE_INFO

//
// 定义操作内存的函数
//

#define kgetnodePD(_Tag)				PDFindNode(		(PVOID)g_ListHead__ProcessData, (ULONG)_Tag )
#define kbuildnodePD(_Tag, bInsert)		PDBuildNode(	(PVOID)g_ListHead__ProcessData, (ULONG)_Tag, (BOOL)bInsert )
#define kbuildnodePDF(Tag1, Tag2)		PDBuildNodeForce((PVOID)g_ListHead__ProcessData, (ULONG)Tag1, (LPWSTR)Tag2 )
#define kConfResetPD()					PDConfResetAll(	(PVOID)g_ListHead__ProcessData )
#define kRemovePD(_Tag)					PDDeleteNode(	(PVOID)g_ListHead__ProcessData, (ULONG)_Tag )
#define kCheckPD(_Tag)					PDCheckOut(		(PVOID)g_ListHead__ProcessData, (ULONG)_Tag )						
#define kfreePD()						PDDistroyAll(	(PVOID)g_ListHead__ProcessData )


// 定义宏:插入节点至链表尾
#define kInsertTailPD( _pNode ) \
{								\
	EnterCrit( g_ListHead__ProcessData->QueueLockList );	\
	InsertTailList( (PLIST_ENTRY) &g_ListHead__ProcessData->ListHead, (PLIST_ENTRY) _pNode ); \
	g_ListHead__ProcessData->nTotalCounts ++ ;	\
	LeaveCrit( g_ListHead__ProcessData->QueueLockList );	\
}


//#define g_default_FileRootPath		 L"\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%"
#define g_default_FileRootPath		 L"\\Device\\HarddiskVolume1\\ProteinBox\\SUDAMI\\DefaultBox"
//#define g_default_KeyRootPath		 L"\\REGISTRY\\USER\\Sandbox_%USER%_%SANDBOX%" 
#define g_default_KeyRootPath		 L"\\REGISTRY\\USER\\ProteinBox_SUDAMI_DefaultBox"
//#define g_default_IpcRootPath		 L"\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%"
#define g_default_IpcRootPath		 L"\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%"



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
// size - 0x078
//

typedef struct _NODE_INFO_C_ {

/*0x000 */	WCHAR BoxName[ MAX_PATH ] ;	// eg:"DefaultBox"
/*0x044 */  ULONG BoxNameLength ;

/*0x048 */  WCHAR RegisterUserID[ MAX_PATH ] ; // 调用ZwQueryInformationProcess()传递参数ProcessSessionInformation得到的SID
									  //eg:"S-1-5-21-583907252-261903793-1177238915-1003"
/*0x04C */  ULONG RegisterUserIDLength ; 

/*0x050 */  ULONG SessionId ;

/*0x058 */  WCHAR FileRootPath[ MAX_PATH ] ;  // eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
/*0x05C */  ULONG FileRootPathLength ;

/*0x060 */	WCHAR KeyRootPath[ MAX_PATH ] ;	// eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
/*0x064 */	ULONG KeyRootPathLength ;

/*0x068 */  WCHAR LpcRootPath1[ MAX_PATH ] ;			// eg:"\Sandbox\AV\DefaultBox\Session_0"
/*0x06C */  ULONG LpcRootPath1Length ;

/*0x070 */  WCHAR LpcRootPath2[ MAX_PATH ] ;			// eg:"_Sandbox_AV_DefaultBox_Session_0"
/*0x074 */  ULONG LpcRootPath2Length ;

} NODE_INFO_C, *PNODE_INFO_C ;


#pragma pack(1) // 表示按照1byte对齐
typedef struct _PROCESS_NODE_INFO_ // size - 0x180
{
	struct _PROCESS_NODE_INFO_* pFlink ;	// 上个节点
	struct _PROCESS_NODE_INFO_* pBlink ;	// 下个节点
	ULONG PID ;			// 当前进程ID
	PNODE_INFO_C pNode_C ;		// 指向大结构体 xxxxxxxxxxxxxxxxxxxxxxx
	LPWSTR lpProcessShortName ;	// 进程短名; eg:"unknown executable image"		
	ULONG ImageNameLength ; // 进程名长度

	BYTE bIsAVDefaultBox ; // 置1表明是重定向目录下的文件"被启动" eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\xx.exe"
	BYTE bSelfExe ;	// 置1表明是表明是自己目录下的程序"被启动";eg:"\Device\HarddiskVolume1\Program Files\Proteinbox\ProteinboxRpcSs.exe"
	BYTE bDiscard ;	// 置1表明是要销毁的结点	
	BYTE bReserved1 ;


	BYTE bReserved2 ;
	BYTE bDropAdminRights ;
	BYTE bReserved3 ;
	BYTE bReserved4 ;

	BOOL bProcessNodeInitOK ; // 当前节点是否初始化完毕

	// 和ImageNotify Inject Dll 相关
	struct 
	{
		PVOID BeCoveredOldData ; // 保存原PE块内容,R3的DLL被加载后负责用该部分数据恢复已改写的PE
		PVOID BeCoveredAddr ;	 // 被覆盖的PE内存块的起始地址
		ULONG BeCoveredSize ;	 // 被覆盖的PE内存块的大小

		ULONG IncrementCounts ;  // 同个进程会加载N个镜像导致多次的进入ImageNofity回调中;该变量的作用是保证只修正一次当前PE块

	} XImageNotifyDLL ;
	
	struct 
	{
		BYTE  bFlag_NotifyInternetAccessDenied ; // 阻止程序访问网络以后是否通知用户
		/* 配置文件中的<InternetAccess>提供一个网络访问的白名单，只允许白名单里的程序访问网络，其它程序禁止访问网络，类似应用程序防火墙功能
		在Internet Access 加入Firefox,会在全局设置 [GlobalSettings] 加入一个程序组，组名<InternetAccess_Firefox>,这个组只有一个组成员。 */

		BOOL bFlagInited ; // pNodeHead1,2,3所指向的Node结点,初始化完毕,置1
		PERESOURCE pResource ;				// 以下3个节点的资源锁
		LIST_ENTRY OpenFilePathListHead ;	// 保存允许直接操作的字符串,诸如:"\\Device\\NamedPipe\\protected_storage", g_Array_Device[] 
		LIST_ENTRY ClosedFilePathListHead ;	// 保存禁止访问的字符串,诸如:"\\Device\\LanmanRedirector","\\Device\\Mup" ,比如禁止访问"D:\Work"目录
		LIST_ENTRY ReadFilePathListHead ;	// 保存配置文件中 @ReadFilePath 对应的内容
	} XFilePath ;

	struct 
	{
		BYTE bFlag_NotifyStartRunAccessDenied ;
		BYTE bFlag_Denny_Access_KnownDllsSession ;  // eg: 'Y'或者"N",是否禁止访问全局Session对象" "\\KnownDlls\\*"

		BOOL bFlagInited ; // Node结点,初始化完毕,置1
		PERESOURCE pResource ;				// 以下2个节点的资源锁
		LIST_ENTRY OpenIpcPathListHead ;	/* 保存允许直接操作的字符串,诸如:"*\\BaseNamedObjects*\\PS_SERVICE_STARTED", "\\RPC Control\\OLE*",
											"\\RPC Control\\epmapper", "\\RPC Control\\protected_storage", g_Array_BaseNamedObjects[], g_Array_RPC[]
											*/
		LIST_ENTRY ClosedIpcPathListHead ;	// 保存禁止访问的字符串,即配置文件中 @ClosedIpcPath 对应的内容
	} XIpcPath ;

	struct 
	{
		BYTE bFlag_BlockFakeInput ; // 是否禁止虚拟按键; TRUE为禁止
		BYTE bFlag_BlockWinHooks ;  // 是否禁止NtUserSetWindowsHookEx,NtUserSetWinEventHook的调用
		BYTE bFlag_BlockSysParam ;	// 是否禁止NtUserSystemParametersInfo,NtUserSetSysColors的调用
		BYTE bFlag_SendPostMessage_allowed ; // 是否允许NtUser**发送消息. TRUE为允许
		
		HWND CurrentHwnd ; // 调用NtUserGetForegroundWindow得到受害者的窗口句柄,临时存储到此处
		// 若沙箱中的程序首次操作该窗体,沙箱的R3会弹出禁止消息,若第N次(N>=2)操作相同的窗体,不再弹出消息提示框
		// 若又开始操作其他窗体,则重新弹出提示框,这里就是临时存储当前的HWND,便于让R3决定是否弹框提示用户

		PVOID TranshipmentStation ; //  调用ZwAllocateVirtualMemory()分配的内存地址; 大小为0x1000
		// 通过NtUserGetClassName()函数得到当前hwnd对应的ClassName. 该指针仅作为当前进程的临时内存用,
		// 每次都会将获取到的内容拷贝至另一块较小的内存,而后自己就清空掉.  该内存无需释放,进程销毁时自然就没了

		PVOID VictimClassName ; // 受害者的窗口字符串指针,动态申请的内存,进程销毁时需要释放掉

		BOOL bFlagInited ; // WndListHead结点,初始化完毕,置1
		PERESOURCE pResource ;	// 以下节点的资源锁
		LIST_ENTRY WndListHead ; // 允许访问的窗口类名	
	} XWnd ;

	struct 
	{
		BOOL bFlagInited ; // Node结点,初始化完毕,置1
		PERESOURCE pResource ;				// 以下2个节点的资源锁
		LIST_ENTRY DirectListHead ;	// 保存允许直接操作的字符串( @OpenKeyPath )
		LIST_ENTRY DennyListHead ;	// 保存禁止访问的字符串,即配置文件中 @ClosedKeyPath 对应的内容
		LIST_ENTRY ReadOnlyListHead ; // 只读访问 Read-Only Access ( @ReadKeyPath )

	} XRegKey ;

/*0x0E0 */ ULONG FileTraceFlag ;
/*0x0E4 */ ULONG PipeTraceFlag ;

/*0x0E8 */ 
/*0x0EC */ PVOID pResource2 ;
/*0x0F0 */ PVOID pNode_RegHive ;

/*0x0F4 */ BYTE bFlag_BuildNode_RegeditKey_OK ;

/*0x0F8 */ PVOID pNodeHead_WhiteList_OpenKeyPath ;
/*0x104 */ PVOID pNodeHead_WhiteList_ClosedKeyPath ;
/*0x110 */ PVOID pNodeHead_WhiteList_ReadKeyPath ;

/*0x11C */ ULONG KeyTraceFlag ;

/*0x124 */ BYTE bFlag_BuildNode_RPC_BaseNamedObjects_OK ;


/*0x140 */ ULONG Flag_OpenProcedure_ThreadProcess ;// & 3有值,表明要调用DbgPrint打印信息; &2有值,表明允许操作当前进程/线程; &1有值,表明禁止操作当前进程/线程 
/*
if ( Flag_OpenProcedure_ThreadProcess & 3 )
  {
    if ( status >= 0 )
    {
      if ( Flag_OpenProcedure_ThreadProcess & 1 )
      {
        szInfo2 = 'A'; // Allowed
        goto _printf_;
      }
    }
    else
    {
      if ( Flag_OpenProcedure_ThreadProcess & 2 )
      {
        szInfo2 = 'D'; // Denney
_printf_:
        swprintf(&szInfo1, L"(P%c) %08X %06d", szInfo2, GrantedAccess, PID);
        DbgPrintEx((int)&szInfo1, (int)off_281A8);
        return status;
      }
    }
  }

  范例: (003472) SBIE (PA) 00000400 001672 
*/


/*0x164 */ HWND CurrentHwnd ; // 调用NtUserGetForegroundWindow得到受害者的窗口句柄,临时存储到此处
// 若沙箱中的程序首次操作该窗体,沙箱的R3会弹出禁止消息,若第N次(N>=2)操作相同的窗体,不再弹出消息提示框
// 若又开始操作其他窗体,则重新弹出提示框,这里就是临时存储当前的HWND,便于让R3决定是否弹框提示用户

/*0x168 */ PVOID pstrClassName ; // 调用ZwAllocateVirtualMemory()分配的内存地址; 大小为0x1000
								 // 通过NtUserGetClassName()函数得到当前hwnd对应的ClassName.	
/*0x16C */ PVOID pImageName2 ;
/*0x170 */ PVOID pNodeHead_WhiteList_Wnd ; // 指向结构体 Wnd_Head_Info, 这里面的ClassName都是白名单,即允许沙箱中的程序向包含这些类名的外界窗体发送消息
/*0x174 */
/*0x178 */
/*0x17C */ ULONG GuiTraceFlag ; // 根据标志位,设置指定的错误号,让沙箱的R3弹框提示禁止消息

} PROCESS_NODE_INFO, *LPPROCESS_NODE_INFO ;
#pragma pack()


//
// 进程链表的总节点
//

typedef struct _PROCESS_NODE_INFO_HEAND_ 
{
/*0x000 */ int			nTotalCounts ;
/*0x004 */ PERESOURCE	QueueLockList	; // 链表的自旋锁
/*0x008 */ PROCESS_NODE_INFO ListHead ;

} PROCESS_NODE_INFO_HEAND, *LPPROCESS_NODE_INFO_HEAND ;

extern LPPROCESS_NODE_INFO_HEAND g_ListHead__ProcessData ;
extern BOOL g_ProcessDataManager_Inited_ok ;


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
InitProcessData (
	);

BOOL 
PDCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
PDDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
PDAllocateNode(
	OUT PVOID _pCurrenList_
	);

PVOID
PDBuildNode (
	IN PVOID _TotalHead,
	IN ULONG PID,
	IN BOOL bInsert
	);

NTSTATUS
PDBuildNodeEx (
	OUT PVOID pNode,
	IN ULONG PID
	);

PVOID
PDBuildNodeForce (
	IN PVOID _TotalHead,
	IN ULONG PID,
	IN LPWSTR szFullImageName
	);

PVOID
PDCopyNodeC (
	IN PVOID Buffer
	);

PVOID
PDBuildNodeC (
	IN PVOID pInBuffer
	);

BOOL
__PDBuildNodeC (
	OUT PVOID _pNode
	);

BOOL 
PDFixupNode(
	OUT PVOID _pNode,
	IN LPCWSTR wszName
	);

NTSTATUS
PDEnumProcessEx(
	IN LPWSTR szBoxName,
	IN BOOL bFlag,
	OUT int *pArrays
	);

PVOID
PDFindNode (
	IN PVOID _TotalHead ,
	IN ULONG PID
	);

VOID
PDConfResetAll (
	IN PVOID _TotalHead
	);

VOID
PDClearNode (
	IN 	PVOID pNode
	);

BOOL
PDDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG PID
	);

VOID
PDDistroyAll (
	IN PVOID _TotalHead
	);

VOID
PDCheckOut(
	IN PVOID _TotalHead,
	IN ULONG InvalidPID
	);

VOID
PDWalkNodes (
	IN PVOID _TotalHead
	);


//////////////////////////////////////////////////////////////////////////


BOOL
CreateRootBox (
	IN PVOID _pNode
	);

NTSTATUS
CreateRootBoxEx (
	IN HANDLE hFile
	);

BOOL
CreateSessionDirectoryObject (
	IN PVOID _pNode
	);

BOOL
HandlerRegHive (
	IN PVOID _pNode
	);

BOOL 
Check_IsValid_Characters(
	IN LPWSTR pBuffer
	);

PVOID
ParseCharacters (
	IN PVOID pInBuffer
	);


///////////////////////////////   END OF FILE   ///////////////////////////////