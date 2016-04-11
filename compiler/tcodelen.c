#include "tcodelen.h"

#include "tbytecode.h"
#include "global.h"

#include "tanalysis.h"


const char* const tcodelen_bytecodeformats[] = BYTE_CODE_FORMAT;
/* //////////////////////////////////////////////////////////////////////// */

#define tcodelen_Code()         (sizeof(u8))
#define tcodelen_DataType()     (sizeofData(datatypeType))
#define tcodelen_AccessFlag()   (sizeofData(accessflagType))
#define tcodelen_Integer()      (sizeofData(IntegerType))
#define tcodelen_Double()       (sizeofData(DoubleType))
#define tcodelen_Index()        (sizeofData(IndexType))
#define tcodelen_String(str)    (sizeof(u16) + (long)strlen(str))
#define tcodelen_Address()      (sizeofData(PtrType))

/* //////////////////////////////////////////////////////////////////////// */
long tcodelen_ByteCode(u8 code,u8 type,const char *str)
{
	int  codelength = 0;
	const char* format;

	format = tcodelen_bytecodeformats[code];
	codelength += tcodelen_Code();

	/* print the parameters */
	while(*format)
	{
		switch(*format)
		{
		case 'F':
			codelength += tcodelen_AccessFlag();
			break;
		case 'T':
			codelength += tcodelen_DataType();
			break;
		case 'I':
			codelength += tcodelen_Integer();
			break;
		case 'V':
			switch(type)
			{
			case IntegerType:
				codelength += tcodelen_Integer();
				break;
			case DoubleType:
				codelength += tcodelen_Double();
				break;
			case StringType:
				codelength += tcodelen_String(str);
				break;
			}
			break;
		}
		format++;
	}
	return codelength;
}


/* //////////////////////////////////////////////////////////////////////// */
long tcodelen_StatementsK(ParseTreeNode* statementsKNode)
{
	long length = 0;
	ParseTreeNode* stmtNode = statementsKNode->child[0];
	
	while(stmtNode != NULL)
	{
		switch(stmtNode->nodekind)
		{
		case DimK:
			length += tcodelen_DimK_Local(stmtNode);
			break;
		case AssignK:
			length += tcodelen_AssignK(stmtNode);
			break;
		case CallK:
			length += tcodelen_CallK_Stmt(stmtNode);
			break;
		case IfK:
			length += tcodelen_IfK(stmtNode);
			break;
		case WhileK:
			length += tcodelen_WhileK(stmtNode);
			break;
		case DoLoopWhileK:
			length += tcodelen_DoLoopWhileK(stmtNode);
			break;
		case DoWhileLoopK:
			length += tcodelen_DoWhileLoopK(stmtNode);
			break;
		case ExitDoK:
			length += tcodelen_ExitDoK(stmtNode);
			break;
		case ExitForK:
			length += tcodelen_ExitForK(stmtNode);
			break;
		case ReturnK:
			length += tcodelen_ReturnK(stmtNode);
			break;
		case ForK:
			length += tcodelen_ForK(stmtNode);
			break;
		case SelectK:
			length += tcodelen_SelectK(stmtNode);
			break;
		case EraseK:
			length += tcodelen_EraseK(stmtNode);
			break;
		case FreeK:
			length += tcodelen_FreeK(stmtNode);
			break;
		}
		stmtNode = stmtNode->sibling;
	} 

	/* POP COUNT_LOCAL */
	length += tcodelen_ByteCode(C_POP,NullType,NULL);
	
	return length;
}

long tcodelen_DimK_Local(ParseTreeNode* dimKNode)
{
	long length = 0;
	ParseTreeNode* DimVarNode;

	DimVarNode = dimKNode->child[0];
	while(DimVarNode != NULL)
	{
		length += tcodelen_ByteCode(C_PUSH,NullType,NULL);
		DimVarNode = DimVarNode->sibling;
	}
	return length;
}

long tcodelen_AssignK(ParseTreeNode* assignKNode)
{
	long length = 0;
	length += tcodelen_ExpK(assignKNode->child[1]);
	length += tcodelen_VarnameK_Store(assignKNode->child[0]);
	return length;
}

