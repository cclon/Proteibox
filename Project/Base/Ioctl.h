#pragma once

#include <winioctl.h>


//
// The device driver IOCTLs
//

#define IOCTL_BASE	0x800
#define MY_CTL_CODE(i) CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_HELLO				MY_CTL_CODE( 0 )
#define IOCTL_PROTEINBOX		MY_CTL_CODE( 1 )

const WCHAR g_PBLinkName[] = L"\\\\.\\ProteinBoxDrv" ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


#define IOCTL_PROTEINBOX_FLAG__LowerLimit	0x12340000
#define IOCTL_PROTEINBOX_FLAG__UperLimit	IOCTL_PROTEINBOX_FLAG__LowerLimit + 0x100

// 索引号是否非法; TRUE表示非法
#define IS_INVALID_IOCTL_FLAG( l ) ( l == 0xFFFFFFFF || l <= IOCTL_PROTEINBOX_FLAG__LowerLimit || l >= IOCTL_PROTEINBOX_FLAG__UperLimit )


typedef enum _IOCTL_PROTEINBOX_FLAG_ 
{
	FLAG_Ioctl_GetSysVersion		= IOCTL_PROTEINBOX_FLAG__LowerLimit + 1, // 0x12340001
	FLAG_Ioctl_GetWork 				= 0x12340002, 
	FLAG_Ioctl_WhiteOrBlack			= 0x12340003, // 判定字符串的黑白
	FLAG_Ioctl_GetLicense			= 0x12340004,
	FLAG_Ioctl_SetLicense			= 0x12340005,
	FLAG_Ioctl_StartProcess			= 0x12340006, // 在沙箱中启动进程
	FLAG_Ioctl_QueryProcess			= 0x12340007, 
	FLAG_Ioctl_QueryBoxPath			= 0x12340008,
	FLAG_octl_QueryProcessPath		= 0x12340009,
	FLAG_Ioctl_QueryPathList		= 0x1234000A,
	FLAG_Ioctl_EnumProcessEx		= 0x1234000B,
	FLAG_Ioctl_DisableForceProcess	= 0x1234000C,
	FLAG_Ioctl_HookTramp			= 0x1234000D,
	FLAG_Ioctl_EnumBoxs				= 0x1234000E,
	FLAG_Ioctl_HandlerConf			= 0x1234000F, // 操作配置文件 ProteinBox.ini
	FLAG_Ioctl_ReloadConf			= 0x12340010,
	FLAG_Ioctl_CreateDirOrLink		= 0x12340011, // 创建符号链接
	FLAG_Ioctl_DuplicateObject		= 0x12340012, // 处理复制句柄的操作
	FLAG_Ioctl_GetInjectSaveArea	= 0x12340013, // 获取ImageNofity修改的PE数据
	FLAG_Ioctl_RenameFile			= 0x12340014,
	FLAG_Ioctl_SetUserName			= 0x12340015,
	FLAG_Ioctl_HookShadowSSDT		= 0x12340016,
	FLAG_Ioctl_DriverUnload			= 0x12340017,
	FLAG_Ioctl_GetSetDeviceMap		= 0x12340018,
	FLAG_Ioctl_1					= 0x12340019,
	FLAG_Reserved2					= 0x1234001A,
	FLAG_Ioctl_MonitorControl		= 0x1234001B,
	FLAG_Ioctl_MonitorPut			= 0x1234001C,
	FLAG_Ioctl_MonitorGet			= 0x1234001D,
	FLAG_Ioctl_GetUnmountHive		= 0x1234001E,
	FLAG_Ioctl_GetFileName			= 0x1234001F,
	FLAG_Ioctl_ClearUpNode			= 0x12340020,
	FLAG_Ioctl_SetLsaAuthPkg		= 0x12340021,
	FLAG_Ioctl_IoCreateFileSpecifyDeviceObjectHint	= 0x12340022,
	FLAG_Ioctl_2					= 0x12340023,
	FLAG_Ioctl_HookObject			= 0x12340024,
	FLAG_Ioctl_QueryProcessPath		= 0x12340025, 


} IOCTL_PROTEINBOX_FLAG ;




