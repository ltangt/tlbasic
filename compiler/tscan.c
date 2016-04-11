#include "tscan.h"

#include <string.h>
#include <stdio.h>
#include "global.h"
#include "tmem.h"
#include "tstring.h"
#include "tfile.h"
#include "ttable.h"

int          tscan_LineNo;
char        *tscan_TextBuf;
char        *tscan_Ptr;
char         tscan_sourcefilename[MAX_CHAR];
char         tscan_current_dir[MAX_CHAR];
extern char *g_SystemIncludeDirectory;

/* define the keywords */
typedef struct _tcan_Keyword
{
	int  Value;
	char Name[16];
}tscan_Keyword;

/* set of keywords */
const tscan_Keyword tscan_KeywordsList[] =
{
	TK_IF,               "if",
	TK_THEN,             "then",
	TK_ELSE,             "else",
	TK_ELSEIF,           "elseif",
	TK_DO,               "do",
	TK_WHILE,            "while",
	TK_WEND,             "wend",
	TK_LOOP,             "loop",
	TK_FOR,              "for",
	TK_TO,               "to",
	TK_STEP,             "step",
	TK_NEXT,             "next",
	TK_EXIT,             "exit",
	TK_RETURN,           "return",
	TK_SELECT,           "select",
	TK_CASE,             "case",
	TK_DEFAULT,          "default",
	TK_DIM,              "dim",
	TK_NEW,              "new",
	TK_SHARED,           "shared",
	TK_AS,               "as",
	TK_ERASE,            "erase",
	TK_FREE,             "free",
	TK_FUNCTION,         "function",
	TK_SUB,              "sub",
	TK_TYPE,             "type",
	TK_DECLARE,          "declare",
	TK_NATIVE,           "native",
	TK_END,              "end",
	TK_AND,              "and",
	TK_OR,               "or",
	TK_NOT,              "not",
	TK_MOD,              "mod",
	TK_STRING,           "String",
	TK_INTEGER,          "Integer",
	TK_BYTE,             "Byte",
	TK_SHORT,            "Short",
	TK_LONG,             "Long",
	TK_BOOLEAN,          "Boolean",
	TK_DOUBLE,           "Double",
	TK_FLOAT,            "float", 
	TK_DATE,             "Date",
	TK_INCLUDE,          "include",
	TK_CONST,            "Const",
	TK_NULL,             "Null",
	TK_TRUE,             "True",
	TK_FALSE,            "False",
};

/* stack node of the scanner */
typedef struct _tscan_StackNode
{
	char *text;
	char *ptr;
	int  lineno;
	char sourcefilename[MAX_CHAR];
	char currentdir[MAX_CHAR];

	struct _tscan_StackNode *pLast;
	struct _tscan_StackNode *pNext;
}tscan_StackNode;

tscan_StackNode *tscan_Stack = NULL;
tscan_StackNode *tscan_Stack_TopNode = NULL;

void      tscan_Push();
void      tscan_Pop();
int       tscan_IsStackEmpty();

int       tscan_FileDirPath(const char* filepath,char* dir);
int       tscan_IsAbsolutPath(const char* filepath);


int       tscan_IncludeFileSeek(const char* srcfilepath,char* destfilepath);

#define   SEEKINC_RET_NOT_EXIST   0
#define   SEEKINC_RET_SUCCESS     1
#define   SEEKINC_RET_ALREADY_INCLUDED 2

ttable_Tab* tscan_IncludeFileTab;  


/**
* Initialize the scanner
* @param pTextBuf : the text buffer of the source code
* @param srcfilename : the file name of the source code
*/
void tscan_Init(char *pTextBuf,const char* srcfilename)
{
	tscan_TextBuf = pTextBuf;
	tscan_LineNo   = 1;
	tscan_Ptr    = pTextBuf;
	strcpy(tscan_sourcefilename,srcfilename);
	
	tscan_Stack = NULL;
	tscan_Stack_TopNode  = NULL;
	tscan_IncludeFileTab = ttable_create(MAX_TAB);
}

/**
* Release the scanner
*/
void tscan_Release()
{
	ttable_release(tscan_IncludeFileTab);
}


/**
* Get the current line number of the scanner
*/
int tscan_GetCurrentLineNo()
{
	return tscan_LineNo;
}

/**
* Get the current source file name
*/
void tscan_GetCurrentSourcefilename(char *filename)
{
	strcpy(filename,tscan_sourcefilename);
}

