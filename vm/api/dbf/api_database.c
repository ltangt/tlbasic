#include "api_database.h"

/*=========================================================================
* FUNCTION:      api_db_open
* TYPE:          function 
* OVERVIEW:      打开数据库，分配空间，读取信息 
                 Function DBOpen(DBFileName as string) as Database
*   parameters:  数据库名字 
*   returns:     数据库指针 ，若操作失败，返回NULL 
*=======================================================================*/
 void api_db_open()
{
	Database* db;
	u8*   par_filename;
	int i;
		
	par_filename   = tNativeFunction_PopParamAsString();

	db = malloc(sizeof(Database) );	
	
	strcpy(db->fileName,par_filename);

	db->fp = fopen(par_filename,"rb+");
	
	/*跳过版本 年月日*/
	fseek(db->fp,4,SEEK_SET);			
    
	/*记录总数*/
	fread(&db->RecordCount,sizeof(s32),1,db->fp);	
    
    /*当前记录为1*/ 
	db->RecordNo = 1;
	
    /*头文件长*/
	fread(&db->HeadLen,sizeof(s16),1,db->fp);
	
    /*记录长*/
	fread(&db->RecordLen,sizeof(s16),1,db->fp);
	db->FieldCount=(db->HeadLen/32)-1;
	
	fseek(db->fp, db->HeadLen,SEEK_SET);
	/*分配65536个记录指针*/ 
	db->Record = malloc(sizeof(u8 *) * 65536);
	/*为每个记录分配空间*/
    for (i=0;i<(int)(db->RecordCount);i++)
	{
		db->Record[i] = malloc(sizeof(u8) * db->RecordLen);
		fread(db->Record[i],db->RecordLen,1,db->fp);
	}
    
	/*处理字段描述*/
    for(i=0;i<(int)(db->FieldCount);i++)
    {
        fseek(db->fp,32*(i+1),SEEK_SET);           
        fread(db->Fld[i].Name,sizeof(u8),11,db->fp);
        if (db->Fld[i].Name[0]==0x0D)
        {
           db->FieldCount=i;
           break;
        }
        fread(&db->Fld[i].Type,sizeof(u8),1,db->fp);
		fread(&db->Fld[i].Offset,sizeof(s16),1,db->fp);
		fread(&db->Fld[i].Segment,sizeof(s16),1,db->fp);
		fread(&db->Fld[i].Len,sizeof(u8),1,db->fp);		
		fread(&db->Fld[i].DecLen,sizeof(u8),1,db->fp);
	}
    /*将记录缓冲清空，其中第一个字符设置为空格 */ 
    memset(db->RecordBuf,32,4096);
	        
	tNativeFunction_PushReturnAsType(db);
}

