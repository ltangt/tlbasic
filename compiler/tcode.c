#include "tcode.h"

#include "tbytecode.h"
#include "tfile.h"
#include "global.h"
#include "tscan.h"
#include "tcodelen.h"
#include "tcodewriter.h"
#include "tanalysis.h"

tCodeFile *tcode_codefile;
int        tcode_Error;

/*=========================================================================*/
void tcode_printError(char* msg,ParseTreeNode* curNode);
/* void tcode_writeNativeFunctions(); */

/*=========================================================================*/
/* generate the corresponding code to the parse tree node */
static int  tcode_ProgramK(ParseTreeNode* programKNode);
static int  tcode_SubK(ParseTreeNode* subKNode);
static int  tcode_FunctionK(ParseTreeNode* functionKNode);
static int  tcode_DimK_Global(ParseTreeNode* dimKNode);
static int  tcode_StatementsK(ParseTreeNode* statementsKNode);
static int  tcode_DimK_Local(ParseTreeNode* dimKNode);
static int  tcode_AssignK(ParseTreeNode* assignKNode);
static int  tcode_ExpK(ParseTreeNode* expKNode);
static int  tcode_OpK(ParseTreeNode* opKNode);
static int  tcode_ConstNumK(ParseTreeNode* constNumKNode);
static int  tcode_ConstDoubleK(ParseTreeNode* constDoubleKNode);
static int  tcode_ConstStrK(ParseTreeNode* constStrKNode);
static int  tcode_VarnameK_Load(ParseTreeNode* varnameKNode);
static int  tcode_VarnameK_Store(ParseTreeNode* varnameKNode);
static int  tcode_PrimitiveVarnamesK_Load(ParseTreeNode* primitivesKNode);
static int  tcode_PrimitiveVarnamesK_Store(ParseTreeNode* primitivesKNode);
static int  tcode_CallK_Stmt(ParseTreeNode* callKNode);
static int  tcode_CallK_Exp(ParseTreeNode* callKNode);
static int  tcode_IfK(ParseTreeNode* ifKNode);
static int  tcode_WhileK(ParseTreeNode* whileKNode);
static int  tcode_ForK(ParseTreeNode* forKNode);
static int  tcode_DoWhileLoopK(ParseTreeNode* doloopKNode);
static int  tcode_DoLoopWhileK(ParseTreeNode* doloopKNode);
static int  tcode_ExitForK(ParseTreeNode* exitforKNode);
static int  tcode_ExitDoK(ParseTreeNode* exitdoKNode);
static int  tcode_ReturnK(ParseTreeNode* returnKNode);
static int  tcode_SelectK(ParseTreeNode* selectKNode);
static int  tcode_EraseK(ParseTreeNode* eraseKNode);
static int  tcode_FreeK(ParseTreeNode* freeKNode);
static int  tcode_NewK(ParseTreeNode* newKNode);

/*=========================================================================*/
tCodeFile* tcode_gen(ParseTreeNode* parsetree)
{
	/* initialize the struct of code file */
	tcode_codefile = tcodefile_create();
	/* initialize the Error flag */
	tcode_Error = 0;
	/* initialize the SubAndFun table */
	tanalysis_SubAndFun_create();
	/* initialize the GlobalVar table */
	tanalysis_GlobalVar_create();
	/* initialize the TypeDefine table */
	tanalysis_TypeDefine_create();
	/* initialize the address record stacks */
	tanalysis_InitializeAddressStacks();
	/* initialize the native-use table */
	tanalysis_UsedNativeFunc_init();

	/* begin to generate code */
	tcode_ProgramK(parsetree);
	///* write used native functions */
	//tcode_writeNativeFunctions();

	/* release the native-use table */
	tanalysis_UsedNativeFunc_release();
	/* release the address record stacks */
	tanalysis_ReleaseAddressStacks();
	/* free SubAndFun table */
	tanalysis_SubAndFun_release();
	/* free GlobalVar table */
	tanalysis_GlobalVar_release();
	/* free TypeDefine table */
	tanalysis_TypeDefine_release();

	/* return the result */
	if(tcode_Error)
		return NULL;
	else
		return tcode_codefile;
}

/*=========================================================================*/
void tcode_printError(char* msg,ParseTreeNode* curNode)
{
	char errormsg[MAX_MSG];
	sprintf(errormsg,"%s : Code generate error at line %d : %s",curNode->srcfilename,curNode->lineno,msg);
	fprintf(g_listfp,errormsg);
	tcode_Error = 1;
}


/*=========================================================================*/
static int tcode_ProgramK(ParseTreeNode* programKNode)
{
	ParseTreeNode *ChildNode;
	
	/* collect the global informations into tables */
	tanalysis_collectGlobalNodes(programKNode);

	/* get the children node  */
	ChildNode = programKNode->child[0];
	while(ChildNode != NULL && !tcode_Error)
	{
		switch(ChildNode->nodekind)
		{
		case SubK: 
			tcode_SubK(ChildNode);
			break;
		case FunctionK:
			tcode_FunctionK(ChildNode);
			break;
		case DimK:
			tcode_DimK_Global(ChildNode);
			break;

		}
		ChildNode = ChildNode->sibling;
	}
	return 1;
}

static int tcode_DimK_Global(ParseTreeNode* dimKNode)
{
	u8  type;
	u8* name;
	ParseTreeNode* DimVarNode = dimKNode->child[0];

	while(DimVarNode != NULL)
	{
		name = (u8*)DimVarNode->attr_string;
		if(DimVarNode->dimension > 0)
			type = PtrType;
		else
			type = (u8)DimVarNode->attr_int;
		tcodefile_insert_globalvar(tcode_codefile,(const char*)name,type);

		DimVarNode = DimVarNode->sibling;
	}

	/* global variable can not be initialized in definition */
	if(dimKNode->child[1] != NULL)
	{
		tcode_printError("global variable can not be initialized in definition!",dimKNode);
		return 0;
	}
	return 1;
}