/**
* Check the symbol is a english letter
*/
int tscan_IsLetter(int ch)
{
	if(ch >= 'a' && ch <= 'z')
		return 1;
	if(ch >= 'A' && ch <= 'Z')
		return 1;
	if(ch == '_')
		return 1;
	return 0;
}

/**
* Check the symbol is a digit
*/
int tscan_IsDigit(int ch)
{
	if(ch >= '0' && ch <='9')
		return 1;
	else 
		return 0;
}

/**
* Check the symbol si a hex letter
*/
int tscan_IsHexLetter(int ch)
{
	if(ch >= '0' && ch <= '9')
		return 1;
	if(ch >= 'A' && ch <= 'F')
		return 1;
	if(ch >= 'a' && ch <= 'f')
		return 1;
	return 0;
}

/**
* Get the next token.if is encounter the line end then return TK_LINEEN
* @param pTokenName : the token content 
* @return : the type of the token
*/
TokenType tscan_GetToken(char* pTokenName)
{
	TokenType tokentype;
	char *ptext = pTokenName;

	/* go through the blank */
_gothrough: 
	while(*tscan_Ptr == ' ' || *tscan_Ptr=='\t' || *tscan_Ptr == '\r')
	{
		tscan_Ptr++; 
	}

	if(*tscan_Ptr == '\'')
	{
		while(*tscan_Ptr != '\n' && *tscan_Ptr != '\0')
			tscan_Ptr++;
		goto _gothrough;
	}

	/* general symbol scanning */
	*pTokenName = '\0';
	switch(*tscan_Ptr)
	{
	case '\0':
		tscan_Ptr++;
		if(!tscan_IsStackEmpty())
		{
			/* free current text of scanner buffer */
			free(tscan_TextBuf);
			tscan_Pop();
			goto _gothrough;
		}
		else
			return TK_FILEEND;
		break;
	case '\n':
		tscan_LineNo++;
		tscan_Ptr++;
		return TK_LINEEND;
	case '+':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_PLUS;
	case '-':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_MINUS;
	case '*':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_MUL;
	case '/':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_DIV;
	case '(':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_LP;
	case ')':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_RP;
	case '[':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_MID_LP;
	case ']':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_MID_RP;
	case '{':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_BIG_LP;
	case '}':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_BIG_RP;
	case ';':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_SEMI;
	case '.':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_DOT;
	case ',':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_COMMA;
	case '$':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_DOLLAR;
	case '#':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_SHARP;
	case '&':  /* HEX NUMBER */
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		if(*tscan_Ptr == 'H')
		{	
			char temp[MAX_CHAR];
			int  l = 0,t;
			str_addchar(pTokenName,*tscan_Ptr);
			tscan_Ptr++;
			while( *tscan_Ptr != '\0'  && tscan_IsHexLetter(*tscan_Ptr) )
				temp[l++] = *(tscan_Ptr++);
			temp[l] = 0;
			sscanf(temp,"%x",&t);
			sprintf(pTokenName,"%d",t);
			return TK_NUM;
		}
		else
			return TK_ERROR;
	case '0': /* HEX Number 0x */
		if(*(tscan_Ptr+1) == 'x')
		{
			char temp[MAX_CHAR];
			int  l = 0,t;
			tscan_Ptr += 2;
			while( *tscan_Ptr != '\0'  && tscan_IsHexLetter(*tscan_Ptr) )
				temp[l++] = *(tscan_Ptr++);
			temp[l] = 0;
			sscanf(temp,"%x",&t);
			sprintf(pTokenName,"%d",t);
			return TK_NUM;
		}
		break;
	case ':':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_COLON;
	case '=':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		return TK_EQ;
	case '>':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		if(*tscan_Ptr == '=')
		{
			str_addchar(pTokenName,*tscan_Ptr);
			tscan_Ptr++;
			return TK_LG_EQ;
		}
		else
			return TK_LG;
		break;
	case '<':
		str_addchar(pTokenName,*tscan_Ptr);
		tscan_Ptr++;
		if(*tscan_Ptr == '=')
		{
			str_addchar(pTokenName,*tscan_Ptr);
			tscan_Ptr++;
			return TK_LT_EQ;
		}
		else if(*tscan_Ptr == '>')
		{
			str_addchar(pTokenName,*tscan_Ptr);
			tscan_Ptr++;
			return TK_NOTEQ;
		}
		else 
			return TK_LT;
		break;
	default:
		break;
	}

	if(tscan_IsDigit(*tscan_Ptr)) 	/* NUM or FLOAT */
	{
		while( *tscan_Ptr != '\0'  && tscan_IsDigit(*tscan_Ptr))
			*(ptext++) = *(tscan_Ptr++);
		if(*tscan_Ptr == '.')
		{
			*(ptext++) = *(tscan_Ptr++);
			while( *tscan_Ptr != '\0'  && tscan_IsDigit(*tscan_Ptr))
				*(ptext++) = *(tscan_Ptr++);
			tokentype = TK_DOUBLENUM;
		}
		else
			tokentype = TK_NUM;
		*ptext = '\0';
	}
	else if(tscan_IsLetter(*tscan_Ptr)) /* ID */
	{
		while( *tscan_Ptr != '\0' && (tscan_IsLetter(*tscan_Ptr) || tscan_IsDigit(*tscan_Ptr)))
			*(ptext++) = *(tscan_Ptr++);
		*ptext = '\0';
		tokentype = TK_ID;
	}
	else if(*tscan_Ptr == '"') /* STR	or LETTER */
	{
		tscan_Ptr++;
		if(*(tscan_Ptr+1) == '"' && (*(tscan_Ptr+2) == 'C' || *(tscan_Ptr+2) == 'c')) /* if it is character */
		{
			*ptext++ = *tscan_Ptr;
			*ptext   ='\0';
			tscan_Ptr += 3;
			return TK_CHARACTER;
		}
		else
		{
			while(*tscan_Ptr != '"')
			{
				*ptext = *tscan_Ptr;
				ptext++;
				tscan_Ptr++;
			}
			*ptext = '\0';
			tscan_Ptr++;
			return TK_STR;
		}
	}
	else
	{
		*ptext = *tscan_Ptr;
		tokentype = TK_ERROR;
	}

	/* tscan the token is a keywords or not */
	if(tokentype == TK_ID) 
	{
		int i;
		int keyword_count = sizeof(tscan_KeywordsList) / sizeof(tscan_Keyword);
		for(i=0; i<keyword_count; i++)
		{
			if(str_nocasecmp(pTokenName,tscan_KeywordsList[i].Name) == 0)
			{
				tokentype = (TokenType)tscan_KeywordsList[i].Value;
				return tokentype;
			}
		}
	}


	return tokentype;
}

