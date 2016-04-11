#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tmem.h"
#include "tstring.h"

void str_lowercase(char *pText)
{
	char *c = pText;
	while(*c != '\0')
	{ 
		if(*c >= 'A' && *c <= 'Z')
			*c += 'a' - 'A';
		c++;
	}
}

void str_addchar(char *str,char ch) 
{
	long len = (long)strlen(str);
	str[len] = ch;
	str[len+1] = '\0';
}

char* str_copynew(const char* str)
{
	char* newstr = (char*)malloc(strlen(str)+1);
	strcpy(newstr,str);
	return newstr;
}

int str_nocasecmp(const char* str1,const char* str2)
{
	char c1,c2;
	char d = (char)('a' - 'A');
	while(*str1 != 0 && *str2 != 0)
	{
		c1 = (*str1 >= 'A' && *str1 <= 'Z') ? ((*str1) + d) : (*str1);
		c2 = (*str2 >= 'A' && *str2 <= 'Z') ? ((*str2) + d) : (*str2);
		if(c1 != c2)
			return 1;
		str1++;
		str2++;
	}

	if(((*str1)^(*str2)) != 0 )
		return 1;

	return 0;
}

char* str_addstr(const char *str1,const char* str2)
{
	char* newstr = (char*)malloc((long)(strlen(str1)+strlen(str2)+1));
	sprintf(newstr,"%s%s",str1,str2);
	return newstr;
}