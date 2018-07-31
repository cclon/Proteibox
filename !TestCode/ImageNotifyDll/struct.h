/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2008/08/13 
* MODULE : struct.h               
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#pragma once

#include <ntddk.h> 
#include <windef.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////


#define dprintf if (DBG) DbgPrint
#define nprintf DbgPrint


#define __x_Debug__  1

#if DBG
#define DBGPRINT(_o_)       \
        if(__x_Debug__)     \
        {                   \
            DbgPrint _o_;   \
        }
#else
#define DBGPRINT(_o_)
#endif


#define kmalloc(_s)	ExAllocatePoolWithTag(NonPagedPool, _s, 'ADUS')
#define kfree(_p)	ExFreePool(_p)


#define  SPIN_LOCK_INITIAL(_m_)       KeInitializeSpinLock(&_m_)
#define  SPIN_LOCK_WAITFOR(_m_, _i_)  KeAcquireSpinLock(&_m_, &_i_)
#define  SPIN_LOCK_RELEASE(_m_, _i_)  KeReleaseSpinLock(&_m_,  _i_)


#define MAX_PATH 260
#define ARRAYSIZEOF(x)	sizeof (x) / sizeof (x[0])


ULONG g_CR0 ;

#define Writable()                    \
    __asm { push  eax             }    \
    __asm { mov   eax, cr0        }    \
    __asm { mov   g_CR0, eax      }    \
    __asm { and   eax, 0FFFEFFFFh }    \
    __asm { mov   cr0, eax        }    \
    __asm { pop   eax             }

#define UnWritable()                  \
    __asm { push  eax             }    \
    __asm { mov   eax, g_CR0      }    \
    __asm { mov   cr0, eax        }    \
    __asm { pop   eax             }


#define LongAlign(P) (                \
    ((((ULONG)(P)) + 3) & 0xfffffffc) \
)

#define WordAlign(P) (                \
    ((((ULONG)(P)) + 1) & 0xfffffffe) \
)

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

#define PtrOffset(B,O) ((ULONG)((ULONG)(O) - (ULONG)(B)))

#define SetFlag(F,SF) { \
    (F) |= (SF);        \
}

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)


///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////



NTSTATUS
ZwQuerySystemInformation(
    IN ULONG SystemInformationClass,
	IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength
	);

NTSTATUS 
ZwOpenProcess(
   OUT PHANDLE ProcessHandle, 
   IN ACCESS_MASK DesiredAccess, 
   IN POBJECT_ATTRIBUTES ObjectAttributes, 
   IN PCLIENT_ID ClientId
   );

NTSTATUS
PsLookupProcessByProcessId(
    IN HANDLE ProcessId,
    OUT PEPROCESS *Process
    );


NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess (
    IN HANDLE           ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID           ProcessInformation,
    IN ULONG            ProcessInformationLength,
    OUT PULONG          ReturnLength OPTIONAL
	);


// ZwProtectVirtualMemory won't resolve! Need to extract from dll manually.

NTSYSAPI 
NTSTATUS 
NTAPI 
ZwPulseEvent( 
	HANDLE h, 
	PULONG p 
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwProtectVirtualMemory(
    IN HANDLE ProcessHandle,
    IN PVOID *BaseAddress,
    IN ULONG *NumberOfBytesToProtect,
    IN ULONG NewAccessProtection,
    OUT PULONG OldAccessProtection
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenDirectoryObject(
    __out PHANDLE DirectoryHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
    
///////////////////////////////   END OF FILE   ///////////////////////////////