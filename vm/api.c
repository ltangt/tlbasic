#include "api.h"
#include "api_database.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tmem.h"
#include "tstring.h"



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


const tNativeFunPtr  API_FuncTab[] = {
	api_get,
	api_set,

	api_io_print_int,
	api_io_print_str,
	api_io_print_double,
	api_io_print_char,
	api_io_print_newline,
	
	api_io_input_int,
	api_io_input_str,
	api_io_input_double,
	api_io_input_char,

	api_io_file_open,
	api_io_file_read,
	api_io_file_write,
	api_io_file_close,
	api_io_file_length,

	// 库操作
	api_db_open,
	api_db_close,

	// 记录的操作
	api_db_recordcount,
	api_db_recordNo,
	api_db_delete,
	api_db_update,
	api_db_blank,
	api_db_appendblank,
	api_db_append,
	api_db_moveto,
	api_db_movenext,
	api_db_movelast,
	api_db_moveFirst,
	api_db_eof,
	api_db_isdelete,

	api_db_getmemo,
	api_db_updatememo,

	api_db_locate,

	api_db_seek,
	api_db_count,
	api_db_sum,
};


/*=========================================================================*/
// Native Functions

// get and set
static void api_get()
{
	u8*   par2_param = tNativeFunction_PopParamAsString();
	Database *par1_db   = tNativeFunction_PopParamAsPtr();
		
	dbFieldValue getFiledValue = db_get_filed_col(par2_param,par1_db,par1_db->RecordNo,1);
	switch (getFiledValue.type)
	{
	case 'C':
	case 'N':
		tNativeFunction_PushReturnAsString(getFiledValue.value.pstr_val);
		break;
	case 'D':
		tNativeFunction_PushReturnAsString(getFiledValue.value.date_val);
		break;
	case 'I':
		tNativeFunction_PushReturnAsInteger(getFiledValue.value.int_val);
		break;
	//case 'N':
	case 'F':
		tNativeFunction_PushReturnAsDouble(getFiledValue.value.double_val);
		break;
	case 'L':
		tNativeFunction_PushReturnAsInteger(getFiledValue.value.int_val);
		break;
	default:
		tNativeFunction_PushReturnAsNull();
		break;
	}
}

static void api_set()
{
	u8*   par3_param = tNativeFunction_PopParamAsString();
	Database *par2_db   = tNativeFunction_PopParamAsPtr();
	s32		par1_value_integer;
	u8*   par1_value_string;
	f64		par1_value_double;
		
	dbFieldValue getFieldType = db_get_filed_col(par3_param,par2_db,par2_db->RecordNo,0);
	
	switch (getFieldType.type)
	{
	case 'I':
		par1_value_integer = tNativeFunction_PopParamAsInteger();
		db_set_integer(par3_param,par2_db,par1_value_integer);
		break;
	case 'C':
	//case 'N':
		par1_value_string = tNativeFunction_PopParamAsString();
		db_set_string(par3_param,par2_db,par1_value_string);
		break;
	case 'D':
		par1_value_string = tNativeFunction_PopParamAsString();
		db_set_date(par3_param,par2_db,par1_value_string);
		break;
	case 'N':
		par1_value_string  = tNativeFunction_PopParamAsString();
		db_set_numberic(par3_param,par2_db,par1_value_string);
		break;
	case 'F':
		par1_value_double  = tNativeFunction_PopParamAsDouble();
		db_set_double(par3_param,par2_db,par1_value_double);
		break;
	case 'L':
		par1_value_integer = tNativeFunction_PopParamAsInteger();
		db_set_boolean(par3_param,par2_db,par1_value_integer);
		break;
	default:
		printf("字段%s出错\n",par3_param);
		tNativeFunction_PopParamAsPtr();
		break;
	}
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
	mem_free(Param1);
	tNativeFunction_PushReturnAsNull();
}

static void api_io_print_double()
{
	f64 Param1 = tNativeFunction_PopParamAsDouble();
	printf("%.6lf",Param1);
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
	u8* str = mem_alloc(MAX_CHAR);
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
