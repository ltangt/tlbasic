#ifndef _TLBASIC_SCAN_H_
#define _TLBASIC_SCAN_H_

#include "global.h"

typedef enum 
{
/* file end or error  */
	TK_FILEEND,TK_ERROR,
/* line end only for function: tscan_GetTokenInCurLine */
	TK_LINEEND,
/* compiler options */
	TK_INCLUDE,TK_DEFINE,
/* general types */
	TK_NUM,
	TK_DOUBLENUM,
	TK_STR,
	TK_ID,
	TK_NULL,
	TK_TRUE,
	TK_FALSE,
/* logic and control statement */
	TK_IF,TK_ELSE,TK_ELSEIF,TK_THEN,
	TK_DO,TK_WHILE,TK_LOOP,TK_WEND,
	TK_FOR,TK_TO,TK_STEP,TK_NEXT,
	TK_SELECT,TK_CASE,TK_DEFAULT,
	TK_END,TK_GOTO,
	TK_EXIT,
/* variation , subprogram and function definition */
	TK_DIM,	TK_SHARED,TK_NEW,
	TK_AS,
	TK_INTEGER,TK_STRING,TK_BYTE,TK_LONG,TK_BOOLEAN,TK_DOUBLE,TK_DATE,TK_SHORT,TK_FLOAT,
	TK_ERASE,TK_FREE,
	TK_FUNCTION,TK_SUB,TK_TYPE,TK_DECLARE,TK_NATIVE,
	TK_RETURN,
/* constant */
	TK_CONST,
	TK_CHARACTER,
/* base symbol */
	TK_PLUS,     /* + */
	TK_MINUS,    /* - */
	TK_OPPOSITE, /* - */
	TK_MUL,      /* * */
	TK_DIV,      /* / */
	TK_MOD,      /* % */
	TK_DOLLAR,   /* $ */
	TK_SEMI,     /* ; */
	TK_COMMA,    /* , */
	TK_DOT,      /* . */
	TK_COLON,    /* : */
	TK_SHARP,    /* # */
	TK_HEX,      /* &H  */
	TK_AND,      /* AND */
	TK_OR,       /* OR  */
	TK_NOT,      /* NOT */
	TK_EQ,       /* =   */
	TK_NOTEQ,    /* <>  */
	TK_LG,       /* >   */
	TK_LG_EQ,    /* >=  */
	TK_LT,       /* <   */
	TK_LT_EQ,    /* <=  */
	TK_LP,       /* (   */
	TK_RP,       /* )   */
	TK_MID_LP,   /* [   */
	TK_MID_RP,   /* ]   */
	TK_BIG_LP,   /* {   */
	TK_BIG_RP,   /* }   */
}TokenType;


/* initialize the scanner */
void      tscan_Init(char *pTextBuf,const char* srcfilename); 

/* release the scanner */
void      tscan_Release();

/* get the next token */
TokenType tscan_GetToken(char *pTokenName);

/* peek the next token but do not move the pointer of the scanner */
TokenType tscan_PeekToken(char *pTokenName);

/* get the current line number of the scanner  */
int       tscan_GetCurrentLineNo(); 

/* get the current source file name */
void      tscan_GetCurrentSourcefilename(char *filename); 

/* get position */
void      tscan_GetPosition(int *pLineNo,char **ppNext);

/* set position */
void      tscan_SetPosition(int LineNo,char *pNext); 

/* check the symbol is a english letter */
int       tscan_IsLetter(int ch); 

/* check the symbol is a digit */
int       tscan_IsDigit(int ch); 

/* check the symbol is a hex digit */
int       tscan_IsHexLetter(int ch);

/* let scanner change to scan the include file  */
int       tscan_GotoInclude(const char *includefilename);

/* jump scanner to next line */
void      tscan_GotoNextLine(); 

/* print tokens on list file */
void      tscan_printToken(TokenType tokenType, const char* tokenName); 


#endif
