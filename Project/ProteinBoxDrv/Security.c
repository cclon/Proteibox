/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/17 [17:5:2010 - 14:50]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\Security.c
* 
* Description:
*      
*   操作Token等相关系统权限的主模块                         
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Security.h"
#include "Memory.h"
#include "Version.h"
#include "SdtData.h"
#include "ProcessData.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////////

_ZwCreateToken_				g_ZwCreateToken_addr			= NULL ;
_RtlAddAccessAllowedAceEx_	g_RtlAddAccessAllowedAceEx_addr = NULL ;

// 这2个描述符会供其他模块函数使用
PACL g_DefaultDacl_new = NULL ;
PSECURITY_DESCRIPTOR g_SecurityDescriptor_new = NULL ;


HANDLE	g_NewTokenHandle_system			= NULL	;
BOOL	g_bSepVariableInitialization_ok = FALSE ;

PSID g_SeAuthenticatedUsersSid = NULL, g_SeWorldSid = NULL ;

LUID SeChangeNotifyPrivilege;
LUID SeSystemtimePrivilege ;
LUID SeLoadDriverPrivilege;
LUID SeBackupPrivilege;
LUID SeRestorePrivilege;
LUID SeShutdownPrivilege;
LUID SeDebugPrivilege;

//
// LUID 黑名单数组
//

PLUID g_SepFilterPrivileges_Array [ ] =
{
	&SeSystemtimePrivilege,
	&SeLoadDriverPrivilege,
	&SeBackupPrivilege,
	&SeRestorePrivilege,
	&SeShutdownPrivilege,
	&SeDebugPrivilege,
	NULL
};


PRIVILEGE_BLACKLIST g_Privilege_BlackList = { 0, NULL };

PSID SeAliasAdminsSid		= NULL ;
PSID SeAliasPowerUsersSid	= NULL ;

//
// SID 黑名单数组
//

PSID g_SepFilterSid_Array [ ] =
{
	&SeAliasAdminsSid,
	&SeAliasPowerUsersSid,
	NULL
};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+					  降权的相关函数集群					  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
DropAdminRights (
	IN PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 11:07]

Routine Description:
  根据配置文件中的“BlockDrivers”“DropAdminRights”降低当前线程
  TokenPrivileges & TokenGroups的权限,调整ACL.而后调用ZwSetInformationProcess设置更新后的权限
    
