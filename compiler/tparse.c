#include "tparse.h"

#include "tscan.h"
#include "global.h"
#include "ttable.h"
#include "tstack.h"

extern int g_TraceParse;

static int tparse_indentno = 0;
/* macros to increase/decrease indentation */
#define INDENT   tparse_indentno+=4
#define UNINDENT tparse_indentno-=4

static TokenType tparse_Token;                 /* current Token */
static char      tparse_TokenString[MAX_CHAR]; /* text of current Token */
static int       tparse_Error = 0;             /* if it is 1,that is error in the parsing */
static char      tparse_ErrorMsg[MAX_MSG];     /* error message */
static TokenType tparse_Mark_Token;            /* the saved token of the saved parsing data */
static char      tparse_Mark_TokenString[MAX_CHAR]; /* the saved token text of the saved parsing data */
static int       tparse_Mark_ScanLineNo;       /* the saved scan line no of the scanner of the saved parsing data */
static char     *tparse_Mark_pScanNext;        /* the saved scan current pointer of the scanner of saved the parsing data */

int              tparse_ForLoopDepth = 0;      /* Depth of loop structure */
int              tparse_DoLoopDepth  = 0;      /* Depth of loop structure */

/* tools for normal operations */
static char *          tparse_copyString(const char* str);
static ParseTreeNode*  tparse_newNode(NodeKind kind);
static ParseTreeNode*  tparse_copyNode(ParseTreeNode* node);
static void            tparse_printSpaces(void);

/* tools for analysis */
static void      tparse_syntaxError(char * message); /* output the error message to list */
static void      tparse_match(TokenType expected);   /* match a token */
static void      tparse_Mark(void);                  /* save the parser 's current parsing data */
static void      tparse_Restore(void);               /* restore the parser 's parsing data */
static DataType  tparse_match_DataType(char* TypeName); /* match the data type and return the data type */
static void      tparse_SkipLineEndToken(void);      /* skip LINEEND token */

/* constant record table */
ttable_Tab*      tparse_Const_Tab;
static int       tparse_Const_Insert(const char* name,ParseTreeNode* ConstNode);
static ParseTreeNode* tparse_Const_Lookup(const char* name);

/* parsing process functions for generate the parse tree node,
  which the name of the node is same as the name of the function 
*/
static ParseTreeNode*  tparse_ProgramK();
static ParseTreeNode*  tparse_DimK();
static ParseTreeNode*  tparse_SubK();
static ParseTreeNode*  tparse_FunctionK();
static ParseTreeNode*  tparse_TypeK(); 
static ParseTreeNode*  tparse_TypeMemberK();
static ParseTreeNode*  tparse_DeclareFunctionK();
static ParseTreeNode*  tparse_DeclareSubK();
static ParseTreeNode*  tparse_ArglistK();
static ParseTreeNode*  tparse_StatementsK();
static ParseTreeNode*  tparse_IfK();
static ParseTreeNode*  tparse_ForK();
static ParseTreeNode*  tparse_WhileK();
static ParseTreeNode*  tparse_DoWhileLoopK();
static ParseTreeNode*  tparse_DoLoopWhileK();
static ParseTreeNode*  tparse_SelectK();
static ParseTreeNode*  tparse_AssignK();
static ParseTreeNode*  tparse_CallK();
static ParseTreeNode*  tparse_ParamlistK();
static ParseTreeNode*  tparse_ExpK();
static ParseTreeNode*  tparse_ExitForK();
static ParseTreeNode*  tparse_ExitDoK();
static ParseTreeNode*  tparse_ReturnK();
static ParseTreeNode*  tparse_VarnameK();
static ParseTreeNode*  tparse_PrimitiveVarnameK();
static ParseTreeNode*  tparse_ConstNumK();
static ParseTreeNode*  tparse_ConstDoubleK();
static ParseTreeNode*  tparse_ConstStrK();
static ParseTreeNode*  tparse_EraseK();
static ParseTreeNode*  tparse_FreeK();
static ParseTreeNode*  tparse_NewK();

/* parsing process function for passing or modifying parse tree nodes */
static ParseTreeNode*  tparse_Declare();
static ParseTreeNode*  tparse_Statement();
static ParseTreeNode*  tparse_CallOrAssign();
static ParseTreeNode*  tparse_DoLoopStmt();
static ParseTreeNode*  tparse_ExitStatement();
static ParseTreeNode*  tparse_SingleExp();
static ParseTreeNode*  tparse_SimpleExp();
static ParseTreeNode*  tparse_Term();
static ParseTreeNode*  tparse_Factor();
static ParseTreeNode*  tparse_OppositeFactor();
static ParseTreeNode*  tparse_LetterConstNum();
static ParseTreeNode*  tparse_ConstNull();
static ParseTreeNode*  tparse_ConstTrue();
static ParseTreeNode*  tparse_ConstFalse();
/* parsing process functions for non-generate parse tree node */
static void            tparse_Option();
static void            tparse_Include();
static int             tparse_Const();

/**
* start parsing the source code
* @param source : the source code buffer
* @param sourcefilename: the source code file name
* @return : the parse tree
*/
ParseTreeNode* tparse_Start(char* source,const char* sourcefilename)
{
	ParseTreeNode *tparsetree;

	tparse_Error = 0;
	/* initialize the constant record table */
	tparse_Const_Tab = ttable_create(MAX_TAB);
	/* initialize the scanner */
	tscan_Init(source,sourcefilename);
	/* initialize the first token */
	tparse_Token = tscan_GetToken(tparse_TokenString);
	/* get the whole parse tree */
	tparsetree   = tparse_ProgramK();
	/* check the end token */
	tparse_match(TK_FILEEND);
	/* check the result */
	if(tparse_Error)
	{
		if(tparsetree != NULL) 
			tparse_freeParsetree(tparsetree);
		return NULL;
	}

	/* release scanner */
	tscan_Release();
	/* free constant record table */
	ttable_release(tparse_Const_Tab);

	return tparsetree;
}

/**
* copy an new string
*/
static char* tparse_copyString(const char* str)
{
	long  len = (long)strlen(str);
	char* newstr = (char*)malloc(len+1);
	strcpy(newstr,str);
	return newstr;
}

/**
* create an new Parse Tree Node
*/
static ParseTreeNode * tparse_newNode(NodeKind kind)
{
	int i;
	ParseTreeNode *node = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
	for(i=0; i< MAXCHILDREN; i++)
		node->child[i] = NULL;
	node->sibling  = NULL;
	node->nodekind= kind;
	node->attr_string = NULL;
	node->attr_string2= NULL;
	node->attr_int = 0;
	node->lineno   = tscan_GetCurrentLineNo();
	node->modifier = NoneModifier;
	node->dimension = 0;
	node->type     = (DataType)0;
	tscan_GetCurrentSourcefilename(node->srcfilename);
	return node;
}

