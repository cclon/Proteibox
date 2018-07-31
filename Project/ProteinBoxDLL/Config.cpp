#include "stdafx.h"
#include "ProteinBoxDLL.h"
#include "Config.h"

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib") 

#pragma warning(disable : 4995 )

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	m_pProfile = NULL ;
	RtlZeroMemory( m_szName, MAX_PATH );
}

CConfig::~CConfig()
{
}


BOOL CConfig::Load( IN char* filename )
{
	if ( NULL == filename )
	{
		// printf( "error! | CConfig::Load | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	strcpy( m_szName, filename );
	m_pProfile = LoadProfile( m_szName );
	if ( m_pProfile ) { return TRUE ; }

	return FALSE ;
}


VOID CConfig::UnLoad()
{
	FreeProfile( m_pProfile );
}


int 
CConfig::AnalysisTextLine (
	IN const char *pszText
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:25]

Routine Description:
  分析@pszText的内容

--*/
{
	int len = strlen(pszText);
    int ret = T_KEY_MORE;
    char *p1 = NULL;
    
	if (len == 0 || pszText[0] == ';')  // line start with ';' is considered as comment line
    {
        ret = T_BLANKLINE;
    }
    else if ((pszText[0] == '[') && (pszText[len-1] == ']'))
    {
        ret =  T_SECTION;
    }
    else
    {
        p1 = (char*) strchr(pszText, '=');
        if ((p1 != NULL ) && (p1- (char *)pszText >= 1)) 
        {
            ret = T_KEY;
        }
    }

    return ret;
}



void 
CConfig::GetSectionName (
	IN const char *pszText,
	OUT char *section_name
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:25]

Routine Description:
  拷贝内容

--*/
{
	int length = min(strlen(pszText)-2, SECTION_LEN );

	strncpy( section_name, pszText+1,  length );
	section_name[ length ] = '\0';

	return ;
}



int CConfig::GetStringLength ( IN const char *pszText )
{
	int Length = 0 ;

	if ( NULL == pszText )
	{
		return 0 ;
	}

	while ( *pszText++ )
	{
		Length ++ ;
	}

	return Length ;
}



LPCONFIG_VALUE_INFO CConfig::AllocateNodeValue()
{
	LPCONFIG_VALUE_INFO pNode = (LPCONFIG_VALUE_INFO) malloc( sizeof(CONFIG_VALUE_INFO) );
	if ( NULL == pNode ) 
	{  
		// printf( "error! | AllocateNodeValue() | 申请内存失败 \n" );
		return NULL ;
	}
	
	pNode->next			= NULL	;
	pNode->NameLength	= 0		;
	pNode->ValueName	= NULL	;

	return pNode;
}



void CConfig::DistroyNodeValue( IN PVOID ListHead )
{
	LPCONFIG_VALUE_INFO pNodeHead = (LPCONFIG_VALUE_INFO) ListHead ;
	LPCONFIG_VALUE_INFO pNodeCurrent, pValueNext ;
	
	//
	// 1. 参数合法性检测
	//
	
	if ( NULL == ListHead  )
	{
		// printf( "error! | DistroyValue() | 参数不合法 \n" );
		return ;
	}

	//
	// 2. 释放Key节点对应的所有字符串内存
	//

	pNodeCurrent = pNodeHead ;
	while( pNodeCurrent )
	{
		pValueNext = pNodeCurrent->next ; // 得到下个节点

		// 释放当前节点
 		kfree( pNodeCurrent->ValueName );
		kfree( pNodeCurrent );

		pNodeCurrent = pValueNext ;
	}

	return ;
}



LPCONFIG_VALUE_INFO
CConfig::FixNodeValue (
	IN const char* szName,
	IN int Length
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/23 [23:6:2010 - 11:23]

Routine Description:
  分配内存,将字符串@szName保存至新节点中,插入到单向链表的表头
    
Arguments:
  ListHead - 单向链表表头
  szName - 待保存的字符串内容
  Length - 字符串长度; 为0表明该字符串中没有逗号',',可以全部拷贝; 有值,表明其中包含逗号',',必须按照指定的长度拷贝

--*/
{
	LPCONFIG_VALUE_INFO pNode = NULL ;

	//
	// 1. 参数合法性检测
	//

	if ( NULL == szName || 0 == Length )
	{
		// printf( "error! | InsertValue() | 参数不合法 \n" );
		return NULL ;
	}

	//
	// 2.1 分配新节点，填充之
	//

	pNode = AllocateNodeValue() ;
	if ( NULL == pNode ) 
	{  
		// printf( "error! | InsertValue() | 申请内存失败 \n" );
		return NULL ;
	}
	
	// 2.2 为字符串分配内存
	pNode->NameLength = Length + 1 ;
	pNode->ValueName = (char*) malloc( Length+1 );
	if ( pNode->ValueName )
	{
		strncpy( pNode->ValueName, szName, Length );
		pNode->ValueName[ Length ] = '\0' ;
	}

	return pNode ;
}



BOOL
CConfig::BuildNodeValue (
	IN LPCONFIG_VALUE_INFO ListHead,
	IN const char* szName,
	IN int Length
	)
{
	LPCONFIG_VALUE_INFO pNode = NULL ;

	//
	// 1. 参数合法性检测
	//
	
	if ( NULL == ListHead || NULL == szName || 0 == Length )
	{
		// printf( "error! | BuildNodeValue() | 参数不合法 \n" );
		return FALSE ;
	}

	//
	// 2. 填充Value节点
	//

	pNode = FixNodeValue( szName, Length );
	if ( NULL == pNode )
	{
		// printf( "error! | BuildNodeValue() - FixNodeValue() | 填充节点失败 \n" );
		return FALSE ;
	}
	
	//
	// 3. 插入到单向链表的表头
	//
	
	if ( NULL == ListHead->next )
	{
		ListHead->next = pNode ;
	}
	else
	{
		pNode->next = ListHead->next ;
		ListHead->next = pNode ;
	}

	return TRUE ;
}



void 
CConfig::GetKeyInfo (
	IN const char *pszText, 
	OUT char *key_name,
	OUT LPCONFIG_VALUE_INFO *key_value
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:20]

Routine Description:
  比如当前的配置信息为: Name=sudam*,imadu*
  那么@key_name保存的内容为:Name
  @key_value保存的内容为:sudam*,imadu*

--*/
{
	BOOL bOver = FALSE ;

	//
	// 1. 校验参数合法性
	//
	
	if ( NULL == pszText || '\0' == *pszText )
	{
		// printf( "error! | CConfig::GetKeyInfo() | Invalid Paramaters; failed! \n" );
		return ;
	}

	//
	// 2. 获取Key的名字
	//

	char* p1 = (char*) strchr(pszText, '=');
	if ( p1 && key_name ) 
	{ 
		while(pszText < p1) { *key_name++ = *pszText++ ; }
		*key_name	= '\0' ;
		
		p1++ ;
	}
	else
	{
		p1 = (char*) pszText ;
	}
	
	// 为该Value分配一个链表头,不管等号后面有无内容,都要分配一个表头
	*key_value = AllocateNodeValue();
	if ( NULL == *key_value ) { return ; }
	
	LPCONFIG_VALUE_INFO pValueHead = (LPCONFIG_VALUE_INFO) *key_value ;

	if ( '\0' == *p1 ) { return ; } // 等号'='后面无内容

	//
	// 3. 获取Value的内容
	//

	int Length = 0 ;
	char* pStart = p1 ;
	char* pEnd = strchr( p1, ',' );
	
	if ( NULL == pEnd )
	{
		bOver = TRUE ;
		Length = GetStringLength( pStart ); // 等号后有内容,但是无逗号,说明就一个字符串
	}
	else
	{
		Length = (int) (pEnd - pStart) ; // 取第一截逗号前的字符串内容,填充至表头
	}

	pValueHead->NameLength = Length + 1 ;
	pValueHead->ValueName = (char*) malloc( Length+1 );
	if ( pValueHead->ValueName )
	{
		strncpy( pValueHead->ValueName, pStart, Length );
		pValueHead->ValueName[ Length ] = '\0' ;
	}

	if ( TRUE == bOver ) { return ; }

	pStart += Length + 1 ;
	if ( '\0' == *pStart ) { return ; }

	pEnd = strchr( pStart, ',' );
	if ( NULL == pEnd ) 
	{ 
		BuildNodeValue( pValueHead, pStart, GetStringLength(pStart) ); 
		return ;
	}

	// 以逗号','为截断，保存各段字符串到链表中
	while( *pStart != '\0' )
	{
		// 得到当前字符串的长度
		Length = (int) (pEnd - pStart) ;

		// 若长度为0，表明内容为"=,2222"(等号连着逗号) 或者 "11,,22"(2个连着的逗号)，跳过继续
		if ( 0 == Length ) { goto _WHILE_NEXT_ ; }

		// 长度不为0，表明有内容，拷贝到新申请的内存中
		if ( FALSE == BuildNodeValue( pValueHead, pStart, Length ) ) { goto _WHILE_NEXT_ ; }

_WHILE_NEXT_ :
		pStart += Length + 1 ;
		pEnd = strchr( pStart, ',' );
		if ( NULL == pEnd ) 
		{ 
			// 说明是最后一个字符串了。
			BuildNodeValue( pValueHead, pStart, GetStringLength(pStart) ); 
			break ;
		}
	}

	return ;
}



void 
CConfig::GetKeyInfoEx (
	IN const char *pszText, 
	OUT LPCONFIG_VALUE_INFO *key_value
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:20]

Routine Description:
  该函数是对换行的上段Key的内容进行追加,eg.当前的配置信息为: sudam*,imadu*
  @key_value为链表头,将sudam*,imadu*建立对应的节点,插入至链表中

--*/
{
	char* p1 = (char*) pszText ;
	LPCONFIG_VALUE_INFO pValueHead = (LPCONFIG_VALUE_INFO) *key_value ;

	//
	// 1. 校验参数合法性
	//
	
	if ( (NULL == p1) || ('\0' == *p1) )
	{
		// printf( "error! | CConfig::GetKeyInfoEx() | Invalid Paramaters; failed! \n" );
		return ;
	}

	//
	// 2. 追加Value的内容
	//

	int Length = 0 ;
	char* pStart = p1 ;
	char* pEnd	 = strchr( p1, ',' );
	if ( NULL == pEnd ) 
	{ 
		BuildNodeValue( pValueHead, pStart, GetStringLength(pStart) ); 
		return ;
	}

	while( *pStart != '\0' )
	{
		// 得到当前字符串的长度
		Length = (int) (pEnd - pStart) ;

		// 若长度为0，表明内容为"=,2222"(等号连着逗号) 或者 "11,,22"(2个连着的逗号)，跳过继续
		if ( 0 == Length ) { goto _WHILE_NEXT_ ; }

		// 长度不为0，表明有内容，拷贝到新申请的内存中
		BuildNodeValue( pValueHead, pStart, Length );

_WHILE_NEXT_ :
		pStart += Length + 1 ;
		pEnd = strchr( pStart, ',' );
		if ( NULL == pEnd ) // 说明是最后一个字符串了。
		{
			BuildNodeValue( pValueHead, pStart, GetStringLength(pStart) ); 
			break ;
		}
	}

	return ;
}


void
CConfig::FreeProfile (
	IN CONFIG_SECTION_INFO *pProfileInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:25]

Routine Description:
  释放内存中建立的节点信息

--*/
{
    CONFIG_SECTION_INFO *pTempSec = NULL;
    CONFIG_SECTION_INFO *pCurrentSec = pProfileInfo;
    CONFIG_KEY_INFO *pTempKey = NULL;
    CONFIG_KEY_INFO *pCurrentKey = NULL;

    while ( pCurrentSec != NULL )
    {
        pTempSec = pCurrentSec;
        pCurrentKey = pTempSec->KeyHead;
        pCurrentSec = pCurrentSec->next;

        while (pCurrentKey != NULL)
        {
            pTempKey = pCurrentKey;
            pCurrentKey = pCurrentKey->next;

			DistroyNodeValue( (PVOID)pTempKey->ValueHead );
            free( pTempKey );
        }

        free( pTempSec );
    }

	return ;
}



CONFIG_SECTION_INFO *
CConfig::LoadProfile (
	IN const char *filename
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:25]

Routine Description:
  将配置文件从磁盘加载至内存

--*/
{
	char buff[ 256 ] = {'\0'};
	CONFIG_SECTION_INFO *pResult = NULL;
	CONFIG_SECTION_INFO *pLastSec = NULL;
	CONFIG_SECTION_INFO *pTempSec = NULL;
	CONFIG_KEY_INFO *pTempKey = NULL;
	CONFIG_KEY_INFO *pLastKey = NULL;
	char* lpLastKeyName = NULL ;
	int Length = 0 ;

	FILE *fp = fopen(filename, "r");
	if (fp != NULL)
	{
		while ( !feof(fp) )
		{
			if ( NULL == fgets(buff, 256, fp) ) { continue ; }
			
			Length = strlen( buff ) -1 ;

			if ( 0x0a == *(PBYTE)((int)buff + Length) )
			{
				buff[ Length ] = '\0';    // erase last 0x0a(\n)
			}

			switch ( AnalysisTextLine(buff) )
			{
			case T_SECTION:
				pTempSec = (CONFIG_SECTION_INFO *) malloc( sizeof(CONFIG_SECTION_INFO) );
				CHECK_MEM( pTempSec );
				GetSectionName(buff, pTempSec->SectionName);

				pTempSec->next = NULL;
				pTempSec->KeyHead = NULL;
				if (pResult == NULL)
				{
					pResult = pTempSec;
				}
				
				if (pLastSec != NULL)
				{
					pLastSec->next = pTempSec;
				}

				pLastSec = pTempSec;
				pLastKey = NULL;
				break;
				
			case T_KEY:
				if (pLastSec != NULL)
				{
					pTempKey = (CONFIG_KEY_INFO *) malloc( sizeof(CONFIG_KEY_INFO) );
					CHECK_MEM( pTempKey );
					GetKeyInfo( buff, pTempKey->KeyName, &pTempKey->ValueHead );
					lpLastKeyName = pTempKey->KeyName ;
					pTempKey->next = NULL;
					if (pLastKey != NULL)
					{
						pLastKey->next = pTempKey;
					}
					else
					{
						pLastSec->KeyHead = pTempKey;
					}

					pLastKey = pTempKey;
				}
				break;

			case T_KEY_MORE:
				if (pLastKey != NULL)
				{
					GetKeyInfoEx( buff, &pLastKey->ValueHead );
				}
				break;

			case T_BLANKLINE:
				break ;
				
			default:
				// printf( "profile %s format not support \n", filename );
				FreeProfile(pResult);
				fclose(fp);
				return NULL;
			} 
		}

		fclose(fp);
	}

	return pResult;
}



void 
CConfig::SaveProfile (
	IN const char *filename,
	IN CONFIG_SECTION_INFO *pProfileInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:25]

Routine Description:
  保存配置文件至磁盘

--*/
{
	CONFIG_SECTION_INFO *pCurrentSec = pProfileInfo;
	CONFIG_KEY_INFO *pCurrentKey = NULL;

	FILE *fp = fopen(filename, "w");
	if (fp != NULL)
	{
		while (pCurrentSec != NULL)
		{
		    if (pCurrentSec != pProfileInfo)
		    {   
		        fprintf(fp,"\n");
		    }

			fprintf(fp,"[%s]\n",pCurrentSec->SectionName);
			pCurrentKey = pCurrentSec->KeyHead;
			while (pCurrentKey != NULL)
			{
				// !!!!!!!!!!!!!!!!!!!!!!
				LPCONFIG_VALUE_INFO pValueCurrent = pCurrentKey->ValueHead ;
				fprintf(fp,"%s=", pCurrentKey->KeyName);

				while ( pValueCurrent )
				{
					if ( pValueCurrent->ValueName )
					{
						fprintf(fp,"%s", pValueCurrent->ValueName);
					}
					
					pValueCurrent = pValueCurrent->next ;
					if ( pValueCurrent ) { fprintf(fp,","); } 
				}

				fprintf(fp,"\n");
				pCurrentKey = pCurrentKey->next;
			}

			pCurrentSec = pCurrentSec->next;
		}

		fclose(fp);
	}

	return ;
}



BOOL 
CConfig::GetProfileStringPrivate (
	IN const char *section,
	IN const char *key,
	OUT char* pszRetStr
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/22 [22:6:2010 - 15:25]

Routine Description:
  获取配置文件中指定对象的内容

--*/
{
	CONFIG_SECTION_INFO *pTempSec = m_pProfile;
	CONFIG_KEY_INFO     *pTempKey = NULL;
	BOOL bRet = FALSE ;

	if( NULL == m_pProfile ) { return FALSE ; }

	while ( pTempSec != NULL )
	{
		if (strcmp(pTempSec->SectionName, section) == 0)
		{
			pTempKey = pTempSec->KeyHead;
			while (pTempKey != NULL)
			{
				if (strcmp(pTempKey->KeyName, key) == 0)
				{
					strncpy( pszRetStr, pTempKey->ValueHead->ValueName, pTempKey->ValueHead->NameLength ) ;
					bRet =  true;
					break;
				}
				pTempKey = pTempKey->next;
			}
			break;
		}

		pTempSec = pTempSec->next;
	}

    return bRet;
}


//////////////////////////////////////////////////////////////////////////



/*=============================================================================
NAME:  GetProfileString
DESCRIPTION:
    retrieve string from profile.
PARAMS:
    INPUT: filename, the profile filename to read.
           section, section name, has format [xxxx].
           key, key name, has format key=xxxxxxx.
           defaultStr, if the section and section does not exist, defaultStr will
           be the returned to buffer pszRetStr.
    OUTPUT:pszRetStr, a buffer used to store the returned string.
RETURN:
    NA
SPECIAL NOTES:
    NA (no available)
==============================================================================*/

void
CConfig::GetConfString (
	IN const char *section, 
    IN const char *key, 
    OUT LPCONFIG_VALUE_INFO  *pszRetStr
	)
{
	CONFIG_SECTION_INFO *pTempSec = m_pProfile;
	CONFIG_KEY_INFO     *pTempKey = NULL;
	
	if( NULL == m_pProfile ) { return ; }
	
	while ( pTempSec != NULL )
	{
		if (strcmp(pTempSec->SectionName, section) == 0)
		{
			pTempKey = pTempSec->KeyHead;
			while (pTempKey != NULL)
			{
				if (strcmp(pTempKey->KeyName, key) == 0)
				{
					*pszRetStr = pTempKey->ValueHead ;
					break;
				}
				pTempKey = pTempKey->next;
			}
			break;
		}
		
		pTempSec = pTempSec->next;
	}
	
    return ;
}



int 
CConfig::GetConfInt (
	IN const char *section, 
    IN const char *key, 
    IN int defaultValue
	)
{
    int ret = defaultValue;
	char s[ 10 ] = {'\0'};

    if ( TRUE == GetProfileStringPrivate(section, key, s) )
    {
        ret = atoi(s) ;
    }

    return ret;
}



double 
CConfig::GetConfDouble (
	IN const char *section, 
    IN const char *key, 
    IN double defaultValue
	)
{
    double ret = defaultValue;
    char s[ 20 ] = {'\0'};
    
	if ( TRUE == GetProfileStringPrivate(section, key, s) )
    {
        ret = atof(s);
    }

    return ret;
}

/*=============================================================================
NAME:  WriteConfString
DESCRIPTION:
    Write an string to profile. If the specified section and key doesn't
    exist, this function will create the key with specified section.
PARAMS:
    INPUT: filename, the profile filename to read.
           section, section name, has format [xxxx].
           key, key name, has format key=xxxxxxx.
           pszWriteStr, the string to be written
    OUTPUT:NA
RETURN:
    NA
SPECIAL NOTES:
	NA (not available)
==============================================================================*/

void 
CConfig::WriteConfString (
	IN const char *section,
	IN const char *key,
	IN const char *pszWriteStr
	)
{
	CONFIG_SECTION_INFO *pTempSec = m_pProfile;
	CONFIG_KEY_INFO     *pTempKey = NULL;
	BOOL bRet = FALSE ;
	
	if( NULL == m_pProfile ) { return ; }

	while ( pTempSec != NULL )
	{
		if (strcmp(pTempSec->SectionName, section) == 0)
		{
			pTempKey = pTempSec->KeyHead;
			while (pTempKey != NULL)
			{
				if (strcmp(pTempKey->KeyName, key) == 0)
				{
					// 销毁旧的链表
					DistroyNodeValue( (PVOID)pTempKey->ValueHead );

					// 重新建立新节点
					GetKeyInfo( pszWriteStr, NULL, &pTempKey->ValueHead );
					bRet =  TRUE ;
					break;
				}
				pTempKey = pTempKey->next;
			}

			/* not found, create the key */
			if ( bRet == FALSE )
			{
				pTempKey = (CONFIG_KEY_INFO *)malloc(sizeof(CONFIG_KEY_INFO));
				CHECK_MEM(pTempKey);
				strcpy(pTempKey->KeyName, key);
				GetKeyInfo( pszWriteStr, NULL, &pTempKey->ValueHead );
				
				/* insert the new key to the first item*/
				pTempKey->next = pTempSec->KeyHead;
				pTempSec->KeyHead = pTempKey;
				bRet = TRUE ;
			}

			break;
		}

		pTempSec = pTempSec->next;
	}

	/* section not found, create section and key */
	if ( FALSE == bRet )
	{
		pTempSec = (CONFIG_SECTION_INFO *)malloc(sizeof(CONFIG_SECTION_INFO));
		CHECK_MEM(pTempSec);
		pTempKey = (CONFIG_KEY_INFO *)malloc(sizeof(CONFIG_KEY_INFO));
		CHECK_MEM(pTempKey);

		GetKeyInfo( pszWriteStr, NULL, &pTempKey->ValueHead );
		strcpy(pTempKey->KeyName, key);
		pTempKey->next = NULL;

		strcpy(pTempSec->SectionName, section);
		pTempSec->KeyHead = pTempKey;

		pTempSec->next = m_pProfile;
		m_pProfile = pTempSec;
	}

	SaveProfile( m_szName, m_pProfile );

	return ;
}

/*=============================================================================
NAME:  WriteConfInt
DESCRIPTION:
    Write an integer to profile. If the specified section and key doesn't
    exist, this function will create the key with specified section.
PARAMS:
    INPUT: filename, the profile filename to read.
           section, section name, has format [xxxx].
           key, key name, has format key=xxxxxxx.
           value, the integer to be written
    OUTPUT:NA
RETURN:
    NA
SPECIAL NOTES:
	NA (not available)
==============================================================================*/

void 
CConfig::WriteConfInt (
	IN const char *section, 
    IN const char *key, 
    IN int value
	)
{
    char s[ 20 ] = {'\0'};
    
	sprintf( s, "%d",value );
    WriteConfString( section, key, s );
	return ;
}



void 
CConfig::WriteConfDouble (
	IN const char *section, 
    IN const char *key, 
    IN double value
	)
{
    char s[30]={'\0'};
    int len = 0;
    char *p = NULL;
    
    /* avoid overflow */
	if(value > 9.99E21)
		value = 9.99E21;
	if (value < -9.99E21)
		value = -9.99E21;
    sprintf(s, "%.6f",value);
    
    /* remove extra 0 */
    len = strlen(s);
    p = &s[len-1];
    while (*p == '0')
    {
        *p = '\0';
        p--;
    }

    WriteConfString( section, key, s );
	return ;
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

CConfigEx::CConfigEx( ) :
	m_bInitOK(FALSE),
	m_bAbortThread(FALSE),
	m_hEvent_InitConfigData_wakeup_R0(NULL),
	m_hEvent_InitConfigData_wakeup_R3(NULL),
	m_hEvent_waitfor_r0(NULL),
	m_hEvent_wakeup_r0(NULL),
	m_hWorkThread(NULL)
{	
	BOOL bRet = FALSE ;

	RtlZeroMemory( m_szProcteiBoxMainIni, MAX_PATH );
	RtlZeroMemory( m_szProcteiBoxTemplateIni, MAX_PATH );

	strcpy( m_szEvent_InitConfigData_wakeup_R0, "Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R0" );
	strcpy( m_szEvent_InitConfigData_wakeup_R3, "Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R3" );
	strcpy( m_szEvent_waitfor_r0, "Global\\Proteinbox_ConfEvent_waitfor_r0" );
	strcpy( m_szEvent_wakeup_r0, "Global\\Proteinbox_ConfEvent_wakeup_r0" );

	bRet = InitData();
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | CConfigEx() - InitData(); | 构造函数初始化失败! \n" );
	}
}

CConfigEx::~CConfigEx()
{
}


BOOL CConfigEx::Load()
{
	return CConfig::Load( m_szProcteiBoxMainIni ) ;
}
	

VOID CConfigEx::UnLoad()
{
	// 停止工作线程
	_StopThread();

	// 关闭事件句柄
	if ( m_hEvent_InitConfigData_wakeup_R0 )
	{
		CloseHandle( m_hEvent_InitConfigData_wakeup_R0 );
		m_hEvent_InitConfigData_wakeup_R0 = NULL;
	}
	
	if ( m_hEvent_InitConfigData_wakeup_R3 )
	{
		CloseHandle( m_hEvent_InitConfigData_wakeup_R3 );
		m_hEvent_InitConfigData_wakeup_R3 = NULL;
	}

	if ( m_hEvent_waitfor_r0 )
	{
		CloseHandle( m_hEvent_waitfor_r0 );
		m_hEvent_waitfor_r0 = NULL;
	}

	if ( m_hEvent_wakeup_r0 )
	{
		CloseHandle( m_hEvent_wakeup_r0 );
		m_hEvent_wakeup_r0 = NULL;
	}

	// 释放内存中的配置文件数据
	CConfig::UnLoad() ;
}


BOOL CConfigEx::InitData()
{
	BOOL bRet = FALSE ;
	
	if ( TRUE == m_bInitOK ) { return TRUE ; }
	
	//
	// 1. 初始化配置文件
	//

	__InitData( m_szProcteiBoxMainIni, CONFIG_FILE_NAME_MAIN );
	
	// 判断文件是否存在
    bRet = PathFileExistsA( m_szProcteiBoxMainIni );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - PathFileExistsA() | 文件不存在:%s \n", m_szProcteiBoxMainIni );
        return FALSE;
	}
	
	__InitData( m_szProcteiBoxTemplateIni, CONFIG_FILE_NAME_TMPL );
	// 判断文件是否存在
    bRet = PathFileExistsA( m_szProcteiBoxTemplateIni );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - PathFileExistsA() | 文件不存在:%s \n", m_szProcteiBoxTemplateIni );
		return FALSE;
	}

	//
	// 2. 创建事件对象和工作线程
	//

	m_hEvent_InitConfigData_wakeup_R0 = CreateEventA( NULL, FALSE, FALSE, m_szEvent_InitConfigData_wakeup_R0 );
	if ( NULL == m_hEvent_InitConfigData_wakeup_R0 )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - CreateEvent | 创建事件对象失败: %s \n", m_szEvent_InitConfigData_wakeup_R0 );
		return FALSE ;
	}

	m_hEvent_InitConfigData_wakeup_R3 = CreateEventA( NULL, FALSE, FALSE, m_szEvent_InitConfigData_wakeup_R3 );
	if ( NULL == m_hEvent_InitConfigData_wakeup_R3 )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - CreateEvent | 创建事件对象失败: %s \n", m_szEvent_InitConfigData_wakeup_R3 );
		return FALSE ;
	}

	m_hEvent_waitfor_r0 = CreateEventA( NULL, FALSE, FALSE, m_szEvent_waitfor_r0 );
	if ( NULL == m_hEvent_waitfor_r0 )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - CreateEvent | 创建事件对象失败: %s \n", m_szEvent_waitfor_r0 );
		return FALSE ;
	}

	m_hEvent_wakeup_r0 = CreateEventA( NULL, FALSE, FALSE, m_szEvent_wakeup_r0 );
	if ( NULL == m_hEvent_wakeup_r0 )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - CreateEvent | 创建事件对象失败: %s \n", m_szEvent_waitfor_r0 );
		return FALSE ;
	}

	bRet = _StartThread();
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | CConfigEx::InitData() - _StartThread(); | 创建工作线程失败\n" );
		return FALSE ;
	}
	
	m_bInitOK = TRUE ;
	return TRUE ;
}



