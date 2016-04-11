/*
============================================================================
Name        : tcompiler.h
Author      : tangl_99
Version     : 0.1
Copyright   : tangl_99
Description : Compiler Modules
============================================================================
*/
#ifndef _TLBASIC_COMPILER_H_
#define _TLBASIC_COMPILER_H_
#include <stdio.h>


int tcompile(char* source,const char* sourcefilename,FILE* list_os,FILE* code_os);
int tcompile_File(const char* sourcefilename, const char *codefilename,const char *listfilename);

#endif