--*/
{
	BOOL   bRet = FALSE ;
	ULONG  ProtectedProcess, pProtectedProcess, OldFlag, offset = 0 ;
	HANDLE hToken = NULL ;
	HANDLE hTokenNew = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PROCESS_ACCESS_TOKEN TokenInfo ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pNode )
	{
		dprintf( "error! | DropAdminRights(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 查询配置文件,决定是否降权
	//

	if ( FALSE == GetConfigurationA( "BlockDrivers" ) )
	{
		dprintf( "ok! DropAdminRights() - GetConfigurationA( \"BlockDrivers\" ) | 配置文件中指明无需禁止驱动加载,也无需降权 \n" );
		return TRUE ;
	}

	pNode->bDropAdminRights = (BYTE) GetConfigurationA( "DropAdminRights" );

	//
	// 3. 获取当前进程的Token句柄
	//

	status = ZwOpenProcessToken( NtCurrentProcess(), TOKEN_ALL_ACCESS, &hToken );
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | DropAdminRights() - ZwOpenProcessToken(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	//
	// 4. 根据旧的Token得到新的权限 - hTokenNew
	//

	hTokenNew = DropAdminRights_phrase1( hToken, pNode->bDropAdminRights );
	if ( NULL == hTokenNew )
	{
		dprintf( "error! | DropAdminRights() - DropAdminRights_phrase1(); | NULL == hTokenNew \n" );
		goto _END_ ;
	}

	if ( hTokenNew == hToken )
	{
		dprintf( "ok! | DropAdminRights() - DropAdminRights_phrase1(); | Token被操作后仍然没有变化:0x%08lx,成功返回 \n",hTokenNew );
		goto _OK_ ;
	}

	//
	// 5. 最后一步,为当前进程设置新的hTokenNew
	//

	if ( FALSE == g_Version_Info.IS_before_vista )
	{
		// 在Vista极其以后的系统,EPROCESS里新增一个控制进程权限的标志位:ProtectedProcess, 故在操作进程权限前,先屏蔽掉一些高权限方可操作
		if ( TRUE == g_Version_Info.IS___win7 )
		{
			if ( g_Version_Info.BuildNumber < 0x1DB0 )
				offset = g_Version_Info.BuildNumber < 0x1BBC ? 0x25C : 0x268 ;
			else
				offset = 0x26C ;
		}
		else
		{
			offset = 0x224 ; // vista
		}

		// 确保ProtectedProcess地址的有效性
		pProtectedProcess = (ULONG)IoGetCurrentProcess() + offset ;

		if ( FALSE == MmIsAddressValid( (PVOID)pProtectedProcess ) )
		{
			dprintf( "error! | DropAdminRights() - MmIsAddressValid( pProtectedProcess ); | \n" );
			goto _END_ ;
		}

		// 先屏蔽一些权限,使得我们可操作当前进程
		ProtectedProcess = *(PULONG) pProtectedProcess ;

		OldFlag = ProtectedProcess & ABOVE_NORMAL_PRIORITY_CLASS ; 
		*(PULONG) pProtectedProcess = ProtectedProcess & ~(ProtectedProcess & ABOVE_NORMAL_PRIORITY_CLASS) ; 
	}

	// Do it!
	TokenInfo.Token	 = hTokenNew ;
	TokenInfo.Thread = 0 ;

	status = ZwSetInformationProcess( 
		NtCurrentProcess(),
		ProcessAccessToken, 
		&TokenInfo,
		sizeof (PROCESS_ACCESS_TOKEN) 
		);

	// 还原被屏蔽的权限
	if ( FALSE == g_Version_Info.IS_before_vista ) { *(PULONG) pProtectedProcess |= OldFlag ; }

	//
	// 6. 清理工作
	//

	ZwClose( hTokenNew );

	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | DropAdminRights() - ZwSetInformationProcess(); | (status=0x%08lx) \n", status );
		goto _END_ ;
	}

_OK_ :
	bRet = TRUE ;
_END_ :
	ZwClose( hToken );
	return bRet ;
}



HANDLE
DropAdminRights_phrase1 (
	IN HANDLE TokenHandle, 
	IN BOOL bDropAdminRights
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 15:13]

Routine Description:
  降低当前线程 TokenPrivileges & TokenGroups 的权限,调整ACL    
    
Arguments:
  TokenHandle - 待操作的进程Token句柄
  bDropAdminRights - 是否降权

Return Value:
  降低权限后得到的TokenHandle
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	ULONG DefaultDacl_Length = 0 ;
	PACL  Dacl = NULL ;
	PSID  Sid  = NULL ;
	PTOKEN_STATISTICS		LocalStatistics		= NULL	;
	PTOKEN_USER				LocalUser			= NULL	;
	PTOKEN_GROUPS			LocalGroups			= NULL	;
	PTOKEN_PRIVILEGES		LocalPrivileges		= NULL	;
	PTOKEN_OWNER			LocalOwner			= NULL	;
	PTOKEN_PRIMARY_GROUP	LocalPrimaryGroup	= NULL	;
	PTOKEN_DEFAULT_DACL		LocalDefaultDacl	= NULL	;
	PTOKEN_DEFAULT_DACL		NewDefaultDacl		= NULL	;
	PTOKEN_SOURCE			LocalSource			= NULL	;
	OBJECT_ATTRIBUTES ObjectAttributes ; 
	SECURITY_QUALITY_OF_SERVICE SecurityQos ;

	//
	// 1. 根据@TokenHandle,查询其对应的各种信息
	//
	
	if (	FALSE == QueryInformationToken( TokenHandle, TokenStatistics, &LocalStatistics, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenUser, &LocalUser, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenGroups, &LocalGroups, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenPrivileges, &LocalPrivileges, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenOwner, &LocalOwner, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenPrimaryGroup, &LocalPrimaryGroup,0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenDefaultDacl, &LocalDefaultDacl, &DefaultDacl_Length )
		||	FALSE == QueryInformationToken( TokenHandle, TokenSource, &LocalSource, 0 )
		)
	{
		dprintf( "error! | DropAdminRights_phrase1() - QueryInformationToken(); | \n" );
		goto _CLEAR_UP_ ;
	}

	//
	// 2. 操作LocalPrivileges & LocalGroups 数组
	//

	bRet = DropAdminRights_phrase2( LocalPrivileges, LocalGroups, bDropAdminRights );
	if ( FALSE == bRet )
	{
		dprintf( "error! | DropAdminRights_phrase1() - DropAdminRights_phrase2(); | \n" );
		goto _CLEAR_UP_ ;
	}

	SecurityQos.Length					= sizeof( SecurityQos );
	SecurityQos.ImpersonationLevel		= LocalStatistics->ImpersonationLevel ;
	SecurityQos.ContextTrackingMode	= SECURITY_STATIC_TRACKING ;
	SecurityQos.EffectiveOnly			= FALSE ;

	ObjectAttributes.SecurityQualityOfService = &SecurityQos ;

	InitializeObjectAttributes (
		&ObjectAttributes,
		NULL,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

	//
	// 3. 获取 ZwCreateToken 的原始地址,调用之
	//

	bRet = kgetaddrSDT( ZwCreateToken );
	if ( FALSE == bRet )
	{
		dprintf( "error! | DropAdminRights_phrase1() - DropAdminRights_phrase2(); | 无法获取ZwCreateToken的地址 \n" );
		goto _CLEAR_UP_ ;
	}

	status = g_ZwCreateToken_addr(
		&TokenHandle,
		TOKEN_ALL_ACCESS,
		&ObjectAttributes,
		LocalStatistics->TokenType ,
		&LocalStatistics->AuthenticationId,
		&LocalStatistics->ExpirationTime,
		LocalUser,
		LocalGroups,
		LocalPrivileges,
		LocalOwner,
		LocalPrimaryGroup,
		LocalDefaultDacl,
		LocalSource
		);

	//
	// 3.1 对特殊情况进一步处理
	//

	if ( bDropAdminRights &&  STATUS_INVALID_OWNER == status )
	{
		// 构造一个新的ACL
		NewDefaultDacl = (PTOKEN_DEFAULT_DACL) kmallocMM( DefaultDacl_Length + 0x80, MTAG___NewDefaultDacl );
		if ( NULL == NewDefaultDacl )
		{
			dprintf( "error! | DropAdminRights_phrase1() - kmallocMM( MTAG___NewDefaultDacl ); | NULL == NewDefaultDacl \n" );
			goto _CLEAR_UP_ ;
		}

		memcpy( NewDefaultDacl, LocalDefaultDacl, DefaultDacl_Length );

		NewDefaultDacl->DefaultDacl = Dacl = (PACL)( (ULONG)NewDefaultDacl + 4 );
		NewDefaultDacl->DefaultDacl->AclSize += 0x80 ;
		Sid = LocalUser->User.Sid ;

		RtlAddAccessAllowedAce( Dacl, ACL_REVISION2, GENERIC_ALL, Sid );

		// 根据新的ACL再次创建Token
		status = g_ZwCreateToken_addr (
			&TokenHandle,
			TOKEN_ALL_ACCESS,
			&ObjectAttributes,
			LocalStatistics->TokenType ,
			&LocalStatistics->AuthenticationId,
			&LocalStatistics->ExpirationTime,
			LocalUser,
			LocalGroups,
			LocalPrivileges,
			(PTOKEN_OWNER)&Sid,
			LocalPrimaryGroup,
			NewDefaultDacl,
			LocalSource
			);

		if ( !NT_SUCCESS(status) )
		{
			dprintf( "error! | DropAdminRights_phrase1() - ZwCreateToken(); | 在STATUS_INVALID_OWNER == status的前提下再次调用ZwCreateToken()失败 - (status=0x%08lx) \n", status );
			goto _CLEAR_UP_ ;
		}

		SetSecurityObject( (HANDLE)0xFFFFFFFF, Dacl );
		SetSecurityObject( (HANDLE)0xFFFFFFFE, Dacl );
		SetSecurityObject( TokenHandle, Dacl		 );
	}

	if ( !NT_SUCCESS(status) )
	{			 
		dprintf( "error! | DropAdminRights_phrase1() - ZwCreateToken(); | (status=0x%08lx) \n", status );
	}

_CLEAR_UP_ :
	kfree( (PVOID)LocalStatistics );
	kfree( (PVOID)LocalUser );
	kfree( (PVOID)LocalGroups );
	kfree( (PVOID)LocalPrivileges );
	kfree( (PVOID)LocalOwner );
	kfree( (PVOID)LocalPrimaryGroup );
	kfree( (PVOID)LocalDefaultDacl );
	kfree( (PVOID)LocalSource );
	return TokenHandle ;
}



BOOL
DropAdminRights_phrase2 (
	IN PTOKEN_PRIVILEGES LocalPrivileges, 
	IN PTOKEN_GROUPS LocalGroups, 
	IN BOOL bDropAdminRights
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 16:15]

Routine Description:
  1. 对于TokenPrivileges的数组,若@bDropAdminRights为TRUE，则将该数组的nCounts降低为1，
  仅保留（0x17）SE_CHANGE_NOTIFY_PRIVILEGE权限；否则也要去掉以下权限：
  #define SE_SYSTEMTIME_PRIVILEGE           0x0c
  #define SE_LOAD_DRIVER_PRIVILEGE          0x10
  #define SE_BACKUP_PRIVILEGE               0x11
  #define SE_RESTORE_PRIVILEGE              0x12
  #define SE_SHUTDOWN_PRIVILEGE             0x13
  #define SE_DEBUG_PRIVILEGE                0x14

  2.对于TokenGroups的数组,若@bDropAdminRights为TRUE，若其中的SID如下所示则把当前SID抹去
  0x201,0x050000000,0x20,0x220 或者 0x201,0x050000000,0x20,0x223
  
  #define DOMAIN_ALIAS_RID_ADMINS        (0x00000220L)
  #define DOMAIN_ALIAS_RID_POWER_USERS   (0x00000223L)
    
Arguments:
  LocalPrivileges -
  LocalGroups -
  bDropAdminRights - 是否降权    
    
--*/
{
	BOOL  bIsWhite = FALSE ;
	ULONG BlackListPrivilegeCount = 0, PrivilegeCount = 0, RealPrivilegeCount = 0 ;
	ULONG BlackIndex = 0, PrivilegeIndex = 0 ; 
	ULONG GroupCount = 0, RealGroupCount = 0 ;
	ULONG GroupIndex = 0, BlackGroupIndex = 0 ;
	PSID_AND_ATTRIBUTES		Groups = NULL ;
	SID_AND_ATTRIBUTES		GroupsCurrent ;
	PLUID_AND_ATTRIBUTES	Privileges = NULL ;
	LUID_AND_ATTRIBUTES		PrivilegesCurrent ;
	LPPRIVILEGE_SET_EX		BlackListPrivilege = NULL ;
	PSID *pCurrentSID = NULL ;

	//
	// 1. 初始化各种必需的数据
	//

	if ( FALSE == SepVariableInitialization() )
	{
		dprintf( "error! | DropAdminRights_phrase2() - SepVariableInitialization(); | 初始化各种必需的数据失败! \n" );
		return FALSE ;
	}

	//
	// 2. 调整 TokenPrivileges 数组
	//

	Privileges		= LocalPrivileges->Privileges		;
	PrivilegeCount	= LocalPrivileges->PrivilegeCount	;

	BlackListPrivilege		= g_Privilege_BlackList.BlackListPrivilege	;
	BlackListPrivilegeCount	= BlackListPrivilege->PrivilegeCount		;

	if ( PrivilegeCount && BlackListPrivilegeCount )
	{
		//
		// 1.1 对当前数组中的每个单元进行排查,一旦发现是黑名单成员,立马清除掉
		//

		for ( PrivilegeIndex=0; PrivilegeIndex < PrivilegeCount; PrivilegeIndex++ )
		{
			bIsWhite = FALSE ; // 每次循环开始都默认置黑,认为一切都是邪恶的
			PrivilegesCurrent = Privileges[ PrivilegeIndex ] ;

			if ( bDropAdminRights )
			{
				// 1.1.1 若@bDropAdminRights为TRUE，则将该数组的nCounts降低为1，仅保留 SE_CHANGE_NOTIFY_PRIVILEGE 权限
				if ( RtlEqualLuid( &PrivilegesCurrent.Luid, &SeChangeNotifyPrivilege) ) 
				{
					bIsWhite = TRUE ; // 表明会保留该 SeChangeNotifyPrivilege 权限
				}
			}
			else
			{
				// 1.1.2 @bDropAdminRights为FALSE时，会在黑名单中查看是否有当前单元,有则移除掉
				do
				{
					if ( RtlEqualLuid( &BlackListPrivilege->Privilege[ BlackIndex ].Luid, &PrivilegesCurrent.Luid ) ) 
					{
						break ;
					}

					++ BlackIndex ; // 小循环的计数
				}
				while ( BlackIndex < BlackListPrivilegeCount );

				// 1.1.3 当笑循环结束时,遍历黑名单数组结束都未找到当前单元,表明当前单元不在黑名单中,置白
				if ( BlackIndex >= BlackListPrivilegeCount ) 
				{
					bIsWhite = TRUE ;
				}
			}

			if ( TRUE == bIsWhite ) { continue ; }

			//
			// 1.2 去掉当前的Privileges单元,因为它在黑名单中
			//
			
			RealPrivilegeCount = LocalPrivileges->PrivilegeCount - 1 ;
			if ( RealPrivilegeCount )
			{
				// 1.2.1 用数组中最后一个单元 替换掉 当前位置的黑单元
				PrivilegesCurrent.Luid		 = LocalPrivileges->Privileges[ RealPrivilegeCount ].Luid		;
				PrivilegesCurrent.Attributes = LocalPrivileges->Privileges[ RealPrivilegeCount ].Attributes	;
			}

			LocalPrivileges->PrivilegeCount = RealPrivilegeCount ;
		}
	}

	//
	// 2. 调整 TokenGroups 数组
	//

	GroupCount	= LocalGroups->GroupCount	;
	Groups		= LocalGroups->Groups		;

	if ( 0 == GroupCount ) { return TRUE; }
	
	for ( GroupIndex=0; GroupIndex < GroupCount; GroupIndex++ )
	{
		GroupsCurrent = Groups[ GroupIndex ] ;

		for( BlackGroupIndex = 0; BlackGroupIndex < ARRAYSIZEOF( g_SepFilterSid_Array ); ++ BlackGroupIndex )
		{
			pCurrentSID = g_SepFilterSid_Array[ BlackGroupIndex ] ;
			if ( NULL == pCurrentSID ) { break ; }

			if ( RtlEqualSid( GroupsCurrent.Sid, *pCurrentSID ) )
			{
				RealGroupCount = LocalGroups->GroupCount - 1 ;
				if ( RealGroupCount )
				{
					GroupsCurrent.Sid		 = LocalGroups->Groups[ RealGroupCount ].Sid		;
					GroupsCurrent.Attributes = LocalGroups->Groups[ RealGroupCount ].Attributes	;
				}

				LocalGroups->GroupCount = RealGroupCount;
			}
		}
	}

	return TRUE ;
}



BOOL
SepVariableInitialization(
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 15:15]

Routine Description:
  初始化工作,主要是得到LUID & SID黑名单列表,供后期过滤用
    
--*/
{
	ULONG Size = 0, i = 0 ;
	ULONG SidWithTwoSubAuthorities = 0 ;
	PLUID Temp = NULL ;
	LPPRIVILEGE_SET_EX BlackListPrivilege = NULL ;
	static SID_IDENTIFIER_AUTHORITY  SepNtAuthority = SECURITY_NT_AUTHORITY ;

	if ( TRUE == g_bSepVariableInitialization_ok ) { return TRUE ; }

	//
	// 1. 初始化各种LUID
	//

	SeChangeNotifyPrivilege		 = RtlConvertLongToLuid( SE_CHANGE_NOTIFY_PRIVILEGE		 );
	SeSystemtimePrivilege		 = RtlConvertLongToLuid( SE_SYSTEMTIME_PRIVILEGE		 );
	SeLoadDriverPrivilege		 = RtlConvertLongToLuid( SE_LOAD_DRIVER_PRIVILEGE		 );
	SeBackupPrivilege			 = RtlConvertLongToLuid( SE_BACKUP_PRIVILEGE			 );
	SeRestorePrivilege			 = RtlConvertLongToLuid( SE_RESTORE_PRIVILEGE			 );
	SeShutdownPrivilege			 = RtlConvertLongToLuid( SE_SHUTDOWN_PRIVILEGE			 );
	SeDebugPrivilege			 = RtlConvertLongToLuid( SE_DEBUG_PRIVILEGE				 );

	//
	// 2. 初始化 LUID 黑名单
	//

	g_Privilege_BlackList.BlackListPrivilegeSetSize = Size = sizeof( PRIVILEGE_SET_EX ) + 5 * (ULONG)sizeof( LUID_AND_ATTRIBUTES );

	g_Privilege_BlackList.BlackListPrivilege = BlackListPrivilege = (LPPRIVILEGE_SET_EX) kmallocMM( Size, MTAG___Privilege_BlackList );
	if ( NULL == BlackListPrivilege )
	{
		dprintf( "error! | SepVariableInitialization() -kmallocMM(MTAG___Privilege_BlackList); | 申请内存失败 \n" );
		return FALSE ;
	}

	BlackListPrivilege->PrivilegeCount = 6 ;

	for ( i=0; i<6; i++ )
	{
		Temp = g_SepFilterPrivileges_Array[ i ] ;

		BlackListPrivilege->Privilege[ i ].Luid = *Temp ;
		BlackListPrivilege->Privilege[ i ].Attributes = 0 ;
	}

	//
	// 3. 初始化 SID 黑名单 
	//

	SidWithTwoSubAuthorities = RtlLengthRequiredSid( 2 );

	SeAliasAdminsSid	 = (PSID) kmallocMM( SidWithTwoSubAuthorities, MTAG___SeAliasAdminsSid		);
	SeAliasPowerUsersSid = (PSID) kmallocMM( SidWithTwoSubAuthorities, MTAG___SeAliasPowerUsersSid	);
	if ( NULL == SeAliasAdminsSid || NULL == SeAliasPowerUsersSid )
	{
		dprintf( "error! | SepVariableInitialization() -kmallocMM(MTAG___SeAliasAdminsSid); | 申请内存失败 \n" );
		return FALSE ;
	}

	RtlInitializeSid( SeAliasAdminsSid,     &SepNtAuthority, 2 );
	RtlInitializeSid( SeAliasPowerUsersSid, &SepNtAuthority, 2 );

	*(RtlSubAuthoritySid( SeAliasAdminsSid,     1 )) = DOMAIN_ALIAS_RID_ADMINS ;
	*(RtlSubAuthoritySid( SeAliasPowerUsersSid, 1 )) = DOMAIN_ALIAS_RID_POWER_USERS ;

	g_bSepVariableInitialization_ok = TRUE ;
	return TRUE ;
}



BOOL
QueryInformationToken(
	IN HANDLE TokenHandle,
	IN int TokenInformationClass,
	OUT PVOID* out_TokenInformation,
	OUT PULONG out_TokenInformationLength OPTIONAL
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 15:26]

Routine Description:
  对调用ZwQueryInformationToken函数的包装,务必由调用者释放内存

--*/
{
	ULONG ReturnLength = 0 ;
	PVOID TokenInformation = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == out_TokenInformation )
	{
		dprintf( "error! | QueryInformationToken(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2.计算所需内存大小,申请内存,获取信息
	//

	status = ZwQueryInformationToken (
		TokenHandle,
		(TOKEN_INFORMATION_CLASS)TokenInformationClass,
		0,
		0,
		&ReturnLength
		);

	if ( STATUS_BUFFER_TOO_SMALL != status ) 
	{ 
		dprintf( "error! | QueryInformationToken() - ZwQueryInformationToken(); | (status=0x%08lx), TokenInformationClass=%d \n", status, TokenInformationClass );
		return FALSE ;
	}

	TokenInformation = (PVOID) kmalloc( ReturnLength );
	if ( NULL == TokenInformation )
	{
		dprintf( "error! | QueryInformationToken() - kmallocMM(); | 申请内存失败,TokenInformationClass=%d \n", TokenInformationClass );
		return FALSE ;
	}

	status = ZwQueryInformationToken (
		TokenHandle,
		(TOKEN_INFORMATION_CLASS)TokenInformationClass,
		TokenInformation,
		ReturnLength,
		&ReturnLength
		);

	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | QueryInformationToken() - ZwQueryInformationToken(); | (status=0x%08lx),TokenInformationClass=%d \n", status, TokenInformationClass );
		return FALSE ;
	}

	*out_TokenInformation = (PVOID) TokenInformation ;
	if ( out_TokenInformationLength ) { *out_TokenInformationLength = (ULONG) ReturnLength ; }
	 
	return TRUE ;
}



NTSTATUS
SetSecurityObject (
	IN HANDLE Handle,
	IN PACL Dacl
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 11:46]

Routine Description:
  对调用ZwSetSecurityObject函数的包装
        
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL  ; 
	SECURITY_DESCRIPTOR SecurityDescriptor ;

	status = RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | SetSecurityObject() - RtlCreateSecurityDescriptor(); | (status=0x%08lx) \n", status );
		return status ;
	}

	status = RtlSetDaclSecurityDescriptor( &SecurityDescriptor, TRUE, Dacl, FALSE );
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | SetSecurityObject() - RtlSetDaclSecurityDescriptor(); | (status=0x%08lx) \n", status );
		return status ;
	}

	status = ZwSetSecurityObject( Handle, DACL_SECURITY_INFORMATION, &SecurityDescriptor );
	return status ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+           驱动初始化阶段创建安全描述符等函数集群          +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL CreateAcl()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 22:05]