typedef struct _IOCTL_GETSYSVERSION_BUFFER_ // size - 0x4
{
/*0x000 */ LPWSTR wszVersion ;

} IOCTL_GETSYSVERSION_BUFFER, *PIOCTL_GETSYSVERSION_BUFFER ;


typedef enum _IOCTL_ENUMBOXS_FLAG_
{
	_IOCTL_ENUMBOXS_FLAG_GetSandboxsCounts_  = 3,
	_IOCTL_ENUMBOXS_FLAG_GetSandboxsCurName_ = 5,

} IOCTL_ENUMBOXS_FLAG ;


typedef struct _IOCTL_ENUMBOXS_BUFFER_ // size - 0x24
{
/*0x000 */ int Flag ; // 3 - 第一次通信,告诉R3沙箱个数; 5 - 再次通信,找到@CurrentIndex对应的节点,拷贝沙箱名
/*0x004 */ 
		union
		{
			int CurrentIndex ; // 指定当前要获取的节点编号;
			int* SandboxsCounts ; // [OUT] 存放沙箱的个数
		} u ;

/*0x008 */ PVOID lpData ; // [OUT] R3缓冲区,用来存放获取到的沙箱名[ Length <= MAX_PATH ]
/*0x00C */ int INDataLength ; // R3缓冲区大小

} IOCTL_ENUMBOXS_BUFFER, *LPIOCTL_ENUMBOXS_BUFFER ;


typedef struct _IOCTL_STARTPROCESS_BUFFER_ // size - 0x24
{
/*0x000 */ LPWSTR wszBoxName ;			// 当前沙箱名
/*0x004 */ LPWSTR wszRegisterUserID ;	// 当前用户SID; eg: S-1-5-21-425828121-612454800-1421510826-500_Classes
/*0x008 */ HANDLE* new_hToken ;			// 保存新句柄,供R3创建"被SB"的进程时使用	
/*0x00C */ HANDLE* new_ImpersonationToken ;

} IOCTL_STARTPROCESS_BUFFER, *LPIOCTL_STARTPROCESS_BUFFER ;


typedef struct _IOCTL_QUERYPROCESS_BUFFER_ // size - 0x38
{
/*0x000 */ ULONG PID ;

/*0x004 */ LPWSTR lpBoxName ; // 保存当前的沙箱名
/*0x008 */ ULONG BoxNameMaxLength ;

/*0x00C */ LPWSTR lpCurrentProcessShortName ; // 保存当前的进程短名
/*0x010 */ ULONG ProcessShortNameMaxLength ;

/*0x014 */ LPWSTR lpRegisterUserID ; // 当前进程对应的SID
/*0x018 */ ULONG RegisterUserIDMaxLength ;

/*0x01C */ PULONG SessionId ; // 当前进程对应的SessionId

/*0x020 */  LPWSTR FileRootPath ; 
/*0x024 */  ULONG FileRootPathLength ;

/*0x028 */	LPWSTR KeyRootPath;	
/*0x02C */	ULONG KeyRootPathLength ;

/*0x030 */  LPWSTR LpcRootPath ;
/*0x034 */  ULONG LpcRootPathLength ;

} IOCTL_QUERYPROCESS_BUFFER, *LPIOCTL_QUERYPROCESS_BUFFER ;



//////////////////////////////////////////////////////////////////////////

typedef enum _IOCTL_HANDLERCONF_FUNCTION_TYPE_ 
{
	_Ioctl_Conf_function_InitData_		= 0x1001, // R3发送该Ioctl,将配置文件中的所有信息全部交给R0存储 
	_Ioctl_Conf_function_VerifyData_	= 0x1002, // R3发送该Ioctl,让R0填充Buffer,其中包含了读/写指定建的信息
	_Ioctl_Conf_function_ReceiveData_	= 0x1003, // R3发送该Ioctl,将读取到得配置文件信息传递到R0中	
	_Ioctl_Conf_function_Reload_		= 0x1004, // 重新加载配置文件,即R3重新读取配置文件后,将所有更新后的数据抛给驱动	

} IOCTL_HANDLERCONF_FUNCTION_TYPE ;


