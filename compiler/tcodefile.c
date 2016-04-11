#include "tcodefile.h"

#include <stdio.h>
#include "tbytecode.h"
#include "global.h"
#include "tmem.h"
#include "tstring.h"

const char* const tcodefile_bytecodenames[] = BYTE_CODE_NAMES;
const char* const tcodefile_bytecodeformats[] = BYTE_CODE_FORMAT;

void       tcodefile_printBytecodes(const u8* codebuf,u32 codesize);
static int printCode(const u8* codebuf);
static int printString(const u8* codebuf);
static int printDataType(u8  type);
static int printAccessFlag(u8 flag);

tCodeFile* tcodefile_create()
{
	tCodeFile* newCodefile = (tCodeFile*)malloc(sizeof(tCodeFile));

	memset(newCodefile,0,sizeof(tCodeFile));
	newCodefile->magic = MAGIC;
	newCodefile->magic_version = MAGIC_VERSION;
	newCodefile->num_global_Variables = 0;
	newCodefile->num_functions = 0;
	
	return newCodefile;
}


void tcodefile_insert_globalvar(tCodeFile* codefile,const char *name,u8 type)
{
	int   num_var  = codefile->num_global_Variables;
	int   name_len = (u16)strlen(name);
	
	codefile->global_Variables[num_var].name = (u8*)str_copynew(name);
	codefile->global_Variables[num_var].name_len = (u16)name_len;
	codefile->global_Variables[num_var].type = (u8)type;
	codefile->num_global_Variables++;
}

void tcodefile_insert_function(tCodeFile* codefile,const char *name,u8 type,u8* code,u32 codesize)
{
	tFunction* fun;
	int num_fun = codefile->num_functions;		
	u16 name_len= (u16)strlen(name);
	if(str_nocasecmp(name,ENTRY_FUNC_NAME) == 0)
		fun = &codefile->functions[0];
	else
	{
		if(codefile->functions[0].code == NULL)
			fun = &codefile->functions[num_fun+1];
		else
			fun = &codefile->functions[num_fun];
	}

	fun->name     = (u8*)str_copynew(name);
	fun->name_len = name_len;
	fun->codesize = codesize;
	fun->code     = (u8*)tmem_copynew(code,codesize);
	fun->type     = type;

	codefile->num_functions++;
}

void tcodefile_release(tCodeFile* codefile)
{
	int i;
	for(i=0;i<codefile->num_global_Variables; i++)
	{
		free(codefile->global_Variables[i].name);
	}

	for(i=0;i<codefile->num_functions; i++)
	{
		free(codefile->functions[i].name);
		free(codefile->functions[i].code);
	}

	free(codefile);
}

void tcodefile_toStream(tCodeFile* codefile,FILE* os)
{
	int i;
	tGlobal_Variable *var;
	tFunction         *fun;

	fwrite(&codefile->magic,sizeof(u32),1,os);
	fwrite(&codefile->magic_version,sizeof(u16),1,os);

	fwrite(&codefile->num_global_Variables,sizeof(u16),1,os);
	for(i=0; i<codefile->num_global_Variables; i++)
	{
		var = &codefile->global_Variables[i];
		fwrite(&var->type,sizeof(u8),1,os);
	}

	fwrite(&codefile->num_functions,sizeof(u16),1,os);
	for(i=0; i<codefile->num_functions; i++)
	{
		fun = &codefile->functions[i];
		fwrite(&fun->type,sizeof(u8),1,os);
		fwrite(&fun->codesize,sizeof(u32),1,os);
		fwrite(fun->code,fun->codesize,1,os);
	}

}