/**
* peek the next token
*/
TokenType tscan_PeekToken(char *pTokenName)
{
	TokenType tokenType;
	int Old_LineNo = tscan_LineNo;
	char *pOld_Next= tscan_Ptr;
	tokenType = tscan_GetToken(pTokenName);

	tscan_LineNo = Old_LineNo;
	tscan_Ptr  = pOld_Next;

	return tokenType;
}

/**
* print tokens on list file
*/
void tscan_printToken(TokenType tokenType, const char* tokenName)
{
	char msg[MAX_CHAR];
	switch(tokenType)
	{
	case TK_LINEEND:
		sprintf(msg,"LINEEND ");
		break;
	case TK_ID:
		sprintf(msg,"ID : %s",tokenName);
		break;
	case TK_NUM:
		sprintf(msg,"NUM : %s",tokenName);
		break;
	case TK_DOUBLENUM:
		sprintf(msg,"DOUBLENUM: %s",tokenName);
		break;
	case TK_STR:
		sprintf(msg,"STR : %s",tokenName);
		break;		
	case TK_CHARACTER:
		sprintf(msg,"CHARACTER: %s",tokenName);
		break;
	case TK_FILEEND:
		sprintf(msg,"FILEEND ");
		break;
	case TK_PLUS:
		sprintf(msg,"PLUS ");
		break;
	case TK_MINUS:
		sprintf(msg,"MINUS ");
		break;
	case TK_MUL:
		sprintf(msg,"MUL ");
		break;
	case TK_DIV:
		sprintf(msg,"DIV ");
		break;
	case TK_MOD:
		sprintf(msg,"MOD ");
		break;
	case TK_SEMI:
		sprintf(msg,"SEMI ");
		break;
	case TK_COMMA:
		sprintf(msg,"COMMA ");
		break;
	case TK_DOT:
		sprintf(msg,"DOT ");
		break;
	case TK_COLON:
		sprintf(msg,"COLON ");
		break;
	case TK_SHARP:
		sprintf(msg,"SHARP ");
		break;
	case TK_DOLLAR:
		sprintf(msg,"DOLLAR ");
		break;
	case TK_AND:
		sprintf(msg,"AND ");
		break;
	case TK_OR:
		sprintf(msg,"OR ");
		break;
	case TK_NOT:
		sprintf(msg,"NOT ");
		break;
	case TK_EQ:
		sprintf(msg,"EQ ");
		break;
	case TK_NOTEQ:
		sprintf(msg,"NOTEQ ");
		break;
	case TK_LG:
		sprintf(msg,"LG ");
		break;
	case TK_LG_EQ:
		sprintf(msg,"LG_EQ ");
		break;
	case TK_LT:
		sprintf(msg,"LT ");
		break;
	case TK_LT_EQ:
		sprintf(msg,"LT_EQ ");
		break;
	case TK_LP:
		sprintf(msg,"LP ");
		break;
	case TK_RP:
		sprintf(msg,"RP ");
		break;
	case TK_MID_LP:
		sprintf(msg,"MID_LP ");
		break;
	case TK_MID_RP:
		sprintf(msg,"MID_RP ");
		break;
	case TK_BIG_LP:
		sprintf(msg,"BIG_LP ");
		break;
	case TK_BIG_RP:
		sprintf(msg,"BIG_RP ");
		break;
	default:
		{
			int i;
			int keyword_count = sizeof(tscan_KeywordsList) / sizeof(tscan_Keyword);
			for(i=0; i<keyword_count; i++)
			{
				if(tokenType == tscan_KeywordsList[i].Value)
				{
					sprintf(msg,"KEYWORDS: %s ",tscan_KeywordsList[i].Name);
					break;
				}
			}
			/* if it is a unknown token (error) */
			if( i== keyword_count) 
			{
				sprintf(msg,"Unknown token %d ", tokenType);
			}
			break;
		}
	}
	fprintf(g_listfp,msg);
}

