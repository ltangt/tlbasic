#ifndef _TLBASIC_STRING_H_
#define _TLBASIC_STRING_H_

void  str_lowercase(char *pText);
void  str_addchar(char *str,char ch);
char* str_addstr(const char *str1,const char* str2);
char* str_copynew(const char* str);
int   str_nocasecmp(const char* str1,const char* str2);

#endif