void tcodefile_print(tCodeFile* codefile)
{
	char msg[MAX_MSG];
	int  i;
	tGlobal_Variable *VarPtr;
	tFunction         *FunPtr;

	fprintf(g_listfp,"---------------------------------------------------\r\n");
	// print out the magic and version
	sprintf(msg,"magic: %x\r\n magic_version: %x\r\n",codefile->magic,codefile->magic_version);
	fprintf(g_listfp,msg);
	// print out the global Variables
	sprintf(msg,"num_global_Variables: %d\r\n",codefile->num_global_Variables);
	fprintf(g_listfp,msg);
	for(i=0;i<codefile->num_global_Variables;i++)
	{
		VarPtr = &codefile->global_Variables[i];
		sprintf(msg,"%d  : name: %s, type: %d \r\n",i,VarPtr->name,VarPtr->type);
		fprintf(g_listfp,msg);
	} 
	/* print out the functions */
	sprintf(msg,"num_functions: %d\r\n",codefile->num_functions);
	fprintf(g_listfp,msg);
	for(i=0; i<codefile->num_functions; i++)
	{
		FunPtr = &codefile->functions[i];
		sprintf(msg,"%d function : name: %s, type: %d codesize: %ld \r\n",i,FunPtr->name,FunPtr->type,FunPtr->codesize);
		fprintf(g_listfp,msg);
		// print the byte code of the function
		tcodefile_printBytecodes(FunPtr->code,FunPtr->codesize);
	}
	fprintf(g_listfp,"---------------------------------------------------\r\n");
	
}

void tcodefile_printBytecodes(const u8* codebuf,u32 codesize)
{
	u32 i = 0;
	u8  code;
	while(i< codesize)
	{
		fprintf(g_listfp,"%.7ld    ",i);
		code = codebuf[0];
		if(code < 0 || code >= BYTECODE_COUNT)
		{
			fprintf(g_listfp,"Unknown bytecode in printing Bytecodes in address : %d  bytecode: %d \r\n",i,code);
			return;
		}
		i += printCode(codebuf+i);
	}
}

int printCode(const u8* codebuf)
{
	int  codelength = 0;
	int  len = 0;
	int  i = 0;
	const char* format;
	u8 type = NullType;
	u8 code = *(u8*)codebuf;


	format = tcodefile_bytecodeformats[code];
	codebuf++;
	codelength++;

	/* print the byte code name */
	fprintf(g_listfp,"%s ",tcodefile_bytecodenames[code]);
	/* print the parameters */
	while(*format)
	{
		switch(*format)
		{
		case 'F':
			len = printAccessFlag(*codebuf);
			break;
		case 'T':
			len = printDataType(*codebuf);
			type = *codebuf;
			break;
		case 'I':
			fprintf(g_listfp,"%d",*(int*)(codebuf));
			len = sizeofData(IntegerType);
			break;
		case 'V':
			switch(type)
			{
			case IntegerType:
				fprintf(g_listfp,"%d",*(int*)(codebuf));
				len = sizeofData(IntegerType);
				break;
			case DoubleType:
				fprintf(g_listfp,"%.15lf",*(double*)(codebuf));
				len = sizeofData(DoubleType);
				break;
			case StringType:
				len = printString(codebuf);
				break;
			}
		}
		fprintf(g_listfp," ");
		codelength += len;
		codebuf += len;
		format++;
	}
	fprintf(g_listfp,"\r\n");
	return codelength;
}

int printString(const u8* codebuf)
{
	char str[MAX_CHAR];
	u16  length = *(u16*)codebuf;
	
	memcpy(str,codebuf+2,length);
	str[length] = '\0';
	fprintf(g_listfp,"'");
	fprintf(g_listfp,str);
	fprintf(g_listfp,"'");

	return length+2;
}

int printDataType(u8 type)
{
	switch(type)
	{
	case NullType:
		fprintf(g_listfp,"NullType");
		break;
	case IntegerType:
		fprintf(g_listfp,"IntegerType");
		break;
	case ShortType:
		fprintf(g_listfp,"ShortType");
		break;
	case FloatType:
		fprintf(g_listfp,"FloatType");
		break;
	case StringType:
		fprintf(g_listfp,"StringType ");
		break;
	case TypeType:
		fprintf(g_listfp,"TypeType");
		break;
	case PtrType:
		fprintf(g_listfp,"PtrType");
		break;
	case ByteType:
		fprintf(g_listfp,"ByteType");
		break;
	case DoubleType:
		fprintf(g_listfp,"DoubleType");
		break;
	case DateType:
		fprintf(g_listfp,"DateType");
		break;
	default:
		fprintf(g_listfp,"Unknown Data Type");
		break;
	}
	return 1;
}

int printAccessFlag(u8 flag)
{
	switch(flag)
	{
	case ACCESS_FLAG_GLOBAL:
		fprintf(g_listfp,"global");
		break;
	case ACCESS_FLAG_LOCAL:
		fprintf(g_listfp,"local");
		break;
	}
	return 1;
}

