#include "tcodefile.h"

#include <stdio.h>
#include "tbuffereader.h"
#include "global.h"
#include "tmem.h"
#include "tstring.h"

#include "tbytecode.h"

/*=========================================================================
* FUNCTION:      tcodefile_create
* TYPE:          public interface
* OVERVIEW:      create a tCodeFile data structure 
* INTERFACE:
*   parameters:  
*   returns:     new empty tCodeFile data structure 
*=======================================================================*/
tCodeFile* tcodefile_create()
{
	tCodeFile* newCodefile = (tCodeFile*)mem_alloc(sizeof(tCodeFile));

	memset(newCodefile,0,sizeof(tCodeFile));
	newCodefile->magic = MAGIC;
	newCodefile->magic_version = MAGIC_VERSION;
	newCodefile->num_global_Variables = 0;
	newCodefile->num_functions = 0;
	//newCodefile->num_nativefunc = 0;

	return newCodefile;
}

/*=========================================================================
* FUNCTION:      tcodefile_load
* TYPE:          public interface
* OVERVIEW:      load code to construct the code file structure 
* INTERFACE:
*   parameters:  fp FILE* : the code file structure
*   returns:     RET_LOAD_NOTCODEFILE : the code buffer is not right format
*                RET_LOAD_NOTMAGICVERSION: code version is over the VM version
*                RET_LOAD_ERROR: unknown error
*                RET_LOAD_SUCCESS: success
*=======================================================================*/
int tcodefile_load(tCodeFile* codefile,FILE* fp)
{
	tGlobal_Variable *var;
	tFunction         *fun;
	u32 magic = 0;
	u16 version;
	u16 num;
	u32 i;

	/* check the file format */
	fread(&magic,sizeof(u32),1,fp);
	if(magic != codefile->magic)
		return RET_LOAD_NOTCODEFILE;
	
	/* check the file version */
	fread(&version,sizeof(u16),1,fp);
	if(version > codefile->magic_version)
		return RET_LOAD_NOTMAGICVERSION;
	
	/* read number of the global Variables */
	fread(&num,sizeof(u16),1,fp);
	codefile->global_Variables = (tGlobal_Variable*)mem_alloc(num*sizeof(tGlobal_Variable));
	codefile->num_global_Variables = num;

	/* load global Variables */
	for(i=0;i<codefile->num_global_Variables; i++)
	{
		var = &codefile->global_Variables[i];
		
		//fread(&var->name_len,sizeof(u16),1,fp);
		//var->name = (u8*)mem_alloc(var->name_len+1);
		//fread(var->name,var->name_len,1,fp);
		//var->name[var->name_len] = 0;

		fread(&var->type,sizeof(u8),1,fp);
	}

	/* read number of the function */
	fread(&num,sizeof(u16),1,fp);
	// codefile->functions = (tFunction*)mem_alloc(num*16);
	codefile->functions = (tFunction*)mem_alloc(num*sizeof(tFunction));
	codefile->num_functions = num;

	/* load functions */
	for(i=0;i<codefile->num_functions; i++)
	{
		fun = &codefile->functions[i];
		
		/*fread(&fun->name_len,sizeof(u16),1,fp);
		fun->name = (u8*)mem_alloc(fun->name_len+1);
		fread(fun->name,fun->name_len,1,fp);
		fun->name[fun->name_len] = 0;*/

		fread(&fun->type,sizeof(u8),1,fp);
		fread(&fun->codesize,sizeof(u32),1,fp);
		fun->code = (u8*)mem_alloc(fun->codesize);
		fread(fun->code,fun->codesize,1,fp);
	}

	///* load number of used native functions */
	//fread(&num,sizeof(u16),1,fp);
	//codefile->nativefunc = (tNativeFuncDesc*)mem_alloc(num*sizeof(tNativeFuncDesc));
	//codefile->num_nativefunc = num;

	///* load used native functions descriptions */
	//for(i=0;i<codefile->num_nativefunc;i++)
	//{
	//	nativefunc = &codefile->nativefunc[i];

	//	fread(&nativefunc->name_len,sizeof(u16),1,fp);
	//	nativefunc->name = (u8*)mem_alloc(nativefunc->name_len+1);
	//	fread(nativefunc->name,nativefunc->name_len,1,fp);
	//	nativefunc->name[nativefunc->name_len] = 0;

	//	fread(&nativefunc->type,sizeof(u8),1,fp);
	//}
	return RET_LOAD_SUCCESS;
}

/*=========================================================================
* FUNCTION:      tcodefile_release
* TYPE:          public interface
* OVERVIEW:      release the code file structure
* INTERFACE:
*   parameters:  codefile tCodeFile*: the tCodeFile structure to release
*   returns:     
*=======================================================================*/
void tcodefile_release(tCodeFile* codefile)
{
	int i;

	
	mem_free(codefile->global_Variables);
	

	for(i=0;i<codefile->num_functions; i++)
	{
		mem_free(codefile->functions[i].code);
	}
	mem_free(codefile->functions);

	mem_free(codefile);
}