/**
* copy an Parse Tree Node
*/
static ParseTreeNode* tparse_copyNode(ParseTreeNode* node)
{
	ParseTreeNode* newNode = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
	memcpy(newNode,node,sizeof(ParseTreeNode));
	if(node->attr_string != NULL)
	{
		newNode->attr_string = tparse_copyString(node->attr_string);
	}
	if(node->attr_string2 != NULL)
	{
		newNode->attr_string2 = tparse_copyString(node->attr_string2);
	}
	return newNode;
}

/**
* print the syntax errors
*/
static void tparse_syntaxError(char * message)
{ 
	char srcfilename[MAX_CHAR];
	int lineno = tscan_GetCurrentLineNo();
	tscan_GetCurrentSourcefilename(srcfilename);
	sprintf(tparse_ErrorMsg,"%s : Syntax error at line %d: %s",srcfilename,lineno,message);
	fprintf(g_listfp,tparse_ErrorMsg);
	tparse_Error = 1;
}


/**
* print the usually kind of error: unexpected token
*/
static void tparse_syntaxError_unexpectedToken()
{
	char msg[MAX_CHAR];
	sprintf(msg,"Unexpected symbol : %s \r\n",tparse_TokenString);
	tparse_syntaxError(msg);
}


/**
* get the next token and match the token
*/
static void tparse_match(TokenType expected)
{ 
	if (tparse_Token == expected) 
	{
		if(g_TraceParse) /* if enable the Tracing */
		{
			char szTrace[64];
			sprintf(szTrace,"Trace : line %d matched token :",tscan_GetCurrentLineNo());
			fprintf(g_listfp,szTrace);
			tscan_printToken(tparse_Token,tparse_TokenString);
			fprintf(g_listfp,"\r\n");
		}
		tparse_Token = tscan_GetToken(tparse_TokenString);
	}
	else  /* if the next token is not the expected token */
	{
		tparse_syntaxError("unexpected symbol -> ");
		tscan_printToken(tparse_Token,tparse_TokenString);
		fprintf(g_listfp," ;  miss token: ");
		tscan_printToken(expected,NULL);
		fprintf(g_listfp,"\r\n");
	}
}

/**
* save the parsing data of the current parsing
*/
static void tparse_Mark()
{
	tparse_Mark_Token = tparse_Token;
	strcpy(tparse_Mark_TokenString,tparse_TokenString);
	tscan_GetPosition(&tparse_Mark_ScanLineNo,&tparse_Mark_pScanNext);
}


/**
* restore the parsing data at last mark position
*/
static void tparse_Restore()
{
	tparse_Token = tparse_Mark_Token;
	strcpy(tparse_TokenString,tparse_Mark_TokenString);
	tscan_SetPosition(tparse_Mark_ScanLineNo,tparse_Mark_pScanNext);
}


/**
* constant table insert
*/
static int tparse_Const_Insert(const char* name,ParseTreeNode* ConstNode)
{
	if(tparse_Const_Lookup(name) != NULL)
	{
		char msg[MAX_MSG];
		sprintf(msg,"the name of constant is already defined : %s\r\n",ConstNode->attr_string);
		tparse_syntaxError(msg);
		return 0;
	}

	return ttable_insert(tparse_Const_Tab,name,ConstNode,sizeof(ParseTreeNode));
}

/**
* constant table look up
*/
static ParseTreeNode* tparse_Const_Lookup(const char* name)
{
	return (ParseTreeNode*)ttable_lookup(tparse_Const_Tab,name);
}

/**
* print tparse_indentno blanks to list
*/
static void tparse_printSpaces(void)
{
	int i;
	for (i=0;i<tparse_indentno;i++)
		fprintf(g_listfp," ");
}


/**
* print out the parse tree to the list
*/
void tparse_printParsetree(ParseTreeNode *tree)
{
	int i;
	char msg[MAX_MSG];

	if(tparse_indentno == 0)
		fprintf(g_listfp,"// ------------------------------------------------------------------\r\n");
	INDENT;
	while(tree != NULL)
	{
		/* print out the corresponding spaces */
		tparse_printSpaces();
		/* print the tree node */
		switch(tree->nodekind)
		{
		case ProgramK:
			fprintf(g_listfp,"ProgramK: ");
			break;
		case DimK:
			fprintf(g_listfp,"DimK: ");
			break;
		case DimVarK:
			fprintf(g_listfp,"DimVarK: ");
			break;
		case SubK:
			fprintf(g_listfp,"SubK: ");
			break;
		case FunctionK:
			fprintf(g_listfp,"FunctionK: ");
			break;
		case TypeK:
			fprintf(g_listfp,"TypeK: ");
			break;
		case DeclareFunctionK:
			fprintf(g_listfp,"DeclareFunctionK: "); 
			break;
		case DeclareSubK:
			fprintf(g_listfp,"DeclareSubK: ");
			break;
		case ArglistK:
			fprintf(g_listfp,"ArglistK: ");
			break;
		case ArgDefK:
			fprintf(g_listfp,"ArgDefK: ");
			break;
		case StatementsK:
			fprintf(g_listfp,"StatementsK: ");
			break;
		case ExpK:
			fprintf(g_listfp,"ExpK: ");
			break;
		case OpK:
			fprintf(g_listfp,"OpK: ");
			break;
		case ConstNumK:
			fprintf(g_listfp,"ConstNumK: ");
			break;
		case ConstDoubleK:
			fprintf(g_listfp,"ConstDoubleK: ");
			break;
		case ConstStrK:
			fprintf(g_listfp,"ConstStrK: ");
			break;
		case CallK:
			fprintf(g_listfp,"CallK: ");
			break;
		case ParamlistK:
			fprintf(g_listfp,"ParamlistK: ");
			break;
		case VarnameK:
			fprintf(g_listfp,"VarnameK: ");
			break;
		case PrimitiveVarnameK:
			fprintf(g_listfp,"PrimitiveVarnameK: ");
			break;
		case AssignK:
			fprintf(g_listfp,"AssignK: ");
			break;
		case IfK:
			fprintf(g_listfp,"IfK: ");
			break;
		case ElseK:
			fprintf(g_listfp,"ElseK: ");
			break;
		case ForK:
			fprintf(g_listfp,"ForK: ");
			break;
		case WhileK:
			fprintf(g_listfp,"WhileK: ");
			break;
		case SelectK:
			fprintf(g_listfp,"SelectK: ");
			break;
		case EraseK:
			fprintf(g_listfp,"EraseK: ");
			break;
		case NewK:
			fprintf(g_listfp,"NewK: ");
			break;
		}
		/* print the attributes of the tree node */
		if(tree->nodekind == OpK)
		{
			switch(tree->attr_int)
			{
			case TK_AND:
				sprintf(msg," AND");
				break;
			case TK_OR:
				sprintf(msg," OR");
				break;
			case TK_EQ:
				sprintf(msg," =");
				break;
			case TK_LT:
				sprintf(msg," <");
				break;
			case TK_LT_EQ:
				sprintf(msg," <=");
				break;
			case TK_LG:
				sprintf(msg," >");
				break;
			case TK_LG_EQ:
				sprintf(msg," >=");
				break;
			case TK_NOTEQ:
				sprintf(msg," <>");
				break;
			case TK_MUL:
				sprintf(msg," *");
				break;
			case TK_DIV:
				sprintf(msg," //");
				break;
			case TK_MOD:
				sprintf(msg," MOD");
				break;
			case TK_PLUS:
				sprintf(msg," +");
				break;
			case TK_MINUS:
				sprintf(msg," -");
				break;
			default:
				sprintf(msg," Unknown Op: %d",tree->attr_int);
			}
			fprintf(g_listfp,msg);
		}
		else
		{
			sprintf(msg,"   int: %d ",tree->attr_int);
			fprintf(g_listfp,msg);
			if(tree->attr_string != NULL)
			{
				sprintf(msg,"  string: %s ",tree->attr_string);
				fprintf(g_listfp,msg);
			}
			if(tree->nodekind == ConstDoubleK)
			{
				sprintf(msg,"   float: %f ",tree->attr_double);
				fprintf(g_listfp,msg);
			}
			if(tree->attr_string2 != NULL)
			{
				sprintf(msg,"  string2: %s ",tree->attr_string2);
				fprintf(g_listfp,msg);
			}

		}
		/* print dimension */
		sprintf(msg," dimension = %d",tree->dimension);
		fprintf(g_listfp,msg);
		/* print the modifier of the tree node */
		memset(msg,0,sizeof(msg));
		switch(tree->modifier)
		{
		case NativeModifier:
			sprintf(msg," NativeModifier");
			fprintf(g_listfp,msg);
			break;
		}
		fprintf(g_listfp,"\r\n");
		/* print the children of the tree node */
		for(i=0;i<MAXCHILDREN; i++)
			tparse_printParsetree(tree->child[i]);
		/* move to sibling tree node to print */
		tree = tree->sibling;
	}
	UNINDENT;
}


