#pragma once

#include <ntifs.h>

//////////////////////////////////////////////////////////////////////////


//
// naked 函数的开篇 & 结局
//

#define Prolog() \
{    \
	_asm push ebp    \
	_asm mov ebp, esp    \
	_asm sub esp, __LOCAL_SIZE    \
	_asm pushfd    \
	_asm pushad    \
}

#define FinisDenny()    \
{    \
	_asm popad    \
	_asm popfd    \
	_asm mov esp, ebp    \
	_asm pop ebp    \
}

#define FinisOrig()    \
{    \
	_asm popad    \
	_asm popfd    \
	_asm mov esp, ebp    \
	_asm pop ebp    \
	_asm _emit   0x68	\
	_asm _emit   0x00	\
	_asm _emit   0x00	\
	_asm _emit   0x00	\
	_asm _emit   0x00	\
	_asm ret			\
}

//
// fake 函数的实例模板 (针对Header Jmp类型; 若是call hook类型,该模板还得改改)
//

/*
// VOID NtUserBlockInput( IN BOOL fBlockIt );

VOID _declspec(naked) fake_NtUserBlockInput()
{
	BOOL fBlockIt ;

	// 1. 前置处理
	Prolog();
	fBlockIt = FALSE ;

	// 2. 判断是否禁止虚拟按键,若放行则走原流程
	__asm
	{
		mov	eax, dword ptr [ebp+8]
		mov	fBlockIt, eax
	}

	if ( FALSE == fBlockIt ) { goto _Call_Orignal_NtUserBlockInput_ ; }

_Denny_for_NtUserBlockInput_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// 修改返回值
		ret 4h
	}

_Call_Orignal_NtUserBlockInput_ :
	FinisOrig();
}
*/


//////////////////////////////////////////////////////////////////////////

BOOL LoadInlineHookEngine ();
VOID UnloadInlineHookEngine () ;

BOOLEAN
HookCode95 (
    IN PVOID  pTarFunction,
    IN PVOID  pNewFunction,
    IN ULONG  HookNumber
    );

VOID
UnhookCode95 (
    IN PVOID  pNewFunction
    );

//////////////////////////////////////////////////////////////////////////

typedef enum _INLINE_ENGINE_TYPES
{
    Automatic,
    CallHookE8,
    CallHookFF15,
    InlineHookPre2,  // 短跳+长跳转
    InlineHookPre1,  // 开头5字节JMP
    InlineHookDep2,  // push+ret
    InlineHookDep1,  // 深度JMP
    InlineCrazyPatch

    //
    // let's do better
    //

} INLINE_ENGINE_TYPES;


//////////////////////////////////////////////////////////////////////////