/*=========================================================================
* FUNCTION:      api_db_close
* TYPE:          function 
* OVERVIEW:      写回信息，关闭数据库
                 Function DBClose(Db as Database)
*   parameters:  数据库指针
*   returns:     
*=======================================================================*/
 void api_db_close()
{
	Database *par_db;
	int i;
	u8 temp;
 
    par_db = tNativeFunction_PopParamAsPtr();
	
	/*把数据写回*/
	/* 跳过版本 年月日*/
	fseek(par_db->fp,4,SEEK_SET);
	fwrite(&par_db->RecordCount,sizeof(s32),1,par_db->fp);
	fwrite(&par_db->HeadLen,sizeof(s16),1,par_db->fp);
	fwrite(&par_db->RecordLen,sizeof(s16),1,par_db->fp);

	/*写回字段*/
	for(i=0;i<(int)(par_db->FieldCount);i++)
    {
        fseek(par_db->fp,32*(i+1),SEEK_SET);             
		fwrite(&par_db->Fld[i].Name,sizeof(u8),11,par_db->fp);
		fwrite(&par_db->Fld[i].Type,sizeof(u8),1,par_db->fp);
		fwrite(&par_db->Fld[i].Offset,sizeof(s16),1,par_db->fp);
		fwrite(&par_db->Fld[i].Segment,sizeof(s16),1,par_db->fp);
		fwrite(&par_db->Fld[i].Len,sizeof(u8),1,par_db->fp);
		fwrite(&par_db->Fld[i].DecLen,sizeof(u8),1,par_db->fp);
	}

	/*写回记录*/
	fseek(par_db->fp,par_db->HeadLen,SEEK_SET);		
	for (i=0;i<(int)(par_db->RecordCount);i++)
	{      
		fwrite(par_db->Record[i],par_db->RecordLen,1,par_db->fp);
	}
	/*写结束标志 0x1A */ 
	temp = 0x1A;
	fwrite(&temp,1,1,par_db->fp);

    /*关闭数据库*/ 
	fclose(par_db->fp);
	/*释放指针*/ 
	for (i=0; i<(int)(par_db->RecordCount); i++)
		free(par_db->Record[i]);
	free(par_db->Record);
	free(par_db);

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_recordcount
* TYPE:          function 
* OVERVIEW:      到数据库中有效记录个数
*				 Function DBRecordCount(Db as Database) as Integer
*   parameters:  数据库指针
*   returns:     有效记录个数-类型u32 
*=======================================================================*/
 void api_db_recordcount()
{
	Database *par_db;
	u32 recordcount;
 
    par_db = tNativeFunction_PopParamAsPtr();
	
	recordcount = par_db->RecordCount;

	tNativeFunction_PushReturnAsInteger(recordcount);
}

/*=========================================================================
* FUNCTION:      api_db_recordNo
* TYPE:          function 
* OVERVIEW:      得到当前记录号，以1开始
                 Function DBRecordNo(Db as Database) as Integer
*   parameters:  数据库指针
*   returns:     当前记录号-类型u32 
*=======================================================================*/
 void api_db_recordNo()
{
	Database *par_db;
	u32 recordNo;
 
    par_db = tNativeFunction_PopParamAsPtr();
	
	recordNo = par_db->RecordNo;

	tNativeFunction_PushReturnAsInteger(recordNo);
}

/*=========================================================================
* FUNCTION:      api_db_delete
* TYPE:          function 
* OVERVIEW:      删除当前记录
				 Function DBDelete(Db as Database)
*   parameters:  数据库指针
*   returns:     
*=======================================================================*/
 void api_db_delete()
{
	Database *par_db;
	 
    par_db = tNativeFunction_PopParamAsPtr();
	//par_db->RecordCount--;
	
	memset(par_db->Record[par_db->RecordNo-1],'*',1);
	
	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_update
* TYPE:          function 
* OVERVIEW:      将记录缓冲区内容更新到当前记录
				 Function DBUpdate(Db as Database) as boolean
*   parameters:  数据库指针
*   returns:     1 - treu; 0 - false
*=======================================================================*/
 void api_db_update()
{
	Database *par_db;
		 
    par_db = tNativeFunction_PopParamAsPtr();
    if (par_db == NULL)
    {
       tNativeFunction_PushReturnAsInteger(0); 
       return; 
    }
    	
	memcpy(par_db->Record[par_db->RecordNo-1],par_db->RecordBuf,par_db->RecordLen);
		
	tNativeFunction_PushReturnAsInteger(1);
}

/*=========================================================================
* FUNCTION:      api_db_blank
* TYPE:          function 
* OVERVIEW:      将记录缓冲区设置为0 (清除)
				 Function DBBlank(Db as Database)
*   parameters:  数据库指针
*   returns:     
*=======================================================================*/
 void api_db_blank()
{
	Database *par_db;
	 
    par_db = tNativeFunction_PopParamAsPtr();
	
	memset(par_db->RecordBuf,32,4096);

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_appendblank
* TYPE:          function 
* OVERVIEW:      在数据库尾添加一个空记录，并把记录缓冲区设置为空
				 Function DBAppendBlank(Db as Database) as boolean
*   parameters:  数据库指针
*   returns:     1 - treu; 0 - false
*=======================================================================*/
 void api_db_appendblank()
{
	Database *par_db;

	par_db = tNativeFunction_PopParamAsPtr(); 
    if (par_db == NULL)
    {
       tNativeFunction_PushReturnAsInteger(0); 
       return; 
    }
    
	par_db->Record[par_db->RecordCount] = malloc( sizeof(u8) * par_db->RecordLen);
	 if (par_db->Record[par_db->RecordCount] == NULL)
    {
       tNativeFunction_PushReturnAsInteger(0); 
       return; 
    }
    memset(par_db->Record[par_db->RecordCount],0,par_db->RecordLen);
	par_db->RecordCount ++ ;
	
	memset(par_db->RecordBuf,32,4096);
	
	tNativeFunction_PushReturnAsInteger(1);
}

/*=========================================================================
* FUNCTION:      api_db_append
* TYPE:          function 
* OVERVIEW:      用当前缓冲区的内容在数据库尾添加一个记录
				 Function DBAppend(Db as Database) as boolean
*   parameters:  数据库指针
*   returns:     1 - treu; 0 - false
*=======================================================================*/
 void api_db_append()
{
	Database *par_db;

	par_db = tNativeFunction_PopParamAsPtr(); 
	if (par_db == NULL)
    {
       tNativeFunction_PushReturnAsInteger(0); 
       return; 
    }
    
	par_db->Record[par_db->RecordCount] = malloc( sizeof(u8) * par_db->RecordLen);
	 if (par_db->Record[par_db->RecordCount] == NULL)
    {
       tNativeFunction_PushReturnAsInteger(0); 
       return; 
    }
    memset(par_db->Record[par_db->RecordCount],0,par_db->RecordLen);
	par_db->RecordCount ++ ;
	
	memcpy(par_db->Record[par_db->RecordCount-1],par_db->RecordBuf,par_db->RecordLen);

	tNativeFunction_PushReturnAsInteger(1);
}

/*=========================================================================
* FUNCTION:      api_db_moveto
* TYPE:          function 
* OVERVIEW:      记录指针移动到指定位置
				 Function DBMoveTo(Db as Database, RecordNum as Integer)
*   parameters:  数据库指针, 指定的位置
*   returns:     
*=======================================================================*/
 void api_db_moveto()
{
	Database *par_db;
	s32		par_recordnum;

	par_recordnum = tNativeFunction_PopParamAsInteger();
	par_db = tNativeFunction_PopParamAsPtr(); 
    
	par_db->RecordNo = par_recordnum;	

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_movenext
* TYPE:          function 
* OVERVIEW:      记录指针移动到下一个记录
				 Function DBMoveNext(Db as Database)
*   parameters:  数据库指针
*   returns:     
*=======================================================================*/
 void api_db_movenext()
{
	Database *par_db;

	par_db = tNativeFunction_PopParamAsPtr(); 
    
	par_db->RecordNo++;	

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_movelast
* TYPE:          function 
* OVERVIEW:      记录指针移动到上一个记录
				 Function DBMoveLast(Db as Database)
*   parameters:  数据库指针
*   returns:     
*=======================================================================*/
 void api_db_movelast()
{
	Database *par_db;
	
	par_db = tNativeFunction_PopParamAsPtr(); 
    
	par_db->RecordNo--;	

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_moveFirst
* TYPE:          function 
* OVERVIEW:      记录指针移动到数据库第一个记录
				 Function DBMoveFirst(Db as Database)
*   parameters:  数据库指针
*   returns:     
*=======================================================================*/
 void api_db_moveFirst()
{
	Database *par_db;
	
	par_db = tNativeFunction_PopParamAsPtr(); 
    
	par_db->RecordNo = 1;	

	tNativeFunction_PushReturnAsNull();
}

/*=========================================================================
* FUNCTION:      api_db_eof
* TYPE:          function 
* OVERVIEW:      当前记录指针是否已经指到数据库头（第一个记录前）或数据库尾（最后一个记录后）
				 Function DBEof(Db as Database) as boolean
*   parameters:  数据库指针
*   returns:     1 - treu; 0 - false
*=======================================================================*/
 void api_db_eof()
{
	Database *par_db;
	s32 re;

	par_db = tNativeFunction_PopParamAsPtr(); 
    
	if (par_db->RecordNo <= 0 || par_db->RecordNo > par_db->RecordCount )
		re = 1;
	else re = 0;
	

	tNativeFunction_PushReturnAsInteger(re);
}

/*=========================================================================
* FUNCTION:      api_db_isdelete
* TYPE:          function 
* OVERVIEW:      检查当前记录是否被删除
				 DBIsDelete(Db as Database) as boolean
*   parameters:  数据库指针
*   returns:     1 - treu; 0 - false
*=======================================================================*/
 void api_db_isdelete()
{
	Database *par_db;
	s32 re;

	par_db = tNativeFunction_PopParamAsPtr(); 
    
	if (par_db->Record[par_db->RecordNo - 1][0] == '*')
		re = 1;
	else re = 0;

	tNativeFunction_PushReturnAsInteger(re);
}

/*=========================================================================
* FUNCTION:      db_set_string
* TYPE:          function 
* OVERVIEW:      用指定的字符串 设置指定的 C类型 的字段值 

*   parameters:  字段名,数据库指针,值
*   returns:     
*=======================================================================*/
void db_set_string(u8 *param, Database *db, u8 *value)
{
	int i;
	s16 j = 1;

	/*找到指定的字段,如果没有,直接返回*/
	for (i=0;i<db->FieldCount;i++)
	{
		if (strcmp(db->Fld[i].Name,strupr(param)) == 0)
			break;
		j += db->Fld[i].Len;
	}
	if (i == db->FieldCount)
		return;
	memset(&db->RecordBuf[j],32,db->Fld[i].Len);
	if ( strlen(value) > db->Fld[i].Len)
	{
		memcpy(&db->RecordBuf[j],value,db->Fld[i].Len);
	}
	else
	{
		memcpy(&db->RecordBuf[j],value,strlen(value));
	}
}

/*=========================================================================
* FUNCTION:      db_set_date
* TYPE:          function 
* OVERVIEW:      用指定的日期字符串 设置指定的 D类型 的字段值 

*   parameters:  字段名,数据库指针,值
*   returns:     
*=======================================================================*/
void db_set_date(u8 *param, Database *db, u8 *value)
{
	int i;
	s16 j = 1;
	
	/*找到指定的字段,如果没有,直接返回*/
	for (i=0;i<db->FieldCount;i++)
	{
		if (strcmp(db->Fld[i].Name,strupr(param)) == 0)
			break;
		j += db->Fld[i].Len;
	}
	if (i == db->FieldCount)
		return;
	memset(&db->RecordBuf[j],32,db->Fld[i].Len);
	/* 把值 赋值到 缓冲里面,考虑了月份,日 是否带0的情况 */
	/*把 年 赋值到 缓冲的前4个字节*/
	memcpy(&db->RecordBuf[j],value,4);
	
	if (value[7] == '/')
	{
		/* 月 里面有两个数字 */
		db->RecordBuf[j+4] = value[5];
		db->RecordBuf[j+5] = value[6];
		if (value[9] == '\0')
		{
			/* 日 里面只有一个数字 */
			db->RecordBuf[j+6] = '0';
			db->RecordBuf[j+7] = value[8];
		}
		else
		{
			/* 日 里面有两个数字 */
			db->RecordBuf[j+6] = value[8];
			db->RecordBuf[j+7] = value[9];
		}
	}
	else
	{
		/* 月 里面只有一个数字*/
		db->RecordBuf[j+4] = '0';
		db->RecordBuf[j+5] = value[5];
		if (value[8] == '\0')
		{
			/* 日 里面只有一个数字 */
			db->RecordBuf[j+6] = '0';
			db->RecordBuf[j+7] = value[7];
		}
		else
		{
			/* 日 里面有两个数字 */
			db->RecordBuf[j+6] = value[7];
			db->RecordBuf[j+7] = value[8];
		}
	}
	
}

/*=========================================================================
* FUNCTION:      db_set_integer
* TYPE:          function 
* OVERVIEW:      用指定的整数 设置指定的 I类型 的字段值 

*   parameters:  字段名,数据库指针,值
*   returns:     
*=======================================================================*/
void db_set_integer(u8 *param, Database *db, s32 value)
{
	int i;
	s16 j = 1;
	
	/*找到指定的字段,如果没有,直接返回*/
	for (i=0;i<db->FieldCount;i++)
	{
		if (strcmp(db->Fld[i].Name,strupr(param)) == 0)
			break;
		j += db->Fld[i].Len;
	}
	if (i == db->FieldCount)
		return;
	memset(&db->RecordBuf[j],0,db->Fld[i].Len);
	memcpy(&db->RecordBuf[j],&value,sizeof(value));
}

/*=========================================================================
* FUNCTION:     db_set_double
* TYPE:          function 
* OVERVIEW:      用指定的f64类型值 设置指定的 N类型 的字段值 

*   parameters:  字段名,数据库指针,值
*   returns:     有效记录个数 类型u32 
*=======================================================================*/
void db_set_double(u8 *param, Database *db, f64 value)
{
	int i;
	s16 j = 1;
	int k;
	u8 str_double[256];	/*把数值转换成字符串*/
	
	int istrIntLen;		/*指定的值 的长度*/
	int istrDecLen;		/*指定的值 的小数部分的长度*/
	
	int iFieldIntLen;	/*字段里面小数部分的长度*/

	/*找到指定的字段,如果没有,直接返回*/
	for (i=0;i<db->FieldCount;i++)
	{
		if (strcmp(db->Fld[i].Name,strupr(param)) == 0)
			break;
		j += db->Fld[i].Len;
	}
	if (i == db->FieldCount)
		return;

	/*检查是否有小数点*/
	if (db->Fld[i].DecLen == 0)
		iFieldIntLen = db->Fld[i].Len;
	else
		iFieldIntLen = db->Fld[i].Len - db->Fld[i].DecLen-1;

	/*把数值转换成字符串*/
	sprintf(str_double,"%lf",value);	
	istrIntLen = strcspn(str_double,".");	
	istrDecLen = strlen(str_double) - istrIntLen - 1;	
	
	if (istrIntLen > iFieldIntLen)
	{
		/*如果指定值的长度 比 字段的长度 大,只取字段的长度*/
		for (k = 1; k<=iFieldIntLen; k++)
			db->RecordBuf[j+iFieldIntLen-k] = str_double[istrIntLen - k];
	}
	else
	{
		/*如果指定值的长度 比 字段的长度 小,多余的用 ' '空格填充 */
		for (k = 1; k<=istrIntLen; k++)
			db->RecordBuf[j+iFieldIntLen-k] = str_double[istrIntLen - k];
		for (; k <= iFieldIntLen; k++)
			db->RecordBuf[j+iFieldIntLen-k] = ' ';
	}

	if (istrDecLen > db->Fld[i].DecLen)
	{
		/*如果指定值的小数部分长度 比 字段的小数部分长度 大,只取字段的小数部分长度*/
		for (k=1 ;k <= db->Fld[i].DecLen; k++)
			db->RecordBuf[j+iFieldIntLen+k] = str_double[istrIntLen+k];
	}
	else
	{
		/*如果指定值的小数部分长度 比 字段的小数部分长度 小,多余的用'0'填充*/
		for (k=1 ;k <= istrDecLen; k++)
			db->RecordBuf[j+iFieldIntLen+k] = str_double[istrIntLen+k];
		for (; k <= db->Fld[i].DecLen; k++)
			db->RecordBuf[j+iFieldIntLen+k] = '0';
	}
	/*设置小数点*/
	if (db->Fld[i].DecLen != 0)
		db->RecordBuf[j+iFieldIntLen] = '.';
}

/*=========================================================================
* FUNCTION:      db_get_filed_col
* TYPE:          function 
* OVERVIEW:      得到指定的的字段的值 

*   parameters:  字段名,数据库指针,行号(以1开始),是否需要的到值
*   returns:     dbFieldValue类型 
*=======================================================================*/
dbFieldValue db_get_filed_col(u8 *param, Database *db, s32 col, s32 iget)
{
	dbFieldValue reFiledValue;
	s16 i;
	s16 j = 1;
	u8 temp[256];

//	reFiledValue.value.str_val = NULL;
	
	/*找到指定的字段,如果没有,返回 全为0 的dbFieldValue*/
	for (i=0;i<db->FieldCount;i++)
	{
		if (strcmp(db->Fld[i].Name,strupr(param)) == 0)
			break;
		j += db->Fld[i].Len; 
	}
	if (i == db->FieldCount)
	{
		memset(&reFiledValue,0,sizeof(dbFieldValue));
		return reFiledValue;
	}
	reFiledValue.type = db->Fld[i].Type;
	if (iget)
	{
		switch (db->Fld[i].Type)
		{
		case 'C':
		case 'N':
			reFiledValue.value.pstr_val = malloc( db->Fld[i].Len + 1);
			memset(reFiledValue.value.pstr_val,0,db->Fld[i].Len + 1);
			memcpy(reFiledValue.value.pstr_val,&db->Record[col-1][j],db->Fld[i].Len);
			reFiledValue.value.str_val[db->Fld[i].Len] = '\0';
			break;
		case 'D':
			memcpy(reFiledValue.value.date_val,&db->Record[col-1][j],4);
			reFiledValue.value.date_val[4] = '/';
			reFiledValue.value.date_val[5] = db->Record[col-1][j+4],
			reFiledValue.value.date_val[6] = db->Record[col-1][j+5],
			reFiledValue.value.date_val[7] = '/',
			reFiledValue.value.date_val[8] = db->Record[col-1][j+6],
			reFiledValue.value.date_val[9] = db->Record[col-1][j+7],
			reFiledValue.value.date_val[10] = '\0';
			break;
		case 'I':
			memcpy(&reFiledValue.value.int_val,&db->Record[col-1][j],db->Fld[i].Len);
			break;
		case 'M':
			memcpy(&reFiledValue.value.memo_val,&db->Record[col-1][j],db->Fld[i].Len);
			break;
		//case 'N':
		case 'F':
			memcpy(temp,&db->Record[col-1][j],db->Fld[i].Len);
			temp[db->Fld[i].Len] = '\0';
			reFiledValue.value.double_val = atof(temp);
			break;
		default:
			break;
		}
	}

	return reFiledValue;
}

/*=========================================================================
* FUNCTION:      api_db_getmemo
* TYPE:          function 
* OVERVIEW:      得到当前记录指定的备注字段的内容
				 DBGetMemo(Db as Database, Field as string) as Byte[]
*   parameters:  数据库指针,字段名
*   returns:     指定字段的内容-字符串类型
*=======================================================================*/
void api_db_getmemo()
{
	u8 *par_strfield;
	Database *par_db;
	
	dbFieldValue dbGet;
	FILE *fpCur;
	u8 fptFileName[256];	/*fpt文件的名字*/
	u8 tempSrc[4];			/* 用与交换读取的值的顺序,因为在fpt文件里面,数字存储顺序是反的 */
	u8 tempDest[4];
	s32 iPerLen;			/*每块的长度*/
	s32 iLen;				/*要返回的字符串的长度*/
	u8 *strRe;

	par_strfield = tNativeFunction_PopParamAsString();
	par_db = tNativeFunction_PopParamAsPtr();

	/*从dbf里面得到该字段在fpt文件中的位置*/
	dbGet = db_get_filed_col(par_strfield,par_db,par_db->RecordNo,1);
	
	/*打开文件*/
	memcpy(fptFileName,par_db->fileName,strlen(par_db->fileName) - 4);
	fptFileName[strlen(par_db->fileName) - 4] = '\0';
	strcat(fptFileName,".fpt");
	
	fpCur = fopen(fptFileName,"rb+");

	/*读取长度*/
	
	fseek(fpCur,4,SEEK_SET);

	fread(tempSrc,4,1,fpCur);
	tempDest[0] = tempSrc[3];
	tempDest[1] = tempSrc[2];
	tempDest[2] = tempSrc[1];
	tempDest[3] = tempSrc[0];
	memcpy(&iPerLen,tempDest,4);

	/*定位到指定地方*/
	fseek(fpCur,iPerLen*dbGet.value.memo_val,SEEK_SET);

	/*读取个数*/
	fseek(fpCur,4,SEEK_CUR);
	fread(tempSrc,4,1,fpCur);
	tempDest[0] = tempSrc[3];
	tempDest[1] = tempSrc[2];
	tempDest[2] = tempSrc[1];
	tempDest[3] = tempSrc[0];
	memcpy(&iLen,tempDest,4);
	iLen ++;

	/*读取数据*/
	strRe = malloc(iLen);
	fread(strRe,iLen,1,fpCur);
	//strRe[iLen] = '\0';
	
	fclose(fpCur);
	
	/*返回数据*/ 
	//tNativeFunction_PushReturnAsString(strRe);
	tNativeFunction_PushReturnAsPtr(strRe);
}

/*=========================================================================
* FUNCTION:      api_db_updatememo
* TYPE:          function 
* OVERVIEW:      更新当前记录指定的备注字段的内容
				 DBUpdateMemo(Db as Database, Field as string, Memo as Byte[], Count as Integer) as boolean
*   parameters:  数据库指针,字段名,指定的内容
*   returns:     1 - treu; 0 - false
*=======================================================================*/
void api_db_updatememo()
{
	u8 *par_strfield;
	Database *par_db;
	u8 *par_memo;
	s32	par_count;
	
	FILE *fpCur;
	
	u8 fptFileName[256];	/*fpt文件的名字*/
	u8 tempSrc[4];			/* 用与交换读取的值的顺序,因为在fpt文件里面,数字存储顺序是反的 */
	u8 tempDest[4];
	s32 iPerLen;			/*每块的长度*/
	s32 iTotalNum;			/*块数*/
	s16 i;
	s16 j = 1;
	
	s32 itemp;
	s32 iStringLen = 0;		/*要写入的字符串的长度*/
	s32 iStringNum;			/*要写入的字符串占用的块数*/
	u8 utemp;
	
	par_count = tNativeFunction_PopParamAsInteger();
	par_memo = tNativeFunction_PopParamAsPtr();
	par_strfield = tNativeFunction_PopParamAsString();
	par_db = tNativeFunction_PopParamAsPtr();
	
	/*打开文件*/
	memcpy(fptFileName,par_db->fileName,strlen(par_db->fileName) - 4);
	fptFileName[strlen(par_db->fileName) - 4] = '\0';
	strcat(fptFileName,".fpt");
	
	fpCur = fopen(fptFileName,"rb+");

	/*读取个数*/	
	fread(tempSrc,4,1,fpCur);
	tempDest[0] = tempSrc[3];
	tempDest[1] = tempSrc[2];
	tempDest[2] = tempSrc[1];
	tempDest[3] = tempSrc[0];
	memcpy(&iTotalNum,tempDest,4);

	/*写入dbf地址*/
	for (i=0;i<par_db->FieldCount;i++)
	{
		if (strcmp(par_db->Fld[i].Name,strupr(par_strfield)) == 0)
			break;
		j += par_db->Fld[i].Len; 
	}
	if (i == par_db->FieldCount)
	{
		tNativeFunction_PushReturnAsInteger(0);
		return ;
	}
	memcpy(&par_db->Record[par_db->RecordNo-1][j],&iTotalNum,sizeof(s32));
	
	/*读取长度*/
	fread(tempSrc,4,1,fpCur);
	tempDest[0] = tempSrc[3];
	tempDest[1] = tempSrc[2];
	tempDest[2] = tempSrc[1];
	tempDest[3] = tempSrc[0];
	memcpy(&iPerLen,tempDest,4);

	/*定位到指定地方*/
	fseek(fpCur,iPerLen*iTotalNum+3,SEEK_SET);
	itemp = 1;
	fwrite(&itemp,1,1,fpCur);

	
	//iStringLen = strlen(par_memo);
	iStringLen = par_count;
	iStringNum = (s32) (iStringLen / iPerLen);
	iStringNum ++ ;

	/*写入个数*/
	memcpy(tempSrc,&iStringLen,sizeof(int));
	tempDest[0] = tempSrc[3];
	tempDest[1] = tempSrc[2];
	tempDest[2] = tempSrc[1];
	tempDest[3] = tempSrc[0];
	fwrite(tempDest,4,1,fpCur);
	
	/*写入字符,多余的位置写入0 */
	utemp = 0;
	fwrite(par_memo,iStringLen,1,fpCur);
	fseek(fpCur,((iStringNum)*iPerLen - iStringLen - 9), SEEK_CUR);
	fwrite(&utemp,1,1,fpCur);
	
	/*写入长度*/
	fseek(fpCur,0,SEEK_SET);
	iTotalNum += iStringNum;
	memcpy(tempSrc,&iTotalNum,sizeof(int));
	tempDest[0] = tempSrc[3];
	tempDest[1] = tempSrc[2];
	tempDest[2] = tempSrc[1];
	tempDest[3] = tempSrc[0];
	fwrite(tempDest,4,1,fpCur);

	fclose(fpCur);	
	
	tNativeFunction_PushReturnAsInteger(1);
}
///////////////////////////////////////
/*外部变量申明，在与条件查询相关的函数
*及其辅助函数中使用
*/
u8			*api_db_ext_Condition;					/*字符指针，指向条件语句*/
Database	*api_db_ext_Database;					/*结构体指针，指向当前打开的数据库*/

u8		api_db_ext_Letter;							/*字符变量，存放当前扫描到的字符*/
u8		api_db_ext_Operant[2];						/*字符数组，存放当前扫描到的运算符，
													 *单字节运算符（'+'，'-'，'='...）存放在数组第零号位置
													 *双字节运算符（">=","<=","><"）则按序存放
													 *逻辑型运算符（"and","or"）分别用（"&","|"）表示，然后
													 *存放在数组第零号位置
													 */
u8		par_db_ext_Str[256];
u8		api_db_ext_StrToken[256];
u16		api_db_ext_Length;
u32		api_db_ext_Index;
///////////////////////////////////////
/*从当前记录向下找*/
void api_db_locate()
{
	int	i ;
	s32 par_Boolean = false;
	dbFieldValue par_FdValue;	

	apihelper_db_reset();

	api_db_ext_Condition = tNativeFunction_PopParamAsString();
	api_db_ext_Database = tNativeFunction_PopParamAsPtr();

	for(i = api_db_ext_Database->RecordNo - 1;i < api_db_ext_Database->RecordCount;i ++)
	{ 
		api_db_ext_Index = i;
		api_db_ext_Length = 0;

		par_FdValue = apihelper_db_exp();

		if(api_db_ext_Letter == '\n')
		{
			continue;
		}
		if(par_FdValue.type == 'I')
		{
			if(par_FdValue.value.int_val == true)
			{
				api_db_ext_Database->RecordNo = i + 1;
				par_Boolean = true;
				break;
			}
		}
	}
	tNativeFunction_PushReturnAsInteger(par_Boolean);
}
/*对已经索引的字段从当前记录向下找*/
void api_db_seek(void)
{
	s32	i ;
	dbFieldValue par_FdValue;	

	apihelper_db_reset();

	api_db_ext_Condition = tNativeFunction_PopParamAsString();
	api_db_ext_Database = tNativeFunction_PopParamAsPtr();

	for(i = api_db_ext_Database->RecordNo - 1;i < api_db_ext_Database->RecordCount;i ++)
	{ 
		api_db_ext_Index = i;
		api_db_ext_Length = 0;
		par_FdValue = apihelper_db_exp();
		if(par_FdValue.type == 'I')
		{
			if(par_FdValue.value.int_val == true)
			{
				break;
			}
		}
	}
	tNativeFunction_PushReturnAsInteger(i);
}
/*在整个数据库中对符合条件的记录总数进行统计，不计算删除的记录*/
void api_db_count(void)
{
	int	i;
	s32 par_Count = 0;
	dbFieldValue par_FdValue;	

	apihelper_db_reset();

	api_db_ext_Condition = tNativeFunction_PopParamAsString();
	api_db_ext_Database = tNativeFunction_PopParamAsPtr();

	for(i = 0;i < api_db_ext_Database->RecordCount;i ++)
	{ 
		api_db_ext_Index = i;
		api_db_ext_Length = 0;
		par_FdValue = apihelper_db_exp();
		if(par_FdValue.type == 'I')
		{
			if(par_FdValue.value.int_val == true)
			{
				if(!apihelper_db_isdeleted(i))
					par_Count ++;
			}
		}

	}
	tNativeFunction_PushReturnAsInteger(par_Count);
}
/*在整个数据库中对符合条件的记录的指定字段进行求和*/
void api_db_sum(void)
{
	int	i ;
	int par_FdIndex;
	int par_RdIndex = 0;
	f64 par_Temp;
	f64 par_RtValue = 0;
	u8  *par_FdName;
	dbFieldValue par_FdValue;
	u8 temp[40];

	apihelper_db_reset();

	api_db_ext_Condition = tNativeFunction_PopParamAsString();
	par_FdName	  = tNativeFunction_PopParamAsString();
	api_db_ext_Database = tNativeFunction_PopParamAsPtr();
	/*如果小写，则转换成写*/
	for(i = 0;i < strlen(par_FdName);i ++)
	{
		if(*(par_FdName+i)>='a'&&*(par_FdName+i)<='z')
		{
			*(par_FdName +i) = *(par_FdName +i) - 32;
		}
	}
	
	par_FdIndex = apihelper_db_getfieldindex(par_FdName);

	for(i = 0;i < par_FdIndex; i ++)
	{
		par_RdIndex += api_db_ext_Database->Fld[i].Len;
	}

	for(i = 0;i < api_db_ext_Database->RecordCount;i ++)
	{ 
		if(apihelper_db_isemptycondition())
		{
			
			memcpy(temp,&api_db_ext_Database->Record[i][par_RdIndex + 1],api_db_ext_Database->Fld[par_FdIndex].Len);
			temp[api_db_ext_Database->Fld[par_FdIndex].Len] = '\0';
			par_Temp = atof(temp);
			par_RtValue += par_Temp;
		}
		else
		{
			api_db_ext_Index = i;
			api_db_ext_Length = 0;
			par_FdValue = apihelper_db_exp();
			if(par_FdValue.value.int_val == true)
			{
				memcpy(temp,&api_db_ext_Database->Record[i][par_RdIndex + 1],api_db_ext_Database->Fld[par_FdIndex].Len);
				temp[api_db_ext_Database->Fld[par_FdIndex].Len] = '\0';
				par_Temp = atof(temp);
				par_RtValue += par_Temp;
			}

		}
	}
	tNativeFunction_PushReturnAsDouble(par_RtValue);
}

///////////////////////////////////////
//词法分析
////////////////////////////////////////

/*=========================================================================
* FUNCTION:      apihelper_db_getword
* TYPE:          assistant function 
* OVERVIEW:		 get a word  from condition sentence  
*   parameters:  
*   returns:   
*=======================================================================*/
void apihelper_db_getword()
{
	u8 par_Temp_Str[256];
	int par_BlankCount = 0;
	int par_CharLength = 0;
	int par_HasADoc;

	memset(par_Temp_Str,0,256);
	apihelper_db_reset();

	apihelper_db_getchar();
	apihelper_db_getblank();
	if(apihelper_db_ischletter())
	{
		
		while(apihelper_db_ischletter())
		{
			apihelper_db_concat();
			apihelper_db_getchar();

			apihelper_db_concat();
			apihelper_db_getchar();

			while(apihelper_db_isletter()|apihelper_db_isdigit())
			{
				if(api_db_ext_Length == strlen(api_db_ext_Condition))
					return;
				apihelper_db_concat();
				apihelper_db_getchar();
			}
		}
		apihelper_db_retract();
		if(apihelper_db_isfiledname(api_db_ext_StrToken))
		{
			api_db_ext_Operant[0] = 's';
		}
		else
		{
			api_db_ext_Operant[0] = 'v';
		}
	}
	else
	if(apihelper_db_isletter())
	{
		if(apihelper_db_logicalsign())
		{
			return;
		}
		/*获取字段名，还没有考虑汉字*/
		while(apihelper_db_isletter()||apihelper_db_isdigit())
		{
		//	apihelper_db_getchar();
			apihelper_db_concat();
			apihelper_db_getchar();
			
			par_BlankCount = 0;
			while(apihelper_db_isletter()||apihelper_db_isdigit())
			{
				apihelper_db_concat();
				apihelper_db_getchar();
			}
			while(api_db_ext_Letter == ' ')
			{
				par_BlankCount ++;
				apihelper_db_getchar();
			}

			if(par_BlankCount&&apihelper_db_isletter())
			{
			/*	if(apihelper_db_logicalsign())
				{
					return;
				}*/
				
				while(par_BlankCount)
				{	
					par_CharLength = strlen(api_db_ext_StrToken);
					api_db_ext_StrToken[par_CharLength] = ' ';
					par_BlankCount -- ;
				}

			}
			else
		//	//if(par_BlankCount == 0)
		//	{
				break;
				//apihelper_db_retract();
		//	}
		}

		apihelper_db_retract();

		
		par_CharLength = 0;
		while(par_CharLength <strlen(api_db_ext_StrToken))
		{
			if(api_db_ext_StrToken[par_CharLength] >= 'a'
				&&api_db_ext_StrToken[par_CharLength] <= 'z')
			{
				par_Temp_Str[par_CharLength] = api_db_ext_StrToken[par_CharLength] - 32;
			}
			else
			{
				par_Temp_Str[par_CharLength] = api_db_ext_StrToken[par_CharLength];
			}
			par_CharLength++;
		}

		if(apihelper_db_isfiledname(par_Temp_Str))
		{
			strcpy(api_db_ext_StrToken,par_Temp_Str);
			api_db_ext_Operant[0] = 's';
		}
		else
		{
			api_db_ext_Operant[0] = 'v';
		}
	}
	/*获取操作数*/
	else
	if(apihelper_db_isdigit())
	{
		par_HasADoc = false;
		while(true)
		{
			if(apihelper_db_isdigit())
			{
				apihelper_db_concat();
				apihelper_db_getchar();
				if(api_db_ext_Letter == '.'&& par_HasADoc == false)
				{
					apihelper_db_concat();
					apihelper_db_getchar();
					par_HasADoc = true;
				}
				/*判断是否日期类型*/
				else
				if(api_db_ext_Letter == '/')
				{
					/*在此添加程序，判断是否月份*/
					apihelper_db_getchar();
					/*在此添加程序，判断是否日期*/
				}
				else
				if(!apihelper_db_isdigit())
				{
					break;
				}
			}
		}			

		apihelper_db_retract();
		api_db_ext_Operant[0] = 'v';
	}
	/*获取运算符*/
	else
	if(api_db_ext_Letter == '>')
	{
		apihelper_db_getchar();
		if(api_db_ext_Letter == '=')
		{
			/*得到">="运算符*/
			api_db_ext_Operant[0] = '>';
			api_db_ext_Operant[1] = '=';
		}
		else
		if(api_db_ext_Letter == '<')
		{
			/*得到"!="运算符*/
			api_db_ext_Operant[0] = '<';
			api_db_ext_Operant[1] = '>';
		}
		else
		{
			apihelper_db_retract();
			/*得到">"运算符*/
			api_db_ext_Operant[0] = '>';
		}
	}
	else
	if(api_db_ext_Letter == '<')
	{
		apihelper_db_getchar();
		if(api_db_ext_Letter == '=')
		{
			/*得到"<="运算符*/
			api_db_ext_Operant[0] = '<';
			api_db_ext_Operant[1] = '=';
		}
		else
		{
			apihelper_db_retract();
			/*得到"<"运算符*/
			api_db_ext_Operant[0] = '<';
		}
	}
	else
	if(api_db_ext_Letter == '=')
	{
		/*得到"="运算符*/
		api_db_ext_Operant[0] = '=';
	}
	else
	if(api_db_ext_Letter == '+')
	{
		/*得到'+'运算符*/
		api_db_ext_Operant[0] = '+';
	}
	else
	if(api_db_ext_Letter == '-')
	{
		/*得到'-'运算符*/
		api_db_ext_Operant[0] = '-';
	}
	else
	if(api_db_ext_Letter == '*')
	{
		/*得到'*'运算符*/
		api_db_ext_Operant[0] = '*';
	}
	else
	if(api_db_ext_Letter == '/')
	{
		/*得到'/'运算符*/
		api_db_ext_Operant[0] = '/';
	}
	else
	if(api_db_ext_Letter == '(')
	{
		/*得到'('*/
		api_db_ext_Operant[0] = '(';
	}
	else
	if(api_db_ext_Letter == ')')
	{
		/*得到')'*/
		api_db_ext_Operant[0] = ')';
	}
}
int  apihelper_db_logicalsign(void)
{
	int i = 0;
	if(api_db_ext_Letter == 'a')
	{
		i ++;
		apihelper_db_getchar();
		i ++;
		if(api_db_ext_Letter == 'n')
		{
			apihelper_db_getchar();
			i ++;
			if(api_db_ext_Letter == 'd')
			{
				
				apihelper_db_getchar();
				i ++;
				if(api_db_ext_Letter == ' ')
				{
					api_db_ext_Operant[0] = '&';
					return true;
				}
			}
		}
	}
	else
	if(api_db_ext_Letter == 'o')
	{
		i ++;
		apihelper_db_getchar();
		i ++;
		if(api_db_ext_Letter == 'r')
		{
			apihelper_db_getchar();
			i ++;
			if(api_db_ext_Letter == ' ')
			{
				api_db_ext_Operant[0] = '|';
				return true;
			}
		}
	}

	if(i != 0)
	{
		while(i)
		{
			apihelper_db_retract();
			i--;
		}
		apihelper_db_getchar();
		return false;
	}
	return false;
}
/*=========================================================================
* FUNCTION:      apihelper_db_getchar
* TYPE:          assistant function 
* OVERVIEW:		 get next char from current position
*   parameters:  
*   returns:   
*=======================================================================*/
void apihelper_db_getchar()
{
	api_db_ext_Letter =  *(api_db_ext_Condition + api_db_ext_Length);
	api_db_ext_Length ++;
}

/*=========================================================================
* FUNCTION:      apihelper_db_getblank
* TYPE:          assistant function 
* OVERVIEW:		 filter blank chars
*   parameters:  
*   returns:   
*=======================================================================*/
void apihelper_db_getblank()
{
	while( api_db_ext_Letter == ' ')
		apihelper_db_getchar( );
}
/*=========================================================================
* FUNCTION:      pihelper_db_concat
* TYPE:          assistant function 
* OVERVIEW:		 suspend a character to array(here is api_db_ext_StrToken)
*   parameters:  
*   returns:   
*=======================================================================*/
void apihelper_db_concat()
{
	int nLen = strlen(api_db_ext_StrToken);
	if(nLen >= 0 && nLen < 256)
	{
		api_db_ext_StrToken[nLen] = api_db_ext_Letter;
	}
}
/*=========================================================================
* FUNCTION:      apihelper_db_retract
* TYPE:          assistant function 
* OVERVIEW:		 decrease 1 unit to make a pointer point previous char
*   parameters:  
*   returns:   
*=======================================================================*/
void apihelper_db_retract()
{
	api_db_ext_Length --;
}

/*=========================================================================
* FUNCTION:      apihelper_db_isletter
* TYPE:          assistant function 
* OVERVIEW:		 judge whether a English character ,and transform lowercase into a capital one
*   parameters:  
*   returns:     true:1
*				 false:0
*=======================================================================*/
int apihelper_db_isletter()
{
	
	if(api_db_ext_Letter >= 'A'&&api_db_ext_Letter <= 'Z')
	{
		return true;
	}
	else
	if(api_db_ext_Letter >= 'a'&&api_db_ext_Letter <= 'z')
	{
	//	api_db_ext_Letter = api_db_ext_Letter - 32;
		return true;
	}
	return false;
}

/*=========================================================================
* FUNCTION:      apihelper_db_ischletter
* TYPE:          assistant function 
* OVERVIEW:      Chinese character only when the value of a byte is out 
*				 of ASC2 and belong to GB 2312
*   parameters:  
*          
*   returns:     true:1
*				 false:0
*=======================================================================*/

int	 apihelper_db_ischletter(void)
{
	/*GB 2312 汉字区: B0A1-F7FE*/
	if(api_db_ext_Letter >= 176 && api_db_ext_Letter <= 247)
	{
		return true;
	}
	return false;
}

/*=========================================================================
* FUNCTION:      apihelper_db_isdigit
* TYPE:          assistant function 
* OVERVIEW:		 judge whether a char is a figure type or not
*   parameters:  
*          
*   returns:     true:1
*				 false:0
*=======================================================================*/

int apihelper_db_isdigit()
{
	if(api_db_ext_Letter >= '0'&&api_db_ext_Letter <= '9')
		return true;
	return false;
}


/*=========================================================================
* FUNCTION:      apihelper_db_reset
* TYPE:          assistant function 
* OVERVIEW:		 reset values of those external variable
*   parameters:  
*          
*   returns:     
*=======================================================================*/

void apihelper_db_reset(void)
{
	api_db_ext_Letter = 0;
	memset(api_db_ext_Operant,0,2);
	memset(api_db_ext_StrToken,0,256);
}
/*=========================================================================
* FUNCTION:      aapihelper_db_getvalue
* TYPE:          assistant function 
* OVERVIEW:       pick up number found in condition string
*   parameters:  
*          
*   returns:     dbFieldValue par_LValue
*=======================================================================*/
dbFieldValue apihelper_db_getvalue()
{
	int i;
	int par_StrLength;
	dbFieldValue par_RtValue;

	par_StrLength = strlen(api_db_ext_StrToken);

	par_RtValue.type = 'C';
	par_RtValue.dir	 = 'T';
	par_RtValue.len  = par_StrLength;
	strcpy(par_RtValue.value.str_val,api_db_ext_StrToken);
	for(i = 0;i < par_StrLength;i ++ )
	{
		if(api_db_ext_StrToken[i] == '.')
		{
			par_RtValue.dot = i;
			break;
		}
	}
	if(i == par_StrLength)
	{
		par_RtValue.dot = 0;
	}
	return par_RtValue;
}

/*语法分析函数*/

/*=========================================================================
* FUNCTION:      apihelper_db_exp
* TYPE:          assistant function 
* OVERVIEW:       expression of logical operation
*   parameters:  
*          
*   returns:     dbFieldValue par_LValue
*=======================================================================*/

dbFieldValue apihelper_db_exp(void)
{
	dbFieldValue par_LValue;
	dbFieldValue par_RValue;
	
	apihelper_db_getword();
	
	par_LValue = apihelper_db_term0();
	
	while(api_db_ext_Operant[0] == '&'||api_db_ext_Operant[0] == '|')
	{
		if(api_db_ext_Operant[0] == '&')
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term0();
			if(par_LValue.value.int_val && par_RValue.value.int_val)
			{
				par_LValue.value.int_val = true;
			}
			else
			{
				par_LValue.value.int_val = false;
			}
		}
		else
		if(api_db_ext_Operant[0] == '|')
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term0();
			if(par_LValue.value.int_val || par_RValue.value.int_val)
			{
				par_LValue.value.int_val = true;
			}
			else
			{
				par_LValue.value.int_val = false;
			}
		}
	}
	return par_LValue;
}

/*=========================================================================
* FUNCTION:      apihelper_db_term0
* TYPE:          assistant function 
* OVERVIEW:       expression of relational operation
*   parameters:  
*          
*   returns:     dbFieldValue par_LValue
*=======================================================================*/
dbFieldValue apihelper_db_term0(void)
{
	int boolean = false;
	dbFieldValue par_LValue;
	dbFieldValue par_RValue;

	par_LValue = apihelper_db_term1();
	if(api_db_ext_Operant[0] == '>')
	{
		if(api_db_ext_Operant[1] == '=')
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term1();
			par_LValue = apihelper_db_compop(par_LValue,par_RValue,">=");			
		}
		else
		if(api_db_ext_Operant[1] == '<')
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term1();
			par_LValue = apihelper_db_compop(par_LValue,par_RValue,"><");
	
		}
		else
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term1();
			par_LValue = apihelper_db_compop(par_LValue,par_RValue,">");	
		}
	}
	else
	if(api_db_ext_Operant[0] == '<')
	{
		if(api_db_ext_Operant[1] == '=')
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term1();
			par_LValue = apihelper_db_compop(par_LValue,par_RValue,"<=");
		}
		else
		{
			apihelper_db_getword();
			par_RValue = apihelper_db_term1();
			par_LValue = apihelper_db_compop(par_LValue,par_RValue,"<");
		}
	}
	else
	if(api_db_ext_Operant[0] == '=')
	{
		apihelper_db_getword();
		par_RValue = apihelper_db_term1();
		par_LValue = apihelper_db_compop(par_LValue,par_RValue,"=");
	}
	return par_LValue;
}