/**
* free the memory of the ParseTree
*/
void tparse_freeParsetree(ParseTreeNode *tree)
{
	int i;
	if(tree == NULL) return; 
	for(i=0;i < MAXCHILDREN; i++)
		tparse_freeParsetree(tree->child[i]);
	tparse_freeParsetree(tree->sibling);
	if(tree->attr_string != NULL)
		free(tree->attr_string);
	if(tree->attr_string2 != NULL)
		free(tree->attr_string2);
	free(tree);
}


/**
* match the data type and return the data type
*/
static DataType tparse_match_DataType(char* TypeName)
{
	DataType Type = (DataType)0;
	strcpy(TypeName,tparse_TokenString);
	switch(tparse_Token)
	{
	case TK_INTEGER:
		Type = IntegerType;
		tparse_match(TK_INTEGER);
		break;
	case TK_STRING:
		Type = StringType;
		tparse_match(TK_STRING);
		break;
	case TK_BYTE:
		Type = ByteType;
		tparse_match(TK_BYTE);
		break;
	case TK_SHORT:
		Type = ShortType;
		tparse_match(TK_SHORT);
		break;
	case TK_LONG:
		Type = LongType;
		tparse_match(TK_LONG);
		break;
	case TK_BOOLEAN:
		Type = IntegerType;
		tparse_match(TK_BOOLEAN);
		break;
	case TK_DOUBLE:
		Type = DoubleType;
		tparse_match(TK_DOUBLE);
		break;
	case TK_FLOAT:
		Type = FloatType;
		tparse_match(TK_FLOAT);
		break;
	case TK_DATE:
		Type = DateType;
		tparse_match(TK_DATE);
		break;
	case TK_ID:
		Type = TypeType;
		tparse_match(TK_ID);
		break;
	default: /* default type is integer */
		tparse_syntaxError("Unknown Type of Variable \r\n");
		break;
	}
	return Type;
}

/**
* Skip the line end token 
*/
static void tparse_SkipLineEndToken()
{
	while(tparse_Token == TK_LINEEND)
		tparse_match(TK_LINEEND);
}

static ParseTreeNode* tparse_ProgramK()
{
	ParseTreeNode *ProgramNode = tparse_newNode(ProgramK);
	ParseTreeNode *LastNode = NULL,*newNode = NULL;
	int IsEndProgram = 0;

	while(!IsEndProgram && !tparse_Error)
	{
		newNode = NULL;

		tparse_SkipLineEndToken();

		switch(tparse_Token)
		{
		case TK_DIM: /* global variable definition */
			newNode = tparse_DimK();
			break;
		case TK_SUB: /* subroutine */
			newNode = tparse_SubK();
			break;
		case TK_FUNCTION: /* function */
			newNode = tparse_FunctionK();
			break;
		case TK_DECLARE: /* declaration */
			newNode = tparse_Declare();
			break;
		case TK_TYPE: /* type definition */
			newNode = tparse_TypeK();
			break;
		case TK_DOLLAR: /* '$' options */
			tparse_Option();
			continue;
		case TK_CONST: /* constant definition */
			tparse_Const();
			continue;
		case TK_FILEEND:
			IsEndProgram = 1;
			break;
		default:
			IsEndProgram = 1;			
			tparse_syntaxError_unexpectedToken();
			break;
		}
		if(LastNode == NULL)
			ProgramNode->child[0] = newNode;
		else
			LastNode->sibling = newNode;
		LastNode = newNode;
	}
	
	return ProgramNode;
}


static void tparse_Option()
{
	tparse_match(TK_DOLLAR);
	switch(tparse_Token)
	{
	case TK_INCLUDE:
		tparse_Include();
		break;
	}
}

static void tparse_Include()
{
	char  includefilename[MAX_CHAR];
	char  msg[MAX_MSG];

	tparse_match(TK_INCLUDE);
	tparse_match(TK_COLON);

	strcpy(includefilename,tparse_TokenString);
	if(!tscan_GotoInclude(includefilename))
	{
		sprintf(msg,"Cannot open the include file : %s \r\n",includefilename);
		tparse_syntaxError(msg);
	}
	tparse_Token = tscan_GetToken(tparse_TokenString);
}

