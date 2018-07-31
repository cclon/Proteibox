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


#define GetAlpcFunc( X )	g_HookInfo.ALPC.pArray[ X##_TAG ].OrignalAddress


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef ULONG (WINAPI* _NtCreatePort_) (
	OUT PHANDLE PortHandle,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG MaxConnectInfoLength,
	IN ULONG MaxDataLength,
	IN ULONG MaxPoolUsage
	);

ULONG WINAPI
fake_NtCreatePort (
	OUT PHANDLE PortHandle,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG MaxConnectInfoLength,
	IN ULONG MaxDataLength,
	IN ULONG MaxPoolUsage
	);


typedef ULONG (WINAPI* _NtConnectPort_) (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE Qos,
	IN PPORT_VIEW ClientView,
	IN PREMOTE_PORT_VIEW ServerView,
	OUT PULONG MaxMessageLength,
	IN PVOID ConnectionInformation,
	OUT PULONG ConnectionInformationLength
	);

ULONG WINAPI
fake_NtConnectPort (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE Qos,
	IN PPORT_VIEW ClientView,
	IN PREMOTE_PORT_VIEW ServerView,
	OUT PULONG MaxMessageLength,
	IN PVOID ConnectionInformation,
	OUT PULONG ConnectionInformationLength
	);


typedef ULONG (WINAPI* _NtSecureConnectPort_) (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE Qos,
	IN OUT PPORT_VIEW ClientView OPTIONAL,
	IN PSID ServerSid OPTIONAL,
	IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
	OUT PULONG MaxMessageLength OPTIONAL,
	IN OUT PVOID ConnectionInformation OPTIONAL,
	IN OUT PULONG ConnectionInformationLength OPTIONAL
	);

ULONG WINAPI
fake_NtSecureConnectPort (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN PSECURITY_QUALITY_OF_SERVICE Qos,
	IN OUT PPORT_VIEW ClientView OPTIONAL,
	IN PSID ServerSid OPTIONAL,
	IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
	OUT PULONG MaxMessageLength OPTIONAL,
	IN OUT PVOID ConnectionInformation OPTIONAL,
	IN OUT PULONG ConnectionInformationLength OPTIONAL
	);


typedef ULONG (WINAPI* _NtAlpcCreatePort_) (
	OUT PHANDLE PortHandle,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN LPALPC_PORT_ATTRIBUTES PortAttributes
	);

ULONG WINAPI
fake_NtAlpcCreatePort (
	OUT PHANDLE PortHandle,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN LPALPC_PORT_ATTRIBUTES PortAttributes
	);


typedef ULONG (WINAPI* _NtAlpcConnectPort_) (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN LPALPC_PORT_ATTRIBUTES PortAttributes,
	IN ULONG Flags,
	IN PVOID RequiredServerSid, 
	IN PORT_MESSAGE *ConnectionMessage,
	IN PULONG BufferLength,
	OUT LPALPC_MESSAGE_ATTRIBUTES OutMessageAttributes,
	IN LPALPC_MESSAGE_ATTRIBUTES InMessageAttributes,
	IN PLARGE_INTEGER Timeout
	);

ULONG WINAPI
fake_NtAlpcConnectPort (
	OUT PHANDLE PortHandle,
	IN PUNICODE_STRING PortName,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN LPALPC_PORT_ATTRIBUTES PortAttributes,
	IN ULONG Flags,
	IN PVOID RequiredServerSid, 
	IN PORT_MESSAGE *ConnectionMessage,
	IN PULONG BufferLength,
	OUT LPALPC_MESSAGE_ATTRIBUTES OutMessageAttributes,
	IN LPALPC_MESSAGE_ATTRIBUTES InMessageAttributes,
	IN PLARGE_INTEGER Timeout
	);


typedef ULONG (WINAPI* _NtAlpcQueryInformation_) (
	OUT PHANDLE PortHandle,
	IN ALPC_PORT_INFORMATION_CLASS PortInformationClass, 
	OUT PVOID PortInformation, 
	IN ULONG Length, 
	OUT PULONG ReturnLength
	);

ULONG WINAPI
fake_NtAlpcQueryInformation (
	OUT PHANDLE PortHandle,
	IN ALPC_PORT_INFORMATION_CLASS PortInformationClass, 
	OUT PVOID PortInformation, 
	IN ULONG Length, 
	OUT PULONG ReturnLength
	);


typedef ULONG (WINAPI* _NtAlpcQueryInformationMessage_) (
	OUT PHANDLE PortHandle,
	IN PORT_MESSAGE *PortMessage, 
	IN ALPC_MESSAGE_INFORMATION_CLASS MessageInformationClass,
	OUT PVOID MessageInformation,
	IN ULONG Length, 
	OUT PULONG ReturnLength
	);

ULONG WINAPI
fake_NtAlpcQueryInformationMessage (
	OUT PHANDLE PortHandle,
	IN PORT_MESSAGE *PortMessage, 
	IN ALPC_MESSAGE_INFORMATION_CLASS MessageInformationClass,
	OUT PVOID MessageInformation,
	IN ULONG Length, 
	OUT PULONG ReturnLength
	);


typedef ULONG (WINAPI* _NtImpersonateClientOfPort_) (
	HANDLE PortHandle,
	PPORT_MESSAGE ClientMessage
	);

ULONG WINAPI
fake_NtImpersonateClientOfPort (
    HANDLE PortHandle,
    PPORT_MESSAGE ClientMessage
	);


typedef ULONG (WINAPI* _NtAlpcImpersonateClientOfPort_) (
	HANDLE PortHandle,
	PPORT_MESSAGE PortMessage,
	PVOID Reserved
	);

ULONG WINAPI
fake_NtAlpcImpersonateClientOfPort (
    HANDLE PortHandle,
    PPORT_MESSAGE PortMessage,
	PVOID Reserved
	);


typedef ULONG (WINAPI* _NtCreateEvent_) (
	OUT PHANDLE EventHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
	IN EVENT_TYPE EventType,
	IN BOOLEAN InitialState
	);

ULONG WINAPI
fake_NtCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );


typedef ULONG (WINAPI* _NtOpenEvent_) (
	OUT PHANDLE EventHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
	);

ULONG WINAPI
fake_NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


typedef ULONG (WINAPI* _NtCreateMutant_) (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    );

ULONG WINAPI
fake_NtCreateMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN BOOLEAN InitialOwner
    );


typedef ULONG (WINAPI* _NtOpenMutant_) (
	OUT PHANDLE MutantHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
	);

ULONG WINAPI
fake_NtOpenMutant (
    OUT PHANDLE MutantHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


typedef ULONG (WINAPI* _NtCreateSemaphore_) (
    IN PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    );

ULONG WINAPI
fake_NtCreateSemaphore (
    IN PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN LONG InitialCount,
    IN LONG MaximumCount
    );


typedef ULONG (WINAPI* _NtOpenSemaphore_) (
	OUT PHANDLE SemaphoreHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
	);

ULONG WINAPI
fake_NtOpenSemaphore (
    OUT PHANDLE SemaphoreHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


typedef ULONG (WINAPI* _NtCreateSection_) (
	OUT PHANDLE SectionHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
	IN PLARGE_INTEGER MaximumSize OPTIONAL,
	IN ULONG SectionPageProtection,
	IN ULONG AllocationAttributes,
	IN HANDLE FileHandle OPTIONAL
	);

ULONG WINAPI
fake_NtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );


typedef ULONG (WINAPI* _NtOpenSection_) (
	OUT PHANDLE SectionHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
	);

ULONG WINAPI
fake_NtOpenSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

///////////////////////////////   END OF FILE   ///////////////////////////////