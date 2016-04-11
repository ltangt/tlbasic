#ifndef _TLBASIC_API_DATABASE_H_
#define _TLBASIC_API_DATABASE_H_

#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _FIELD
{        
    u8 Name[11];     /* 字段名称 */
    u8 Type;         /* 字段类型 */
    s16 Offset;      /*字段偏移*/
	s16 Segment;
    u8 Len;          /*字段长度*/
    u8 DecLen;       /*小数点长度*/
}FIELD;


typedef struct _DATABASE
{
    FILE	*fp;			/*文件指针*/
	s32		RecordCount;	/*记录总数*/
    s16		HeadLen;        /*头文件长度*/
	s16		RecordLen;		/*记录长度*/
    s32		RecordNo;       /*当前记录号*/
    u8		RecordBuf[4096];/*记录缓冲*/
	u8		**Record;       /*记录*/
    s16		FieldCount;     /*字段总数*/
    FIELD	Fld[64];        /*字段描述,最大允许64个字段*/
	u8		fileName[256];	/*文件名*/
}Database;  

/*字段数据*/ 
typedef struct _dbFieldValue
{
	u8 type;   /*该字段的类型* 'C','N','I','D','M'*/  
	union      /*各种类型的数据*/           
	{
		u8		str_val[256];
		u8*		pstr_val;
		s32		int_val;
		s32		memo_val;
		f64		double_val;
		u8		date_val[11];
	}value;
	u16 len,dot;    /*长度和小数长度*/ 
	u8	dir;
}dbFieldValue;               

#define false 0
#define true 1
//////////////////////////////////////////
void api_db_open(void);
void api_db_close(void);

void api_db_recordcount(void);
void api_db_recordNo(void);
void api_db_delete(void);
void api_db_update(void);
void api_db_blank(void);
void api_db_appendblank(void);
void api_db_append(void);
void api_db_moveto(void);
void api_db_movenext(void);
void api_db_movelast(void);
void api_db_moveFirst(void);
void api_db_eof(void);
void api_db_isdelete(void);
void api_db_locate(void);
void api_db_seek(void);
void api_db_count(void);
void api_db_sum(void);

void db_set(u8 *param, Database *db, u8 *value);

void api_db_getmemo(void);
void api_db_updatememo(void);

void db_set_string(u8 *param, Database *db, u8 *value);
void db_set_date(u8 *param, Database *db, u8 *value);
void db_set_integer(u8 *param, Database *db,s32 value);
void db_set_data(u8 *param, Database *db, u8 *value);
void db_set_double(u8 *param, Database *db, f64 value);
void db_get(u8 *param, Database *db);
dbFieldValue db_get_filed_col(u8 *param, Database *db, s32 col, s32 iget);

/*词法分析辅助程序声明*/
void apihelper_db_getword(void);
void apihelper_db_getchar(void);
void apihelper_db_getblank(void);
void apihelper_db_concat(void);
void apihelper_db_retract(void);
int  apihelper_db_isletter(void);
int  apihelper_db_isdigit(void);
int	 apihelper_db_ischletter(void);
int  apihelper_db_logicalsign(void);

/*语法分析辅助函数声明*/
dbFieldValue apihelper_db_exp(void);
dbFieldValue apihelper_db_term0(void);
dbFieldValue apihelper_db_term1(void);
dbFieldValue apihelper_db_term2(void);
dbFieldValue apihelper_db_factor(void);
dbFieldValue apihelper_db_getrecord(void);
dbFieldValue apihelper_db_getvalue(void);
dbFieldValue apihelper_db_op(dbFieldValue lop,dbFieldValue rop,u8 operate);
dbFieldValue apihelper_db_compop(dbFieldValue lop,dbFieldValue rop,u8 *isgreat);
int apihelper_db_isfiledname(u8 *fieldName);
int	apihelper_db_getfieldindex(u8 *fieldName);
int apihelper_db_getfiguredigit(double nParam);
int apihelper_db_isdeleted(int row);
int apihelper_db_isemptycondition(void);
void apihelper_db_reset(void);

#endif	 // _TLBASIC_API_H_