static int tcode_SubK(ParseTreeNode* subKNode)
{
	u8* codebuf;
	u32 codesize;
	u8* name = (u8*)subKNode->attr_string;

	/* create code output writer */
	tcodewriter_create();

	/* create parameters table and collect parameters */
	tanalysis_ArgDef_create();
	tanalysis_collectArgList(subKNode);
	/* create local variables table */
	tanalysis_LocalVar_create();

	/* get the code */
	tcode_StatementsK(subKNode->child[1]);
	
	/* add push return NullType */
	tcodewriter_PUSH(NullType);
	/* add return code: RET Parameter Bytes */
	tcodewriter_RET(tanalysis_ArgDef_GetTopIndexInByte());

	/* write the code of subroutine to code file */
	codebuf	 = (u8*)tcodewriter_getbuf(&codesize);
	tcodefile_insert_function(tcode_codefile,(const char*)name,NullType,codebuf,codesize);

	/* release the local variables table */
	tanalysis_LocalVar_release();
	/* release the parameters table */
	tanalysis_ArgDef_release();
	/* release the writer of code */
	tcodewriter_release();
	/* free the code buffer */
	free(codebuf);

	return 1;
}

static int tcode_FunctionK(ParseTreeNode* functionKNode)
{
	u8* codebuf;
	u32 codesize;
	u8  type = functionKNode->attr_int;
	u8* name = (u8*)functionKNode->attr_string;

	/* create code output writer */
	tcodewriter_create();

	/* create parameters table and collect parameters */
	tanalysis_ArgDef_create();
	tanalysis_collectArgList(functionKNode);
	/* create local variables table */
	tanalysis_LocalVar_create();

	/* generate the code */
	tcode_StatementsK(functionKNode->child[1]);

	/* push a default return value */
	tcodewriter_PUSH(type);
	/* add return code */
	tcodewriter_RET(tanalysis_ArgDef_GetTopIndexInByte());

	/* write the code of function to code file */
	codebuf	 = (u8*)tcodewriter_getbuf(&codesize);
	tcodefile_insert_function(tcode_codefile,(const char*)name,type,codebuf,codesize);

	/* release the local variables table */
	tanalysis_LocalVar_release();
	/* release the parameters table */
	tanalysis_ArgDef_release();
	/* release the writer of code */
	tcodewriter_release();
	/* free the code buffer */
	free(codebuf);

	return 1;
}

static int tcode_StatementsK(ParseTreeNode* statementsKNode)
{
	ParseTreeNode* stmtNode = statementsKNode->child[0];
	long           localvarNum = 0;

	while(stmtNode != NULL && !tcode_Error)
	{
		switch(stmtNode->nodekind)
		{
		case DimK:
			localvarNum += tcode_DimK_Local(stmtNode);
			break;
		case AssignK:
			tcode_AssignK(stmtNode);
			break;
		case CallK:
			tcode_CallK_Stmt(stmtNode);
			break;
		case IfK:
			tcode_IfK(stmtNode);
			break;
		case WhileK:
			tcode_WhileK(stmtNode);
			break;
		case DoLoopWhileK:
			tcode_DoLoopWhileK(stmtNode);
			break;
		case DoWhileLoopK:
			tcode_DoWhileLoopK(stmtNode);
			break;
		case ExitDoK:
			tcode_ExitDoK(stmtNode);
			break;
		case ExitForK:
			tcode_ExitForK(stmtNode);
			break;
		case ReturnK:
			tcode_ReturnK(stmtNode);
			break;
		case ForK:
			tcode_ForK(stmtNode);
			break;
		case SelectK:
			tcode_SelectK(stmtNode);
			break;
		case EraseK:
			tcode_EraseK(stmtNode);
			break;
		case FreeK:
			tcode_FreeK(stmtNode);
			break;
		}
		stmtNode = stmtNode->sibling;
	}

	/* generate the Pop Local Variable codes */
	tcodewriter_POP(localvarNum);

	while(localvarNum > 0)
	{
		tanalysis_LocalVar_pop();
		localvarNum--;
	}

	return 1;
}

static int tcode_DimK_Local(ParseTreeNode* dimKNode)
{
	ParseTreeNode* DimVarNode;
	u8  type;
	int dimension;
	int num_var = 0;

	DimVarNode = dimKNode->child[0];
	while(DimVarNode != NULL)
	{
		type = (u8)DimVarNode->attr_int;
		/* if the variable is type,check the type name */
		if(type == TypeType)
		{
			if(!tanalysis_TypeDefine_lookup(DimVarNode->attr_string2))
			{
				char msg[MAX_MSG];
				sprintf(msg,"Unknown variable type : %s \r\n",DimVarNode->attr_string2);
				tcode_printError(msg,dimKNode);
				break;
			}
		}
		dimension = tanalysis_GetVarDimension(DimVarNode);
		if(dimension > 0) /* if it is array */
			type = PtrType;
		/* push into table */
		tanalysis_LocalVar_push(DimVarNode);
		/* write code */
		tcodewriter_PUSH(type);
		/* init the variable */
		if(dimKNode->child[1] != NULL)
		{
			/* generate the init value */
			tcode_ExpK(dimKNode->child[1]); 
			/* STORE VAR_ADDR */
			tcodewriter_STORE(ACCESS_FLAG_LOCAL,DimVarNode->address);
		}
		
		DimVarNode = DimVarNode->sibling;
		num_var++;
	}
	return num_var;
}

static int tcode_AssignK(ParseTreeNode* assignKNode)
{
	tcode_ExpK(assignKNode->child[1]);
	tcode_VarnameK_Store(assignKNode->child[0]);
	return 1;
}