static ParseTreeNode* tparse_DimK()
{
	DataType VarType;
	char     TypeName[MAX_CHAR];
	ParseTreeNode *DimNode = tparse_newNode(DimK);
	ParseTreeNode *VarNode,*LastVarNode = NULL;
	int      dimension = 0;

	/* match 'Dim' */
	tparse_match(TK_DIM);
	/* if the next token is 'shared',skip it */
	if(tparse_Token == TK_SHARED)
		tparse_match(TK_SHARED);
	while(tparse_Token == TK_ID && !tparse_Error)
	{
		VarNode = tparse_newNode(DimVarK);
		/* match the name of the variable */
		VarNode->attr_string = tparse_copyString(tparse_TokenString);
		tparse_match(TK_ID);
		/* add node */
		if(LastVarNode == NULL)
			DimNode->child[0] = VarNode;
		else
			LastVarNode->sibling = VarNode;
		LastVarNode = VarNode;
		/* match comma */
		if(tparse_Token == TK_COMMA)
			tparse_match(TK_COMMA);
	}
	/* match 'AS' */
	tparse_match(TK_AS);
	/* match type of variable */
	VarType = tparse_match_DataType(TypeName);
	DimNode->attr_string2 = tparse_copyString(TypeName);
	/* if it is a array definition */
	while(tparse_Token == TK_MID_LP)
	{
		tparse_match(TK_MID_LP);
		dimension++;
		tparse_match(TK_MID_RP);
	}
	
	/* pass the data type to all the children DimVarK Nodes */
	VarNode = DimNode->child[0];
	while(VarNode != NULL)
	{
		VarNode->attr_int = VarType;
		VarNode->attr_string2 = tparse_copyString(TypeName);
		VarNode->dimension = dimension;

		VarNode = VarNode->sibling;
	}
	
	DimNode->attr_int = VarType;
	DimNode->dimension = dimension;

	/* initialize variables part */
	if(tparse_Token == TK_EQ)
	{
		tparse_match(TK_EQ);
		DimNode->child[1] = tparse_ExpK();
	}

	return DimNode;
}

static ParseTreeNode* tparse_TypeK()
{
	ParseTreeNode *TypeNode = tparse_newNode(TypeK);
	ParseTreeNode *MemberNode,*LastMemberNode = NULL;

	tparse_match(TK_TYPE);
	TypeNode->attr_string = tparse_copyString(tparse_TokenString);
	tparse_match(TK_ID);

	/* skip the line end */
	tparse_SkipLineEndToken();
	while(tparse_Token != TK_END && !tparse_Error)
	{
		MemberNode = tparse_TypeMemberK();
		if(LastMemberNode == NULL)
			TypeNode->child[0] = MemberNode;
		else
			LastMemberNode->sibling = MemberNode;
		LastMemberNode = MemberNode;
		/* skip the line end */
		tparse_SkipLineEndToken();
	}
	tparse_match(TK_END);
	tparse_match(TK_TYPE);
	return TypeNode;
}

static ParseTreeNode* tparse_TypeMemberK()
{
	ParseTreeNode *MemberNode = tparse_newNode(TypeMemberK);
	ParseTreeNode *VarNode,*LastVarNode = NULL;
	int  VarType;
	char TypeName[MAX_CHAR];
	int  dimension = 0;

	while(tparse_Token == TK_ID && !tparse_Error)
	{
		VarNode = tparse_newNode(DimVarK);
		/* match the name of the variable */
		VarNode->attr_string = tparse_copyString(tparse_TokenString);
		tparse_match(TK_ID);

		/* add node */
		if(LastVarNode == NULL)
			MemberNode->child[0] = VarNode;
		else
			LastVarNode->sibling = VarNode;
		LastVarNode = VarNode;
		/* match comma */
		if(tparse_Token == TK_COMMA)
			tparse_match(TK_COMMA);
	}
	/* match 'AS' */
	tparse_match(TK_AS);
	/* match type of variable */
	VarType = tparse_match_DataType(TypeName);
	MemberNode->attr_string2 = tparse_copyString(TypeName);
	/* if it is a array definition */
	while(tparse_Token == TK_MID_LP)
	{
		tparse_match(TK_MID_LP);
		dimension++;
		tparse_match(TK_MID_RP);
	}
	
	/* pass the data type to all the children DimVarK Node */
	VarNode = MemberNode->child[0];
	while(VarNode != NULL)
	{
		VarNode->attr_int = VarType;
		VarNode->attr_string2 = tparse_copyString(TypeName);		
		VarNode->dimension = dimension;

		VarNode = VarNode->sibling;
	}

	MemberNode->attr_int = VarType;
	MemberNode->dimension= dimension;

	return MemberNode;
}


static ParseTreeNode* tparse_SubK()
{
	ParseTreeNode *SubNode = tparse_newNode(SubK);

	tparse_match(TK_SUB);
	
	SubNode->attr_string = tparse_copyString(tparse_TokenString);
	tparse_match(TK_ID);

	tparse_match(TK_LP);
	SubNode->child[0] = tparse_ArglistK();
	tparse_match(TK_RP); 

	/* set subroutine return data type is null */
	SubNode->attr_int = NullType;

	SubNode->child[1] = tparse_StatementsK();

	tparse_match(TK_END);
	tparse_match(TK_SUB);

	return SubNode;
}

static ParseTreeNode* tparse_FunctionK()
{
	DataType FunType;
	char  TypeName[MAX_CHAR];
	ParseTreeNode *FunNode = tparse_newNode(FunctionK);

	tparse_match(TK_FUNCTION);

	FunNode->attr_string = tparse_copyString(tparse_TokenString);
	tparse_match(TK_ID);

	tparse_match(TK_LP);
	FunNode->child[0] = tparse_ArglistK();
	tparse_match(TK_RP);

	if(tparse_Token == TK_AS) /* if the function has return */
	{
		tparse_match(TK_AS);
		FunType = tparse_match_DataType(TypeName);
		FunNode->attr_int = FunType;
		FunNode->attr_string2 = tparse_copyString(TypeName);
		while(tparse_Token == TK_MID_LP)
		{
			tparse_match(TK_MID_LP);
			FunNode->dimension++;
			tparse_match(TK_MID_RP);
		}
	}
	else
		FunNode->attr_int = NullType;

	FunNode->child[1] = tparse_StatementsK();
	tparse_match(TK_END);
	tparse_match(TK_FUNCTION);

	return FunNode;
}