/**
* get position 
*/
void tscan_GetPosition(int *pLineNo,char **ppNext)
{
	*pLineNo = tscan_LineNo;
	*ppNext  = (char*)tscan_Ptr;
}

/**
* set position
*/
void tscan_SetPosition(int LineNo,char *pNext)
{
	tscan_LineNo = LineNo;
	tscan_Ptr  = pNext;
}

/**
* goto include file and push the current scanner data
*/
int tscan_GotoInclude(const char *includefilename)
{
	long filelen;
	char *include_buf;
	char includefilepath[MAX_CHAR];
	int  seekRet;

	/* push old scanner data */
	tscan_Push();
	/* get the include file path name */
	seekRet = tscan_IncludeFileSeek(includefilename,includefilepath);
	switch(seekRet)
	{
	case SEEKINC_RET_NOT_EXIST:
		return 0;
	case SEEKINC_RET_ALREADY_INCLUDED:
		tscan_Pop(); /* skip the include file */
		break;
	case SEEKINC_RET_SUCCESS:
		/* get the file length */
		filelen = tfile_length(includefilepath);
		if(filelen == 0) /* if the file is empty,skip the going to include */
			return 1;
		/* malloc memory */
		include_buf = (char*)malloc(filelen+1);
		tfile_gettextbuf(includefilepath,include_buf);
		/* go to the new text file to begin scanning */
		tscan_TextBuf = include_buf;
		tscan_Ptr     = include_buf;
		tscan_LineNo  = 1;
		strcpy(tscan_sourcefilename,includefilepath);
		break;
	}
	
	return 1;
}

void tscan_Push()
{
	tscan_StackNode *stackNode = (tscan_StackNode*)malloc(sizeof(tscan_StackNode));
	stackNode->ptr     = tscan_Ptr;
	stackNode->text    = tscan_TextBuf;
	stackNode->lineno  = tscan_LineNo;
	strcpy(stackNode->sourcefilename,tscan_sourcefilename);
	strcpy(stackNode->currentdir,tscan_current_dir);
	stackNode->pNext   = NULL;
	if(tscan_Stack == NULL)
	{
		tscan_Stack = stackNode;
		tscan_Stack->pLast = NULL;
		tscan_Stack_TopNode = stackNode;
	}
	else
	{
		tscan_Stack_TopNode->pNext = stackNode;
		stackNode->pLast = tscan_Stack_TopNode;
		tscan_Stack_TopNode = tscan_Stack_TopNode->pNext;
	}
}