static int tcode_ExpK(ParseTreeNode* expKNode)
{
	ParseTreeNode* Node = expKNode->child[0];
	if(Node == NULL)
		return 1;

	switch(Node->nodekind)
	{
	case OpK:
		tcode_OpK(Node);
		break;
	case CallK:
		tcode_CallK_Exp(Node);
		break;
	case ConstNumK:
		tcode_ConstNumK(Node);
		break;
	case ConstDoubleK:
		tcode_ConstDoubleK(Node);
		break;
	case ConstStrK:
		tcode_ConstStrK(Node);
		break;
	case VarnameK:
		tcode_VarnameK_Load(Node);
		break;
	case ExpK:
		tcode_ExpK(Node);
		break;
	case NewK:
		tcode_NewK(Node);
		break;
	}
	return 1;
}


static int tcode_OpK(ParseTreeNode* opKNode)
{
	ParseTreeNode* ChildNode;
	int i;

	for(i=0;i<2; i++)
	{
		ChildNode = opKNode->child[i];
		if(ChildNode != NULL)
		{
			switch(ChildNode->nodekind)
			{
			case OpK:
				tcode_OpK(ChildNode);
				break;
			case CallK:
				tcode_CallK_Exp(ChildNode);
				break;
			case ConstNumK:
				tcode_ConstNumK(ChildNode);
				break;
			case ConstDoubleK:
				tcode_ConstDoubleK(ChildNode);
				break;
			case ConstStrK:
				tcode_ConstStrK(ChildNode);
				break;
			case VarnameK:
				tcode_VarnameK_Load(ChildNode);
				break;
			case ExpK:
				tcode_ExpK(ChildNode);
				break;
			}
		}
	}
	switch(opKNode->attr_int)
	{
	case TK_AND:
		tcodewriter_AND();
		break;
	case TK_OR:
		tcodewriter_OR();
		break;
	case TK_PLUS:
		tcodewriter_ADD();
		break;
	case TK_MINUS:
		tcodewriter_SUB();
		break;
	case TK_MUL:
		tcodewriter_MUL();
		break;
	case TK_DIV:
		tcodewriter_DIV();
		break;
	case TK_MOD:
		tcodewriter_MOD();
		break;
	case TK_LG:
		tcodewriter_LG();
		break;
	case TK_LT:
		tcodewriter_LT();
		break;
	case TK_EQ:
		tcodewriter_EQ();
		break;
	case TK_LG_EQ:
		tcodewriter_LG_EQ();
		break;
	case TK_LT_EQ:
		tcodewriter_LT_EQ();
		break;
	case TK_NOTEQ:
		tcodewriter_NOT_EQ();
		break;
	case TK_OPPOSITE:
		tcodewriter_OPP();
		break;
	}

	return 1;
}

static int tcode_CallK_Exp(ParseTreeNode* callKNode)
{
	char msg[MAX_MSG];
	int IsNativeCall;
	int function_index,native_index;
	ParseTreeNode *ParamListNode,*ParamNode;
	ParseTreeNode *SubOrFunNode = tanalysis_SubAndFun_lookup(callKNode->attr_string);
	ParseTreeNode *ArgListNode,*ArgNode;
	/* if the called function or subroutine is not exist */
	if(SubOrFunNode == NULL)
	{
		sprintf(msg,"Cannot find the subroutine or function : %s\r\n",callKNode->attr_string);
		tcode_printError(msg,callKNode);
		return 0;
	}
		
	/* generate push the parameters */
	ParamListNode = callKNode->child[0];
	ParamNode = ParamListNode->child[0];
	ArgListNode = SubOrFunNode->child[0];
	ArgNode     = ArgListNode->child[0];
	while(ParamNode!= NULL && !tcode_Error)
	{
		if(ArgNode == NULL)
		{
			sprintf(msg,"The count of parameters is over the definition : %s\r\n",callKNode->attr_string);
			tcode_printError(msg,callKNode);
			return 0;
		}

		tcode_ExpK(ParamNode);
		if(ParamNode->type != ArgNode->attr_int)
		{
			/* convert the parameter value type to the argument value type */
			if(ArgNode->dimension > 0)
				tcodewriter_CONVERT((u8)PtrType);
			else
				tcodewriter_CONVERT((u8)(ArgNode->attr_int));
		}

		ParamNode = ParamNode->sibling;
		ArgNode = ArgNode->sibling;
	}

	/* get the function index */
	function_index = SubOrFunNode->address;
	/* determine call or invoke */
	IsNativeCall = SubOrFunNode->modifier == NativeModifier;
	if(IsNativeCall)
	{
		native_index = SubOrFunNode->address;
		/* native_index = tanalysis_UsedNativeFunc_insert(callKNode->attr_string); */
		native_index+= 2; /* add set and get native function */

		/* INVOKE name */
		tcodewriter_INVOKE(native_index);
	}
	else
	{
		/* CALL index */
		tcodewriter_CALL(function_index);
	}

	return 1;
}