static ParseTreeNode* tparse_Declare()
{
	ParseTreeNode *DeclareNode;
	int IsNative,NativeFuncIndex;

	tparse_match(TK_DECLARE);
	/* check is it a native method */
	if(tparse_Token == TK_NATIVE)
	{
		IsNative = 1;
		tparse_match(TK_NATIVE);
		tparse_match(TK_SHARP);
		NativeFuncIndex = atoi(tparse_TokenString);
		tparse_match(TK_NUM);
	}
	else 
	{
		IsNative = 0;
	}

	switch(tparse_Token)
	{
	case TK_FUNCTION:
		DeclareNode = tparse_DeclareFunctionK();
		break;
	case TK_SUB:
		DeclareNode = tparse_DeclareSubK();
		break;
	default:
		if(IsNative)
			tparse_syntaxError("Unexpected token after the 'DECLARE NATIVE' \r\n");
		else
			tparse_syntaxError("Unexpected token after the 'DECLARE' \r\n");
		break;
	}
	/* add the native or none modifier */
	if(IsNative)
	{
		DeclareNode->modifier = NativeModifier;
		DeclareNode->address  = NativeFuncIndex;
	}
	
	return DeclareNode;
}

static ParseTreeNode* tparse_DeclareFunctionK()
{
	ParseTreeNode *DeclareFunNode = tparse_newNode(DeclareFunctionK);
	char TypeName[MAX_CHAR];

	tparse_match(TK_FUNCTION);
	DeclareFunNode->attr_string = tparse_copyString(tparse_TokenString);
	tparse_match(TK_ID);
	
	tparse_match(TK_LP);
	DeclareFunNode->child[0] = tparse_ArglistK();
	tparse_match(TK_RP);

	if(tparse_Token == TK_AS) /* if the function has return */
	{
		tparse_match(TK_AS);
		DeclareFunNode->attr_int = tparse_match_DataType(TypeName);
		DeclareFunNode->attr_string2 = tparse_copyString(TypeName);
		while(tparse_Token == TK_MID_LP)
		{
			tparse_match(TK_MID_LP);
			DeclareFunNode->dimension++;
			tparse_match(TK_MID_RP);
		}
	}
	else
		DeclareFunNode->attr_int = NullType;

	return DeclareFunNode;
}

static ParseTreeNode* tparse_DeclareSubK()
{
	ParseTreeNode *DeclareSubNode = tparse_newNode(DeclareSubK);
	tparse_match(TK_SUB);
	DeclareSubNode->attr_string = tparse_copyString(tparse_TokenString);
	tparse_match(TK_ID);

	tparse_match(TK_LP);
	DeclareSubNode->child[0] = tparse_ArglistK();
	tparse_match(TK_RP);

	return DeclareSubNode;
}


static ParseTreeNode* tparse_ArglistK()
{
	DataType VarType;
	int  dimension;
	char TypeName[MAX_CHAR];
	ParseTreeNode *ArglistNode = tparse_newNode(ArglistK);
	ParseTreeNode *LastNode  = NULL,*newNode;

	if(tparse_Token == TK_RP) /* if there is no argdef */
		return ArglistNode;

	while(!tparse_Error)
	{
		dimension = 0;
		newNode = tparse_newNode(ArgDefK);
		/* save the name */
		newNode->attr_string = tparse_copyString(tparse_TokenString);
		tparse_match(TK_ID);
		/* 'AS' */
		tparse_match(TK_AS);		
		/* save the type */
		VarType = tparse_match_DataType(TypeName);
		newNode->attr_int = VarType;
		newNode->attr_string2 = tparse_copyString(TypeName);
		/* if it is a array definition */
		while(tparse_Token == TK_MID_LP)
		{
			tparse_match(TK_MID_LP);
			dimension++;
			tparse_match(TK_MID_RP);
		}
		newNode->dimension = dimension;
		
		/* link to the node list */
		if(LastNode == NULL)
			ArglistNode->child[0] = newNode;
		else
			LastNode->sibling = newNode;
		LastNode = newNode;

		if(tparse_Token == TK_COMMA)
		{
			tparse_match(TK_COMMA);
			continue;
		}
		else if(tparse_Token == TK_RP)
			break;
		else
		{
			tparse_syntaxError_unexpectedToken();
			break;
		}
	}
	return ArglistNode;
}

static ParseTreeNode* tparse_StatementsK()
{
	ParseTreeNode *statementsNode = tparse_newNode(StatementsK);
	ParseTreeNode *LastNode = NULL,*stmtNode;
	while(!tparse_Error)
	{
		stmtNode = tparse_Statement();
		if(stmtNode == NULL) break;

		if(LastNode == NULL)
			statementsNode->child[0] = stmtNode;
		else
			LastNode->sibling = stmtNode;
		LastNode = stmtNode;
	}
	return statementsNode;
}

static ParseTreeNode*  tparse_Statement()
{
	ParseTreeNode* stmtNode;
	
	tparse_SkipLineEndToken();
	switch(tparse_Token)
	{
	case TK_DIM: /* dim statement */
		stmtNode = tparse_DimK();
		break;
	case TK_IF: /* if statement */
		stmtNode = tparse_IfK();
		break;
	case TK_FOR: /* for statement */
		stmtNode = tparse_ForK();
		break;
	case TK_WHILE: /* while ... wend statement */
		stmtNode = tparse_WhileK();
		break;
	case TK_DO: /* do [while ...] ... loop [ while .. ] */
		stmtNode = tparse_DoLoopStmt();
		break;
	case TK_EXIT: /* exit for | exit do */
		stmtNode = tparse_ExitStatement();
		break;
	case TK_RETURN:
		stmtNode = tparse_ReturnK();
		break;
	case TK_SELECT: /* select case statement */
		stmtNode = tparse_SelectK();
		break;
	case TK_ID: /* exp or assign statement */
		stmtNode = tparse_CallOrAssign();
		break;
	case TK_ERASE:
		stmtNode = tparse_EraseK();
		break;
	case TK_FREE:
		stmtNode = tparse_FreeK();
		break;
	case TK_LINEEND:
	case TK_END:
	default:
		return NULL;
	}
	return stmtNode;
}

static ParseTreeNode* tparse_CallOrAssign()
{
	ParseTreeNode *node;
	char           NextTokenString[MAX_CHAR];
	TokenType      NextToken;

	NextToken = tscan_PeekToken(NextTokenString);
	switch(NextToken)
	{
	case TK_MID_LP:
	case TK_EQ:
	case TK_DOT:
		node = tparse_AssignK();
		break;
	case TK_LP:
		node = tparse_CallK();
		break;
	default:
		{
			tparse_syntaxError("Unknown statement.\r\n");
			return NULL;
		}
	}
	return node;
}

static ParseTreeNode* tparse_AssignK()
{
	ParseTreeNode *AssignNode = tparse_newNode(AssignK);
	
	AssignNode->child[0] = tparse_VarnameK();
	tparse_match(TK_EQ);
	AssignNode->child[1] = tparse_ExpK();

	return AssignNode;
}

