#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tfile.h"

int tfile_getbuf(const char *filename,char *buf)
{
	FILE *fp;
	long filelength;
	filelength = tfile_length(filename);

	if(filelength == -1)
		return 0;

	fp = fopen(filename,"rb");
	if(fp == NULL)
		return 0;

	fread(buf,filelength,1,fp);
	fclose(fp);

	return 1;
}

int tfile_gettextbuf(const char *filename,char *buf)
{
	int ret;
	int filelen = tfile_length(filename);	
	if(filelen == -1)
		return 0;

	ret = tfile_getbuf(filename,buf);
	if(!ret)
		return 0;
	buf[filelen] = '\0';
	return 1;
}

int tfile_putbuf(const char *filename,const char *buf,long len)
{
	FILE *fp;
	fp = fopen(filename,"wb");
	if(fp == NULL)
		return 0;
	fwrite(buf,len,1,fp);
	fclose(fp);
	return 1;
}

long tfile_length(const char *filename)
{
	FILE *fp;
	long fileLength;
	long curSeek;
	fp = fopen(filename,"rb");
	if(fp == NULL) 
		return -1;
	curSeek = ftell(fp);
	fseek(fp,0,SEEK_END);
	fileLength  = ftell(fp);
	fseek(fp,curSeek,SEEK_SET);
	fclose(fp);
	return fileLength;
}

int tfile_exist(const char  *filename)
{
	FILE *fp;
	fp = fopen(filename,"rb");
	if(fp == NULL) 
		return 0;
	fclose(fp);
	return 1;
}

int tfile_save(FILE* stream,const char* filename)
{
	long  filesize;
	char* buf;
	FILE* fp;

	fp = fopen(filename,"wb");
	if(fp == NULL)
		return 0;

	fseek(stream,0L,SEEK_END);
	filesize = ftell(stream);
	fseek(stream,0L,SEEK_SET);
	buf = (char*)malloc(filesize);
	fread(buf,filesize,1,stream);
	fwrite(buf,filesize,1,fp);
	fclose(fp);
	free(buf);

	return 1;
}


