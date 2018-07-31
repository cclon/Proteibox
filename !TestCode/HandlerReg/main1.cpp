/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/01/05 [5:1:2010 - 15:25]
* MODULE : D:\Program\R0\Coding\沙箱\SandBox\Code\HandlerReg\main1.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <iostream.h>
#include "Test.h"

//////////////////////////////////////////////////////////////////////////

int main()
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	while ( bOK )
	{
		printf (
			"请选择操作:	\n"
			"0. ESC			\n"
			"1. 新建键值: ZwCreateKey	\n"
			"2. 打开键值: ZwOpenKey		\n"
			"3. 设置Value: ZwSetValueKey \n"
			"4. 重命名键值	\n"
			"5. 读键值		\n"
			"6. 删键值		\n"
			"-------------------------------------------\n"
			);
		
		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			break;
			
		case 1:
			TestCreateKey ();
			break;
			
		case 2:
			TestOpenKey ();
			break;
			
		case 3:
			TestSetValueKey ();
			break;
			
		case 4:
			break;

		case 5:
			break;
			
		default:
			break;
		}
	}
	
	//	getchar() ;
	return 0;
}

///////////////////////////////   END OF FILE   ///////////////////////////////