static ParseTreeNode* tparse_CallK()
{
	char *callName;
	ParseTreeNode *CallNode;

	callName = tparse_copyString(tparse_TokenString);

	CallNode = tparse_newNode(CallK);
	CallNode->attr_string = callName;
	tparse_match(TK_ID);
	if(tparse_Token == TK_LP) /* if this is a '(' */
	{
		tparse_match(TK_LP);
		CallNode->child[0]    = tparse_ParamlistK();
		tparse_match(TK_RP);
	}
	else /* if this is a parameter */
	{
		CallNode->child[0] = tparse_ParamlistK();
	}

	return CallNode;
}

static ParseTreeNode* tparse_ParamlistK()
{
	ParseTreeNode *paramlistNode = tparse_newNode(ParamlistK);
	ParseTreeNode *lastNode = NULL,*paramNode;

	while(!tparse_Error && tparse_Token != TK_RP)
	{
		paramNode = tparse_ExpK();
		if(lastNode == NULL)
			paramlistNode->child[0] = paramNode;
		else
			lastNode->sibling = paramNode;
		lastNode = paramNode;

		if(tparse_Token == TK_COMMA)
			tparse_match(TK_COMMA);

	}
	return paramlistNode;
}

static ParseTreeNode* tparse_ExpK()
{
	ParseTreeNode *ExpNode = tparse_newNode(ExpK);
	ParseTreeNode *tempNode;
	ParseTreeNode *PriNode;

	PriNode = tparse_SingleExp();
	while((tparse_Token== TK_AND) || (tparse_Token==TK_OR))
	{
		tempNode = tparse_newNode(OpK);
		if(tempNode != NULL)
		{
			tempNode->child[0] = PriNode;
			tempNode->attr_int = tparse_Token;
			PriNode = tempNode;
			tparse_match(tparse_Token);
			PriNode->child[1] = tparse_SingleExp();
		}
		
	}
	ExpNode->child[0] = PriNode;
	return ExpNode;
}

static ParseTreeNode* tparse_SingleExp()
{
	ParseTreeNode *tempNode;
	ParseTreeNode *PriNode = tparse_SimpleExp();
	while((tparse_Token== TK_EQ) || (tparse_Token==TK_NOTEQ) || (tparse_Token==TK_LG) ||
		(tparse_Token==TK_LT) || (tparse_Token==TK_LG_EQ) || (tparse_Token==TK_LT_EQ))
	{
		tempNode = tparse_newNode(OpK);
		if(tempNode != NULL)
		{
			tempNode->child[0] = PriNode;
			tempNode->attr_int = tparse_Token;
			PriNode = tempNode;
			tparse_match(tparse_Token);
			PriNode->child[1] = tparse_SimpleExp();
		}
	}
	return PriNode;
}

static ParseTreeNode* tparse_SimpleExp()
{
	ParseTreeNode *tempNode;
	ParseTreeNode *PriNode = tparse_Term();
	while((tparse_Token == TK_PLUS) || (tparse_Token == TK_MINUS))
	{
		tempNode = tparse_newNode(OpK);
		if(tempNode != NULL)
		{ 
			tempNode->child[0] = PriNode;
			tempNode->attr_int = tparse_Token;
			PriNode = tempNode;
			tparse_match(tparse_Token);
			PriNode->child[1] = tparse_Term();
		}
	}
	return PriNode;
}

static ParseTreeNode* tparse_Term()
{
	ParseTreeNode *tempNode;
	ParseTreeNode *PriNode = tparse_Factor();
	while((tparse_Token == TK_MUL) || (tparse_Token == TK_DIV) || (tparse_Token== TK_MOD))
	{
		tempNode = tparse_newNode(OpK);
		if(tempNode != NULL)
		{
			tempNode->child[0] = PriNode;
			tempNode->attr_int = tparse_Token;
			PriNode = tempNode;
			tparse_match(tparse_Token);
			PriNode->child[1] = tparse_Factor();
		}
	}
	return PriNode;
}

static ParseTreeNode* tparse_Factor()
{
	ParseTreeNode* factorNode = NULL;
	switch(tparse_Token)
	{
	case TK_NUM:
		factorNode = tparse_ConstNumK();
		break;
	case TK_DOUBLENUM:
		factorNode = tparse_ConstDoubleK();
		break;
	case TK_STR:
		factorNode = tparse_ConstStrK();
		break;
	case TK_CHARACTER:
		factorNode = tparse_LetterConstNum();
		break;
	case TK_ID: /* Call or Variable Name */
		{
			ParseTreeNode* ConstNode;
			char NextTokenString[MAX_CHAR];

			if(tscan_PeekToken(NextTokenString) == TK_LP)
			{
				factorNode = tparse_CallK();
			}
			else if((ConstNode = tparse_Const_Lookup(tparse_TokenString)) != NULL)
			{
				tparse_match(TK_ID);
				factorNode = tparse_copyNode(ConstNode);
			}
			else
			{
				factorNode = tparse_VarnameK();
			}
			break;
		}
	case TK_LP: /* ( */
		tparse_match(TK_LP); /* ( */
		factorNode = tparse_ExpK();
		tparse_match(TK_RP); /* ) */
		break;
	case TK_NEW:
		factorNode = tparse_NewK();
		break;
	case TK_MINUS:
		factorNode = tparse_OppositeFactor();
		break;
	case TK_TRUE:
		factorNode = tparse_ConstTrue();
		break;
	case TK_FALSE:
		factorNode = tparse_ConstFalse();
		break;
	case TK_NULL:
		factorNode = tparse_ConstNull();
		break;
	default:
		{
			char msg[MAX_MSG];
			sprintf(msg,"Unknown call or variable name : %s\r\n",tparse_TokenString);
			tparse_syntaxError(msg);
			break;
		}
	}
	return factorNode;
}

static ParseTreeNode*  tparse_OppositeFactor()
{
	ParseTreeNode* OppNode = tparse_newNode(OpK);

	tparse_match(TK_MINUS);
	OppNode->attr_int = TK_OPPOSITE;
	OppNode->child[0] = tparse_Factor();

	return OppNode;
}

static ParseTreeNode* tparse_ConstNumK()
{
	ParseTreeNode *ConstNumNode = tparse_newNode(ConstNumK);
	
	ConstNumNode->attr_int = atoi(tparse_TokenString);
	ConstNumNode->type     = IntegerType;
	tparse_match(TK_NUM);
	return ConstNumNode;
}

static ParseTreeNode*  tparse_LetterConstNum()
{
	ParseTreeNode *ConstNumKNode = tparse_newNode(ConstNumK);

	ConstNumKNode->attr_int = tparse_TokenString[0];
	ConstNumKNode->type     = IntegerType;
	tparse_match(TK_CHARACTER);
	return ConstNumKNode;
}

