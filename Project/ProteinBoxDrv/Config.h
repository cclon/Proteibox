#pragma once

#include "ConfigData.h"

//////////////////////////////////////////////////////////////////////////


#define g_ConfEvent_waitfor_r0	 L"\\BaseNamedObjects\\Global\\Proteinbox_ConfEvent_waitfor_r0"
#define g_ConfEvent_wakeup_r0	 L"\\BaseNamedObjects\\Global\\Proteinbox_ConfEvent_wakeup_r0"

#define kGetConfSingle( SeactionName, KeyName, pOutBuffer )		GetConfigurationSW( SeactionName, KeyName, MAX_PATH, pOutBuffer )
#define kGetConfMulti( SeactionName, KeyName )					GetConfigurationMW( SeactionName, KeyName )


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef enum _IOCTL_HANDLERCONF_FUNCTION_TYPE_ 
{
	_Ioctl_Conf_function_InitData_		= 0x1001, // R3发送该Ioctl,将配置文件中的所有信息全部交给R0存储 
	_Ioctl_Conf_function_VerifyData_	= 0x1002, // R3发送该Ioctl,让R0填充Buffer,其中包含了读/写指定建的信息 (废弃)
	_Ioctl_Conf_function_ReceiveData_	= 0x1003, // R3发送该Ioctl,将读取到的配置文件信息传递到R0中	(废弃) 
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


typedef struct _IOCTL_HANDLERCONF_BUFFER_ReceiveData_ // size - 0x08
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
		IOCTL_HANDLERCONF_BUFFER_IniDataA		InitData	;
		IOCTL_HANDLERCONF_BUFFER_VerifyData		VerifyData  ; 
		IOCTL_HANDLERCONF_BUFFER_ReceiveData	ReceiveData ;
		IOCTL_HANDLERCONF_BUFFER_RELAOD			ReloadData	;
	} u ;

} IOCTL_HANDLERCONF_BUFFER, *LPIOCTL_HANDLERCONF_BUFFER ;



typedef struct _IOCTL_HANDLERCONF_GLOBAL_ 
{
/*0x000 */ IOCTL_HANDLERCONF_BUFFER_VerifyData VerifyDataCurrent ;
/*0x004 */ PB_CONFIG_COMMON ValueListHead ;

} IOCTL_HANDLERCONF_GLOBAL, *LPIOCTL_HANDLERCONF_GLOBAL ;

extern LPIOCTL_HANDLERCONF_GLOBAL g_Ioctl_HandlerConf_GlobalData ;



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
InitConfig (
	);

NTSTATUS
Ioctl_HandlerConf (
	IN PVOID pNode,
	IN PVOID pInBuffer
	);
BOOL
Ioctl_HandlerConf_InitData (
	IN PVOID pInBuffer
	);

BOOL
Ioctl_HandlerConf_VerifyData (
	OUT PVOID pOutBuffer
	);

BOOL
Ioctl_HandlerConf_ReceiveData (
	IN PVOID pInBuffer
	);

BOOL Ioctl_HandlerConf_Reload();

BOOL
LoadConfig (
	);

BOOL
GetConfigurationA (
	IN PCHAR KeyName
	);

BOOL
GetConfigurationW (
	IN LPWSTR KeyName
	);

BOOL
GetConfigurationSA (
	IN PCHAR SeactionName,
	IN PCHAR KeyName,
	OUT LPSTR pOutBuffer
	);

BOOL
GetConfigurationSW (
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN ULONG MaxLength,
	OUT LPWSTR pOutBuffer
	);

PVOID
GetConfigurationMW (
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName
	);

BOOL
ConfMatch (
	IN LPWSTR lpBuffer,
	IN LPWSTR Tag
	);

VOID
AbortWait_for_DriverUnload (
	);


///////////////////////////////   END OF FILE   ///////////////////////////////