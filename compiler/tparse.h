#ifndef _TLBASIC_PARSE_H_
#define _TLBASIC_PARSE_H_
#include "global.h"

#define MAXCHILDREN 4

typedef enum
{
	ProgramK,
	OptionK,
	IncludeK,
	DimK,
	DimVarK,
	SubK,
	FunctionK,
	TypeK,
	TypeMemberK,
	DeclareFunctionK,
	DeclareSubK,
	ArglistK,
	ArgDefK,
	StatementsK,
	IfK,
	ElseK,
	WhileK,
	ForK,
	DoWhileLoopK,
	DoLoopWhileK,
	ExitForK,
	ExitDoK,
	ReturnK,
	SelectK,
	CaseK,
	DefaultK,
	AssignK,
	CallK,
	ParamlistK,
	ExpK,
	OpK,
	VarnameK,
	PrimitiveVarnameK,
	ConstNumK,
	ConstDoubleK,
	ConstStrK,
	EraseK,
	FreeK,
	NewK,
}NodeKind;

typedef enum
{
	NoneModifier = 0,
	NativeModifier = 1,
}Modifier;

typedef struct _ParseTreeNode
{ 
	struct _ParseTreeNode * child[MAXCHILDREN]; /* child nodes */
	struct _ParseTreeNode * sibling;            /* sibling node */
	int lineno;                                 /* line no */
	NodeKind nodekind;                          /* kind of the node  */
	/* attributes: */
	int    attr_int;                      /* integer attribute */
	double attr_double;                   /* double attribute */
	char * attr_string;                   /* string attribute  */
	char * attr_string2;                  /* string attribute2 */
	DataType type;                        /* data type */
	Modifier modifier;                    /* modifier  */
	int      dimension;                   /* dimension */
	int      address;                     /* in execute stack index,only for code generate */
	char     srcfilename[MAX_FILENAME_LEN];      /* source file name */
} ParseTreeNode;

/**
* start parsing the source code
* @param source : the source code buffer
* @param sourcefilename: the source code file name
* @return : the parse tree
*/
ParseTreeNode* tparse_Start(char *source,const char *sourcefilename);

/**
* release a parse tree
*/
void tparse_freeParsetree(ParseTreeNode *tree);

/**
* print out a parse tree
*/
void tparse_printParsetree(ParseTreeNode *tree);


#endif
