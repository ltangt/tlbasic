/*
============================================================================
Name        : tcompiler.c
Author      : tangl_99
Version     : 0.1
Copyright   : tangl_99
Description : Compiler Modules
============================================================================
*/
#include "tcompiler.h"

#include "tparse.h"
#include "tcode.h"
#include "tcodefile.h"
#include "global.h"
#include "tfile.h"

extern int g_TraceParse;   /* if it enable parsing trace list */
extern int g_TraceCodeGen; /* if it enable generating code list */
  
/**
* Compile process
* @param source : source code buffer
* @param sourcefilename : source file name
* @param list_os : list output stream
* @param code_os : generate code output steam
* @return : result of the compile process
*/ 
int tcompile(char* source,const char* sourcefilename,FILE* list_os,FILE* code_os)
{
	int ret;
	ParseTreeNode *programParseTree;
	tCodeFile     *codefile;

	/* initialize the list and code output streams */
	g_listfp = list_os;
	g_codefp = code_os;
	rewind(list_os);
	rewind(code_os);

	/* parsing the source code */
 	programParseTree = tparse_Start(source,sourcefilename); 
	if(programParseTree == NULL)
		return 0;

	/* print the parse tree */
	if(g_TraceParse) /* if enable printing the trace */
		tparse_printParsetree(programParseTree);

	/* generate the struct of code file */
	codefile = tcode_gen(programParseTree);

	/* write the code file */
	if(codefile != NULL)
	{
		tcodefile_toStream(codefile,code_os);
		if(g_TraceCodeGen)
			tcodefile_print(codefile);
		
		tcodefile_release(codefile);
		fprintf(list_os,"Success!\r\n");
		ret = 1;
	}
	else
		ret = 0;

	/* free the parse tree */
	tparse_freeParsetree(programParseTree);
	return ret;
}


/**
* compile source in files
*/
int tcompile_File(const char* sourcefilename, const char *codefilename,const char *listfilename)
{
	char *sourcebuf;
	long sourcelen;
	FILE *code_os;
	FILE *list_os;
	int  ret = 0;

	/* initialize source buffer */
	sourcelen = tfile_length(sourcefilename);
	if(sourcelen == -1)
		return RET_ERROR_COMPILE_SOURCEFILE;
	
	sourcebuf = (char*)malloc(sourcelen+1);
	if(sourcebuf == NULL)
		return RET_ERROR_COMPILE_SOURCEFILE;

	/* read source file text */
	ret = tfile_gettextbuf(sourcefilename,sourcebuf);
	if(!ret)
	{
		free(sourcebuf);
		return RET_ERROR_COMPILE_SOURCEFILE;
	}

	/* initialize list output stream */
	list_os = tmpfile();
	if(list_os == NULL)
	{
		free(sourcebuf);
		return RET_ERROR_COMPILE_LISTFILE;
	}

	/* initialize code output stream */
	code_os = tmpfile();
	if(code_os == NULL)
	{
		free(sourcebuf);
		fclose(list_os); 
		return RET_ERROR_COMPILE_CODEFILE;
	} 

	/* start compiling */
	if(tcompile(sourcebuf,sourcefilename,list_os,code_os))
	{
		/* write code file */
		if(!tfile_save(code_os,codefilename))
			ret = RET_ERROR_COMPILE_CODEFILE;
		else
			ret = RET_COMPILE_SUCCESS;
	}
	else
		ret = RET_ERROR_COMPILE_COMPILE;

	/* write list file */
	if(!tfile_save(list_os,listfilename))
		ret = RET_ERROR_COMPILE_LISTFILE;

	/* free memory */
	fclose(list_os);
	fclose(code_os);
	free(sourcebuf);
	return ret;
}
