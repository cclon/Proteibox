/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/28 [28:5:2010 - 14:45]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\Config.c
* 
* Description:
*      
*   操作配置文件的主模块                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "DispatchIoctl.h"
#include "Memory.h"
#include "ConfigThread.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////


#define g_CurrentBox_FullFilePathW	L"\\Device\\HarddiskVolume1\\ProteinBox\\SUDAMI\\DefaultBox"
#define g_CurrentBox_FullRegPathW	L"\\REGISTRY\\USER\\ProteinBox_SUDAMI_DefaultBox"
#define g_CurrentBox_FullIpcPathW	L"\\ProteinBox\\SUDAMI\\DefaultBox\\Session_0"

#define g_ForceProcessPathW	L"C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE"

LPIOCTL_HANDLERCONF_GLOBAL g_Ioctl_HandlerConf_GlobalData = NULL ;


BOOL g_ConfigData_init_ok = FALSE ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                和R3通信,操作配置文件的主函数              +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
InitConfig (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 10:17]

Routine Description:
  驱动初始化时读取配置文件,将其中的全部信息保存至内存,供后续读/写
    
--*/
{
	BOOL bRet = FALSE ;

	// 1. 初始化GrayList链表
	bRet = InitConfigData();
	if ( FALSE == bRet )
	{
		dprintf( "error! | InitConfig() - InitConfigData(); | \n" );
		return FALSE ;
	}

#if 0
	// 2. 初始化该被废弃的全局变量
	if ( NULL == g_Ioctl_HandlerConf_GlobalData )
	{
		g_Ioctl_HandlerConf_GlobalData = (LPIOCTL_HANDLERCONF_GLOBAL) kmallocMM( sizeof(IOCTL_HANDLERCONF_GLOBAL), MTAG___IOCTL_HANDLERCONF_GLOBAL ) ;
		if ( NULL == g_Ioctl_HandlerConf_GlobalData )
		{
			dprintf( "error! | InitConfig() - kmallocMM | NULL == g_Ioctl_HandlerConf_GlobalData,分配内存失败 \n" );
			return FALSE ;
		}
	}
#endif

	// 3. 创建一个线程来处理配置文件信息
	CreateConfigThread( FALSE );
	
	return TRUE ;
}