static int tcode_CallK_Stmt(ParseTreeNode* callKNode)
{
	char msg[MAX_MSG];
	int IsNativeCall;
	int function_index,native_index;
	ParseTreeNode *ParamListNode,*ParamNode;
	ParseTreeNode *SubOrFunNode = tanalysis_SubAndFun_lookup(callKNode->attr_string);
	ParseTreeNode *ArgListNode,*ArgNode;
	/* if the called function or subroutine is not exist */
	if(SubOrFunNode == NULL)
	{
		sprintf(msg,"Cannot find the subroutine or function : %s\r\n",callKNode->attr_string);
		tcode_printError(msg,callKNode);
		return 0;
	}
	
	/* generate the parameters */
	ParamListNode = callKNode->child[0];
	ParamNode = ParamListNode->child[0];
	ArgListNode = SubOrFunNode->child[0];
	ArgNode     = ArgListNode->child[0];
	while(ParamNode!= NULL && !tcode_Error)
	{
		if(ArgNode == NULL)
		{
			sprintf(msg,"The count of parameters is over the definition : %s\r\n",callKNode->attr_string);
			tcode_printError(msg,callKNode);
			return 0;
		}

		tcode_ExpK(ParamNode);
		if(ParamNode->type != ArgNode->attr_int)
		{
			/* convert the parameter value type to the argument value type */
			if(ArgNode->dimension > 0)
				tcodewriter_CONVERT((u8)PtrType);
			else
				tcodewriter_CONVERT((u8)(ArgNode->attr_int));
		}

		ParamNode = ParamNode->sibling;
		ArgNode = ArgNode->sibling;
	}
	/* get the function index */
	function_index = SubOrFunNode->address;
	/* determine call or invoke */
	IsNativeCall = SubOrFunNode->modifier == NativeModifier;
	if(IsNativeCall)
	{
		native_index = SubOrFunNode->address;
		/* native_index = tanalysis_UsedNativeFunc_insert(callKNode->attr_string); */
		native_index+= 2; /* add set and get native function */

		/* INVOKE name */
		tcodewriter_INVOKE(native_index);
	}
	else
	{
		/* CALL index */
		tcodewriter_CALL(function_index);
	}
	/* POP return value */
	tcodewriter_POP(1);	
	return 1;
}


static int tcode_ConstNumK(ParseTreeNode* constNumKNode)
{
	tcodewriter_CONST_Integer(constNumKNode->attr_int);
	constNumKNode->type = IntegerType;
	return 1;
}

static int tcode_ConstDoubleK(ParseTreeNode* constDoubleKNode)
{
	tcodewriter_CONST_Double(constDoubleKNode->attr_double);
	constDoubleKNode->type = DoubleType;
	return 1;
}

static int tcode_ConstStrK(ParseTreeNode* constStrKNode)
{
	tcodewriter_CONST_String(constStrKNode->attr_string);
	constStrKNode->type = StringType;
	return 1;
}

static int tcode_VarnameK_Load(ParseTreeNode* varnameKNode)
{
	tcode_PrimitiveVarnamesK_Load(varnameKNode->child[0]);
	/* if it is get */
	if(varnameKNode->child[1] != NULL) 
	{
		tcode_ConstStrK(varnameKNode->child[1]);
		tcodewriter_INVOKE(NATIVEFUNC_GET);
		varnameKNode->type = NullType; /* Unknown data type */
	}
	return 1;
}

static int tcode_PrimitiveVarnamesK_Load(ParseTreeNode* primitivesKNode)
{
	char msg[MAX_MSG];
	ParseTreeNode* PriVarNameNode;
	ParseTreeNode* DimVarNode;
	ParseTreeNode* IndexExpNode;
	int  IsGlobaVar;
	int  Address;
	int  find;
	int  priType,type;
	int  dimension;
	u8   access;
	char TypeName[MAX_CHAR];

	PriVarNameNode = primitivesKNode;
	find = tanalysis_GetVariable(PriVarNameNode->attr_string,&IsGlobaVar,&Address,&priType,&dimension);
	/* if cannot find the definition of the variable */
	if(!find)
	{
		sprintf(msg,"the variable : %s have not been defined before! \r\n",PriVarNameNode->attr_string);
		tcode_printError(msg,primitivesKNode);
		return 0;
	}
	if(IsGlobaVar)
		access = ACCESS_FLAG_GLOBAL;
	else
		access = ACCESS_FLAG_LOCAL;

	if(PriVarNameNode->child[0] != NULL) /* if it is a array */
	{
		/* FORCE_LOAD access address PtrType */
		tcodewriter_FORCE_LOAD(access,Address,PtrType);

		IndexExpNode = PriVarNameNode->child[0];
		while(IndexExpNode != NULL)
		{
			/* check the number of dimension now */
			dimension--;
			if(dimension < -1 || ( dimension == -1 && priType != StringType)) /* if dimension is over definition */
			{
				char msg[MAX_CHAR];
				sprintf(msg,"the dimension of variable %s is over the definition.\r\n",PriVarNameNode->attr_string);
				tcode_printError(msg,primitivesKNode);
			}

			if(IndexExpNode->sibling != NULL)
				type = PtrType;
			else
			{
				/* if it is access byte of the string */
				if(dimension == -1 && priType == StringType)
					type = ByteType;
				else
					type = priType;
			}
			/* generate the index of the element of the array */
			tcode_ExpK(IndexExpNode);
			tcodewriter_CONST_Integer(sizeofData((DataType)type));
			tcodewriter_MUL();
			/* HEAP_LOAD type */
			tcodewriter_HEAP_LOAD(type);

			IndexExpNode = IndexExpNode->sibling;
		}
	}
	else
	{
		/* LOAD access address */
		tcodewriter_LOAD(access,Address);
	}

	/* get the variable type name */
	tanalysis_GetVariableTypeName(PriVarNameNode->attr_string,TypeName);
	PriVarNameNode = PriVarNameNode->sibling;
	while(PriVarNameNode != NULL && !tcode_Error)
	{
		DimVarNode = tanalysis_TypeDefine_findmember(TypeName,PriVarNameNode->attr_string);
		if(DimVarNode == NULL)
		{
			sprintf(msg,"Cannot the member of %s : %s. \r\n",TypeName,PriVarNameNode->attr_string);
			tcode_printError(msg,primitivesKNode);
			return 0;
		}
		/* get the address of the member */
		Address  = DimVarNode->address;
		/* get the type of the member */
		priType  = DimVarNode->attr_int;
		/* get the dimension of the member */
		dimension= DimVarNode->dimension;

		/* CONST IntegerType Address */
		tcodewriter_CONST_Integer(Address);
		if(PriVarNameNode->child[0] != NULL) /* if it is a array */
		{
			/* HEAP_LOAD type */
			tcodewriter_HEAP_LOAD(PtrType);

			IndexExpNode = PriVarNameNode->child[0];
			while(IndexExpNode != NULL)
			{
				/* check the number of dimension now */
				dimension--;
				if(dimension < -1 || ( dimension == -1 && priType != StringType) ) /* if dimension is over definition */
				{
					char msg[MAX_CHAR];
					sprintf(msg,"the dimension of variable %s is over the definition.\r\n",PriVarNameNode->attr_string);
					tcode_printError(msg,primitivesKNode);
				}

				if(IndexExpNode->sibling != NULL)
					type = PtrType;
				else
				{
					/* if it is access byte of the string */
					if(dimension == -1 && priType == StringType)
						type = ByteType;
					else
						type = priType;
				}

				/* generate the index of the element of the array */
				tcode_ExpK(IndexExpNode); 
				tcodewriter_CONST_Integer(sizeofData((DataType)type));
				tcodewriter_MUL();
				/* HEAP_LOAD type */
				tcodewriter_HEAP_LOAD(type);

				IndexExpNode = IndexExpNode->sibling;
			}
		}
		else /* if it is not array */
		{
			/* HEAP_LOAD type */
			tcodewriter_HEAP_LOAD(priType);
		}

		/* turn to the next type */
		strcpy(TypeName,DimVarNode->attr_string2); 
		PriVarNameNode = PriVarNameNode->sibling;
	}
	return 1;
}