/*=========================================================================
* FUNCTION:      apihelper_db_term1
* TYPE:          assistant function 
* OVERVIEW:       expression of addition or subtration
*   parameters:  
*          
*   returns:     dbFieldValue par_LValue
*=======================================================================*/
dbFieldValue apihelper_db_term1(void)
{
	dbFieldValue par_LValue;
	dbFieldValue par_RValue;

	par_LValue = apihelper_db_term2();
	while(api_db_ext_Operant[0] == '+'||api_db_ext_Operant[0] == '-')
	{
		switch(api_db_ext_Operant[0])
		{
		case '+':
			apihelper_db_getword();
			par_RValue = apihelper_db_term2();
			par_LValue = apihelper_db_op(par_LValue,par_RValue,'+');
			break;
		case '-':
			apihelper_db_getword();
			par_RValue = apihelper_db_term2();
			par_LValue = apihelper_db_op(par_LValue,par_RValue,'-');
			break;
		}
	}
	
	return par_LValue;
}

/*=========================================================================
* FUNCTION:      apihelper_db_term2
* TYPE:          assistant function 
* OVERVIEW:       expression of multiply or division
*   parameters:  
*          
*   returns:     dbFieldValue par_LValue
*=======================================================================*/
dbFieldValue apihelper_db_term2(void)
{
	dbFieldValue par_LValue;
	dbFieldValue par_RValue;

	par_LValue = apihelper_db_factor();
	while(api_db_ext_Operant[0] == '*'||api_db_ext_Operant[0] == '/')
	{
		switch(api_db_ext_Operant[0])
		{		
		case '*':
			apihelper_db_getword();
			par_RValue = apihelper_db_factor();
			par_LValue = apihelper_db_op(par_LValue,par_RValue,'*');	 
		break;
		case '/':
		apihelper_db_getword();
		par_RValue = apihelper_db_factor();
		par_LValue = apihelper_db_op(par_LValue,par_RValue,'/');
		break;
		}
	}
	return par_LValue;
}