BOOL
LoadConfig (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/29 [29:6:2010 - 18:25]

Routine Description:
  激活R3的线程,让其做完事情,等待传下来的数据包

--*/
{
	BOOL bRet = FALSE ;
	BOOL bOK = FALSE ;
	HANDLE hEvent = NULL ;
	LARGE_INTEGER Timeout ;
	ULONG i = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	
	// 1. 设置事件1为TRUE,激活R3等待的线程,其便会将数据全部传下来
	bRet = SetEvent( g_ConfEvent_waitfor_r0, EVENT_MODIFY_STATE );
	if( FALSE == bRet )
	{
		dprintf( "error! | LoadConfig() - SetEvent(); | \"%s\" \n", g_ConfEvent_waitfor_r0 );
		return FALSE ;
	}

	// 2. 等待R3返回结果，只等一分钟; R3把配置文件的数据传完了，就会将该事件至TRUE，激活我们
	bRet = OpenEvent( g_ConfEvent_wakeup_r0, EVENT_ALL_ACCESS, &hEvent );
	if( FALSE == bRet )
	{
		dprintf( "error! | LoadConfig() - OpenEvent(); | \"%ws\" \n", g_ConfEvent_wakeup_r0 );
		return FALSE ;
	}

	dprintf( "LoadConfig() | 等待R3传数据Ing,等待的超时时间为1分钟 ... \n" );

	Timeout.QuadPart = -10 * 1000 * 1000 ;   // 1 second
	for ( i = 0; i < 60; i++ ) 
	{
		status = ZwWaitForSingleObject( hEvent, FALSE, &Timeout );
		if ( STATUS_TIMEOUT == status ) { continue ; }

		if( STATUS_SUCCESS == status )
		{
			dprintf( "LoadConfig() | 已等到R3返回的结果 \n" );
			bOK = TRUE ;
			break ;
		}
	}

	ZwResetEvent( hEvent, 0 ); // 不管等待结果怎样,都重新将该事件置假
	ZwClose( hEvent );

	if ( FALSE == bOK )
	{
		dprintf( "Ioctl_HandlerConf() | (R3返回结果) 等待超时.(status=0x%08lx) \n", status );
		return FALSE ;
	}

	g_ConfigData_init_ok = TRUE ;
	return TRUE ;
}



NTSTATUS
Ioctl_HandlerConf (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/28 [28:6:2010 - 18:04]

Routine Description:
  驱动中置事件1为真,让R3等待的线程激活,R3便发一个控制码下来,获取R0的数据,其中包含的信息可以是读写指定键值的黑/白名单信息. eg:

  [GlobalSetting]
  OpenFilePath=firefox.exe,%Favorites%
  OpenIpcPath=*\BaseNamedObjects*\MAPI-HP*,*\BaseNamedObjects*\OLKCRPC.OBJ=*
  sudam*,imadu*
  CloseIpcPath=

  以上为配置文件信息,先需知道[GlobalSetting]下OpenIpcPath对应的内容.此时R3会帮忙收集.
  通信期间需要一个等待事件,R0有请求时,会激活该等待事件,并发送IOCTL到R3,R3的线程被激活后
  便会处理R0的该请求,将R0需要的数据填充到Buffer中,并至另一个事件为真,让R0结束等待,接收数据
    
--*/
{
	BOOL bRet = FALSE ;
	PVOID pBody = NULL ;
	PIOCTL_PROTEINBOX_BUFFER pData = (PIOCTL_PROTEINBOX_BUFFER) pInBuffer ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_HandlerConf(); \n" );

	if ( NULL == pData )
	{
		dprintf( "error! | Ioctl_HandlerConf(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	pBody = (PVOID) getIoctlBufferBody( pInBuffer );
	if ( NULL == pBody )
	{
		dprintf( "error! | Ioctl_HandlerConf() - getIoctlBufferBody(); | NULL == pBody \n" );
		return STATUS_INVALID_ADDRESS ;
	}

	switch ( pData->Head.LittleIoctlCode )
	{
	case _Ioctl_Conf_function_InitData_ :
		{
		//	dprintf( "Ioctl_HandlerConf() | R3发送该Ioctl,将配置文件中的所有信息全部交给R0存储  \n" );
			bRet = Ioctl_HandlerConf_InitData( pBody );
		}
		break ;

	case _Ioctl_Conf_function_VerifyData_ : // (废弃)
		{
			dprintf( "Ioctl_HandlerConf() | R3发送该Ioctl,让R0填充Buffer,其中包含了读/写指定建的信息 \n" );
			bRet = Ioctl_HandlerConf_VerifyData( pBody );
		}
		break ;

	case _Ioctl_Conf_function_ReceiveData_ : // (废弃)
		{
			dprintf( "Ioctl_HandlerConf() | R3发送该Ioctl,将读取到得配置文件信息传递到R0中	 \n" );
			bRet = Ioctl_HandlerConf_ReceiveData( pBody );
		}
		break ;

	case _Ioctl_Conf_function_Reload_ :
		{
			dprintf( "Ioctl_HandlerConf() | Reload Config	 \n" );
			bRet = Ioctl_HandlerConf_Reload();
		}
		break ;

	default :
		dprintf( "error! | Ioctl_HandlerConf() | 不合法的请求 \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	if ( FALSE == bRet ) { return STATUS_UNSUCCESSFUL ; }
	
	return STATUS_SUCCESS ;
}



BOOL
Ioctl_HandlerConf_InitData (
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/02 [2:7:2010 - 17:02]

Routine Description:
  驱动启动时,会发送Ioctl告之R3,R3便将配置文件中的所有信息全部交给R0存储
    
--*/
{
	BOOL bRet = FALSE ;
	PVOID pNode = NULL ;
	WCHAR wszSeactionName[	MAX_PATH ] = L"" ;
	WCHAR wszKeyName[		MAX_PATH ] = L"" ;
	WCHAR wszValueName[		MAX_PATH ] = L"" ;
	IOCTL_HANDLERCONF_BUFFER_IniDataW DataW ;
	LPIOCTL_HANDLERCONF_BUFFER_IniDataA lpData = (LPIOCTL_HANDLERCONF_BUFFER_IniDataA) pInBuffer ;

	// 1. 校验参数合法性
	if ( NULL == lpData || NULL == lpData->SeactionName || NULL == lpData->KeyName || NULL == lpData->ValueName || 0 == lpData->ValueNameLength )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2.char --> wchar
	bRet = a2w( lpData->SeactionName, wszSeactionName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - a2w(); | lpData->SeactionName \n" );
		return FALSE ;
	}

	bRet = a2w( lpData->KeyName, wszKeyName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - a2w(); | lpData->KeyName \n" );
		return FALSE ;
	}

	bRet = a2w( lpData->ValueName, wszValueName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - a2w(); | lpData->ValueName \n" );
		return FALSE ;
	}
	
	DataW.SeactionName = wszSeactionName ;
	DataW.SeactionNameLength = ( wcslen(wszSeactionName) + 1 ) * sizeof(WCHAR) ; 

	DataW.KeyName = wszKeyName ;
	DataW.KeyNameLength = ( wcslen(wszKeyName) + 1 ) * sizeof(WCHAR) ;

	DataW.ValueName = wszValueName ;
	DataW.ValueNameLength = ( wcslen(wszValueName) + 1 ) * sizeof(WCHAR) ;


	// 3.建立对应节点
	bRet = CDBuildNode( (PVOID)g_ProteinBox_Conf_TranshipmentStation, &DataW );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - kBuildNodeCD(); | \n" );
		return FALSE ;
	}

	return TRUE ;
}



BOOL
Ioctl_HandlerConf_VerifyData (
	OUT PVOID pOutBuffer
	)
{	
	return TRUE ;
}



BOOL
Ioctl_HandlerConf_ReceiveData (
	IN PVOID pInBuffer
	)
{
	return TRUE ;
}



BOOL Ioctl_HandlerConf_Reload()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 14:34]

Routine Description:
  重新加载配置文件,即R3重新读取配置文件后,将所有更新后的数据抛给驱动
    
--*/
{
	// 初始化配置文件相关信息
	if ( FALSE == InitConfig() )
	{
		dprintf( "error! | Ioctl_HandlerConf_Reload() - InitConfig(); \n" );
		return FALSE;
	}

	// 此时驱动创建一个等待线程,等待R3激活,而后R3将配置数据抛给驱动,Reload成功
	return TRUE;
}



static const LPWSTR g_ConfContextAllowed_Array[ ] = // 配置文件中放行的字符串种类
{
	L"yes",
	L"y",
	L"ok",
	L"1",
};

static const LPWSTR g_ConfContextDenney_Array[ ] = // 配置文件中禁止的字符串种类
{
	L"no",
	L"n",
	L"0",
};


BOOL
GetConfigurationA (
	IN PCHAR KeyName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/28 [28:5:2010 - 16:23]

Routine Description:
  查询配置文件中对应@KeyName的状态;所有的BOOL相关配置都从这里查,eg:

  [AllowedOrDenney]
  OpenProtectedStorage=y
    
Arguments:
  Context - 配置文件中的项目

Return Value:
  BOOL
    
--*/
{
	BOOL bRet = TRUE ;
	WCHAR wszKeyName[MAX_PATH] = L"";
	
	if ( NULL == KeyName ) { return FALSE; }

	bRet = a2w( KeyName, wszKeyName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | GetConfigurationA() - a2w(); | KeyName=%s \n", KeyName );
		return FALSE ;
	}

	bRet = GetConfigurationW(wszKeyName);
	return bRet ;
}



BOOL
GetConfigurationW (
	IN LPWSTR KeyName
	)
{
	int Index = 0;
	BOOL bRet = FALSE;
	LPWSTR ptr = NULL;
	WCHAR Context[MAX_PATH] = L"";

	bRet = kGetConfSingle( L"AllowedOrDenney", KeyName, Context );
	if ( FALSE == bRet ) { return FALSE; }

	for ( Index = 0; Index < ARRAYSIZEOF(g_ConfContextAllowed_Array); Index++ )
	{
		ptr = g_ConfContextAllowed_Array[ Index ];
		if( 0 == _wcsnicmp(Context, ptr, wcslen(ptr)) )
		{
			return TRUE;
		}
	}

	return FALSE ;
}



BOOL
GetConfigurationSW (
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN ULONG MaxLength,
	OUT LPWSTR pOutBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 14:20]

Routine Description:
  查询配置文件中key保存的单个字符串信息
    
Arguments:
  SeactionName - eg: [GlobalSetting]
  KeyName - eg: ForceProcess
  MaxLength - 字符串的最大长度
  pOutBuffer - 保存查询到得单个字符串
    
--*/
{
	ULONG length = 0 ;
	LPPB_CONFIG_KEY pNode = NULL ;
	IOCTL_HANDLERCONF_BUFFER_IniDataW lpData ;

	// 1. 校验参数合法性
	if ( NULL == SeactionName || NULL == KeyName || NULL == pOutBuffer )
	{
		dprintf( "error! | GetConfigurationSW(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( FALSE == g_ConfigData_init_ok )
	{
		dprintf( "error!| GetConfigurationSW(); | 驱动中的无配置文件数据! \n" );
		return FALSE ;
	}

	// 2.查找之
	pNode = (LPPB_CONFIG_KEY) GetConfigurationMW( SeactionName, KeyName );
	if ( NULL == pNode )
	{
// 		dprintf( 
// 			"ko! | GetConfigurationSW() - GetConfigurationMW(); | 查询配置信息失败; Section:%ws, Key;%ws \n",
// 			SeactionName, KeyName
// 			);

		return FALSE ;
	}

	// 3. 已找到,拷贝之
	length = MaxLength < pNode->ValueListHead.NameLength ? MaxLength : pNode->ValueListHead.NameLength ;
	RtlCopyMemory( pOutBuffer, pNode->ValueListHead.ValueName, length );
	pOutBuffer[ length / sizeof(WCHAR) ] = UNICODE_NULL ;

	return TRUE ;
}



PVOID
GetConfigurationMW (
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 14:20]

Routine Description:
  查询配置文件中key保存的一系列字符串信息,返回字符串链表的表头(LPPB_CONFIG_KEY)
    
Arguments:
  SeactionName - eg: [GlobalSetting]
  KeyName - eg: OpenIpcPath
  
--*/
{
	BOOL bRet = FALSE ;
	SEARCH_INFO OutBuffer = { 0 };

	// 1. 校验参数合法性
	if ( NULL == SeactionName || NULL == KeyName )
	{
		dprintf( "error! | GetConfigurationSW(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	if ( FALSE == g_ConfigData_init_ok )
	{
		dprintf( "error!| GetConfigurationMW(); | 驱动中的无配置文件数据! \n" );
		return NULL ;
	}

	// 2.查找之
	bRet = kGetKeyNode( SeactionName, KeyName, &OutBuffer );
	if ( _NODE_IS_KEY_ != OutBuffer.NodeType || NULL == OutBuffer.pNode )
	{
#if 0
		dprintf( 
			"error! | GetConfigurationMW() - kGetKeyNode(); | 查询配置信息失败; Section:%ws, Key;%ws \n",
			SeactionName, KeyName
			);
#endif
		return NULL ;
	}

	return OutBuffer.pNode ;
}



BOOL
ConfMatch (
	IN LPWSTR lpBuffer,
	IN LPWSTR Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/10 [10:6:2010 - 17:06]

Routine Description:
  将@lpBuffer整理成链表; 在其中查找Tag,若匹配到则返回TRUE 
    
Arguments:
  lpBuffer - 配置文件中查询得到的内容,格式形如:C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE;C:\\WINDOWS\\NOTEPAD.EXE"; D:\\TestFolder
  Tag - 待匹配的字符串

Return Value:
  TRUE - 已匹配到;
    
--*/
{	
	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == lpBuffer || NULL == Tag )
	{
		dprintf( "error! | ConfMatch(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 将@lpBuffer整理成链表 
	//

	//
	// 3. 匹配之
	//

	if ( 0 == _wcsicmp( Tag, lpBuffer ) )
	{
		return TRUE ;
	}

	return FALSE ;
}



VOID
AbortWait_for_DriverUnload (
	)
{
	BOOL bRet = FALSE ;
	BOOL bOK = FALSE ;
	HANDLE hEvent = NULL ;

	bRet = OpenEvent( g_ConfEvent_wakeup_r0, EVENT_ALL_ACCESS, &hEvent );
	if ( FALSE == bRet ) { return ; }

	ZwSetEvent( hEvent, 0 ); // 将该事件置TRUE
	ZwClose( hEvent );

	return ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////