long tcodelen_ExpK(ParseTreeNode* expKNode)
{
	long length = 0;
	ParseTreeNode* Node = expKNode->child[0];
	if(Node == NULL)
		return 0;

	switch(Node->nodekind)
	{
	case OpK:
		length += tcodelen_OpK(Node);
		break;
	case CallK:
		length += tcodelen_CallK_Exp(Node);
		break;
	case ConstNumK:
		length += tcodelen_ConstNumK(Node);
		break;
	case ConstDoubleK:
		length += tcodelen_ConstDoubleK(Node);
		break;
	case ConstStrK:
		length += tcodelen_ConstStrK(Node);
		break;
	case VarnameK:
		length += tcodelen_VarnameK_Load(Node);
		break;
	case ExpK:
		length += tcodelen_ExpK(Node);
		break;
	case NewK:
		length += tcodelen_NewK(Node);
		break;
	}
	return length;
}

long tcodelen_OpK(ParseTreeNode* opKNode)
{
	long length = 0;
	ParseTreeNode* ChildNode;
	int i;

	for(i=0;i<2; i++) /* C_OPP used only one child */
	{
		ChildNode = opKNode->child[i];
		if(ChildNode != NULL)
		{
			switch(ChildNode->nodekind)
			{
			case OpK:
				length += tcodelen_OpK(ChildNode);
				break;
			case CallK:
				length += tcodelen_CallK_Exp(ChildNode);
				break;
			case ConstNumK:
				length += tcodelen_ConstNumK(ChildNode);
				break;
			case ConstDoubleK:
				length += tcodelen_ConstDoubleK(ChildNode);
				break;
			case ConstStrK:
				length += tcodelen_ConstStrK(ChildNode);
				break;
			case VarnameK:
				length += tcodelen_VarnameK_Load(ChildNode);
				break;
			case ExpK:
				length += tcodelen_ExpK(ChildNode);
				break;
			}
		}
	}
	length += tcodelen_Code(); /* C_ADD,C_SUB,C_MUL,... */
	return length;
}

long tcodelen_ConstNumK(ParseTreeNode* constNumKNode)
{
	long length = 0;
	length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
	return length;
}

long tcodelen_ConstDoubleK(ParseTreeNode* constDoubleKNode)
{
	long length = 0;
	length += tcodelen_ByteCode(C_CONST,DoubleType,NULL);
	return length;
}

long tcodelen_ConstStrK(ParseTreeNode* constStrKNode)
{
	long length = 0;
	length += tcodelen_ByteCode(C_CONST,StringType,constStrKNode->attr_string);
	return length;
}

long tcodelen_VarnameK_Load(ParseTreeNode* varnameKNode)
{
	long length = 0;
	length += tcodelen_PrimitiveVarnamesK_Load(varnameKNode->child[0]);
	/* if it is get */
	if(varnameKNode->child[1] != NULL) 
	{
		length += tcodelen_ConstStrK(varnameKNode->child[1]);
		length += tcodelen_ByteCode(C_INVOKE,NullType,NULL);
	}
	return length;
}