typedef enum _IOCTL_HANDLERCONF_BUFFER_TYPE_ 
{
	_Ioctl_Conf_Read_	= 1,
	_Ioctl_Conf_Write_	= 2,

} IOCTL_HANDLERCONF_BUFFER_TYPE ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_IniDataA_ // size - 0x018
{
/*0x000 */ LPSTR SeactionName ;
/*0x004 */ ULONG SeactionNameLength ;

/*0x008 */ LPSTR KeyName ;
/*0x00C */ ULONG KeyNameLength ;

/*0x010 */ LPSTR ValueName ;
/*0x014 */ ULONG ValueNameLength ;

} IOCTL_HANDLERCONF_BUFFER_IniDataA, *LPIOCTL_HANDLERCONF_BUFFER_IniDataA ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_IniDataW_ // size - 0x018
{
/*0x000 */ LPWSTR SeactionName ;
/*0x004 */ ULONG SeactionNameLength ;

/*0x008 */ LPWSTR KeyName ;
/*0x00C */ ULONG KeyNameLength ;

/*0x010 */ LPWSTR ValueName ;
/*0x014 */ ULONG ValueNameLength ;

} IOCTL_HANDLERCONF_BUFFER_IniDataW, *LPIOCTL_HANDLERCONF_BUFFER_IniDataW ;


#define IOCTL_HANDLERCONF_BUFFER_NAMELENGTH 0x50

typedef struct _IOCTL_HANDLERCONF_BUFFER_VerifyData_ // size - 0x24
{
/*0x000 */ char SeactionName[ IOCTL_HANDLERCONF_BUFFER_NAMELENGTH ] ;	// eg: "GlobalSetting"
/*0x004 */ char KeyName[ IOCTL_HANDLERCONF_BUFFER_NAMELENGTH ] ;		// eg: "OpenIpcPath"
/*0x008 */ IOCTL_HANDLERCONF_BUFFER_TYPE Flag ;		// 读/写标记
/*0x00C */ PVOID Data ;			// 保存R3查询到得内容

} IOCTL_HANDLERCONF_BUFFER_VerifyData, *LPIOCTL_HANDLERCONF_BUFFER_VerifyData ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_ReceiveData_ // size - 0x04
{
/*0x000 */ ULONG NameLength ;
/*0x004 */ LPSTR szName ; // ansi格式

} IOCTL_HANDLERCONF_BUFFER_ReceiveData, *LPIOCTL_HANDLERCONF_BUFFER_ReceiveData ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_RELAOD_ // size - 0x04
{
/*0x000 */ ULONG Reserved ;

} IOCTL_HANDLERCONF_BUFFER_RELAOD, *LPIOCTL_HANDLERCONF_BUFFER_RELAOD ;


//
// 关于Configuration的总结构体
//

typedef struct _IOCTL_HANDLERCONF_BUFFER_ // size - 0x24
{
	union 
	{
		IOCTL_HANDLERCONF_BUFFER_IniDataA		InitData ;
		IOCTL_HANDLERCONF_BUFFER_VerifyData		VerifyData  ; 
		IOCTL_HANDLERCONF_BUFFER_ReceiveData	ReceiveData ;
		IOCTL_HANDLERCONF_BUFFER_RELAOD			ReloadData	;
	} u ;

} IOCTL_HANDLERCONF_BUFFER, *LPIOCTL_HANDLERCONF_BUFFER ;


//////////////////////////////////////////////////////////////////////////

//
// Inline Hook Shadow SSDT相关结构体
//

typedef struct _IOCTL_HOOKSHADOW_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // Shadow ssdt inline hook的开关
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKSHADOW_BUFFER, *LPIOCTL_HOOKSHADOW_BUFFER ;


//
// Object Hook 相关结构体
//

typedef struct _IOCTL_HOOKOBJECT_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // hook开关
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKOBJECT_BUFFER, *LPIOCTL_HOOKOBJECT_BUFFER ;


//
// ImageNotify Dll 相关结构体
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


typedef struct _IOCTL_GETINJECTSAVEAREA_BUFFER_ // size - 0x24
{
	ULONG* BeCoveredAddr ;	// [OUT] 保存被覆盖的PE内存块的起始地址
	ULONG* MaxLength ;		// [IN OUT] R3准备的缓冲区的大小,返回时保存实际的数据大小			
	PVOID lpData ;			// [OUT] R3的缓冲区,用来存放原始的PE内存块数据
	
} IOCTL_GETINJECTSAVEAREA_BUFFER, *LPIOCTL_GETINJECTSAVEAREA_BUFFER ;