VOID CConfigEx::__InitData( LPSTR lpdest, LPSTR lpsrc )
{
	CHAR szFileName[ MAX_PATH ];
	
	GetModuleFileNameA( NULL, szFileName, ARRSIZEOF(szFileName) );
	PathRemoveFileSpecA( szFileName );
	PathAppendA( szFileName, lpsrc );
	
	RtlZeroMemory( lpdest, MAX_PATH );
	strcpy( lpdest, szFileName );	
	return ;
}



// BOOL CConfigEx::GetDrvPointer( CDriver* drv )
// {
// 	m_drv_ProteinBoxDrv	= drv ;
// 	if ( NULL == m_drv_ProteinBoxDrv )
// 	{
// 		MYTRACE( "error! | CConfigEx::GetDrvPointer(); | 参数不合法 \n" );
// 		return FALSE ;
// 	}
// 
// 	return TRUE ;
// }



BOOL CConfigEx::Wakeup_R0_InitConfigData ()
{
	if ( NULL == m_hEvent_InitConfigData_wakeup_R0 ) { return FALSE ; }

	SetEvent( m_hEvent_InitConfigData_wakeup_R0 ); // 置事件为TRUE
	return TRUE ;
}


BOOL CConfigEx::Waitfor_R0_InitConfigData ()
{
	DWORD dwTimeout = 2 * 1000 ; // 2秒钟
	if ( NULL == m_hEvent_InitConfigData_wakeup_R3 ) { return FALSE ; }
	
	DWORD dwWaitResult = WaitForSingleObject( m_hEvent_InitConfigData_wakeup_R3, dwTimeout /*INFINITE*/ );

	if( WAIT_FAILED == dwWaitResult || WAIT_TIMEOUT == dwWaitResult )
	{
		return FALSE ;
	}

	return TRUE ;
}