Routine Description:
  创建一个安全描述符,供后期使用

Return Value:
  BOOL
    
--*/
{
	NTSTATUS			 status = STATUS_UNSUCCESSFUL ;
	ULONG				 AclSize			 = 0x80	 ;
	BOOL				 bRet				 = FALSE ;
	PACL				 pDefaultAcl		 = NULL	 ;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL  ;

	//
	// 1. 分配ACL结构体 & 初始化
	//

	g_DefaultDacl_new = pDefaultAcl = (PACL) kmallocMM( AclSize, MTAG___TokenInformation_DefaultDacl_New );
	if ( NULL == pDefaultAcl )
	{
		dprintf( "error! | CreateAcl() - kmallocMM(); NULL == pDefaultAcl \n" );
		return FALSE ;
	}

	if ( FALSE == InitializeAcl( pDefaultAcl, AclSize, ACL_REVISION ) )
	{
		dprintf( "error! | CreateAcl() - InitializeAcl() \n" );
		return FALSE ;
	}

	//
	// 2. 初始化SID
	//

	bRet = InitializeSid () ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | CreateAcl() - __InitializeSid() \n" );
		return FALSE ;
	}

	//
	// 3. 给ACL添加指定的权限
	//

	Call_RtlAddAccessAllowedAce( pDefaultAcl, g_SeAuthenticatedUsersSid );
	Call_RtlAddAccessAllowedAce( pDefaultAcl, g_SeWorldSid	);

	//
	// 4. 创建安全描述符 & 填充之
	//

	g_SecurityDescriptor_new = pSecurityDescriptor = (PSECURITY_DESCRIPTOR) kmallocMM( 0x40, MTAG___SecurityDescriptor );
	if ( NULL == pSecurityDescriptor )
	{
		dprintf( "error! | CreateAcl() - kmallocMM(); NULL == pSecurityDescriptor \n" );
		return FALSE ;
	}

	status = RtlCreateSecurityDescriptor( pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | CreateAcl() - RtlCreateSecurityDescriptor() | status == 0x%08lx \n", status );
		return FALSE;
	}
	
	status = RtlSetDaclSecurityDescriptor(
		pSecurityDescriptor,
		TRUE, 
		pDefaultAcl,
		FALSE
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | CreateAcl() - RtlSetDaclSecurityDescriptor() | status == 0x%08lx \n", status );
		return FALSE;
	}

	dprintf( "ok! | CreateAcl(); | 创建一个安全描述符\n" );
	return TRUE ;
}