/*=========================================================================
* FUNCTION:      apihelper_db_factor
* TYPE:          assistant function 
* OVERVIEW:       expression 
*   parameters:  
*          
*   returns:     dbFieldValue par_RtValue
*=======================================================================*/

dbFieldValue apihelper_db_factor(void)
{
	dbFieldValue par_LValue;
	if(api_db_ext_Operant[0] == '(')
	{
		if(api_db_ext_Operant[0] == '(')
		{
			par_LValue = apihelper_db_exp();

			if(api_db_ext_Operant[0] != ')')
			{
				/*显示出错信息，退出*/
				printf("语法错误,找不到相应的右括号!\n");
				exit(1);
			}
			else
			{
				apihelper_db_getword();
			}
		}		
	}
	else
	if(api_db_ext_Operant[0] == 's')
	{
		/*获得字段值*/
		par_LValue = apihelper_db_getrecord();
		apihelper_db_getword();
	}
	else
	if(api_db_ext_Operant[0] == 'v')
	{
		/*取得常数*/
		par_LValue = apihelper_db_getvalue();

		apihelper_db_reset();
		apihelper_db_getword();
	}
	else
	{
		/*显示出错信息,退出*/
		printf("语法错误!\n");
		exit(1);
	}
	return par_LValue;
}