typedef struct _IOCTL_WHITEORBLACK_BUFFER_ // size - 0x24
{
	ULONG Flag ;	 // [IN] 标记要匹配的链表(File/Reg/IPC ...)
	LPCWSTR szPath ; // [IN] 待检测的字符串指针
	ULONG PathLength ; // [IN] 待检测的字符串长度

	BOOL* bIsWhite ; // [OUT] 是否为白
	BOOL* bIsBlack ; // [OUT] 是否为黑

} IOCTL_WHITEORBLACK_BUFFER, *LPIOCTL_WHITEORBLACK_BUFFER ;


typedef struct _IOCTL_CREATEDIRORLINK_BUFFER_ // size - 0x24
{
	ULONG Reserved ;

	LPWSTR lpcFullPath ; // lpc的全路径
	ULONG lpcFullPathLength ;

	BOOL bFlag ; // 标记是否使用到下面的2个内容
	LPWSTR lpcFatherPath ; // lpc的父路径
	ULONG lpcFatherPathLength ;

} IOCTL_CREATEDIRORLINK_BUFFER, *LPIOCTL_CREATEDIRORLINK_BUFFER ;


typedef struct _IOCTL_DUPLICATEOBJECT_BUFFER_ // size - 0x14
{
	PHANDLE pTargetHandle ;
	HANDLE FuckedHandle ;
	HANDLE SourceHandle ;
	ACCESS_MASK DesiredAccess ;
	ULONG Options ;

} IOCTL_DUPLICATEOBJECT_BUFFER, *LPIOCTL_DUPLICATEOBJECT_BUFFER ;


typedef struct _IOCTL_GETFILENAME_BUFFER_ // size - 0xC
{
	HANDLE HandleValue ;
	ULONG BufferLength ;
	PVOID lpData ; // [OUT] R3的缓冲区,用来存放@HandleValue对应的对象完整路径

} IOCTL_GETFILENAME_BUFFER, *LPIOCTL_GETFILENAME_BUFFER ;


typedef enum _IOCTL_DISABLEFORCEPROCESS_FLAG_
{
	_FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForce_	 = 3, // 表示开启强制运行的功能
	_FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForce_	 = 5, // 表示关闭强制运行的功能
	_FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForceALL_ = 7, // 表示取消所有的强制运行
	_FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForceALL_  = 9, // 表示开启所有的强制运行

} IOCTL_DISABLEFORCEPROCESS_FLAG ;


typedef struct _IOCTL_DISABLEFORCEPROCESS_BUFFER_ // size - 0x24
{
/*0x000 */ LPWSTR szProcName ; // 待取消的进程全路径
/*0x008 */ int NameLength ; // 待取消的进程全路径长度
/*0x00C */ int Flag ; // [开关] 可动态的开启/关闭 强制运行的功能

} IOCTL_DISABLEFORCEPROCESS_BUFFER, *LPIOCTL_DISABLEFORCEPROCESS_BUFFER ;



typedef struct _IOCTL_QUERYBOXPATH_BUFFER_ // size - 0x38
{
/*0x000 */ LPWSTR lpBoxName ; // [IN] 当前的沙箱名
/*0x004 */ ULONG BoxNamLength ;
/*0x008 */ LPWSTR lpBoxPath ; // [OUT] 保存当前沙箱名对应的全路径
/*0x00C */ ULONG BoxPathMaxLength ;

} IOCTL_QUERYBOXPATH_BUFFER, *LPIOCTL_QUERYBOXPATH_BUFFER ;


typedef struct _IOCTL_ENUMPROCESSEX_BUFFER_ // size - 0x08
{
/*0x000 */ LPWSTR lpBoxName ; // [IN] 当前的沙箱名
/*0x004 */ int* pArray ; // [OUT] 保存PID数组

} IOCTL_ENUMPROCESSEX_BUFFER, *LPIOCTL_ENUMPROCESSEX_BUFFER ;


typedef struct _IOCTL_QUERYPROCESSPATH_BUFFER_ // size - 0xC
{
/*0x000 */ ULONG PID ;

/*0x004 */ LPWSTR FileRootPath ; 
/*0x008 */ ULONG FileRootPathLength ;

} IOCTL_QUERYPROCESSPATH_BUFFER, *LPIOCTL_QUERYPROCESSPATH_BUFFER ;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