long tcodelen_PrimitiveVarnamesK_Load(ParseTreeNode* primitivesKNode)
{
	long length = 0;
	ParseTreeNode* PriVarNameNode;
	ParseTreeNode* IndexExpNode;

	PriVarNameNode = primitivesKNode;
	
	if(PriVarNameNode->child[0] != NULL) /* if it is a array */
	{
		/* LOAD access address PtrType */
		length += tcodelen_ByteCode(C_FORCE_LOAD,NullType,NULL);

		IndexExpNode = PriVarNameNode->child[0];
		while(IndexExpNode != NULL)
		{
			/* generate the index of the element of the array */
			length += tcodelen_ExpK(IndexExpNode);
			length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
			length += tcodelen_ByteCode(C_MUL,NullType,NULL);
			/*  HEAP_LOAD type */
			length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);

			IndexExpNode = IndexExpNode->sibling;
		}
	}
	else
	{
		/*  LOAD access address */
		length += tcodelen_ByteCode(C_LOAD,NullType,NULL);
	}

	PriVarNameNode = PriVarNameNode->sibling;
	while(PriVarNameNode != NULL)
	{
		/*  CONST IntegerType Address */
		length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
		if(PriVarNameNode->child[0] != NULL) /*  if it is a array */
		{
			/*  HEAP_LOAD type */
			length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);

			IndexExpNode = PriVarNameNode->child[0];
			while(IndexExpNode != NULL)
			{
				/*  generate the index of the element of the array */
				length += tcodelen_ExpK(IndexExpNode); 
				length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
				length += tcodelen_ByteCode(C_MUL,NullType,NULL);
				/*  HEAP_LOAD type */
				length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);

				IndexExpNode = IndexExpNode->sibling;
			}
		}
		else /*  if it is not array */
		{
			/*  HEAP_LOAD type */
			length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);
		}

		PriVarNameNode = PriVarNameNode->sibling;
	}

	return length;
}

long tcodelen_VarnameK_Store(ParseTreeNode* varnameKNode)
{
	long length = 0;
	/* if it is set */
	if(varnameKNode->child[1] != NULL)
	{
		length += tcodelen_PrimitiveVarnamesK_Load(varnameKNode->child[0]);
		length += tcodelen_ConstStrK(varnameKNode->child[1]);
		length += tcodelen_ByteCode(C_INVOKE,NullType,NULL);
		length += tcodelen_ByteCode(C_POP,NullType,NULL);
	}
	else
	{
		length += tcodelen_PrimitiveVarnamesK_Store(varnameKNode->child[0]);
	}
	return length;
}

long tcodelen_PrimitiveVarnamesK_Store(ParseTreeNode* primitivesKNode)
{
	long length = 0;
	ParseTreeNode* PriVarNameNode;
	ParseTreeNode* IndexExpNode;

	PriVarNameNode = primitivesKNode;

	if(PriVarNameNode->sibling == NULL) /*  if it is the end primitive node */
	{
		if(PriVarNameNode->child[0] != NULL) /*  if it is a array */
		{
			/*  generate the base address of the array */
			length += tcodelen_ByteCode(C_FORCE_LOAD,NullType,NULL);

			IndexExpNode = PriVarNameNode->child[0];
			while(IndexExpNode != NULL)
			{
				if(IndexExpNode->sibling != NULL)
				{
					/*  generate the index of the element of the array */
					length += tcodelen_ExpK(IndexExpNode);
					length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
					length += tcodelen_ByteCode(C_MUL,NullType,NULL);
					/*  HEAP_LOAD type */
					length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);
				}
				else
				{
					/*  generate the index of the element of the array */
					length += tcodelen_ExpK(IndexExpNode);
					length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
					length += tcodelen_ByteCode(C_MUL,NullType,NULL);
					/*  HEAP_STORE type */
					length += tcodelen_ByteCode(C_HEAP_STORE,NullType,NULL);
				}

				IndexExpNode = IndexExpNode->sibling;
			}
		}
		else /*  if it is not a array */
		{
			/*  STORE access address */
			length += tcodelen_ByteCode(C_STORE,NullType,NULL);
		}
	}
	else /*  if it is not end primitive node */
	{
		/*  LOAD access address */
		length += tcodelen_ByteCode(C_LOAD,NullType,NULL);
		if(PriVarNameNode->child[0] != NULL) /*  if it is a array */
		{
			IndexExpNode = PriVarNameNode->child[0];
			while(IndexExpNode != NULL)
			{
				/*  generate the index of the element of the array */
				length += tcodelen_ExpK(IndexExpNode);
				length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
				length += tcodelen_ByteCode(C_MUL,NullType,NULL);
				/*  HEAP_LOAD type */
				length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);

				IndexExpNode = IndexExpNode->sibling;
			}
		}
		PriVarNameNode = PriVarNameNode->sibling;
		while(PriVarNameNode != NULL)
		{
			/*  CONST IntegerType Address */
			length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);

			if(PriVarNameNode->child[0] != NULL) /*  if it is a array */
			{
				/*  HEAP_LOAD type */
				length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);

				IndexExpNode = PriVarNameNode->child[0];
				while(IndexExpNode != NULL)
				{
					if(IndexExpNode->sibling != NULL)
					{
						/*  generate the index of the element of the array */
						length += tcodelen_ExpK(IndexExpNode);
						length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
						length += tcodelen_ByteCode(C_MUL,NullType,NULL);
						/*  HEAP_LOAD type */
						length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);
					}
					else
					{
						/*  generate the index of the element of the array */
						length += tcodelen_ExpK(IndexExpNode);
						length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
						length += tcodelen_ByteCode(C_MUL,NullType,NULL);
						if(PriVarNameNode->sibling != NULL) /*  not end primitive node */
						{
							/*  HEAP_LOAD type */
							length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);
						}
						else /*  primitive node */
						{
							/*  HEAP_STORE type */
							length += tcodelen_ByteCode(C_HEAP_STORE,NullType,NULL);
						}
					}

					IndexExpNode = IndexExpNode->sibling;
				}
			}
			else /*  if it is not array */
			{
				if(PriVarNameNode->sibling == NULL) /*   if it is end primitive node */
				{
					/*  HEAP_STORE type */
					length += tcodelen_ByteCode(C_HEAP_STORE,NullType,NULL);
				}
				else 
				{
					/*  HEAP_LOAD type */
					length += tcodelen_ByteCode(C_HEAP_LOAD,NullType,NULL);
				}
			}
			
			PriVarNameNode = PriVarNameNode->sibling;
		}
	}

	return length;
}