/*=========================================================================
* FUNCTION:      apihelper_db_getrecord
* TYPE:          assistant function 
* OVERVIEW:      get a field's value of appointed row and then convert it to string
*   parameters:  
*          
*   returns:     dbFieldValue par_RtValue
*=======================================================================*/

dbFieldValue apihelper_db_getrecord()
{
	int par_Loop		= 0;
	int par_Int			= 0;
	int par_LBlank		= 0;
	int par_RBlank		= 0;
	int par_Index		= 0;
	int	par_RecordIndex = 0;
	int par_Temp		= 0;
	int par_RecordLen	= 0;
	int par_RecordRlLen = 0;

	u8	*par_Record		= 0;
	dbFieldValue par_RtValue;

	par_Index = apihelper_db_getfieldindex(api_db_ext_StrToken);

	if(par_Index < 0)
	{
		exit(1);
	}
	while(par_Loop < par_Index)
	{
		par_RecordIndex += (int)api_db_ext_Database->Fld[par_Loop].Len;
		par_Loop++;
	}

	par_RecordLen = api_db_ext_Database->Fld[par_Index].Len;
	par_Record = &api_db_ext_Database->Record[api_db_ext_Index][par_RecordIndex + 1];
	/*获的字段值的有效长度*/
	
	while(*(par_Record + par_Temp)== ' ')
	{
		par_Temp ++;
		par_LBlank++;
	}
	/*为空串*/
	if(par_LBlank == par_RecordLen)
	{
		par_RtValue.type = 'C';
		par_RtValue.len = 0;
		par_RtValue.value.str_val[0] = ' ';
		return par_RtValue;
	}
	par_Temp = par_RecordLen - 1;
	while(*(par_Record + par_Temp) == ' ')
	{
		par_RBlank ++;
		par_Temp --;
	}

	par_RecordRlLen = par_RecordLen - par_LBlank - par_RBlank;
	/*把字段值转化成字符串类型*/
	switch (api_db_ext_Database->Fld[par_Index].Type)
	{
	case 'I'://?
		memcpy(&par_Int,&api_db_ext_Database->Record[api_db_ext_Index][par_RecordIndex+1],4);
		itoa(par_Int,par_RtValue.value.str_val,10);
		break;
	
	case 'N':
		memcpy(par_RtValue.value.str_val,&api_db_ext_Database->Record[api_db_ext_Index][par_RecordIndex + 1 + par_LBlank],par_RecordRlLen);
		break;
	case 'C':
	case 'D':
		memcpy(par_RtValue.value.str_val,&api_db_ext_Database->Record[api_db_ext_Index][par_RecordIndex + 1],par_RecordRlLen);
	default:break;
	}
	/*设置dbFieldValue结构*/
	par_RtValue.type = 'C';
	par_RtValue.dir  = 'F';
	if(api_db_ext_Database->Fld[par_Index].DecLen == 0)
		par_RtValue.dot  = 0;
	else
		par_RtValue.dot = par_RecordRlLen - api_db_ext_Database->Fld[par_Index].DecLen - 1;

	if(api_db_ext_Database->Fld[par_Index].Type == 'I')
	{
		par_RtValue.len  = strlen(par_RtValue.value.str_val);
	}
	else
	{
		par_RtValue.len = par_RecordRlLen;
	}
	
	return par_RtValue;
}