//
// 通信号IOCTL_PROTEINBOX_BUFFER对应的数据结构
//

typedef struct _IOCTL_PROTEINBOX_BUFFER_HEAD_ 
{
/*0x000 */ IOCTL_PROTEINBOX_FLAG Flag ; // 不同的值分别表示需要不同的函数处理当前请求
/*0x004 */ ULONG LittleIoctlCode ;
/*0x008 */ ULONG Reserved ;
	
} IOCTL_PROTEINBOX_BUFFER_HEAD ;


typedef struct _IOCTL_PROTEINBOX_BUFFER_ 
{
/*0x000 */ IOCTL_PROTEINBOX_BUFFER_HEAD Head ;
/*0x008 */ 
/*0x00C */ 
	union 
	{
		IOCTL_GETSYSVERSION_BUFFER GetSysVersion		;
		IOCTL_STARTPROCESS_BUFFER StartProcessBuffer	;
		IOCTL_QUERYPROCESS_BUFFER QueryProcessBuffer	;
		IOCTL_HANDLERCONF_BUFFER  ConfigBuffer			;
		IOCTL_HOOKSHADOW_BUFFER	  HookShadowBuffer		;
		IOCTL_HOOKOBJECT_BUFFER	  HookObjectBuffer		;
		IOCTL_GETINJECTSAVEAREA_BUFFER GetInjectSaveAreaBuffer ;
		IOCTL_WHITEORBLACK_BUFFER	WhiteOrBlackBuffer ;
		IOCTL_CREATEDIRORLINK_BUFFER CreateDirOrLinkBuffer ;
		IOCTL_DUPLICATEOBJECT_BUFFER DuplicateObjectBuffer ;
		IOCTL_GETFILENAME_BUFFER	 GetFileNameBuffer ;
		IOCTL_ENUMBOXS_BUFFER		 EnumBoxsBuffer ;
		IOCTL_DISABLEFORCEPROCESS_BUFFER DisableForceProcessBuffer ;
		IOCTL_QUERYBOXPATH_BUFFER QueryBoxPathBuffer ;
		IOCTL_ENUMPROCESSEX_BUFFER EnumProcessExBuffer ;
		IOCTL_QUERYPROCESSPATH_BUFFER QueryProcessPathBuffer ;

	};
	
} IOCTL_PROTEINBOX_BUFFER, *PIOCTL_PROTEINBOX_BUFFER ;

// 定义宏: 取得Ioctl Buffer的具体数据,即跳过数据头
#define getIoctlBufferBody( l )		( (ULONG)l + sizeof( IOCTL_PROTEINBOX_BUFFER_HEAD ) )


//////////////////////////////////////////////////////////////////////////
//
// rpc 相关
//
//////////////////////////////////////////////////////////////////////////