static ParseTreeNode* tparse_ConstDoubleK()
{
	ParseTreeNode *ConstDoubleNode = tparse_newNode(ConstDoubleK);

	ConstDoubleNode->attr_double = (double)atof(tparse_TokenString);
	ConstDoubleNode->type = DoubleType;
	tparse_match(TK_DOUBLENUM);
	return ConstDoubleNode;
}

static ParseTreeNode* tparse_ConstStrK()
{
	ParseTreeNode *ConstStrNode = tparse_newNode(ConstStrK);
	ConstStrNode->attr_string = tparse_copyString(tparse_TokenString);
	ConstStrNode->type        = StringType;
	tparse_match(TK_STR);
	return ConstStrNode;
}

static ParseTreeNode* tparse_VarnameK()
{
	ParseTreeNode *VarnameNode = tparse_newNode(VarnameK);
	ParseTreeNode *PriVarnameNode,*LastPriVarNode = NULL;

	PriVarnameNode = tparse_PrimitiveVarnameK();
	VarnameNode->child[0] = PriVarnameNode;
	if(PriVarnameNode->child[1] != NULL)
	{
		VarnameNode->child[1] = tparse_copyNode(PriVarnameNode->child[1]); /* copy the get or set parameter,const string node */
		return VarnameNode;
	}
	LastPriVarNode = PriVarnameNode;
	while(tparse_Token == TK_DOT)
	{
		tparse_match(TK_DOT);
		PriVarnameNode = tparse_PrimitiveVarnameK();
		LastPriVarNode->sibling = PriVarnameNode;
		
		if(PriVarnameNode->child[1] != NULL)
		{
			VarnameNode->child[1] = tparse_copyNode(PriVarnameNode->child[1]); /* copy the get or set parameter,const string node */
			return VarnameNode;
		}

		LastPriVarNode = PriVarnameNode;
	}

	return VarnameNode;
}

static ParseTreeNode* tparse_PrimitiveVarnameK()
{
	ParseTreeNode *PriVarnameNode = tparse_newNode(PrimitiveVarnameK);
	ParseTreeNode *ExpNode,*LastExpNode = NULL;

	/* get the name of the variable */
	PriVarnameNode->attr_string = tparse_copyString(tparse_TokenString);
	tparse_match(TK_ID);
	/* if it is a element of array type */
	while(tparse_Token == TK_MID_LP)
	{
		tparse_match(TK_MID_LP);

		/* if it is get or set */
		if(tparse_Token == TK_STR)
		{
			ExpNode = tparse_ConstStrK();
			PriVarnameNode->child[1] = ExpNode;
			tparse_match(TK_MID_RP);
			return PriVarnameNode;
		}

		ExpNode = tparse_ExpK();
		if(LastExpNode == NULL)
			PriVarnameNode->child[0] = ExpNode;
		else
			LastExpNode->sibling = ExpNode;
		LastExpNode = ExpNode;
		tparse_match(TK_MID_RP);
	}

	return PriVarnameNode;
}

static ParseTreeNode* tparse_IfK()
{
	ParseTreeNode *IfNode = tparse_newNode(IfK);
	ParseTreeNode *ElseIfNode,*LastNode,*ElseNode;

	/* if */
	tparse_match(TK_IF);
	IfNode->child[0] = tparse_ExpK();
	tparse_match(TK_THEN);
	IfNode->child[1] = tparse_StatementsK();
	LastNode = IfNode;
	/* elseif */
	while(tparse_Token == TK_ELSEIF && !tparse_Error)
	{
		tparse_match(TK_ELSEIF);
		ElseIfNode = tparse_newNode(IfK);
		ElseIfNode->child[0] = tparse_ExpK();
		tparse_match(TK_THEN);
		ElseIfNode->child[1] = tparse_StatementsK();
		LastNode->child[2]   = ElseIfNode;
		LastNode             = ElseIfNode;
	}
	/* else */
	if(tparse_Token == TK_ELSE)
	{
		tparse_match(TK_ELSE);
		ElseNode = tparse_newNode(ElseK);
		ElseNode->child[0] = tparse_StatementsK();
		LastNode->child[2] = ElseNode;
	}

	tparse_match(TK_END);
	tparse_match(TK_IF);
	return IfNode;
}

static ParseTreeNode* tparse_WhileK()
{
	ParseTreeNode *WhileNode = tparse_newNode(WhileK);

	tparse_match(TK_WHILE);

	WhileNode->child[0] = tparse_ExpK();
	WhileNode->child[1] = tparse_StatementsK();
	
	tparse_match(TK_WEND);

	return WhileNode;
}

static ParseTreeNode* tparse_ForK()
{
	ParseTreeNode *ForNode = tparse_newNode(ForK);

	tparse_match(TK_FOR);
	tparse_ForLoopDepth++;

	ForNode->child[0] = tparse_AssignK();
	tparse_match(TK_TO);
	ForNode->child[1] = tparse_ExpK();
	if(tparse_Token == TK_STEP)
	{
		tparse_match(TK_STEP);
		ForNode->child[2] = tparse_ExpK();
	}

	ForNode->child[3] = tparse_StatementsK();
	
	tparse_match(TK_NEXT);

	/* skip the NEXT loop variable */ 
	if(tparse_Token != TK_LINEEND)
	{
		tscan_GotoNextLine();
		tparse_Token = tscan_GetToken(tparse_TokenString);
	}
	
	tparse_ForLoopDepth--;

	return ForNode;
}

static ParseTreeNode* tparse_DoLoopStmt()
{
	tparse_Mark();
	tparse_match(TK_DO);
	switch(tparse_Token)
	{
	case TK_WHILE:
		tparse_Restore();
		return tparse_DoWhileLoopK();
	default:
		tparse_Restore();
		return tparse_DoLoopWhileK();
	}
}

static ParseTreeNode* tparse_DoWhileLoopK()
{
	ParseTreeNode* DoWhileLoopNode = tparse_newNode(DoWhileLoopK);

	tparse_match(TK_DO);
	tparse_match(TK_WHILE);
	tparse_DoLoopDepth++;

	DoWhileLoopNode->child[0] = tparse_ExpK();
	DoWhileLoopNode->child[1] = tparse_StatementsK();

	tparse_match(TK_LOOP);
	tparse_DoLoopDepth--;

	return DoWhileLoopNode;
}

static ParseTreeNode* tparse_DoLoopWhileK()
{
	ParseTreeNode* DoLoopWhileNode = tparse_newNode(DoLoopWhileK);

	tparse_match(TK_DO);
	tparse_DoLoopDepth++;

	DoLoopWhileNode->child[0] = tparse_StatementsK();
	tparse_match(TK_LOOP);
	if(tparse_Token == TK_WHILE)
	{
		tparse_match(TK_WHILE);
		DoLoopWhileNode->child[1] = tparse_ExpK();
	}
	tparse_DoLoopDepth--;

	return DoLoopWhileNode;
}