/*=========================================================================
* FUNCTION:      apihelper_db_getfieldindex
* TYPE:          assistant function 
* OVERVIEW:      get index of a field 
* INTERFACE:
*   parameters:  
*				u8 *fieldName:field name
*   returns:  
*=======================================================================*/
int apihelper_db_getfieldindex(u8 *fieldName)
{
	int i;
	for(i = 0 ;i < api_db_ext_Database->FieldCount;i ++)
	{
		if(strcmp(api_db_ext_Database->Fld[i].Name,fieldName) == 0)
		{
			return i;
		}
	}
	//printf("没有找到相匹配的字段\n");
	//exit(1);
	return  -1;
}
int apihelper_db_isfiledname(u8 *fieldName)
{
	if(apihelper_db_getfieldindex(fieldName) == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}
/*=========================================================================
* FUNCTION:      apihelper_db_isdeleted
* TYPE:          assistant function 
* OVERVIEW:      get digits of a double type number
* INTERFACE:
*   parameters:  int row:
*              
*   returns:  
*=======================================================================*/
int apihelper_db_isdeleted(int row)
{
	u8 par_DelMark;
	par_DelMark = api_db_ext_Database->Record[row][0];
	if(par_DelMark == '*')
	{
		return true;
	}
	return false;
}
/*=========================================================================
* FUNCTION:      apihelper_db_getfiguredigit
* TYPE:          assistant function 
* OVERVIEW:      get digits of a double type number
* INTERFACE:
*   parameters:  
*              
*   returns:  
*=======================================================================*/
int apihelper_db_getfiguredigit(double nParam)
{
	int i = 0;
	while(true)
	{
		if((nParam /= 10) >= 1)
			i ++;
		else
			return i + 1;
	}
}

/*=========================================================================
* FUNCTION:      apihelper_db_op
* TYPE:          assistant function 
* OVERVIEW:      make a arithmetical operation
* INTERFACE:
*   parameters:  
*               dbFieldValue lop: struct contain L operand
*               dbFieldValue rop: struct contain R operand
*               u8 *isgreat:arithmetical operator e.g(+,-,*,/)
*   
*   returns:    dbFieldValue:par_RtValue
*=======================================================================*/
dbFieldValue apihelper_db_op(dbFieldValue lop,dbFieldValue rop,u8 operate)
{
	double par_LValue;
	double par_RValue;
	double par_Result;

	dbFieldValue par_RtValue;

	if(lop.type == 'C'&&rop.type == 'C')
	{
		par_LValue = atof(lop.value.str_val);
		par_RValue = atof(rop.value.str_val);

		switch(operate)
		{
		case '+':
			par_Result = par_LValue + par_RValue;
			break;
		case '-':
			par_Result = par_LValue - par_RValue;
			break;
		case '*':
			par_Result = par_LValue * par_RValue;
			break;
		case '/':
			if(par_RValue != 0)
			{
				par_Result = par_LValue / par_RValue;
				break;
			}
		default:
			break;
		}
		sprintf(par_RtValue.value.str_val,"%lf",par_Result);

		par_RtValue.type = 'C';
		par_RtValue.dot  = apihelper_db_getfiguredigit(par_Result);
		par_RtValue.len	= strlen(par_RtValue.value.str_val);
	}
	return par_RtValue;
}
/*=========================================================================
* FUNCTION:      apihelper_db_compop
* TYPE:          assistant function
* OVERVIEW:      make a relational operation
* INTERFACE:
*   parameters:  
*               dbFieldValue lop: struct contain L operand
*               dbFieldValue rop: struct contain R operand
*               u8 *isgreat:relational operator e.g(>,<,>=,<=...)
*   returns:   
*               true: 1
*				false:0
*=======================================================================*/

dbFieldValue apihelper_db_compop(dbFieldValue lop,dbFieldValue rop,u8 *isgreat)
{
	int par_LDotPos;
	int par_RDotPos;
	int par_Loop = 0;
	int par_MinLength = 0;
	u8 *par_comp;
	u8 *par_LValue;
	u8 *par_RValue;
	dbFieldValue par_RtValue;

	par_LValue = lop.value.str_val;
	par_RValue = rop.value.str_val;

	par_RtValue.type = 'I';
	
	/*字符类型比较*/
	if((*par_LValue < '0'|| *par_LValue > '9')&&strcmp("=",isgreat)==0)
	{
		if(lop.len < rop.len && lop.dir == 'F'&& rop.dir == 'T')
		{
			par_RtValue.value.int_val = false;
			return par_RtValue;
		}
		else
		if(lop.len > rop.len && lop.dir == 'T'&& lop.dir == 'F')
		{
			par_RtValue.value.int_val = false;
			return par_RtValue;
		}
	
		if(lop.len < rop.len)
		{
			par_MinLength = lop.len;

			while(*par_RValue != *par_LValue)
			{
				par_RValue ++;
				par_Loop ++;
				if(rop.len == lop.len)
				{
					par_RtValue.value.int_val = false;
					return par_RtValue;
				}
				if(par_Loop == rop.len - lop.len + 1)
				{
					par_RtValue.value.int_val = false;
					return par_RtValue;
				}
			}
			par_Loop = 0;
			while(*par_RValue == *par_LValue)
			{
				par_Loop ++;

				par_RValue ++;
				par_LValue ++;

				if(par_Loop == par_MinLength)
				{
					par_RtValue.value.int_val = true;
					return par_RtValue;
				}
			}
			par_RtValue.value.int_val = false;
			return par_RtValue;
		}
		else
		if(lop.len >= rop.len)
		{
			par_MinLength = rop.len;

			while(*par_RValue != *par_LValue)
			{
				par_LValue ++;
				par_Loop ++;
				if(par_Loop == lop.len - rop.len + 1)
				{
					par_RtValue.value.int_val = false;
					return par_RtValue;
				}
			}
			par_Loop = 0;
			while(*par_RValue == *par_LValue)
			{
				par_Loop ++;

				par_RValue ++;
				par_LValue ++;

				if(par_Loop == par_MinLength)
				{
					par_RtValue.value.int_val = true;
					return par_RtValue;
				}
			}
			par_RtValue.value.int_val = false;
			return par_RtValue;
		}
	}
	if(lop.type == 'C'&&rop.type == 'C')
	{
		if(lop.dot == 0)
		{
			par_LDotPos = lop.len;
		}
		else
		{
			par_LDotPos = lop.dot;
		}

		if(rop.dot == 0)
		{
			par_RDotPos = rop.len;
		}
		else
		{
			par_RDotPos = rop.dot;
		}

		if(par_LDotPos > par_RDotPos)
		{
			par_comp = ">";
		}
		else
		if(par_LDotPos < par_RDotPos)
		{
			par_comp = "<";
		}
		else
		{
	
			par_Loop = lop.len + 1;
			while(par_Loop)
			{
				par_Loop --;
			
				if(*par_LValue > *par_RValue)
				{
					par_comp = ">";
					break;
				}
				else
				if(*par_LValue < *par_RValue)
				{
					par_comp = "<";
					break;
				}
				else
				{
					par_LValue ++;
					par_RValue ++;
				}
			}
			if(par_Loop == 0)
			{
				par_comp = "=";
			}
		}
	}
	
	if(*par_comp == *isgreat)
	{
			par_RtValue.value.int_val = true;
	}
	else
	if(strlen(isgreat) == 2)
	{
		if(*par_comp == *(isgreat + 1))
		{
			par_RtValue.value.int_val = true;
		}
	}
	else
		par_RtValue.value.int_val = false;
	return par_RtValue;
}

/*=========================================================================
* FUNCTION:      apihelper_db_isemptycondition
* TYPE:          assistant function
* OVERVIEW:      judge condition is empty or not
* INTERFACE:
*   parameters:  
*              
*   returns:   
*               true: 1
*				false:0
*=======================================================================*/

int apihelper_db_isemptycondition(void)
{
	u32 par_Loop = 0;
	if(strlen(api_db_ext_Condition) == 0)
	{
		return true;
	}
	else
	{
		while(par_Loop < strlen(api_db_ext_Condition))
		{
			if(*(api_db_ext_Condition +par_Loop) != ' ')
			{
				return false;
			}
			par_Loop ++;
		}
		if(par_Loop == strlen(api_db_ext_Condition))
			return true;
		return false;
	}
}