void CConfigEx::SendConfigData()
{
	int nReturn = -1 ;
	NTSTATUS status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER Buffer ;
	CONFIG_SECTION_INFO *pTempSec = m_pProfile;
	CONFIG_KEY_INFO     *pTempKey = NULL;
	
	if( NULL == m_pProfile ) { return ; }

	Buffer.Head.Flag = FLAG_Ioctl_HandlerConf ;
	Buffer.Head.LittleIoctlCode = _Ioctl_Conf_function_InitData_ ;
	
	while ( pTempSec != NULL )
	{
		Buffer.ConfigBuffer.u.InitData.SeactionName = pTempSec->SectionName ;
		Buffer.ConfigBuffer.u.InitData.SeactionNameLength = strlen( pTempSec->SectionName );

		pTempKey = pTempSec->KeyHead;
		while (pTempKey != NULL)
		{
			Buffer.ConfigBuffer.u.InitData.KeyName = pTempKey->KeyName ;
			Buffer.ConfigBuffer.u.InitData.KeyNameLength = strlen( pTempKey->KeyName );

			LPCONFIG_VALUE_INFO pNode = pTempKey->ValueHead ;
			while ( pNode )
			{
				Buffer.ConfigBuffer.u.InitData.ValueName = pNode->ValueName ;
				Buffer.ConfigBuffer.u.InitData.ValueNameLength = strlen( pNode->ValueName );
				
				status = g_ProteinBoxDLL.IoControlEx (
					IOCTL_PROTEINBOX,
					(PVOID)&Buffer,						// InBuffer
					sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
					(PVOID)&Buffer,						// OutBuffer
					sizeof(IOCTL_PROTEINBOX_BUFFER)
					);

				if ( ! NT_SUCCESS(status) )  { return ; }
				pNode = pNode->next ;
 			}

			pTempKey = pTempKey->next;
		}
		
		pTempSec = pTempSec->next;
	}
	
    return ;
}


