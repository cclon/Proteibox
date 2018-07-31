#pragma once

//////////////////////////////////////////////////////////////////////////

#define _sanbox_owner_tag_	'adus'

#define _PB_Global_event_	L"Global\\PB_WorkEvent"


typedef PVOID (/*__stdcall*/ *_PROCAPINUMBERFUNC_)(PVOID, PRPC_IN_HEADER, PVOID);



//
// LPC Port Message
//

typedef struct _PORT_MESSAGE_HEADER // size - 0x18
{
	union
	{                                                                       
		struct
		{                                                                   
			/*0x000*/             short        DataLength;                                        
			/*0x002*/             short        TotalLength;                                       
		}s1;                                                                
		/*0x000*/         ULONG      Length;                                                
	}u1; 

	union
	{                                                                       
		struct
		{                                                                   
			/*0x004*/             short        Type;                                              
			/*0x006*/             short        DataInfoOffset;                                    
		}s2;                                                                
		/*0x004*/         ULONG      ZeroInit;                                              
	}u2; 

	union
	{                                                                       
		/*0x008*/         CLIENT_ID ClientId;
		/*0x008*/         double      DoNotUseThisField;                                     
	};    

	/*0x010*/     ULONG      MessageId;   

	union
	{                                                                       
		/*0x014*/         ULONG      ClientViewSize;                                        
		/*0x014*/         ULONG      CallbackId;                                            
	};  

} PORT_MESSAGE_HEADER, *PPORT_MESSAGE_HEADER;



//
//  PostProcRPC();
// 当RPC处理不成功时,统一返回此结构体,此时RPCBuffer里面只有一个头部和尾部(标记有_sanbox_owner_tag_和错误状态的尾部)
//

typedef struct _POSTPROCRPCINFOEX_ 
{
/*0x000*/ ULONG AlwaysZero ;
/*0x004*/ DWORD Tag ; // _sanbox_owner_tag_

} POSTPROCRPCINFOEX, *LPPOSTPROCRPCINFOEX ;

typedef struct _POSTPROCRPCINFO_ 
{
/*0x000*/ RPC_OUT_HEADER RpcOutHeader ;
/*0x008*/ POSTPROCRPCINFOEX Data ;

} POSTPROCRPCINFO, *LPPOSTPROCRPCINFO ;



#pragma pack(1)
typedef struct _SIGNALNODELITTLE_EX_ // size - 0x1C
{
/*0x000*/ LIST_ENTRY ListEntry ;
/*0x008*/ HANDLE UniqueThread; // 从RpcMsg->ClientId.UniqueThread 取得

/*0x00C*/ BYTE bFlag1 ;
/*0x00D*/ BYTE bProcessLpcRequestFoundNode ;
/*0x00E*/ BYTE bFlag3 ;
/*0x00F*/ BYTE bFlag4 ;

/*0x010*/ HANDLE PortHandle ; // 保存由NtAcceptConnectPort调用后得到的句柄
/*0x014*/ /*LPPOSTPROCRPCINFO*/ PPORT_MESSAGE pRpcData ;
/*0x018*/ /*LPPOSTPROCRPCINFO*/ PPORT_MESSAGE pRpcDataDummy ;

} SIGNALNODELITTLE_EX, *LPSIGNALNODELITTLE_EX ;
#pragma pack()


typedef struct _SIGNALNODELITTLE_  // size - 0x28
{
/*0x000*/ LIST_ENTRY ListEntry ;
/*0x008*/ HANDLE UniqueProcess ; // 从RpcMsg->ClientId.UniqueProcess 取得
/*0x00C*/ ULONG Reserved1 ;
/*0x010*/ FILETIME CreationTime ;
/*0x018*/ struct _SIGNALNODELITTLE_EX_* pFlink ; // 上个节点
/*0x01C*/ struct _SIGNALNODELITTLE_EX_* pBlink ; // 下个节点 
/*0x020*/ ULONG nCounts ;		// 结点总数
/*0x024*/ ULONG Reserved2 ;

} SIGNALNODELITTLE, *LPSIGNALNODELITTLE ;


typedef struct _SIGNALNODE_  // size - 0x18
{
/*0x000*/ LIST_ENTRY ListEntry ;
/*0x008*/ ULONG ApiNumTag ; // ApiNumber的起始值
/*0x00C*/ PVOID pBuffer ;
/*0x010*/ _PROCAPINUMBERFUNC_ Func ; // 处理在此ApiNumTag区域内的函数
/*0x014*/ BOOL bFlag ; // eg:1

} SIGNALNODE, *LPSIGNALNODE ;


typedef struct _TOTOAL_DATA_ // size - 0x54
{
/*0x000*/ struct _SIGNALNODE_* pFlink ; // 上个节点
/*0x004*/ struct _SIGNALNODE_* pBlink ; // 下个节点 
/*0x008*/ ULONG nCounts ;		// 结点总数

/*0x00C*/ struct _SIGNALNODELITTLE_* pFlinkL ; // 上个节点
/*0x010*/ struct _SIGNALNODELITTLE_* pBlinkL ; // 下个节点 
/*0x014*/ ULONG nCountsL ;		// 结点总数

/*0x018*/ RTL_CRITICAL_SECTION CriticalSection ; 
/*0x030*/ LPMMNODETLS TLSData ;
/*0x034*/ HANDLE hEvent ; // CreateEventW(0, 0, 1, L"Global\\Sandboxie_WorkEvent");
/*0x038*/ HANDLE hThread ;
/*0x03C*/ HANDLE PortHandle ;
/*0x040*/ HANDLE HandlesArray[ 4 ];
/*0x050*/ ULONG dwRefCount ;

} TOTOAL_DATA, *LPTOTOAL_DATA ;

//
// PBSrv.exe服务的RPC总结构体
//


typedef struct _PBSRV_LPC_RUQUEST_ 
{
	struct 
	{
/*0x000*/ BYTE BufferLength ;
/*0x001*/ BYTE bFlag1 ;
/*0x002*/ BYTE bFlag2 ;
/*0x003*/ BYTE bFlag3 ;
	} s1 ;

/*0x004*/ ULONG ApiNumber ;
/*0x00C*/ UCHAR Buffer[] ;

} PBSRV_LPC_RUQUEST, *LPPBSRV_LPC_RUQUEST ;


typedef struct _PBSRV_LPC_PORT_CLOSED_ 
{
/*0x000*/ FILETIME CreationTime ;

} PBSRV_LPC_PORT_CLOSED, *LPPBSRV_LPC_PORT_CLOSED ;


typedef struct _PBSRV_API_MESSAGE
{
/*0x000*/ PORT_MESSAGE Header;
/*0x018*/ 
	union
	{
		PBSRV_LPC_RUQUEST LpcRequstBuffer ;
		PBSRV_LPC_PORT_CLOSED LpcPortClosedBuffer ;

	} Data;

} PBSRV_API_MESSAGE, *LPPBSRV_API_MESSAGE ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


//////////////////////////////////////////////////////////////////////////

class CGlobal
{
public:
	CGlobal(void);
	~CGlobal(void);

	LPTOTOAL_DATA GetTotalData();
	VOID DestroyTotalNode( IN LPTOTOAL_DATA pNode );

protected:

private:
	LPTOTOAL_DATA m_TotalData ;
};

extern CGlobal g_CGlobal ;

//////////////////////////////////////////////////////////////////////////

VOID InsertList( LPLIST_ENTRY_EX pHead, PLIST_ENTRY pOld, PLIST_ENTRY pNew );



///////////////////////////////   END OF FILE   ///////////////////////////////