//////////////////////////////////////////////////////////////////////////


BOOL
InitializeAcl (
	IN PACL		pAcl,
	IN DWORD	nAclLength,
	IN DWORD	dwAclRevision
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 14:56]

Routine Description:
  初始化ACL结构体

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	status = RtlCreateAcl ( pAcl, nAclLength, dwAclRevision );
	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | InitializeAcl() - RtlCreateAcl(); status == 0x%08lx \n", status );
		return FALSE;
	}

	return TRUE;
}



BOOL
__InitializeSid (
	IN PSID Sid,
	IN PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
	IN BYTE SubAuthorityCount,
	IN ULONG SubAuthority
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 21:54]

Routine Description:
  构造SID结构体

--*/
{
	PULONG SubAuthority__ = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	status = RtlInitializeSid( Sid, pIdentifierAuthority, SubAuthorityCount );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | InitializeSid() - RtlInitializeSid(); status == 0x%08lx \n", status );
		return FALSE;
	}

	if ( SubAuthority )
	{
		SubAuthority__ = RtlSubAuthoritySid( Sid, 0 );
		*SubAuthority__ = SubAuthority ;
	}

	return TRUE;
}



BOOL InitializeSid ()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 21:53]

Routine Description:
  初始化2个SID
    
--*/
{
	BOOL bRet = FALSE ;
	ULONG SidLength1 = 0 ;
	SID_IDENTIFIER_AUTHORITY SeNtSidAuthority	 = { SECURITY_NT_AUTHORITY } ; 
	SID_IDENTIFIER_AUTHORITY SeWorldSidAuthority = { SECURITY_WORLD_SID_AUTHORITY	 } ;

	SidLength1 = RtlLengthRequiredSid( 1 );
	g_SeAuthenticatedUsersSid	= (PSID) kmallocMM( SidLength1, MTAG___SeAuthenticatedUsersSid );
	g_SeWorldSid				= (PSID) kmallocMM( SidLength1, MTAG___SeWorldSid );
	
	if ( NULL == g_SeAuthenticatedUsersSid || NULL == g_SeWorldSid )
	{
		dprintf( "error! | InitializeSid() -kmallocMM(); | 申请内存失败 \n" );
		return FALSE ;
	}

	bRet = __InitializeSid( g_SeAuthenticatedUsersSid, &SeNtSidAuthority, 1, SECURITY_AUTHENTICATED_USER_RID );
	bRet = __InitializeSid( g_SeWorldSid,	&SeWorldSidAuthority, 1, 0 );

	return bRet ;
}