long tcodelen_CallK_Exp(ParseTreeNode* callKNode)
{
	long length = 0;
	ParseTreeNode *ParamListNode,*ParamNode;
	ParseTreeNode *SubOrFunNode = tanalysis_SubAndFun_lookup(callKNode->attr_string);

	if(SubOrFunNode == NULL)
		return 0;

	/*  generate the parameters */
	ParamListNode = callKNode->child[0];
	ParamNode = ParamListNode->child[0];
	while(ParamNode!= NULL)
	{
		length += tcodelen_ExpK(ParamNode);
		length += tcodelen_ByteCode(C_CONVERT,NullType,NULL);
		ParamNode = ParamNode->sibling;
	}

	if(SubOrFunNode->modifier == NativeModifier)
	{
		length += tcodelen_ByteCode(C_INVOKE,NullType,NULL);
	}
	else
	{
		length += tcodelen_ByteCode(C_CALL,NullType,NULL);
	}
	return length;
}

long tcodelen_CallK_Stmt(ParseTreeNode* callKNode)
{
	long length = 0;
	ParseTreeNode *ParamListNode,*ParamNode;
	ParseTreeNode *SubOrFunNode = tanalysis_SubAndFun_lookup(callKNode->attr_string);

	if(SubOrFunNode == NULL)
		return 0;

	/*  generate the parameters */
	ParamListNode = callKNode->child[0];
	ParamNode = ParamListNode->child[0];
	while(ParamNode!= NULL)
	{
		length += tcodelen_ExpK(ParamNode);
		length += tcodelen_ByteCode(C_CONVERT,NullType,NULL);
		ParamNode = ParamNode->sibling;
	}

	if(SubOrFunNode->modifier == NativeModifier)
	{
		length += tcodelen_ByteCode(C_INVOKE,NullType,NULL);
	}
	else
	{
		length += tcodelen_ByteCode(C_CALL,NullType,NULL);
	}

	/*  POP 1 */
	length += tcodelen_ByteCode(C_POP,NullType,NULL);

	return length;
}

