/*
============================================================================
Name        : tstream.h
Author      : tangl_99
Version     : 0.1
Copyright   : tangl_99
Description : File and stream
============================================================================
*/
#ifndef _TLBASIC_STREAM_H_
#define _TLBASIC_STREAM_H_

#include <stdio.h>

int   tfile_getbuf(const char *filename,char *buf);
int   tfile_gettextbuf(const char *filename,char *buf);
int   tfile_putbuf(const char *filename,const char *buf,long len);
long  tfile_length(const char *filename);
int   tfile_exist(const char  *filename);
int   tfile_save(FILE* stream,const char* filename);

#endif