void tscan_Pop()
{
	tscan_StackNode *stackNode;
	if(tscan_Stack_TopNode == NULL)
		return;
	/* load the top node of the stack of the scanner */
	stackNode = tscan_Stack_TopNode;
	tscan_TextBuf = stackNode->text;
	tscan_Ptr     = stackNode->ptr;
	tscan_LineNo  = stackNode->lineno;
	strcpy(tscan_sourcefilename,stackNode->sourcefilename);
	strcpy(tscan_current_dir,stackNode->currentdir);
	/* move stack top node */
	tscan_Stack_TopNode = tscan_Stack_TopNode->pLast;
	/* free the pop node */
	free(stackNode);
	if(tscan_Stack_TopNode == NULL)
		tscan_Stack = NULL;
}

int tscan_IsStackEmpty()
{
	return tscan_Stack_TopNode == NULL;
}

int tscan_FileDirPath(const char* filepath,char* dir)
{
	char p[MAX_CHAR];
	int  i = strlen(filepath)-1;

	strcpy(p,filepath);
	while(i >= 0)
	{
		if(p[i] =='\\' || p[i] == '/')
			break;
		i--;
	}

	if(i < 0) /* non-direct in filepath */
		return 0;

	p[i+1] = '\0'; /* reserve the '\' */
	strcpy(dir,p);
	return 1;
}

int tscan_IsAbsolutPath(const char* filepath)
{
	const char *p = filepath;
	if(*p == '\\' || *p == '/') /* for linux path name */
		return 1;

	while(*p != '\0') 
	{
		if(*p == ':') /* for windows path name */
			return 1;
		p++;
	}
	return 0;
}

/**
* get the include file path name
* @param srcfilepath: the including file path in the source code.  e.g "inc\xxx.bas"
* @param destfilepath: the actual file path in the file system.  e.g "c:\\lib\\inc\\xxx.bas"
* @return : SEEKINC_RET_SUCCESS,
*           SEEKINC_RET_NOT_EXIST,
*           SEEKINC_RET_ALREADY_INCLUDED 
*/
int tscan_IncludeFileSeek(const char* srcfilepath,char* destfilepath)
{
	char dir[MAX_CHAR];

	memset(dir,0,MAX_CHAR);
	tscan_FileDirPath(srcfilepath,dir);

	/* seek current directory */
	if(tscan_IsAbsolutPath(srcfilepath))
	{
		strcpy(destfilepath,srcfilepath);
		
		/* check the destfilepath if is already included */
		if(ttable_lookup(tscan_IncludeFileTab,destfilepath) != NULL)
			return SEEKINC_RET_ALREADY_INCLUDED;
		else
			ttable_insert(tscan_IncludeFileTab,destfilepath,destfilepath,strlen(destfilepath));

		strcpy(tscan_current_dir,dir); 
		if(tfile_exist(destfilepath))
			return SEEKINC_RET_SUCCESS;
		else
			return SEEKINC_RET_NOT_EXIST;
	}
	else
	{
		sprintf(destfilepath,"%s%s",tscan_current_dir,srcfilepath);
		/* check the destfilepath if is already included */
		if(ttable_lookup(tscan_IncludeFileTab,destfilepath) != NULL)
			return SEEKINC_RET_ALREADY_INCLUDED;
		else
			ttable_insert(tscan_IncludeFileTab,destfilepath,destfilepath,strlen(destfilepath));

		sprintf(tscan_current_dir,"%s%s",tscan_current_dir,dir);
		/* if not exist,seek system include directory */
		if(!tfile_exist(destfilepath))
		{
			if(g_SystemIncludeDirectory == NULL)
				return SEEKINC_RET_NOT_EXIST;
			sprintf(destfilepath,"%s\\%s",g_SystemIncludeDirectory,srcfilepath);
			
			/* check the destfilepath if is already included */
			if(ttable_lookup(tscan_IncludeFileTab,destfilepath) != NULL)
				return SEEKINC_RET_ALREADY_INCLUDED;
			else
				ttable_insert(tscan_IncludeFileTab,destfilepath,destfilepath,strlen(destfilepath));

			sprintf(tscan_current_dir,"%s\\%s",g_SystemIncludeDirectory,dir);
			return tfile_exist(destfilepath);
		}
	}
	return SEEKINC_RET_SUCCESS;
}

/**
* jump scanner current pointer to next line
*/
void tscan_GotoNextLine()
{
	while(*tscan_Ptr != '\0' && *tscan_Ptr !='\n')
		tscan_Ptr++;
	
	if(*tscan_Ptr == '\n')
	{
		tscan_Ptr++;
		tscan_LineNo++;
	}
}
