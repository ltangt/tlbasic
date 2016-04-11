#include "api.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*=========================================================================*/
/* set and get */
static void api_set(void);
static void api_get(void);
// io_print_*
static void api_io_print_int(void);
static void api_io_print_str(void);
static void api_io_print_double(void);
static void api_io_print_char(void);
static void api_io_print_newline(void);

// io_input_*
static void api_io_input_int(void);
static void api_io_input_str(void);
static void api_io_input_double(void);
static void api_io_input_char(void);

// io_file_*
static void api_io_file_open(void);
static void api_io_file_read(void);
static void api_io_file_write(void);
static void api_io_file_close(void);
static void api_io_file_length(void);

#define INSERT_FUNC(func,func_name) tNativeFunction_InsertFunc(func,func_name)
/*=========================================================================*/
void InitializeAPIs()
{
	INSERT_FUNC(api_get,               "get");
	INSERT_FUNC(api_set,               "set");

	INSERT_FUNC(api_io_print_int,      "printint");
	INSERT_FUNC(api_io_print_str,      "printstr");
	INSERT_FUNC(api_io_print_double,   "printdouble");
	INSERT_FUNC(api_io_print_char,     "printchar");
	INSERT_FUNC(api_io_print_newline,  "printnewline");

	INSERT_FUNC(api_io_input_int,      "inputint");
	INSERT_FUNC(api_io_input_str,      "inputstr");
	INSERT_FUNC(api_io_input_double,   "inputdouble");
	INSERT_FUNC(api_io_input_char,     "inputchar");

	INSERT_FUNC(api_io_file_open,     "fileopen");
	INSERT_FUNC(api_io_file_read,     "fileread");
	INSERT_FUNC(api_io_file_write,    "filewrite");
	INSERT_FUNC(api_io_file_close,    "fileclose");
	INSERT_FUNC(api_io_file_length,   "filelength");
}

/*=========================================================================*/
// Native Functions

// get and set
static void api_get()
{
	u8*   par2_param = tNativeFunction_PopParamAsString();
	ptr32 par1_var   = tNativeFunction_PopParamAsPtr();
	s32   ret = 0;

	ret = atoi(par2_param);

	tNativeFunction_PushReturnAsInteger(ret);
}

static void api_set()
{
	u8*   par3_param = tNativeFunction_PopParamAsString();
	ptr32 par2_var   = tNativeFunction_PopParamAsPtr();
	u8*   par1_value = tNativeFunction_PopParamAsString();

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================*/
// io_print_*
static void api_io_print_int()
{
	s32 Param1 = tNativeFunction_PopParamAsInteger();
	printf("%d",Param1);
	tNativeFunction_PushReturnAsNull();
}

static void api_io_print_str()
{
	u8* Param1 = tNativeFunction_PopParamAsString();
	printf("%s",Param1);
	tNativeFunction_PushReturnAsNull();
}

static void api_io_print_double()
{
	f64 Param1 = tNativeFunction_PopParamAsDouble();
	printf("%.12lf",Param1);
	tNativeFunction_PushReturnAsNull();
}

static void api_io_print_char()
{
	u8 Param1 = tNativeFunction_PopParamAsByte();
	printf("%c",Param1);
	tNativeFunction_PushReturnAsNull();      
}

static void api_io_print_newline()
{
	printf("\n");
	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================*/
// io_input_*
static void api_io_input_int()
{
	s32 ret;
	scanf("%d",&ret);
	tNativeFunction_PushReturnAsInteger(ret);
}

static void api_io_input_str()
{
	u8* str = malloc(MAX_CHAR);
	scanf("%s",str);
	tNativeFunction_PushReturnAsString(str);
}

static void api_io_input_double()
{
	f64 ret;
	scanf("%lf",&ret);
	tNativeFunction_PushReturnAsDouble(ret);
}

static void api_io_input_char()
{
	u8 ret[MAX_CHAR];
	scanf("%s",ret);
	//ret = getch();
	tNativeFunction_PushReturnAsByte(ret[0]);
}

/*=========================================================================*/
// io_file_*
static void api_io_file_open()
{
	ptr32 ret;
	u8*   par1_filename;
	s32   par2_mode;
	
	par2_mode = tNativeFunction_PopParamAsInteger();
	par1_filename = tNativeFunction_PopParamAsString();

	if(par2_mode == 0) // read mode
		ret = (ptr32)fopen(par1_filename,"rb");
	else // writer mode
		ret = (ptr32)fopen(par1_filename,"wb");
	tNativeFunction_PushReturnAsType(ret);
}

static void api_io_file_read()
{
	FILE *par1_file;
	u8*   par2_buf;
	s32   par3_offset;
	s32   par4_length;

	par4_length = tNativeFunction_PopParamAsInteger();
	par3_offset = tNativeFunction_PopParamAsInteger();
	par2_buf    = tNativeFunction_PopParamAsPtr();
	par1_file   = tNativeFunction_PopParamAsPtr();

	fread(par2_buf+par3_offset,par4_length,1,par1_file);

	tNativeFunction_PushReturnAsNull();
}

static void api_io_file_write()
{
	FILE *par1_file;
	u8*   par2_buf;
	s32   par3_offset;
	s32   par4_length;

	par4_length = tNativeFunction_PopParamAsInteger();
	par3_offset = tNativeFunction_PopParamAsInteger();
	par2_buf    = tNativeFunction_PopParamAsPtr();
	par1_file   = tNativeFunction_PopParamAsPtr();

	fwrite(par2_buf+par3_offset,par4_length,1,par1_file);

	tNativeFunction_PushReturnAsNull();
}

static void api_io_file_close()
{
	FILE *par_file;
	
	par_file = tNativeFunction_PopParamAsPtr();
	fclose(par_file);

	tNativeFunction_PushReturnAsNull();
}

static void api_io_file_length()
{
	FILE *par_file;
	s32   ret_length;
	s32   cur_seek;

	par_file = tNativeFunction_PopParamAsPtr();
	cur_seek = ftell(par_file);
	fseek(par_file,0L,SEEK_END);
	ret_length = ftell(par_file);
	fseek(par_file,cur_seek,SEEK_SET);

	tNativeFunction_PushReturnAsInteger(ret_length);
}