static int tcode_VarnameK_Store(ParseTreeNode* varnameKNode)
{
	/* if it is set */
	if(varnameKNode->child[1] != NULL)
	{
		tcode_PrimitiveVarnamesK_Load(varnameKNode->child[0]);
		tcode_ConstStrK(varnameKNode->child[1]);
		tcodewriter_INVOKE(NATIVEFUNC_SET);
		tcodewriter_POP(1);
	}
	else
	{
		tcode_PrimitiveVarnamesK_Store(varnameKNode->child[0]);
	}
	return 1;
}

static int tcode_PrimitiveVarnamesK_Store(ParseTreeNode* primitivesKNode)
{
	char msg[MAX_MSG];
	ParseTreeNode* PriVarNameNode;
	ParseTreeNode* DimVarNode;
	ParseTreeNode* IndexExpNode;
	int IsGlobaVar;
	int Address;
	int find;
	int priType,type;
	int dimension;
	u8  access;
	char TypeName[MAX_CHAR];

	PriVarNameNode = primitivesKNode;
	find = tanalysis_GetVariable(PriVarNameNode->attr_string,&IsGlobaVar,&Address,&priType,&dimension);
	/* if cannot find the definition of the variable */
	if(!find)
	{
		char msg[MAX_MSG];
		sprintf(msg,"the variable : %s have not been defined before! \r\n",PriVarNameNode->attr_string);
		tcode_printError(msg,primitivesKNode);
		return 0;
	}
	if(IsGlobaVar)
		access = ACCESS_FLAG_GLOBAL;
	else
		access = ACCESS_FLAG_LOCAL;

	if(PriVarNameNode->sibling == NULL) /* if it is the end primitive node */
	{
		if(PriVarNameNode->child[0] != NULL) /* if it is a array */
		{
			/* generate the base address of the array */
			tcodewriter_FORCE_LOAD(access,Address,PtrType);
			/* tcodewriter_LOAD(access,Address); */

			IndexExpNode = PriVarNameNode->child[0];
			while(IndexExpNode != NULL)
			{
				/* check the number of dimension now */
				dimension--;
				if(dimension < -1 || ( dimension == -1 && priType != StringType)) /* if dimension is over definition */
				{
					char msg[MAX_CHAR];
					sprintf(msg,"the dimension of variable %s is over the definition.\r\n",PriVarNameNode->attr_string);
					tcode_printError(msg,primitivesKNode);
				}

				if(IndexExpNode->sibling != NULL)
				{
					/* generate the index of the element of the array */
					tcode_ExpK(IndexExpNode);
					tcodewriter_CONST_Integer(sizeofData(PtrType));
					tcodewriter_MUL();
					/* HEAP_LOAD type */
					tcodewriter_HEAP_LOAD(PtrType);
				}
				else
				{
					/* if it is access byte of the string */
					if(dimension == -1 && priType == StringType)
						type = ByteType;
					else
						type = priType;
					/* generate the index of the element of the array */
					tcode_ExpK(IndexExpNode);
					tcodewriter_CONST_Integer(sizeofData((DataType)type));
					tcodewriter_MUL();
					/* HEAP_STORE type */
					tcodewriter_HEAP_STORE(type);
				}

				IndexExpNode = IndexExpNode->sibling;

			}
		}
		else /* if it is not a array */
		{
			/* STORE access address */
			tcodewriter_STORE(access,Address);
		}
	}
	else /* if it is not end primitive node */
	{
		/* LOAD access address */
		tcodewriter_LOAD(access,Address);
		if(PriVarNameNode->child[0] != NULL) /* if it is a array */
		{
			IndexExpNode = PriVarNameNode->child[0];
			while(IndexExpNode != NULL)
			{
				/* check the number of dimension now */
				dimension--;
				if(dimension < 0 ) /* if dimension is over definition */
				{
					char msg[MAX_CHAR];
					sprintf(msg,"the dimension of variable %s is over the definition.\r\n",PriVarNameNode->attr_string);
					tcode_printError(msg,primitivesKNode);
				}

				if(IndexExpNode->sibling != NULL)
					type = PtrType;
				else
					type = priType;
				/* generate the index of the element of the array */
				tcode_ExpK(IndexExpNode);
				tcodewriter_CONST_Integer(sizeofData((DataType)type));
				tcodewriter_MUL();
				/* HEAP_LOAD type */
				tcodewriter_HEAP_LOAD(type);

				IndexExpNode = IndexExpNode->sibling;
			}
		}

		tanalysis_GetVariableTypeName(PriVarNameNode->attr_string,TypeName);
		PriVarNameNode = PriVarNameNode->sibling;

		while(PriVarNameNode != NULL && !tcode_Error)
		{
			DimVarNode = tanalysis_TypeDefine_findmember(TypeName,PriVarNameNode->attr_string);
			if(DimVarNode == NULL)
			{
				sprintf(msg,"Cannot the member of %s : %s. \r\n",TypeName,PriVarNameNode->attr_string);
				tcode_printError(msg,primitivesKNode);
				return 0;
			}

			/* get the address of the member */
			Address    = DimVarNode->address;
			/* get the type of the member */
			priType    = DimVarNode->attr_int;
			/* get the dimension of the member */
			dimension  = DimVarNode->dimension;

			/* CONST IntegerType Address */
			tcodewriter_CONST_Integer(Address);

			if(PriVarNameNode->child[0] != NULL) /* if it is a array */
			{
				/* HEAP_LOAD type */
				tcodewriter_HEAP_LOAD(PtrType);

				IndexExpNode = PriVarNameNode->child[0];
				while(IndexExpNode != NULL)
				{
					/* check the number of dimension now */
					dimension--;
					if(dimension < -1 || ( dimension  == -1 && priType != StringType) ) /* if dimension is over definition */
					{
						char msg[MAX_CHAR];
						sprintf(msg,"the dimension of variable %s is over the definition.\r\n",PriVarNameNode->attr_string);
						tcode_printError(msg,primitivesKNode);
					}

					if(IndexExpNode->sibling != NULL)
					{
						/* generate the index of the element of the array */
						tcode_ExpK(IndexExpNode);
						tcodewriter_CONST_Integer(sizeofData(PtrType));
						tcodewriter_MUL();
						/* HEAP_LOAD type */
						tcodewriter_HEAP_LOAD(PtrType);
					}
					else
					{
						/* generate the index of the element of the array */
						tcode_ExpK(IndexExpNode);
						tcodewriter_CONST_Integer(sizeofData((DataType)priType));
						tcodewriter_MUL();
						if(PriVarNameNode->sibling != NULL) /* not end primitive node */
						{
							/* HEAP_LOAD type */
							tcodewriter_HEAP_LOAD(priType);
						}
						else /* end primitive node */
						{
							if(dimension == -1 && priType == StringType)
								type = ByteType;
							else
								type = priType;
							/* HEAP_STORE type */
							tcodewriter_HEAP_STORE(type);
						}
					}
					IndexExpNode = IndexExpNode->sibling;
				}
			}
			else /* if it is not array */
			{
				if(PriVarNameNode->sibling == NULL) /*  if it is end primitive node */
				{
					/* HEAP_STORE type */
					tcodewriter_HEAP_STORE(priType);
				}
				else 
				{
					/* HEAP_LOAD type */
					tcodewriter_HEAP_LOAD(PtrType);
				}
			}
			/* turn to the next type */
			strcpy(TypeName,DimVarNode->attr_string2); 
			PriVarNameNode = PriVarNameNode->sibling;
		}
	}

	return 1;
}

