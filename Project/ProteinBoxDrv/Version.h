#pragma once


//////////////////////////////////////////////////////////////////////////


#define g_ProteinBoxDrv_Version "1.01"

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
// 系统版本信息
//

typedef enum _VERSION_TYPE_ 
{
	__unknow = 0,
	__win2k,
	__winxp,
	__win2003,
	__vista,
	__win7
} VERSION_TYPE ;


typedef struct _SYSTEM_VERSION_INFO_ 
{
	/*0x000 */ ULONG MajorVersion ;
	/*0x004 */ ULONG MinorVersion ;
	/*0x008 */ ULONG BuildNumber  ;
	/*0x00C */ 
	struct {
		ULONG  IS___win2k	: 1 ;
		ULONG  IS___xp      : 1 ;
		ULONG  IS___win2003 : 1 ;
		ULONG  IS___vista   : 1 ;
		ULONG  IS___win7	: 1 ;

		ULONG  IS_before_vista : 1 ; // MajorVersion < 6 || BuildNumber <= 0x1770
		ULONG  Reserved		: 26 ;
	} ;

	/*0x010 */ VERSION_TYPE CurrentVersion ; // 用来标记当前系统

} SYSTEM_VERSION_INFO, *PSYSTEM_VERSION_INFO ;

extern SYSTEM_VERSION_INFO g_Version_Info ;



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
GetVersion(
	);

///////////////////////////////   END OF FILE   ///////////////////////////////