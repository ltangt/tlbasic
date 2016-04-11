/*
============================================================================
Name        : main.c
Author      : tangl_99
Version     : 0.1
Copyright   : tangl_99
Description : Program Command Entry
============================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "tmem.h"
#include "tfile.h"
#include "tstring.h"
#include "tcompiler.h"


const char szVersion[] = "TL's Basic beta version: 0.8.3c update: 2006.8.13\n";
const char szUsage[] = "Usage : basic sourcefile destfile [-I Include Directory]\n";

#ifdef _DEBUG
int g_TraceParse = 1;   /* if it enable parsing trace list*/
int g_TraceCodeGen = 1; /* if it enable generating code list*/
#else
int g_TraceParse = 0;   /* if it enable parsing trace list*/
int g_TraceCodeGen = 0; /* if it enable generating code list*/
#endif

char *g_SystemIncludeDirectory = NULL;
char *g_SourceFileName = NULL;
char *g_DestFileName= NULL;

int  readCommandLineArguments(int argc, char* argv[]);
void releaseCommandLineArguments();

int main(int argc,char *argv[])
{
	int ret_compile; 
#ifdef _DEBUG
	char *debug_argv[] = {"compiler.exe","in.txt","out.bin"};
	argc = 3;
	argv = debug_argv;
#endif
	if(!readCommandLineArguments(argc,argv))
	{
		printf(szVersion); 
		printf(szUsage);
		return 0;
	}
	/*compiler source file*/
	ret_compile = tcompile_File(g_SourceFileName,g_DestFileName,"list.txt"); 
	switch(ret_compile)
	{
	case RET_COMPILE_SUCCESS:
		printf("success.\n");
		break;
	case RET_ERROR_COMPILE_SOURCEFILE:
		printf("Errors in reading the source file.\n");
		break;
	case RET_ERROR_COMPILE_CODEFILE:
		printf("Errors in writing the code file.\n");
		break;
	case RET_ERROR_COMPILE_LISTFILE:
		printf("Errors in writing the list file.\n");
		break;
	case RET_ERROR_COMPILE_COMPILE:
		{
			long  length = tfile_length("list.txt");
			char* listbuf= (char*)malloc(length+1);
			tfile_gettextbuf("list.txt",listbuf);
			printf(listbuf);
			free(listbuf);
		}
		break;
	}
#ifdef _DEBUG
	system("pause");
#endif
	releaseCommandLineArguments();
	return 0; 
}

/**
* read and split command arguments
* @param argc : count of the arguments
* @param argv : arguments
* @param return : 0 : arguments error, 1 : arguments accepted
*/
int readCommandLineArguments(int argc, char* argv[])
{
	int i= 0;
	if(argc < 3)
		return 0;

	g_SourceFileName = str_copynew(argv[1]);
	g_DestFileName   = str_copynew(argv[2]);
	for(i = 3; i< argc; i++) 
	{
		if(strcmp(argv[i],"-I") == 0 && argc > i+1)
		{
			g_SystemIncludeDirectory = str_copynew(argv[i+1]);
			i++;
		}
	}
	return 1;
}

/**
* release the global data for command line arguments
*/
void releaseCommandLineArguments()
{
	free(g_SourceFileName);
	free(g_DestFileName);
	if(g_SystemIncludeDirectory != NULL)
		free(g_SystemIncludeDirectory);
}