static int tcode_IfK(ParseTreeNode* ifKNode)
{
	u32            Then_CodeSize;
	u32            Else_CodeSize;
	ParseTreeNode* ConditionExpKNode = ifKNode->child[0];
	ParseTreeNode* ThenNode = ifKNode->child[1];
	ParseTreeNode* ElseNode = ifKNode->child[2];

	/* get the code size of Then Statements */
	Then_CodeSize = tcodelen_StatementsK(ThenNode);

	/* get the code size of Else Statements */
	if(ElseNode == NULL)
	{
		Else_CodeSize = 0;
	}
	else
	{
		switch(ElseNode->nodekind)
		{
		case ElseK:
			Else_CodeSize = tcodelen_StatementsK(ElseNode->child[0]);
			break;
		case IfK:
			Else_CodeSize = tcodelen_IfK(ElseNode);
			break;
		}
		/* add size of the bytecode : JMP ELSE_SIZE  */
		Then_CodeSize += tcodelen_ByteCode(C_JMP,NullType,NULL);
	}
	
	/* generate the condition  */
	tcode_ExpK(ConditionExpKNode);
	/* FJP THEN_SIZE */
	tcodewriter_FJP(Then_CodeSize);
	/* generate the Then part */
	tcode_StatementsK(ThenNode);
	/* else part */
	if(ElseNode != NULL)
	{
		/* JMP ELSE_SIZE */
		tcodewriter_JMP(Else_CodeSize);
		/* generate the Else part */
		switch(ElseNode->nodekind)
		{
		case ElseK:
			tcode_StatementsK(ElseNode->child[0]);
			break;
		case IfK:
			tcode_IfK(ElseNode);
			break;
		}
	}

	return 1;
}

static int tcode_WhileK(ParseTreeNode* whileKNode)
{
	long WhileBodyCodeSize;
	long ConditionExpCodeSize;

	/* get the while body code size */
	WhileBodyCodeSize = tcodelen_StatementsK(whileKNode->child[1]);
	/* add JMP BEGIN */
	WhileBodyCodeSize+= tcodelen_ByteCode(C_JMP,NullType,NULL);
	/* get condition expression code size */
	ConditionExpCodeSize = tcodelen_ExpK(whileKNode->child[0]);

	/* generate the while condition expression */
	tcode_ExpK(whileKNode->child[0]);
	/* FJP WHILE_END */
	tcodewriter_FJP(WhileBodyCodeSize);
	/* generate the while body part */
	tcode_StatementsK(whileKNode->child[1]);
	/* JMP BEGIN */
	tcodewriter_JMP(-(WhileBodyCodeSize+ConditionExpCodeSize+tcodelen_ByteCode(C_FJP,NullType,NULL)));

	return 1;
}