NTSTATUS
Call_RtlAddAccessAllowedAce (
	IN OUT PACL Acl,
	IN PSID Sid
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	UNICODE_STRING uniBuffer ;
	ULONG Flags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERITED_ACE ;

	if ( NULL == g_RtlAddAccessAllowedAceEx_addr )
	{
		RtlInitUnicodeString( &uniBuffer, L"RtlAddAccessAllowedAceEx" );
		g_RtlAddAccessAllowedAceEx_addr = ( _RtlAddAccessAllowedAceEx_ ) MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( g_RtlAddAccessAllowedAceEx_addr )
	{
		status = g_RtlAddAccessAllowedAceEx_addr ( 
			Acl,
			ACL_REVISION,
			Flags, // 0x13
			0x10000000,
			Sid
			);
	}
	else
	{
		status = RtlAddAccessAllowedAce( Acl, ACL_REVISION, 0x10000000, Sid );
	}

	return status ;
}

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL AdjustPrivilege ()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/18 [18:5:2010 - 20:58]

Routine Description:
  复制系统进程的句柄,赋予一定的权限,保存该句柄至全局变量中,供以后使用
    
--*/
{
	BOOL		bRet		= FALSE				;
	HANDLE		hProcess	= NULL				;
	HANDLE		TokenHandle	= NULL				;
	CLIENT_ID	ClientId	= { NULL, NULL }	;
	NTSTATUS	status		= STATUS_UNSUCCESSFUL ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	SECURITY_QUALITY_OF_SERVICE Qos ;
	TOKEN_PRIVILEGES NewState ;

	//
	// 1.打开系统进程,获得句柄
	//

	ClientId.UniqueThread	= 0;
	ClientId.UniqueProcess	= (HANDLE) 4 ;

	InitializeObjectAttributes(
		&ObjectAttributes,
		NULL,
		OBJ_KERNEL_HANDLE;,
		NULL,
		NULL
		);

	status = ZwOpenProcess( 
		&hProcess ,
		PROCESS_QUERY_INFORMATION ,
		&ObjectAttributes ,
		&ClientId 
		);

	if ( !NT_SUCCESS (status) )
	{
		ClientId.UniqueProcess	= (HANDLE) 8 ;

		status = ZwOpenProcess( 
			&hProcess ,
			PROCESS_QUERY_INFORMATION ,
			&ObjectAttributes ,
			&ClientId 
			);

		if ( !NT_SUCCESS (status) )
		{
			dprintf( "error! | AdjustPrivilege() - ZwOpenProcess() | status == 0x%08lx \n", status );
			return FALSE ;
		}
	}

	//
	// 2. 复制系统进程的句柄
	//

	status = ZwOpenProcessToken( hProcess, 0xF01FF, &TokenHandle );
	ZwClose( hProcess );

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | AdjustPrivilege() - ZwOpenProcessToken() | status == 0x%08lx \n", status );
		return FALSE ;
	}

	Qos.Length				= sizeof( SECURITY_QUALITY_OF_SERVICE ) ;
	Qos.ImpersonationLevel	= SecurityImpersonation ;
	Qos.ContextTrackingMode = 0 ;
	Qos.EffectiveOnly		= FALSE ;

	ObjectAttributes.SecurityQualityOfService = &Qos;

	status = ZwDuplicateToken (
		TokenHandle, 
		0, 
		&ObjectAttributes, 
		0,
		TokenImpersonation,
		&g_NewTokenHandle_system
		);

	ZwClose( TokenHandle );

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | AdjustPrivilege() - ZwDuplicateToken() | status == 0x%08lx \n", status );
		return FALSE ;
	}

	//
	// 3. 赋予克隆的句柄新的权限
	//

	NewState.PrivilegeCount = 3 ;
	NewState.Privileges[ 0 ].Luid.LowPart	= SE_RESTORE_PRIVILEGE ;
	NewState.Privileges[ 0 ].Luid.HighPart	= 0;
	NewState.Privileges[ 0 ].Attributes		= SE_PRIVILEGE_ENABLED ;

	status = ZwAdjustPrivilegesToken (
		g_NewTokenHandle_system ,
		FALSE ,
		&NewState,
		0 ,
		NULL ,
		NULL
		);

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | AdjustPrivilege() - ZwAdjustPrivilegesToken() | status == 0x%08lx \n", status );
		ZwClose( g_NewTokenHandle_system );
		g_NewTokenHandle_system = NULL ;
		return FALSE ;
	}

	dprintf( "ok! | AdjustPrivilege(); | 复制系统进程的句柄 \n" );
	return TRUE ;
}