long tcodelen_IfK(ParseTreeNode* ifKNode)
{
	long length = 0;
	ParseTreeNode* ConditionExpKNode = ifKNode->child[0];
	ParseTreeNode* ThenNode = ifKNode->child[1];
	ParseTreeNode* ElseNode = ifKNode->child[2];

	/*  generate the condition  */
	length += tcodelen_ExpK(ConditionExpKNode);
	/*  FJP THEN_SIZE */
	length += tcodelen_ByteCode(C_FJP,NullType,NULL);
	/*  generate the Then part */
	length += tcodelen_StatementsK(ThenNode);
	/*  else part */
	if(ElseNode != NULL)
	{
		/*  JMP ELSE_SIZE */
		length += tcodelen_ByteCode(C_JMP,NullType,NULL);
		/*  generate else part */
		switch(ElseNode->nodekind)
		{
		case ElseK:
			length += tcodelen_StatementsK(ElseNode->child[0]);
			break;
		case IfK:
			length += tcodelen_IfK(ElseNode);
			break;
		}
	}
	return length;
}

long tcodelen_WhileK(ParseTreeNode* whileKNode)
{
	long length = 0;

	/*  generate the while condition expression */
	length += tcodelen_ExpK(whileKNode->child[0]);
	/*  FJP WHILE_END */
	length += tcodelen_ByteCode(C_FJP,NullType,NULL);
	/*  generate the while body part */
	length += tcodelen_StatementsK(whileKNode->child[1]);
	/*  JMP BEGIN */
	length += tcodelen_ByteCode(C_JMP,NullType,NULL);

	return length;
}

long tcodelen_ForK(ParseTreeNode* forKNode)
{
	long length = 0;
	ParseTreeNode* FirstAssignNode = forKNode->child[0];
	ParseTreeNode* EndExpNode      = forKNode->child[1];
	ParseTreeNode* StepNode        = forKNode->child[2];
	ParseTreeNode* ForBodyNode     = forKNode->child[3];

	/*  generate the first assignment */
	length += tcodelen_AssignK(FirstAssignNode);

	/*  generate the condition expression */
	length += tcodelen_VarnameK_Load(FirstAssignNode->child[0]);
	length += tcodelen_ExpK(EndExpNode);
	length += tcodelen_ByteCode(C_LT_EQ,NullType,NULL);
	/*  FJP FOR_END */
	length += tcodelen_ByteCode(C_FJP,NullType,NULL);
	/*  generate the FOR body */
	length += tcodelen_StatementsK(ForBodyNode);
	/*  generate the Increase Index body */
	length += tcodelen_VarnameK_Load(FirstAssignNode->child[0]);
	if(StepNode == NULL)
	{
		length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
	}
	else
	{
		length += tcodelen_ExpK(StepNode);
	}
	length += tcodelen_ByteCode(C_ADD,NullType,NULL);
	length += tcodelen_VarnameK_Store(FirstAssignNode->child[0]);
	/*  JMP FOR_BEGIN */
	length += tcodelen_ByteCode(C_JMP,NullType,NULL);

	return length;
}

long tcodelen_DoWhileLoopK(ParseTreeNode* doloopKNode)
{
	long length = 0;
	
	/*  generate the while condition expression */
	length += tcodelen_ExpK(doloopKNode->child[0]);
	/*  FJP LOOP END */
	length += tcodelen_ByteCode(C_FJP,NullType,NULL);
	/*  generate the loop body part */
	length += tcodelen_StatementsK(doloopKNode->child[1]);
	/*  JMP BEGIN */
	length += tcodelen_ByteCode(C_JMP,NullType,NULL);

	return length;
}

long tcodelen_DoLoopWhileK(ParseTreeNode* doloopKNode)
{
	long length = 0;
	
	/*  generate the loop body part code */
	length += tcodelen_StatementsK(doloopKNode->child[0]);

	/*  generate the loop while condition part */
	if(doloopKNode->child[1] != NULL)
	{
		/*  generate the loop while condition expression */
		length += tcodelen_ExpK(doloopKNode->child[1]);
		/*  TJP LOOP BEGIN */
		length += tcodelen_ByteCode(C_TJP,NullType,NULL);
	}
	else
	{
		/*  JMP LOOP BEGIN */
		length += tcodelen_ByteCode(C_JMP,NullType,NULL);
	}

	return length;
}