BOOL CConfigEx::_StartThread()
{	
	m_hWorkThread = CreateThread( NULL, 0, WorkThread, this, 0, NULL );	
    if ( NULL == m_hWorkThread )
	{
		MYTRACE( L"error! | CConfigEx::_StartThread(); | 创建工作线程失败\n" );
		return FALSE ;
	}
	
    return TRUE;
}



void CConfigEx::_StopThread()
{
	if( NULL != m_hWorkThread )
    {
        m_bAbortThread = TRUE;
		
        if( NULL != m_hEvent_waitfor_r0 )
        {
            SetEvent( m_hEvent_waitfor_r0 );
        }
		
        WaitForSingleObject( m_hWorkThread, INFINITE );
        CloseHandle( m_hWorkThread );
        m_hWorkThread = NULL;
    }
}


DWORD WINAPI CConfigEx::WorkThread(LPVOID lpParameter)
{
    CConfigEx* pThis = (CConfigEx*)lpParameter;
    if( pThis )
    {
        pThis->_ThreadProc();
    }

    return 0;
}



void CConfigEx::_ThreadProc()
{
	DWORD dwWaitVal ;

	Load() ;

	while ( TRUE )
    {
		MYTRACE( L"工作线程: I'm waiting ... \n" );

        dwWaitVal = WaitForSingleObject( m_hEvent_waitfor_r0, INFINITE );
		if ( TRUE == m_bAbortThread )
		{
			MYTRACE( L"工作线程接收到退出消息, 退出之. \n" );
			break ;
		}

		if ( WAIT_FAILED == dwWaitVal )
            break ;
		
        if ( WAIT_OBJECT_0 != dwWaitVal ) { continue ; }
        
		ResetEvent( m_hEvent_waitfor_r0 ); // 重置事件为FALSE
		
		// R0要操作Conf数据(读取/写入),通过DeviceIoctl取出消息块
		MYTRACE( L"工作线程: I Hive got R0 message. \n" );

		SendConfigData() ;
	
		// 让R0取消等待,收取数据
		SetEvent( m_hEvent_wakeup_r0 ); // 置事件为TRUE
		break;
    }

	return  ;
}





///////////////////////////////   END OF FILE   ///////////////////////////////