#ifndef _TLBASIC_API_DATABASE_H_
#define _TLBASIC_API_DATABASE_H_

#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _FIELD
{        
    u8 Name[11];     /* �ֶ����� */
    u8 Type;         /* �ֶ����� */
    s16 Offset;      /*�ֶ�ƫ��*/
	s16 Segment;
    u8 Len;          /*�ֶγ���*/
    u8 DecLen;       /*С���㳤��*/
}FIELD;


typedef struct _DATABASE
{
    FILE	*fp;			/*�ļ�ָ��*/
	s32		RecordCount;	/*��¼����*/
    s16		HeadLen;        /*ͷ�ļ�����*/
	s16		RecordLen;		/*��¼����*/
    s32		RecordNo;       /*��ǰ��¼��*/
    u8		RecordBuf[4096];/*��¼����*/
	u8		**Record;       /*��¼*/
    s16		FieldCount;     /*�ֶ�����*/
    FIELD	Fld[64];        /*�ֶ�����,�������64���ֶ�*/
	u8		fileName[256];	/*�ļ���*/
}Database;  

/*�ֶ�����*/ 
typedef struct _dbFieldValue
{
	u8 type;   /*���ֶε�����* 'C','N','I','D','M'*/  
	union      /*�������͵�����*/           
	{
		u8		str_val[256];
		u8*		pstr_val;
		s32		int_val;
		s32		memo_val;
		f64		double_val;
		u8		date_val[11];
	}value;
	u16 len,dot;    /*���Ⱥ�С������*/ 
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

/*�ʷ�����������������*/
void apihelper_db_getword(void);
void apihelper_db_getchar(void);
void apihelper_db_getblank(void);
void apihelper_db_concat(void);
void apihelper_db_retract(void);
int  apihelper_db_isletter(void);
int  apihelper_db_isdigit(void);
int	 apihelper_db_ischletter(void);
int  apihelper_db_logicalsign(void);

/*�﷨����������������*/
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