static ParseTreeNode* tparse_ExitStatement()
{
	tparse_Mark();
	tparse_match(TK_EXIT);
	switch(tparse_Token)
	{
	case TK_FOR:
		tparse_Restore();
		return tparse_ExitForK();
	case TK_DO:
		tparse_Restore();
		return tparse_ExitDoK();
	}
	return NULL;
}

static ParseTreeNode* tparse_ExitForK()
{
	ParseTreeNode* ExitForNode = NULL;

	if(tparse_ForLoopDepth == 0)
	{
		tparse_syntaxError("'Exit For' must be in For structure!\r\n");
		return NULL;
	}
	ExitForNode = tparse_newNode(ExitForK);

	tparse_match(TK_EXIT);
	tparse_match(TK_FOR);
	return ExitForNode;
}

static ParseTreeNode* tparse_ExitDoK()
{
	ParseTreeNode* ExitDoNode = NULL; 
	if(tparse_DoLoopDepth == 0)
	{
		tparse_syntaxError("'Exit Do' must be in Do Loop structure!\r\n");
		return NULL;
	}
	ExitDoNode = tparse_newNode(ExitDoK);

	tparse_match(TK_EXIT);
	tparse_match(TK_DO);

	return ExitDoNode;
}

static ParseTreeNode* tparse_ReturnK()
{
	ParseTreeNode* ReturnNode = tparse_newNode(ReturnK);

	tparse_match(TK_RETURN);

	if(tparse_Token != TK_LINEEND) /* if it has return expression */
	{
		
		ReturnNode->child[0] = tparse_ExpK();
	}

	return ReturnNode;
}


static ParseTreeNode* tparse_SelectK()
{
	ParseTreeNode *SelectNode = tparse_newNode(SelectK);
	ParseTreeNode *CaseNode,*LastCaseNode = NULL;
	ParseTreeNode *DefaultNode = NULL;

	tparse_match(TK_SELECT);
	tparse_match(TK_CASE);
	SelectNode->child[0] = tparse_ExpK();

	/* skip the line end */
	tparse_SkipLineEndToken();
	/* collect case parts */
	while(tparse_Token == TK_CASE || tparse_Token == TK_DEFAULT)
	{
		if(tparse_Token == TK_CASE) /* collect case parts */
		{
			tparse_match(TK_CASE);
			CaseNode = tparse_newNode(CaseK);
			CaseNode->child[0] = tparse_ExpK();
			CaseNode->child[1] = tparse_StatementsK();
			tparse_match(TK_END);

			if(LastCaseNode == NULL)
				SelectNode->child[1] = CaseNode;
			else
				LastCaseNode->sibling = CaseNode;
			LastCaseNode = CaseNode;
		}
		else /* collect default part */
		{
			if(SelectNode->child[2] != NULL) /* if already matched a default token */
			{
				tparse_syntaxError("Select Case Statement has already contain default statements!");
				break;
			}
			tparse_match(TK_DEFAULT);
			DefaultNode = tparse_newNode(DefaultK);
			DefaultNode->child[0] = tparse_StatementsK();
			tparse_match(TK_END);
			SelectNode->child[2] = DefaultNode;
		}
		/* skip the line end */
		tparse_SkipLineEndToken();
	}


	tparse_match(TK_END);
	tparse_match(TK_SELECT);

	return SelectNode;
}

static ParseTreeNode* tparse_EraseK()
{
	ParseTreeNode* EraseNode = tparse_newNode(EraseK);

	tparse_match(TK_ERASE);
	EraseNode->child[0] = tparse_VarnameK();

	return EraseNode;
}

static ParseTreeNode*  tparse_FreeK()
{
	ParseTreeNode* EraseNode = tparse_newNode(FreeK);

	tparse_match(TK_FREE);
	EraseNode->child[0] = tparse_VarnameK();

	return EraseNode;
}

static ParseTreeNode* tparse_NewK()
{
	ParseTreeNode *node = tparse_newNode(NewK);
	ParseTreeNode *expNode,*lastExpNode = NULL;
	int  VarType;
	char TypeName[MAX_CHAR];
	int  dimension = 0;

	/* match 'New' */
	tparse_match(TK_NEW);
	/* match type of variable */
	VarType = tparse_match_DataType(TypeName);
	node->attr_string = tparse_copyString(TypeName);
	node->attr_int = VarType;

	
	while(tparse_Token == TK_MID_LP)
	{
		tparse_match(TK_MID_LP); 
		expNode = tparse_ExpK();
		if(lastExpNode == NULL)
			node->child[0] = expNode;
		else
			lastExpNode->sibling = expNode;
		lastExpNode = expNode;
		dimension++;
		tparse_match(TK_MID_RP);
	}
	
	/* if the type is not Type,then it must be array */
	if(VarType != TypeType && dimension < 1)
	{
		tparse_syntaxError("New can only be used for array or type structure\r\n");
	}

	return node;
}

static int tparse_Const()
{
	ParseTreeNode* ConstNode;
	char ConstName[MAX_CHAR];

	tparse_match(TK_CONST); 
	strcpy(ConstName,tparse_TokenString);
	tparse_match(TK_ID);

	tparse_match(TK_EQ);
	switch(tparse_Token)
	{
	case TK_NUM:
		ConstNode = tparse_ConstNumK();
		break;
	case TK_DOUBLENUM:
		ConstNode = tparse_ConstDoubleK();
		break;
	case TK_STR:
		ConstNode = tparse_ConstStrK();
		break;
	default:
		tparse_syntaxError("Unknown type of Constant value\r\n");
		return 0;
	}

	/* insert to constant record table */
	tparse_Const_Insert(ConstName,ConstNode);

	return 1;
}

static ParseTreeNode* tparse_ConstNull()
{
	ParseTreeNode *ConstNumNode = tparse_newNode(ConstNumK);

	ConstNumNode->attr_int = 0;
	ConstNumNode->type     = IntegerType;
	tparse_match(TK_NULL);
	return ConstNumNode;
}

static ParseTreeNode* tparse_ConstTrue()
{
	ParseTreeNode *ConstNumNode = tparse_newNode(ConstNumK);

	ConstNumNode->attr_int = 1;
	ConstNumNode->type     = IntegerType;
	tparse_match(TK_TRUE);
	return ConstNumNode;
}

static ParseTreeNode* tparse_ConstFalse()
{
	ParseTreeNode *ConstNumNode = tparse_newNode(ConstNumK);

	ConstNumNode->attr_int = 0;
	ConstNumNode->type     = IntegerType;
	tparse_match(TK_FALSE);
	return ConstNumNode;
}
