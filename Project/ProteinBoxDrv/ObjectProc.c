/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/12 [12:6:2010 - 16:04]
* MODULE : d:\Work\Program\Coding\沙箱\SandBox\Code\Project\ProteinBoxDrv\ObjectProc.c
* 
* Description:
*      
*   ObjectHook代理函数集中处理模块                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ObjectData.h"
#include "ObjectHook.h"
#include "ProcessData.h"
#include "Security.h"
#include "Version.h"
#include "Config.h"
#include "GrayList.h"
#include "SdtData.h"
#include "ObjectProc.h"


//////////////////////////////////////////////////////////////////////////

extern OBHOOKDATA g_OBHookData[ MaxObjectCounts ] ;


#define _get_ob_orignal_func( Index, Tag )		g_OBHookData[ Index - 1 ].Tag.OrignalAddr
#define get_ob_orignal_func( Index, Tag )		(IS_OBJECT_INDEX(Index) ? _get_ob_orignal_func( Index, Tag ) : (ULONG)0)


_ZwQueryInformationThread_ g_ZwQueryInformationThread_addr = NULL ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID 
fake_TokenOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( TOKEN_INDEX, Open ) ; 

	PreOpenHelper();

 	status = Reduce_TokenPrivilegeGroups( (PVOID)pNode, (HANDLE)Object, GrantedAccess );
 	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_TokenOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( TOKEN_INDEX, Open ) ; 

	PreOpenHelper();

	status = Reduce_TokenPrivilegeGroups( (PVOID)pNode, (HANDLE)Object, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ProcessOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( PROCESS_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPProcessFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ProcessOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( PROCESS_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPProcessFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ThreadOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( THREAD_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPThreadFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ThreadOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( THREAD_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPThreadFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_EventOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( EVENT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_EventOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( EVENT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_MutantOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( MUTANT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_MutantOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( MUTANT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SemaphoreOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( SEMAPHORE_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SemaphoreOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( SEMAPHORE_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SectionOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( SECTION_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SectionOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( SECTION_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_LpcPortOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	PVOID NewObject = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( LPCPORT_INDEX, Open ) ;

	PreOpenHelper();

	NewObject = GetFileObject( Object );
	if ( NULL == NewObject ){ goto _Call_OrignalFunc_ ; }

	status = ObjectOPFilter( (PVOID)pNode, NewObject, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_LpcPortOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	PVOID NewObject = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( LPCPORT_INDEX, Open ) ;

	PreOpenHelper();

	NewObject = GetFileObject( Object );
	if ( NULL == NewObject ){ goto _Call_OrignalFunc_ ; }

	status = ObjectOPFilter( (PVOID)pNode, NewObject, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



NTSTATUS 
fake_FileParse ( 
	IN PVOID DirectoryObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	IopParseFile_Context pBuffer ;
	DEVICE_TYPE DeviceType ;
	LPPDNODE pNode = NULL ;
	OB_PARSE_METHOD Parse = (OB_PARSE_METHOD) get_ob_orignal_func( FILE_INDEX, Parse ) ;

	if ( FALSE == CheckOpenPactInfo( (PVOID)&pBuffer, AccessMode, (ULONG)Context, AccessState ) )
	{
		goto _Call_OrignalFunc_ ; // 表明是内核态的调用,不用关心,走原始函数流程
	}

	PreParseHelper();

	DeviceType = ((PFILE_OBJECT)DirectoryObject)->DeviceObject->DeviceType ;
	status = ObjectPAFileDeviceFilter (
		RemainingName,
		DeviceType,
		(PVOID)pNode,
		DirectoryObject,
		(PVOID)&pBuffer
		);

	if( ! NT_SUCCESS( status ) ) { return status ; }

_Call_OrignalFunc_ :
	if ( Parse ) { status =  Parse( DirectoryObject, ObjectType, AccessState, AccessMode, Attributes, CompleteName, RemainingName, Context, SecurityQos, Object ); }
	return status ;
}



NTSTATUS 
fake_DeviceParse ( 
	IN PVOID DirectoryObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	IopParseFile_Context pBuffer ;
	DEVICE_TYPE DeviceType ;
	LPPDNODE pNode = NULL ;
	OB_PARSE_METHOD Parse = (OB_PARSE_METHOD) get_ob_orignal_func( DEVICE_INDEX, Parse ) ; 

	if ( FALSE == CheckOpenPactInfo( (PVOID)&pBuffer, AccessMode, (ULONG)Context, AccessState ) )
	{
		goto _Call_OrignalFunc_ ; // 表明是内核态的调用,不用关心,走原始函数流程
	}

	PreParseHelper();

	DeviceType = ((PDEVICE_OBJECT)DirectoryObject)->DeviceType ;
	status = ObjectPAFileDeviceFilter (
		RemainingName,
		DeviceType,
		(PVOID)pNode,
		DirectoryObject,
		(PVOID)&pBuffer
		);

	if( ! NT_SUCCESS( status ) ) { return status ; }

_Call_OrignalFunc_ :
	if ( Parse ) { status =  Parse( DirectoryObject, ObjectType, AccessState, AccessMode, Attributes, CompleteName, RemainingName, Context, SecurityQos, Object ); }
	return status ;
}



NTSTATUS
fake_CmpParseKey (
	IN PVOID ParseObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPPDNODE pNode = NULL ;
	OB_PARSE_METHOD Parse = (OB_PARSE_METHOD) get_ob_orignal_func( KEY_INDEX, Parse ) ; 

	if ( 0 == AccessMode ) { goto _Call_OrignalFunc_ ; }

	PreParseHelper();

	status = ObjectKeyFilter( (PVOID)pNode, Context, ParseObject, AccessState, RemainingName );
	if( ! NT_SUCCESS( status ) ) { return status;  }

_Call_OrignalFunc_ :
	if ( Parse ) { status =  Parse( ParseObject, ObjectType, AccessState, AccessMode, Attributes, CompleteName, RemainingName, Context, SecurityQos, Object ); }
	return status ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    Object终极Filter                       +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS 
ObjectOPProcessFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	)
{

	BOOL bRet = FALSE ;

	if ( ! OpenReason ) { return STATUS_SUCCESS ; }

	//
	// 1. 若操作的是沙箱中的进程,放行之
	//

	bRet = IsApprovedPID( _pNode, (ULONG)Object, GrantedAccess & 0xFFEDFBEF );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	//
	// 2. 排除特例: 对于操作沙箱外部进程的一个特例
	//

	bRet = __CaptureStackBackTrace( OpenReason, 2 );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	return STATUS_ACCESS_DENIED ;
}



NTSTATUS
ObjectOPThreadFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	)
{
	BOOL bRet = FALSE ;
	PVOID ProcessObj = NULL;

	if ( ! OpenReason ) { return STATUS_SUCCESS ; }

	//
	// 1. 若操作的是沙箱中的进程,放行之
	//

	ProcessObj = (PVOID)IoThreadToProcess( Object );
	bRet = IsApprovedPID( _pNode, (ULONG)Object, GrantedAccess & 0xFFEDFFB7 );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	//
	// 2. 排除特例: 对于操作沙箱外部进程的一个特例
	//

	bRet = __CaptureStackBackTrace( OpenReason, 2 );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	return STATUS_ACCESS_DENIED ;
}



NTSTATUS
ObjectOPFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/03 [3:8:2010 - 11:25]

Routine Description:
  通过文件对象@FileObject得到文件名,查看其是否在相应的黑白名单中. 若在黑名单中,直接禁止掉;
  若在白名单中,也要禁止掉对"\\KnownDlls\\*"下的对象的2个访问权限:
  #define DELETE                0x00010000
  #define SECTION_EXTEND_SIZE   0x0010

Return Value:
  放行 - STATUS_SUCCESS
  禁止 - STATUS_ACCESS_DENIED,STATUS_OBJECT_NAME_NOT_FOUND
    
--*/
{
	BOOL bRet = TRUE, bHasNoName = FALSE ;
	BOOL bIsSelfSession = FALSE ;
	BOOL bIsWhite = FALSE, bIsBlack =FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	UNICODE_STRING NameInfo = { 0 };
	ULONG NameInfoLength = 0 ;
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	// 1. 校验参数合法性
	if ( NULL == pNode )
	{
		OBTrace( "error! | ObjectOPFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_SUCCESS ;
	}

	// 2. 建立相应的黑白名单
	if ( FALSE == pNode->XIpcPath.bFlagInited )
	{
		bRet = BuildGrayList_for_OpenProcedure( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			OBTrace( "error! | ObjectOPFilter() - BuildGrayList_for_IopParse(); | 建立灰名单失败. \n" );
			pNode->bDiscard = 1 ;
			return STATUS_PROCESS_IS_TERMINATING;
		}
	}

	// 3. 得到对象名
// 	NameInfo.Buffer = NULL;
// 	status = QueryNameString( Object, &NameInfo, &NameInfoLength );
// 	if ( ! NT_SUCCESS(status) || _key_unknown_executable_image_for_QueryNameString_ == NameInfoLength ) { return status; }

 	ObjectNameInfo = SepQueryNameString( Object, &bHasNoName ); // 由调用者释放内存
 	if ( NULL == ObjectNameInfo )
 	{
 		if ( bHasNoName ) {
 			return STATUS_SUCCESS ;
 		} else {
 			return STATUS_UNSUCCESSFUL ;
 		}
 	}

	if ( MyRtlCompareUnicodeString( (LPName_Info)ObjectNameInfo, pNode->pNode_C->LpcRootPath1, pNode->pNode_C->LpcRootPath1Length) )
	{
		bIsSelfSession = TRUE ;
	}

	status = STATUS_SUCCESS;
	if ( FALSE == bIsSelfSession )
	{
		WCHAR		OutName[ MAX_PATH ]	= L"" ;
		PERESOURCE	QueueLockList = pNode->XIpcPath.pResource ;
		LPWSTR		lpCompareName = ObjectNameInfo->Name.Buffer ;

		// 过黑名单
		bIsBlack = GLFindNodeEx( &pNode->XIpcPath.ClosedIpcPathListHead, QueueLockList, lpCompareName, OutName );
		if ( bIsBlack ) // 在黑名单ClosedIpcPath中
		{
			status = STATUS_ACCESS_DENIED ; 

			if ( pNode->XIpcPath.bFlag_NotifyStartRunAccessDenied ) // 若阻止程序访问网络以后标记要通知用户,对于以下情况则不通知.
			{
				if ( L'*' == OutName[0] && ! OutName[1] )
				{
					pNode->XIpcPath.bFlag_NotifyStartRunAccessDenied = 0 ;
				}
			}
		}

		// 过白名单
		bIsWhite = GLFindNodeEx( &pNode->XIpcPath.OpenIpcPathListHead, QueueLockList, lpCompareName, OutName );
		if ( bIsWhite ) // 在白名单.OpenIpcPath中
		{
			if ( (L'\\' == OutName[0]) && (L'K' == OutName[1]) && (L'n' == OutName[2]) )
			{
				if ( 0 == _wcsicmp( OutName, L"\\KnownDlls\\*" ) )
				{
					ULONG DenneyAccess = SECTION_EXTEND_SIZE | DELETE ;

					if ( GrantedAccess & DenneyAccess )
					{
						status = STATUS_ACCESS_DENIED ;
					}

					if ( pNode->XIpcPath.bFlag_Denny_Access_KnownDllsSession )
					{
						status = STATUS_OBJECT_NAME_NOT_FOUND ;
					}
				}
			}
		}

		if ( bIsBlack || FALSE == bIsWhite )
			status = STATUS_ACCESS_DENIED;
	}

	if ( STATUS_ACCESS_DENIED == status )
	{
		if ( __CaptureStackBackTrace( OpenReason, 2 ) ) 
		{ 
			status = STATUS_SUCCESS; 
		}
		else
		{
			dprintf( "ko! | ObjectOPFilter() - __CaptureStackBackTrace() | 异常调用，拒绝掉 (ObjName=%ws) \n", ObjectNameInfo->Name.Buffer );
		}


	
		
		
		//
		// just fotr test,等R3的hook点都测验完了,这个再去掉.要不然好多东西都会被拒绝,跑起来费劲啊.
		//

	//	status = STATUS_SUCCESS; 
	}

//	kfree( NameInfo.Buffer );
	kfree( (PVOID)ObjectNameInfo );
	return status ;
}



NTSTATUS
ObjectPAFileDeviceFilter (
	IN OUT PUNICODE_STRING RemainingName,
	IN DEVICE_TYPE DeviceType,
	IN PVOID _pNode,
	IN PVOID DirectoryObject,
	IN PVOID pBuffer
	)
{
	BOOL bRet = FALSE ;
	BOOL bDeviceType_Is_Pipe_Mailslot = FALSE, bIs_DefaultBox_Self = FALSE ;
	BOOL bSuspiciousOption = FALSE ;
	BOOL bNeed2Free = FALSE ;
	LPWSTR pw = NULL, pTmp = NULL ;
	LPWSTR StartPtr = NULL ;
	LPWSTR lpCompareName = NULL ;
	WCHAR OutName[ MAX_PATH ]	= L"" ;
	ULONG StartIndex = 0 ;
	ULONG NameIndex = 0 ;
	ULONG Disposition, CreateOptions ; 
	UNICODE_STRING Name = { 0 };
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PERESOURCE QueueLockList = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;
	LPIopParseFile_Context Buffer = (LPIopParseFile_Context) pBuffer ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pNode || NULL == pBuffer )
	{
		OBTrace( "error! | ObjectPAFileDeviceFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_SUCCESS ;
	}

	//
	// 2. 建立相应的黑白名单
	//

	if ( FALSE == pNode->XFilePath.bFlagInited )
	{
		bRet = BuildGrayList_for_IopParse( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			OBTrace( "error! | ObjectPAFileDeviceFilter() - BuildGrayList_for_IopParse(); | 建立灰名单失败. \n" );
			pNode->bDiscard = 1 ;
			return STATUS_PROCESS_IS_TERMINATING;
		}
	}

	//  
	// 3. 只过滤特定的设备对象,其他的不管
	//

	if (   DeviceType != FILE_DEVICE_DISK
		&& DeviceType != FILE_DEVICE_NAMED_PIPE
		&& DeviceType != FILE_DEVICE_MAILSLOT
		&& DeviceType != FILE_DEVICE_NETWORK
		&& DeviceType != FILE_DEVICE_MULTI_UNC_PROVIDER
		&& DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM
		&& DeviceType != FILE_DEVICE_DFS 
		)
	{
		return STATUS_SUCCESS;
	}

	if ( NULL == RemainingName || 0 == RemainingName->Length ) { return STATUS_SUCCESS; }

	status = GetRemainingName( DirectoryObject, RemainingName, &Name ); // 成功则需要调用者释放内存
	if( ! NT_SUCCESS( status ) ) { return STATUS_SUCCESS ; }

	//  
	// 4. 处理对象类型为"管道(PIPE)"或者"油槽(MAILSLOT)"的情况
	//  

	if ( FILE_DEVICE_NAMED_PIPE == DeviceType || FILE_DEVICE_MAILSLOT == DeviceType )
	{
		bDeviceType_Is_Pipe_Mailslot = TRUE ;

		pw = Name.Buffer ;
		if ( _wcsnicmp( pw, L"\\Device\\", 8 ) )
			goto _Check_DesiredAccess_ ;

		pw += 8 ;
		if ( 0 == _wcsnicmp( pw, L"NamedPipe", 9) )
		{
			pTmp = pw + 9 ;
		}
		else if ( 0 == _wcsnicmp( pw, L"MailSlot", 8 ) )
		{
			pTmp = pw + 8 ;
		}
		else
		{
			goto _Check_DesiredAccess_ ;
		}

		if ( pTmp && *pTmp )
		{			
			if ( '\\' == *pTmp )
			{
				++ pTmp ;
				if ( 0 != _wcsnicmp( pTmp, pNode->pNode_C->LpcRootPath2, pNode->pNode_C->LpcRootPath2Length / sizeof(WCHAR) - 1 ) )
					goto _Check_DesiredAccess_;

				bIs_DefaultBox_Self = TRUE ;
		//		StartIndex = (ULONG) ((PCHAR)pTmp - (PCHAR)Name.Buffer) / sizeof(WCHAR)  ;
		//		StartPtr = &pTmp[ pNode->pNode_C->LpcRootPath2Length / sizeof(WCHAR) ] ;
			}
	
			pTmp =  NULL ;
		}
	}
	else
	{
		if ( 0 == _wcsnicmp( Name.Buffer, pNode->pNode_C->FileRootPath, pNode->pNode_C->FileRootPathLength / sizeof(WCHAR) - 1 ) )
		{
			bIs_DefaultBox_Self = TRUE ;
		}
	} 

	//  
	// 5. 权限过滤
	//  

_Check_DesiredAccess_ :

	if ( bDeviceType_Is_Pipe_Mailslot )
	{
		bSuspiciousOption = TRUE  ;     // 如果打开文件对象是Pipe | Mailslot,则标记为"有允许该操作的可能性,会在白名单中查看是否允许当前的越轨操作"
		
		if ( pTmp && !*pTmp && !(Buffer->OriginalDesiredAccess & 0x7FEDFF56) )
		{
			bSuspiciousOption = FALSE ;
		}
	}
	else
	{
		if ( Buffer->OriginalDesiredAccess & 0x7FEDFF56 )
			bSuspiciousOption = TRUE ; // 如果打开文件的方式DesiredAccess有问题,则标记为"有允许该操作的可能性,会在白名单中查看是否允许当前的越轨操作"
	}

	if ( Buffer->u.AccessMode )
	{
		// 对于用户态的情况
		if ( FILE_OPEN != Buffer->Disposition ) // 如果打开文件的方式不是"FILE_OPEN",则标记为"有允许该操作的可能性,会在白名单中查看是否允许当前的越轨操作" 
			bSuspiciousOption = TRUE ;                        

		CreateOptions = Buffer->CreateOptions;
		if ( Buffer->CreateOptions & FILE_DELETE_ON_CLOSE )  // 若打开文件的方式有"打开后关闭文件"的属性,则标记为"有允许该操作的可能性,会在白名单中查看是否允许当前的越轨操作"          
			bSuspiciousOption = TRUE ; 
	}
	else
	{        
		// 是KernelMode
		Disposition = 0xFFFFFFFF ;
		CreateOptions = 0xFFFFFFFF ;
	}

	 // 5.0 若对象名是沙箱本身"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
	status = STATUS_SUCCESS ;
	if ( bIs_DefaultBox_Self ) 
	{ 
		if ( !(Buffer->Options & 4) && (Buffer->OriginalDesiredAccess & 0x52000106) )
		{
			if ( !(CreateOptions & FILE_NON_DIRECTORY_FILE) || (CreateOptions & FILE_DIRECTORY_FILE) )
			{
				status = STATUS_ACCESS_DENIED ;
			}
		}

		goto _END_ ;
	}        

	QueueLockList = pNode->XFilePath.pResource ;

	// 5.1 处理网络路径
	lpCompareName = Name.Buffer ;

	if ( 0 == _wcsnicmp( lpCompareName, L"\\Device\\LanmanRedirector", 0x18 ) ) {
		NameIndex = 0x18 ;
	} else if ( 0 == _wcsnicmp( lpCompareName, L"\\Device\\Mup\\;LanmanRedirector", 0x1D ) ) {
		NameIndex = 0x1D ;
	}
	else {
		NameIndex = 0 ;
	}

	if ( NameIndex )
	{
		pw = &lpCompareName[ NameIndex ] ;

		if ( '\\' == lpCompareName[ NameIndex ] && pw[ 1 ] )
		{
			if ( ';' == pw[ 1 ] ) { pw = wcschr( pw + 2, '\\ '); }

			if ( pw && *pw && pw[ 1 ] )
			{
				ULONG l = wcslen( pw + 1 ) * sizeof( WCHAR ) + 0x26 ;
				LPWSTR NewName = (LPWSTR) kmalloc( l );

				if ( NULL == NewName )
				{
					status = STATUS_ACCESS_DENIED ;
				}
				else
				{
					l = (wcslen( L"\\Device\\Mup" ) + 1) * sizeof(WCHAR) ;
					
					wcscpy( NewName, L"\\Device\\Mup" );
					*(NewName + l) = L'\\';
					wcscpy( NewName + l + 1, pw + 1 );

					lpCompareName = NewName ;
					bNeed2Free =  TRUE ;
				}
			}
		}
	}

	// 5.2 第一步总是过黑名单 ClosedFilePath; 若当前行为是要操作"禁止访问的目录",拒绝掉
	bRet = GLFindNodeEx( &pNode->XFilePath.ClosedFilePathListHead, QueueLockList, lpCompareName, OutName );
	if ( bRet ) // 在黑名单ClosedFilePath中
	{
		if ( pNode->XFilePath.bFlag_NotifyInternetAccessDenied ) // 若阻止程序访问网络以后标记要通知用户,对于以下情况则不通知.
		{
			if ( 0 == wcscmp( OutName, L"\\Device\\Afd*") )
			{
				pNode->XFilePath.bFlag_NotifyInternetAccessDenied = 0 ;
			}
		}

		status = STATUS_ACCESS_DENIED ;
		goto _END_ ;
	}

	// 5.3 灰或白,若当前行为不可疑,直接方行
	if ( FALSE == bSuspiciousOption ) { goto _END_ ; }  
	
	// 5.4 行为可疑,过白名单 OpenFilePath;
	bRet = GLFindNodeEx( &pNode->XFilePath.OpenFilePathListHead, QueueLockList, Name.Buffer, OutName );
	if ( bRet ) // 在白名单OpenFilePath中
	{
		// 若同时又在黑名单ReadFilePath中,还得禁止掉;
		bRet = GLFindNodeEx( &pNode->XFilePath.ReadFilePathListHead, QueueLockList, Name.Buffer, OutName );
		if ( bRet ) { status = STATUS_ACCESS_DENIED ; }
	}
	else
	{
		// 5.5 灰名单,且行为可疑,直接禁止掉
		status = STATUS_ACCESS_DENIED ;
	}

_END_ :
	if ( bNeed2Free ) { kfree( (PVOID)lpCompareName ); }
	kfree( (PVOID)Name.Buffer );

	return status ;
}



BOOL g_Monitor_Key = FALSE ; // 必须等到R3的ProteinBox.dll完成注册表的重定向后,驱动中的fake_CmParseKey才能开启

NTSTATUS
ObjectKeyFilter (
	IN PVOID _pNode,
	IN PVOID Context,
	IN PVOID ParseObject,
	IN OUT PACCESS_STATE AccessState,
	IN OUT PUNICODE_STRING RemainingName
	)
{
	BOOL bRet = TRUE, bDangerousAccess = FALSE ;
	PERESOURCE QueueLockList = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	UNICODE_STRING Name = { 0 };
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	// 1. 校验参数合法性
	if( FALSE == g_Monitor_Key ) { return STATUS_SUCCESS; }

	if ( NULL == _pNode )
	{
		dprintf( "error! | ObjectKeyFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	// 2. 建立相应的黑白名单,建立失败则将该节点标记为"已遗弃",返回拒绝标志位
	if ( FALSE == pNode->XWnd.bFlagInited ) 
	{
		bRet = BuildGrayList_for_RegKey( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			dprintf( "error! | ObjectKeyFilter() - BuildGrayList_for_RegKey(); | 建立灰名单失败. \n" );
			pNode->bDiscard = 1 ;
			return STATUS_PROCESS_IS_TERMINATING ;
		}
	}

	// 3.获取对象名的完整路径.
	status = GetRemainingName( ParseObject, RemainingName, &Name ); // 成功则需要调用者释放内存
	if( ! NT_SUCCESS( status ) ) { return STATUS_ACCESS_DENIED ; }

	// 4. 放过自身目录下的RegKey操作
	if ( 0 == _wcsnicmp( Name.Buffer, pNode->pNode_C->KeyRootPath, pNode->pNode_C->KeyRootPathLength / sizeof(WCHAR) - 1 ) ) { status = STATUS_SUCCESS ; goto _CLEARUP_ ; }

	// 5. 黑名单不管什么权限访问,都予以阻止
	QueueLockList = pNode->XRegKey.pResource ;

	bRet = GLFindNodeEx( &pNode->XRegKey.DennyListHead, QueueLockList, Name.Buffer, NULL );
	if ( bRet ) { status = STATUS_ACCESS_DENIED ; goto _CLEARUP_ ; } // 在黑名单ClosedKeyPath中

	// 6.1 非黑只过滤特定的权限,若没有危险的权限,会放行掉
	bDangerousAccess = FALSE ;
	if ( AccessState->OriginalDesiredAccess & 0x7FEDFFE6 ) { bDangerousAccess = TRUE ; }

	if ( Context )
	{
		if ( g_Version_Info.IS___win2k || g_Version_Info.IS___xp || *(BYTE *)Context & 1 )
		{
			bDangerousAccess = TRUE ;
		}
	}

	if ( FALSE == bDangerousAccess ) { status = STATUS_SUCCESS ; goto _CLEARUP_ ; } // 没有危险的权限,直接放行

	// 6.2 有危险的权限,对于非白名单的操作,一律禁止
	bRet = GLFindNodeEx( &pNode->XRegKey.DirectListHead, QueueLockList, Name.Buffer, NULL );
	if ( FALSE == bRet ) { status = STATUS_ACCESS_DENIED ; goto _CLEARUP_ ; } 

	// 6.3 有危险的权限,即使在白名单中,也要禁止以对 @ReadKeyPath指定键值(Read Only RegKey)的访问
	bRet = GLFindNodeEx( &pNode->XRegKey.ReadOnlyListHead, QueueLockList, Name.Buffer, NULL );
	if ( bRet ) { 
		status = STATUS_ACCESS_DENIED ;  
	} else {
		status = STATUS_SUCCESS ;
	}

_CLEARUP_ :
	kfree( (PVOID)Name.Buffer );
	return status ;
}


BOOL
IsApprovedTID (
	IN PVOID _pNode,
	IN HANDLE TID,
	OUT HANDLE *PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/25 [25:8:2010 - 0:11]

Routine Description:
  调用ZwOpenThread,ZwQueryInformationThread得到@TID对应的PID,保存到@PID中,
  调用IsApprovedPIDEx()检查此PID是否是沙箱中的某个结点,外界进程不会在此结点链表中. 

Return Value:
  TRUE - 操作合法,允许之;
  FALSE - 当前PID不存在于链表中,说明是操作的是沙箱外的进程,需要禁止
    
--*/
{
	BOOL bRet = TRUE ;
	HANDLE hThread = NULL ;
	CLIENT_ID ClientId ;
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjectAttributes ;
	THREAD_BASIC_INFORMATION ThreadInfo ; 

	// 1. 校验参数合法性
	if ( NULL == _pNode || NULL == TID )
	{
		dprintf( "error! | IsApprovedTID(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2.获得线程句柄
	InitializeObjectAttributes( &ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );

	ClientId.UniqueProcess =  0 ;
	ClientId.UniqueThread  = TID ;

	status = ZwOpenThread (
		&hThread,
		THREAD_QUERY_INFORMATION,
		&ObjectAttributes,
		&ClientId
		);

	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | IsApprovedTID() - ZwOpenThread(); | status=0x8lx \n", status );
		return FALSE ;
	}

	// 3. 获取 ZwQueryInformationThread 的原始地址,调用之
	bRet = kgetaddrSDT( ZwQueryInformationThread );
	if ( FALSE == bRet )
	{ 
		dprintf( "error! | IsApprovedTID() - kgetaddrSDT(); | 无法获取ZwQueryInformationThread地址 \n" );
		return FALSE ; 
	}

	status = g_ZwQueryInformationThread_addr (
		hThread ,
		ThreadBasicInformation ,
		&ThreadInfo ,
		sizeof (ThreadInfo) ,
		NULL
		);

	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | IsApprovedTID() - ZwQueryInformationThread(); | status=0x8lx \n", status );
		ZwClose( hThread );
		return FALSE ;
	}

	// 4. 检查TID对应的PID是否属于沙箱
	if ( PID )
	{
		*PID = ThreadInfo.ClientId.UniqueProcess ;
	}

	bRet = IsApprovedPIDEx( _pNode, (ULONG)ThreadInfo.ClientId.UniqueProcess );
	
	ZwClose( hThread );
	return bRet ;
}



BOOL 
IsApprovedPID (
	IN PVOID _pNode,
	IN ULONG Process,
	IN ULONG GrantedAccess
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/17 [17:6:2010 - 17:13]

Routine Description:
  调用PsGetProcessId得到EPROCESS对应的PID,并调用Is_CurrentPID_match_Node()进行合法性检查
  若是沙箱外的进程,返回FALSE,表明是未经许可的操作,拒绝掉操作沙箱外进程的行为.  

Arguments:
  Process - 进程对象
  GrantedAccess - 相应的操作权限

--*/
{
	ULONG PID = 0, bRet = 0 ;

	if ( NULL == _pNode || 0 == Process || 0 == GrantedAccess )
	{
	//	dprintf( "error! | IsApprovedPID(); | Invalid Paramaters; failed! (Process=0x%08lx, GrantedAccess=%d) \n", Process, GrantedAccess );
		return TRUE ; // 默认放过,即返回TRUE,表明是许可的行为; 以免遇到意外情况系统被卡死
	}

	if ( g_PsGetProcessId_addr ) 
	{
		PID = (ULONG) g_PsGetProcessId_addr( (PEPROCESS)Process );
	}
	else
	{
		dprintf( "error! | IsApprovedPID(); | g_PsGetProcessId_addr 地址为空，导致获取PID失败，悲剧了。 \n" );
	}

	if ( 0 == PID )
	{
		dprintf( "error! | IsApprovedPID(); | 0 == PID \n" );
		return TRUE ;
	}
	
	if ( !GrantedAccess || IsApprovedPIDEx( _pNode, PID ) )
	{
		return TRUE ;// 操作的是沙箱中的程序,放行之
	}

//	dprintf( "当前程序正在操作沙箱外部的进程(PID=%d),已被拒绝掉. \n", PID );
	return FALSE;
}



BOOL
IsApprovedPIDEx (
	IN PVOID _pNode,
	IN ULONG PID
	)
{
	LPPDNODE pNodeVictim = NULL ;				// 受害者			
	LPPDNODE pNodeInvader = (LPPDNODE) _pNode ; // 入侵者

	// 1. 校验参数合法性
	if ( NULL == pNodeInvader )
	{
		dprintf( "error! | IsApprovedPIDEx(); | Invalid Paramaters; failed! \n" );
		return TRUE ; // 默认放过,即返回TRUE,表明是许可的行为; 以免遇到意外情况系统被卡死
	}

	// 2. 查找PID对应的进程总节点
	pNodeVictim = (LPPDNODE) kgetnodePD( PID );
	if ( NULL == pNodeVictim ) { return FALSE ; } // 若受害者不在沙箱内,不保护

	if ( pNodeVictim->bDiscard || pNodeVictim->bReserved1 ) { return FALSE ; } // 若受害者在沙箱内,但是被遗弃了,不保护

	if ( pNodeInvader->pNode_C->SessionId != pNodeVictim->pNode_C->SessionId ) { return FALSE ; } // 若都是沙箱内的进程,但用户ID不同,不保护

	if ( pNodeInvader->pNode_C->BoxNameLength != pNodeVictim->pNode_C->BoxNameLength ) { return FALSE ; } // 若不是同一个沙箱的进程,不保护
	if ( _wcsicmp( pNodeInvader->pNode_C->BoxName, pNodeVictim->pNode_C->BoxName ) ) { return FALSE ; } // 若不是同一个沙箱的进程,不保护

	return TRUE ; // 默认保护,表明是操作的沙箱中的程序,会放行当前操作
}



BOOL
__CaptureStackBackTrace (
	IN OB_OPEN_REASON OpenReason,
	IN ULONG __FramesToSkip
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/17 [17:6:2010 - 17:33]

Routine Description:
  这个函数很重要，通过栈回溯(指定N层),判定函数调用链是否合法。
  本工程中有3处调用此函数，都是从fake_xx函数经过2层栈进来的，所以@__FramesToSkip 为2。
  如果其他地方调用该函数，要自己清楚经过了几层栈才进来的，然后指定@__FramesToSkip
    
Arguments:
  OpenReason - 待过滤的参数
  __FramesToSkip - 需要跳过的堆栈层数

--*/
{
	ULONG Frames[ 32 ] ;
	ULONG Addr = 0 ;
	ULONG FramesToSkip = __FramesToSkip + 1 ;

	if ( TRUE == g_Version_Info.IS___win7 )
	{
		if ( 1 == RtlCaptureStackBackTrace( FramesToSkip + 2, 1, (PVOID*)Frames, NULL) )
		{
			Addr = Frames[ 0 ];
			if ( (Addr >= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointerWithTag_addr_vista) 
				&& (Addr <= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointerWithTag_addr_vista + 0xF0) 
				)
			{
				if ( 1 == RtlCaptureStackBackTrace( FramesToSkip + 3, 1, (PVOID*)Frames, NULL) )
				{
					Addr = Frames[ 0 ];
					if ( (Addr >= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointer_addr_vista) 
						&& (Addr <= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointer_addr_vista + 0x50) 
						)
					{
						++ FramesToSkip ;
					}
				}
			}
		}
	}

	if ( OpenReason && OpenReason != 1 )
	{
		if ( 2 != OpenReason ) { return FALSE ; }

		if ( 1 != RtlCaptureStackBackTrace( FramesToSkip + 2, 1, (PVOID*)Frames, NULL ) ) { return FALSE ; }
		
		Addr = Frames[ 0 ];
		if ( Addr > g_CaptureStackBackTrace_needed_data->KeFindConfigurationEntry_addr )
		{ 
			dprintf( "__CaptureStackBackTrace(); 匹配到合法地址,返回TRUE. (Addr=0x%08lx) \n", Addr );
			return TRUE ;
		}
	}
	else
	{
		if ( 1 == RtlCaptureStackBackTrace( FramesToSkip + 3, 1, (PVOID*)Frames, NULL ) ) 
		{ 
			Addr = Frames[ 0 ];
			if ( Addr > g_CaptureStackBackTrace_needed_data->KeFindConfigurationEntry_addr )
			{ 
				dprintf( "__CaptureStackBackTrace(); 匹配到合法地址,返回TRUE. (Addr=0x%08lx) \n", Addr );
				return TRUE ;
			}
		}

		if ( (TRUE == g_Version_Info.IS_before_vista) || (g_Version_Info.BuildNumber <= 0x1770) ) { return FALSE ; }

		if ( 1 != RtlCaptureStackBackTrace( FramesToSkip + 6, 1, (PVOID*)Frames, NULL ) ) { return FALSE ; }

		Addr = Frames[ 0 ];
		if ( Addr >= g_CaptureStackBackTrace_needed_data->PsCreateSystemThread_addr
			&& Addr <= g_CaptureStackBackTrace_needed_data->PsCreateSystemThread_addr + 0xC0
			)
		{ 
			dprintf( "__CaptureStackBackTrace(); 匹配到合法地址,返回TRUE. (Addr=0x%08lx) \n", Addr );
			return TRUE ;
		}
	}

	return FALSE ;
}



NTSTATUS
GetRemainingName (
	IN PVOID Object,
	IN PUNICODE_STRING RemainingName,
	OUT PUNICODE_STRING Name
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/22 [22:7:2010 - 15:46]

Routine Description:
  通过对象调用ObQueryNameString获取对象名,而后和@RemainingName拼接得到完整路径.
  成功则需要调用者释放内存, eg: kfree( (PVOID)Name.Buffer );
    
Arguments:
  Object - 待查询的对象体
  RemainingName - 待拼接的UNICODE字符串
  Name - 保存拼接后得到的全路径

Return Value:
  NTSTATUS
    
--*/
{
	BOOL bHasNoName = FALSE ;
	ULONG StartIndex = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;

	// 1. 校验参数合法性
	if ( NULL == Object || NULL == RemainingName || NULL == Name )
	{
		dprintf( "error! | GetRemainingName(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	// 2. 得到对象名
	ObjectNameInfo = SepQueryNameString( Object, &bHasNoName );
	if ( NULL == ObjectNameInfo )
	{
	//	dprintf( "error! | GetRemainingName() - SepQueryNameString(); | NULL == ObjectNameInfo \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	// 3. 拼接对象名 & RemainingName	
	Name->Length		= 0 ;
	Name->MaximumLength = ObjectNameInfo->Name.Length + 2 * sizeof(WCHAR) + RemainingName->Length + UNICODE_NULL ;
	Name->Buffer		= kmalloc( Name->MaximumLength );

	if ( NULL == Name->Buffer )
	{
		dprintf( "error! | GetRemainingName() - kmalloc(); | 申请内存失败. \n" );
		kfree( (PVOID)ObjectNameInfo );
		return STATUS_UNSUCCESSFUL ;
	}

	RtlZeroMemory( Name->Buffer, Name->MaximumLength );
	RtlAppendUnicodeStringToString( Name, &ObjectNameInfo->Name );

	StartIndex = Name->Length / sizeof(WCHAR);
	if ( '\\' != Name->Buffer[ StartIndex ] && '\\' != RemainingName->Buffer[ 0 ] )
	{
		RtlAppendUnicodeToString( Name, L"\\" );
	}

	if ( '\\' == Name->Buffer[ StartIndex ] && '\\' == RemainingName->Buffer[ 0 ] )
	{
		Name->Buffer[ StartIndex ] = UNICODE_NULL ;
		Name->Length -- ;
		Name->MaximumLength -- ;
	}

	RtlAppendUnicodeStringToString( Name, RemainingName );
	Name->Buffer[ Name->Length / sizeof(WCHAR) ] = L'\0' ;

	kfree( (PVOID)ObjectNameInfo );
	return STATUS_SUCCESS ;
}



BOOL
CheckOpenPactInfo (
	IN PVOID Buffer,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Context,
	IN PACCESS_STATE AccessState
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/12 [12:6:2010 - 17:42]

Routine Description:
  分析IopParseFile函数的参数Context & AccessState,从中取出有用信息,保存至于参数1 中:
  *(DWORD *)(pBuffer + 4)    = *(DWORD *)(Context + 0x34);   // Disposition
  *(DWORD *)(pBuffer + 8)    = *(DWORD *)(Context + 0x20);   // CreateOptions
  *(DWORD *)(pBuffer + 0xC)  = *(DWORD *)(Context + 0x30);   // Options
  *(DWORD *)(pBuffer + 0x10) = *(DWORD *)(_AccessState_ + 0x18);     // OriginalDesiredAccess

  kd> dd f73c190c l14
  f73c190c 00000100 00000001 00000000 00000101
  f73c191c 40000000

Arguments:
  pBuffer - 保存参数中的有用信息
  AccessMode - 一般为KernelMode
  Context - IopParseFile函数的参数8,结构体指针nt!_OPEN_PACKET
  AccessState - IopParseFile函数的参数3,结构体指针nt!_ACCESS_STATE

Return Value:
  #define IO_NO_PARAMETER_CHECKING 0x100
  参数Context合法 & 是内核态的调用(即AccessMode为KernelMode) & Context.Options 的第3位存在.
  这3个条件同时成立则返回成功;否则返回失败. 也就是说内核态的调用我们放行,只关心用户态中沙箱进程的调用.

--*/
{
	BOOL bIsUserMode = FALSE ;
	LPIopParseFile_Context pBuffer = (LPIopParseFile_Context) Buffer ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pBuffer )
	{
		OBTrace( "error! | CheckOpenPactInfo(); | Invalid Paramaters; 走原始函数流程  \n" );
		return FALSE ;
	}

	if ( Context && 8 == *(WORD *)Context )
	{
		pBuffer->u.AccessMode = 1;
		if ( FALSE == g_Version_Info.IS_before_vista )
			Context += 8 ;

		pBuffer->Disposition	= *(DWORD *)(Context + 0x34) ;
		pBuffer->CreateOptions	= *(DWORD *)(Context + 0x20) ;
		pBuffer->Options		= *(DWORD *)(Context + 0x30) ;
	}
	else
	{
		pBuffer->u.AccessMode	= 0 ;
		pBuffer->Disposition	= 0 ;
		pBuffer->CreateOptions	= 0 ;
		pBuffer->Options		= 0 ;
	}

	pBuffer->u.LimitLow = AccessMode ;
	pBuffer->OriginalDesiredAccess = AccessState->OriginalDesiredAccess ;

	bIsUserMode = AccessMode || pBuffer->u.AccessMode && pBuffer->Options & 4 ;
	return bIsUserMode ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////