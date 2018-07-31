// WildcardsPatternMatch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////

int wildcmp(const char *wild, const char *string) 
{
	const char *cp = NULL, *mp = NULL;
	
	while ( (*string) && (*wild != '*') )
	{
		if ((*wild != *string) && (*wild != '?')) 
		{
			return 0;
		}

		wild++;
		string++;
	}
	
	while (*string) 
	{
		if (*wild == '*')
		{
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string+1;

		} 
		else if ((*wild == *string) || (*wild == '?')) 
		{
			wild++ ;
			string++ ;
		} 
		else 
		{
			wild = mp;
			string = cp++;
		}
	}
	
	while (*wild == '*')
	{
		wild++;
	}

	return !*wild ;
}


//////////////////////////////////////////////////////////////////////////

const char ParentStrings[ MAX_PATH ] = "*\\ab*\\c.*" ;
char ChildrenStrings[ MAX_PATH ] = "\\ab..a\\c.xxx" ;


int main(int argc, char* argv[])
{
	int ret = 0 ;

	ret = wildcmp( ParentStrings, ChildrenStrings );

	if ( ret )
	{
		printf( "匹配成功,是其子串! \n" );
	}

	return 0;
}