long tcodelen_ExitForK(ParseTreeNode* exitforKNode)
{
	long length = 0;
	/*  JMP FOR_END */
	length += tcodelen_ByteCode(C_JMP,NullType,NULL);

	return length;
}

long tcodelen_ExitDoK(ParseTreeNode* exitdoKNode)
{
	long length = 0;
	/*  JMP DOLOOP_END */
	length += tcodelen_ByteCode(C_JMP,NullType,NULL);

	return length;
}

long tcodelen_ReturnK(ParseTreeNode* returnKNode)
{
	long length = 0;
	ParseTreeNode* ReturnExpNode = returnKNode->child[0];

	if(ReturnExpNode != NULL)
	{
		/*  generate the return expression */
		length += tcodelen_ExpK(ReturnExpNode);
	}
	else
	{
		length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
	}

	/*  POP NUM_LOCAL */
	length += tcodelen_ByteCode(C_POP_RESTOP,NullType,NULL);
	/*  RET INDEX_ARG */
	length += tcodelen_ByteCode(C_RET,NullType,NULL);

	return length;

}

long tcodelen_SelectK(ParseTreeNode* selectKNode)
{
	long length = 0;
	ParseTreeNode* CaseNode;
	ParseTreeNode* DefaultNode;

	CaseNode = selectKNode->child[1];
	while(CaseNode != NULL)
	{
		/*  generate the Select expression */
		length += tcodelen_ExpK(selectKNode->child[0]);
		/*  generate the Case expression */
		length += tcodelen_ExpK(CaseNode->child[0]);
		/*  C_EQ */
		length += tcodelen_ByteCode(C_EQ,NullType,NULL);
		/*  FJP CASE_END */
		length += tcodelen_ByteCode(C_FJP,NullType,NULL);
		/*  generate the Case statements */
		length += tcodelen_StatementsK(CaseNode->child[1]);
		/*  JMP SELECT_END */
		length += tcodelen_ByteCode(C_JMP,NullType,NULL);

		CaseNode = CaseNode->sibling;
	}

	DefaultNode = selectKNode->child[2];
	if(DefaultNode != NULL)
	{
		length += tcodelen_StatementsK(DefaultNode->child[0]);
	}

	return length;
}

long tcodelen_EraseK(ParseTreeNode* eraseKNode)
{
	long length = 0;
	ParseTreeNode* VarNameNode;
	VarNameNode = eraseKNode->child[0];

	/*  load the address */
	length += tcodelen_VarnameK_Load(VarNameNode);
	/*  FREE */
	length += tcodelen_ByteCode(C_FREE,NullType,NULL);

	return length;
}

long tcodelen_FreeK(ParseTreeNode* freeKNode)
{
	long length = 0;
	ParseTreeNode* VarNameNode;
	VarNameNode = freeKNode->child[0];

	/*  load the address */
	length += tcodelen_VarnameK_Load(VarNameNode);
	/*  FREE */
	length += tcodelen_ByteCode(C_FREE,NullType,NULL);

	return length;
}

long tcodelen_NewK(ParseTreeNode* newKNode)
{
	long length = 0;
	ParseTreeNode* IndexNode;
	int  dimension = 0;
	if(newKNode->child[0] != NULL) /*  if it is not a array allocation */
	{
		IndexNode = newKNode->child[0];
		while(IndexNode != NULL)
		{
			length += tcodelen_ExpK(IndexNode);
			length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
			length += tcodelen_ByteCode(C_MUL,NullType,NULL);

			dimension++;
			IndexNode = IndexNode->sibling;
		}

		if(dimension >= 2)
			length += tcodelen_ByteCode(C_ALLOC_ARRAY,NullType,NULL);
		else
			length += tcodelen_ByteCode(C_ALLOC,NullType,NULL);
	}
	else /*  if it is a type allocation */
	{
		length += tcodelen_ByteCode(C_CONST,IntegerType,NULL);
		length += tcodelen_ByteCode(C_ALLOC,NullType,NULL);
	}
	return length;
}