NTSTATUS 
RtlGetUserSid(
	IN HANDLE ProcessHandle,
	OUT LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 15:22]

Routine Description:
  得到当前用户的SID    
    
Arguments:
  RegisterUserID - 保存得到的SID

--*/
{
	HANDLE TokenHandle;
	UCHAR Buffer[256];
	PSID_AND_ATTRIBUTES SidBuffer;
	ULONG ReturnLength;
	UNICODE_STRING SidString;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == RegisterUserID )
	{
		dprintf( "error! | RtlGetUserSid(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	//
	// 2. 打开进程或线程,获取对应的Token
	//

	if ( (HANDLE)0xFFFFFFFE != ProcessHandle )
	{
		status = ZwOpenProcessToken ( ProcessHandle, TOKEN_QUERY, &TokenHandle );
	}
	else
	{
		// Open the thread token
		status = ZwOpenThreadToken ( (HANDLE)0xFFFFFFFE, TOKEN_QUERY, TRUE, &TokenHandle );
		if ( ! NT_SUCCESS(status) )
		{
			// We failed, is it because we don't have a thread token? 
			if (status != STATUS_NO_TOKEN) 
			{
				dprintf( "error! | RtlGetUserSid() - ZwOpenThreadToken() | status = 0x%08lx \n", status );
				return status ;
			}

			// It is, so use the process token
			status = ZwOpenProcessToken ( NtCurrentProcess(), TOKEN_QUERY, &TokenHandle );
		}
	}

	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | RtlGetUserSid() - ZwOpenProcessToken() | status = 0x%08lx \n", status );
		return status ;
	}

	//
	// 3. 查询Token包含的具体信息
	//

	SidBuffer = (PSID_AND_ATTRIBUTES)Buffer ;
	status = ZwQueryInformationToken (
		TokenHandle,
		TokenUser,
		(PVOID)SidBuffer,
		sizeof(Buffer),
		&ReturnLength
		);

	// Close the handle and handle failure
	ZwClose( TokenHandle );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | RtlGetUserSid() - ZwQueryInformationToken() | status = 0x%08lx \n", status );
		return status ;
	}

	//
	// 4. 转换成SID格式
	//

	status = RtlConvertSidToUnicodeString( &SidString, SidBuffer[0].Sid, TRUE );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | RtlGetUserSid() - RtlConvertSidToUnicodeString() | status = 0x%08lx \n", status );
		return status ;
	}

	wcscpy( RegisterUserID, SidString.Buffer );
	RtlFreeUnicodeString( &SidString ); // 拷贝完后一定要失败内存

	return STATUS_SUCCESS ;
}