typedef enum _SBIESRV_APINUM_
{
	_PBSRV_APINUM_PStoreGetNode_			= 0x1101,
	_PBSRV_APINUM_PStoreGetChildNode_		= 0x1102,
	_PBSRV_APINUM_PStoreGetGrandchildNode_	= 0x1103,
	_PBSRV_APINUM_PStoreEnumSubtypes_		= 0x1104,
	_PBSRV_APINUM_PStoreEnumItems_			= 0x1105,

	_PBSRV_APINUM_StartPBDrv_	= 0x1201,
	_PBSRV_APINUM_KillProcess_  = 0x1203,
	_PBSRV_APINUM_TerminateBox_ = 0x1204,
	_PBSRV_APINUM_StopPBDrv_	= 0x1205, 

	_PBSRV_APINUM_StartService_			= 0x1301,
	_PBSRV_APINUM_EnumServiceStatus_	= 0x1302,
	_PBSRV_APINUM_GetSandboxedServices_ = 0x1303,
	_PBSRV_APINUM_StartBoxedService_	= 0x1304,

	_PBSRV_APINUM_SetupDiGetDevicePropertyW_			= 0x1401,
	_PBSRV_APINUM_CM_Get_Device_Interface_List_filter_	= 0x1402,
	_PBSRV_APINUM_CM_Get_Device_Interface_Alias_ExW_	= 0x1403,
	_PBSRV_APINUM_CM_Get_Device_Interface_Property_ExW_	= 0x1404,
	_PBSRV_APINUM_CM_Get_Class_Property_ExW_Win7_		= 0x1405,
	_PBSRV_APINUM_CM_Get_DevNode_Status_				= 0x1406,
	_PBSRV_APINUM_WinStaQueryInformationW_				= 0x1407,

	_PBSRV_APINUM_PipeCommunication_	= 0x1501,
	_PBSRV_APINUM_CloseLittleHandle_	= 0x1502,
	_PBSRV_APINUM_NtSetInformationFile_ = 0x1503,
	_PBSRV_APINUM_NtReadFile_			= 0x1504,
	_PBSRV_APINUM_NtWriteFile_			= 0x1505,
	_PBSRV_APINUM_ClearUpFile_			= 0x15FF,

	_PBSRV_APINUM_CryptUnprotectData_	= 0x1601,
	_PBSRV_APINUM_CryptProtectData_		= 0x1602,

	_PBSRV_APINUM_MarkFileTime_		= 0x1701,
	_PBSRV_APINUM_SetFileShortName_	= 0x1702,
	_PBSRV_APINUM_NtLoadKey_		= 0x1703,
	_PBSRV_APINUM_NtCloseWinsxs_	= 0x1901,
	_PBSRV_APINUM_NtFsControlFile_	= 0x1903,


	_PBSRV_APINUM_INIGetCurUserSection_ = 0x1801,
	_PBSRV_APINUM_GetAppVersion_		= 0x18AA,

	_PBSRV_APINUM_HandlerWhiteCLSID_	= 0x1B01,
	_PBSRV_APINUM_CoUnmarshalInterface_ = 0x1B06,
	_PBSRV_APINUM_CoMarshalInterface_	= 0x1B07,


} SBIESRV_APINUM ;


#define PB_PORT_MESSAGE_MAXLength	0x200

typedef struct _PB_PORT_MESSAGE_ 
{
/*0x000*/ PORT_MESSAGE Header ;
/*0x018*/ 
	union
	{
		UCHAR Buffer[ PB_PORT_MESSAGE_MAXLength ] ;
	};

} PB_PORT_MESSAGE, *LPPB_PORT_MESSAGE ;


typedef struct _RPC_IN_HEADER_ 
{
/*0x000 */  DWORD	DataLength ;
/*0x004 */  DWORD	Flag ;

} RPC_IN_HEADER, *PRPC_IN_HEADER ;


typedef struct _RPC_OUT_HEADER_ 
{
/*0x000 */  ULONG ReturnLength ;
/*0x004 */  
	union
	{
		NTSTATUS Status ;
		DWORD ErrorCode ;
	} u ;		

} RPC_OUT_HEADER, *LPRPC_OUT_HEADER ;


// RpcTerminateBox()
typedef struct _RPC_PROCTERMINATEBOX_INFO_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ DWORD PID ;
/*0x00C*/ WCHAR szBoxName[ 0x22 ];

} RPC_PROCTERMINATEBOX_INFO, *LPRPC_PROCTERMINATEBOX_INFO ;


// RpcKillProcess
typedef struct _RPC_PROCKILLPROCESS_INFO_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ DWORD PID ;

} RPC_PROCKILLPROCESS_INFO, *LPRPC_PROCKILLPROCESS_INFO ;


typedef struct _PB_BOX_INFO_  
{

	WCHAR BoxName[ MAX_PATH ] ;
	ULONG BoxNameLength ;

	WCHAR ProcessName[ MAX_PATH ] ;
	ULONG ProcessNameLength ;

	WCHAR SID[ MAX_PATH ] ;
	ULONG SIDLength ;

	ULONG SessionId ;
	WCHAR FileRootPath[ MAX_PATH ] ;	// eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
	ULONG FileRootPathLength ;

	WCHAR KeyRootPath[ MAX_PATH ] ;		// eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
	ULONG KeyRootPathLength ;

	WCHAR LpcRootPath[ MAX_PATH ] ;		// eg:"\Sandbox\AV\DefaultBox\Session_0"
	ULONG LpcRootPathLength ;

} PB_BOX_INFO, *LPPB_BOX_INFO ;



///////////////////////////////   END OF FILE   ///////////////////////////////