static int tcode_ForK(ParseTreeNode* forKNode)
{
	ParseTreeNode* FirstAssignNode = forKNode->child[0];
	ParseTreeNode* EndExpNode      = forKNode->child[1];
	ParseTreeNode* StepNode        = forKNode->child[2];
	ParseTreeNode* ForBodyNode     = forKNode->child[3];
	long ForBodyCodeSize;
	long IncreaseBodyCodeSize;
	long ConditionExpCodeSize;
	long start_addr,end_addr; /* address of the for statement code begin and end in code output stream */

	/* generate the first assignment */
	tcode_AssignK(FirstAssignNode);

	/* get the FOR body code and code size */
	ForBodyCodeSize = tcodelen_StatementsK(ForBodyNode);
	
	/* get the increase body code and code size */
	IncreaseBodyCodeSize = tcodelen_VarnameK_Load(FirstAssignNode->child[0]);
	if(StepNode == NULL)
	{
		/* CONST IntegerType 1 */
		IncreaseBodyCodeSize += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
	}
	else
	{
		/* Expression */
		IncreaseBodyCodeSize += tcodelen_ExpK(StepNode);
	}
	IncreaseBodyCodeSize += tcodelen_ByteCode(C_ADD,NullType,NULL);
 	IncreaseBodyCodeSize += tcodelen_VarnameK_Store(FirstAssignNode->child[0]);

	/* get condition expression code size */
	ConditionExpCodeSize = tcodelen_VarnameK_Load(FirstAssignNode->child[0]);
	ConditionExpCodeSize+= tcodelen_ExpK(EndExpNode);
	ConditionExpCodeSize+= tcodelen_ByteCode(C_LT_EQ,NullType,NULL);
	ConditionExpCodeSize+= tcodelen_ByteCode(C_FJP,NullType,NULL); /* FJP FOR_END */

	/* Push start and end code address */
	start_addr = tcodewriter_codelength();
	end_addr   = start_addr + ConditionExpCodeSize + ForBodyCodeSize +IncreaseBodyCodeSize + tcodelen_ByteCode(C_JMP,NullType,NULL);
	tanalysis_PushForLoopAddress(start_addr,end_addr);

	/* generate the condition expression */
	tcode_VarnameK_Load(FirstAssignNode->child[0]);
	tcode_ExpK(EndExpNode);
	tcodewriter_LT_EQ();
	tcodewriter_FJP(ForBodyCodeSize+IncreaseBodyCodeSize+tcodelen_ByteCode(C_JMP,NullType,NULL));

	/* generate the FOR body */
	tcode_StatementsK(ForBodyNode);

	/* generate the Increase Index body */
	tcode_VarnameK_Load(FirstAssignNode->child[0]);
	if(StepNode == NULL)
	{
		/* CONST integer 1 */
		tcodewriter_CONST_Integer(1);
	}
	else
	{
		tcode_ExpK(StepNode);
	}
	tcodewriter_ADD();
	tcode_VarnameK_Store(FirstAssignNode->child[0]);

	/* JMP BEGIN */
	tcodewriter_JMP(start_addr - end_addr);

	/* Pop start and end code address */
	tanalysis_PopForLoopAddress(&start_addr,&end_addr);

	return 1;
}

static int tcode_DoWhileLoopK (ParseTreeNode* doloopKNode)
{
	long LoopBodyCodeSize;
	long ConditionExpCodeSize;
	long start_addr,end_addr; /* address of the for statement code begin and end in code output stream */

	/* get the condition body code size */
	ConditionExpCodeSize = tcodelen_ExpK(doloopKNode->child[0]);
	/* get the loop body code size */
	LoopBodyCodeSize = tcodelen_StatementsK(doloopKNode->child[1]);

	/* Push start and end code address */
	start_addr = tcodewriter_codelength();
	end_addr   = start_addr + ConditionExpCodeSize + LoopBodyCodeSize + tcodelen_ByteCode(C_FJP,NullType,NULL) + tcodelen_ByteCode(C_JMP,NullType,NULL);
	tanalysis_PushDoLoopAddress(start_addr,end_addr);

	/* generate the while condition expression */
	tcode_ExpK(doloopKNode->child[0]);
	/* FJP LOOP END */
	tcodewriter_FJP(LoopBodyCodeSize+tcodelen_ByteCode(C_JMP,NullType,NULL));
	/* generate the loop body part */
	tcode_StatementsK(doloopKNode->child[1]);
	/* JMP BEGIN */
	tcodewriter_JMP(start_addr - end_addr);

	/* Pop start and end code address */
	tanalysis_PopDoLoopAddress(&start_addr,&end_addr);

	return 1;
}

static int tcode_DoLoopWhileK(ParseTreeNode* doloopKNode)
{
	long LoopBodyCodeSize;
	long ConditionExpCodeSize;
	long start_addr,end_addr; /* address of the for statement code begin and end in code output stream */

	/* get loop body part code size */
	LoopBodyCodeSize = tcodelen_StatementsK(doloopKNode->child[0]);
	/* get loop condition part code size */
	ConditionExpCodeSize = tcodelen_ExpK(doloopKNode->child[1]);

	/* Push start and end code address */
	start_addr = tcodewriter_codelength();
	if(doloopKNode->child[1] != NULL)
		end_addr = start_addr + ConditionExpCodeSize + LoopBodyCodeSize + tcodelen_ByteCode(C_TJP,NullType,NULL);
	else
		end_addr = start_addr + LoopBodyCodeSize + tcodelen_ByteCode(C_JMP,NullType,NULL);
	tanalysis_PushDoLoopAddress(start_addr,end_addr);

	/* generate the loop body part code */
	tcode_StatementsK(doloopKNode->child[0]);

	/* generate the loop while condition part */
	if(doloopKNode->child[1] != NULL) /* if the while condition is not NULL */
	{
		/* generate the loop while condition expression */
		tcode_ExpK(doloopKNode->child[1]);
		/* TJP LOOP BEGIN */
		tcodewriter_TJP(start_addr - end_addr);
	}
	else /* if the while condition is NULL */
	{
		/* JMP LOOP BEGIN */
		tcodewriter_JMP(start_addr - end_addr);
	}

	/* Pop start and end code address */
	tanalysis_PopDoLoopAddress(&start_addr,&end_addr);

	return 1;
}