BOOL 
ProcessIdToSessionId(
	IN DWORD dwProcessId OPTIONAL,
	OUT PULONG pSessionId
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 17:00]

Routine Description:
  得到指定进程对应的SessionId
    
Arguments:
  dwProcessId - 指定的进程,为空则为自身进程
  pSessionId - 保存待获取的SessionId

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PROCESS_SESSION_INFORMATION SessionInformation;
	OBJECT_ATTRIBUTES ObjectAttributes;
	CLIENT_ID ClientId;
	HANDLE ProcessHandle;
	ULONG ReturnLength = 4 ; 

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pSessionId )
	{
		dprintf( "error! | ProcessIdToSessionId(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. 确定进程句柄
	//

	if ( 0 == dwProcessId )
	{
		ProcessHandle = NtCurrentProcess() ;
	}
	else
	{
		ClientId.UniqueProcess	= (HANDLE)dwProcessId ;
		ClientId.UniqueThread	= 0 ;

		InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

		status = ZwOpenProcess(
			&ProcessHandle,
			PROCESS_QUERY_INFORMATION,
			&ObjectAttributes,
			&ClientId
			);

		if( ! NT_SUCCESS(status) )
		{
			dprintf( "error! | ProcessIdToSessionId() - ZwOpenProcess() | status = 0x%08lx \n", status );
			return FALSE ;
		}
	}

	//
	// 3. 查询进程句柄对应的SessionId
	//

	status = ZwQueryInformationProcess(
		ProcessHandle,
		ProcessSessionInformation,
		&SessionInformation,
		sizeof(SessionInformation),
		&ReturnLength
		);

	if( NT_SUCCESS(status) )
	{
		*pSessionId = SessionInformation.SessionId;
		return TRUE;
	}

	return FALSE ;
}



NTSTATUS
Reduce_TokenPrivilegeGroups (
	IN PVOID _pNode,
	IN HANDLE Token,
	IN ACCESS_MASK GrantedAccess
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	PTOKEN_GROUPS		LocalGroups		= NULL ;
	PTOKEN_PRIVILEGES	LocalPrivileges = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == Token || NULL == pNode )
	{
		dprintf( "error! | Reduce_TokenPrivilegeGroups(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	//
	// 2. 降权
	//
	
	if ( GrantedAccess & 4 )
	{
		status = SeQueryInformationToken( Token, TokenGroups, &LocalGroups );
		if ( NT_SUCCESS(status) )
		{
			status = SeQueryInformationToken( Token, TokenPrivileges, &LocalPrivileges );
			if ( NT_SUCCESS(status) )
			{
				if ( TRUE == DropAdminRights_phrase2( LocalPrivileges, LocalGroups, pNode->bDropAdminRights ) ) 
				{
					status = STATUS_ACCESS_DENIED;
				}
			}
			else
			{	
				dprintf( "error! | Reduce_TokenPrivilegeGroups() - SeQueryInformationToken(); | (status=0x%08lx) \n", status );
			}
		} 
		else 
		{	
			dprintf( "error! | Reduce_TokenPrivilegeGroups() - SeQueryInformationToken(); | (status=0x%08lx) \n", status );
		}

		kfree( LocalGroups );
		kfree( LocalPrivileges );
	}

	return status ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////