static int tcode_ExitForK(ParseTreeNode* exitforKNode)
{
	long ForStartAddr,ForEndAddr;
	int  JumpSize;

	/* Pop start and end code address of the this For statement */
	tanalysis_PopForLoopAddress(&ForStartAddr,&ForEndAddr);
	JumpSize = ForEndAddr -(tcodewriter_codelength() + tcodelen_ByteCode(C_JMP,NullType,NULL));
	/* JMP LOOP_END */
	tcodewriter_JMP(JumpSize);
	/* Push Back Address */
	tanalysis_PushForLoopAddress(ForStartAddr,ForEndAddr);
	return 1;
}

static int tcode_ExitDoK(ParseTreeNode* exitdoKNode)
{
	long DoLoopStartAddr,DoLoopEndAddr;
	int  JumpSize;

	/* Pop start and end code address of the this Do Loop statement */
	tanalysis_PopDoLoopAddress(&DoLoopStartAddr,&DoLoopEndAddr);
	JumpSize = DoLoopEndAddr - (tcodewriter_codelength()+tcodelen_ByteCode(C_JMP,NullType,NULL));	
	/* JMP LOOP_END */
	tcodewriter_JMP(JumpSize);
	/* Push Back Address */
	tanalysis_PushDoLoopAddress(DoLoopStartAddr,DoLoopEndAddr);
	return 1;
}

static int tcode_ReturnK(ParseTreeNode* returnKNode)
{
	u32 num_localvar = 0;
	ParseTreeNode* ReturnExpNode = returnKNode->child[0];
	if(ReturnExpNode != NULL)
	{
		/* generate the return expression */
		tcode_ExpK(ReturnExpNode);
	}
	else
	{
		/* instead by RETURN 0 */
		tcodewriter_CONST_Integer(0);
	}
	/* count the local variables */
	num_localvar = tanalysis_LocalVar_getCount();
	/* POP NUM_LOCAL */
	tcodewriter_POP_RESTOP(num_localvar);
	/* RET INDEX_ARG */
	tcodewriter_RET(tanalysis_ArgDef_GetTopIndexInByte());


	return 1;
}

static int tcode_SelectK(ParseTreeNode* selectKNode)
{
	long EndAddr;
	long JumpSize;
	ParseTreeNode* CaseNode;
	ParseTreeNode* DefaultNode;

	EndAddr = tcodewriter_codelength() + tcodelen_SelectK(selectKNode);
	CaseNode = selectKNode->child[1];
	while(CaseNode != NULL)
	{
		/* generate the Select expression */
		tcode_ExpK(selectKNode->child[0]);
		/* generate the Case expression */
		tcode_ExpK(CaseNode->child[0]);
		/* C_EQ */
		tcodewriter_EQ();
		/* FJP CASE_END */
		JumpSize = tcodelen_StatementsK(CaseNode->child[1]) + tcodelen_ByteCode(C_FJP,NullType,NULL);
		tcodewriter_FJP(JumpSize);
		/* generate the Case statements */
		tcode_StatementsK(CaseNode->child[1]);
		/* JMP SELECT_END */
		JumpSize = EndAddr - (tcodewriter_codelength() + tcodelen_ByteCode(C_JMP,NullType,NULL));
		tcodewriter_JMP(JumpSize);
		CaseNode = CaseNode->sibling;
	}

	DefaultNode = selectKNode->child[2];
	if(DefaultNode != NULL)
	{
		/* generate the Default case statements */
		tcode_StatementsK(DefaultNode->child[0]);
	}

	return 1;
}

static int tcode_EraseK(ParseTreeNode* eraseKNode)
{
	ParseTreeNode* VarNameNode;

	VarNameNode = eraseKNode->child[0];
	/* load the address */
	tcode_VarnameK_Load(VarNameNode);
	/* FREE */
	tcodewriter_FREE();

	return 1;
}

static int tcode_FreeK(ParseTreeNode* eraseKNode)
{
	ParseTreeNode* VarNameNode;

	VarNameNode = eraseKNode->child[0];
	/* load the address */
	tcode_VarnameK_Load(VarNameNode);
	/* FREE */
	tcodewriter_FREE();

	return 1;
}

static int tcode_NewK(ParseTreeNode* newKNode)
{
	ParseTreeNode* IndexNode;
	int dimension = 0;
	u8  type = (u8)newKNode->attr_int;
	if(newKNode->child[0] != NULL) /* if it is a array allocation */
	{
		IndexNode = newKNode->child[0];
		while(IndexNode != NULL)
		{
			tcode_ExpK(IndexNode);
			if(IndexNode->sibling != NULL)
				tcodewriter_CONST_Integer(sizeofData(PtrType));
			else
				tcodewriter_CONST_Integer(sizeofData((DataType)type));
			tcodewriter_MUL();

			dimension++;
			IndexNode = IndexNode->sibling;
		}

		if(dimension >= 2)
			tcodewriter_ALLOC_ARRAY(dimension);
		else
			tcodewriter_ALLOC();
	}
	else /* if it is a type allocation */
	{
		s32 typesize;
		if(!tanalysis_TypeDefine_memsize(newKNode->attr_string,(int*)(&typesize)))
		{
			char msg[MAX_MSG];
			sprintf(msg,"Can not find the Type defintion: %s \r\n",newKNode->attr_string);
			tcode_printError(msg,newKNode);
			return 0;
		}
		
		tcodewriter_CONST_Integer(typesize);
		tcodewriter_ALLOC();
	}

	return 1;
}
