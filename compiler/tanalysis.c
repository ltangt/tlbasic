#include "tanalysis.h"

#include "tcodefile.h"
#include "global.h"
#include "tstring.h"
#include "tmem.h"
#include "ttable.h"
#include "tstack.h"


/*=========================================================================*/
// name tables and stacks 
ttable_Tab*  tanalysis_SubAndFun_Tab;
ttable_Tab*  tanalysis_TypeDefine_Tab;
ttable_Tab*  tanalysis_GlobalVar_Tab;
ttable_Tab*  tanalysis_Const_Tab;

int          tanalysis_GlobalVar_TopIndexInByte = 0;
tstack*      tanalysis_LocalVar_Stack;
int          tanalysis_LocalVar_TopIndexInByte = 0;
tstack*      tanalysis_ArgDef_Stack;
int          tanalysis_ArgDef_TopIndexInByte= 0;

/*=========================================================================*/
char        *tanalysis_UsedNativeFunc_Tab[MAX_FUNCTION];
int          tanalysis_UsedNativeFunc_Num = 0;

/*=========================================================================*/
// address stacks 
tstack* tanalysis_ForLoopAddressStack;
tstack* tanalysis_DoLoopAddressStack;

extern void tcode_printError(char* msg,ParseTreeNode* curNode);

/*=========================================================================*/
int tanalysis_InitializeAddressStacks()
{
	tanalysis_ForLoopAddressStack = tstack_create(SMART_STACK_SIZE);
	tanalysis_DoLoopAddressStack  = tstack_create(SMART_STACK_SIZE);
	return 1;
}

/*=========================================================================*/
int tanalysis_ReleaseAddressStacks()
{
	tstack_release(tanalysis_ForLoopAddressStack);
	tstack_release(tanalysis_DoLoopAddressStack);
	return 1;
}

/*=========================================================================*/
int tanalysis_PushForLoopAddress(long start_addr,long end_addr)
{
	tstack_PushLong(tanalysis_ForLoopAddressStack,start_addr);
	tstack_PushLong(tanalysis_ForLoopAddressStack,end_addr);
	return 1;
}

int tanalysis_PushDoLoopAddress(long start_addr,long end_addr)
{
	tstack_PushLong(tanalysis_DoLoopAddressStack,start_addr);
	tstack_PushLong(tanalysis_DoLoopAddressStack,end_addr);
	return 1;
}

int tanalysis_PopForLoopAddress(long *start_addr,long *end_addr)
{
	*end_addr   = tstack_PopLong(tanalysis_ForLoopAddressStack);
	*start_addr = tstack_PopLong(tanalysis_ForLoopAddressStack);
	return 1;
}

int tanalysis_PopDoLoopAddress(long *start_addr, long *end_addr)
{
	*end_addr   = tstack_PopLong(tanalysis_DoLoopAddressStack);
	*start_addr = tstack_PopLong(tanalysis_DoLoopAddressStack);
	return 1;
}


/*=========================================================================*/
void tanalysis_SubAndFun_create()
{
	tanalysis_SubAndFun_Tab = ttable_create(MAX_TAB);
}

int tanalysis_SubAndFun_insert(ParseTreeNode* DefNode)
{
	if(tanalysis_SubAndFun_lookup(DefNode->attr_string))
	{
		char msg[MAX_MSG];
		sprintf(msg,"this function or subroutione has been already defined : %s \r\n",DefNode->attr_string);
		tcode_printError(msg,DefNode);
		return 0;
	}

	return ttable_insert(tanalysis_SubAndFun_Tab,DefNode->attr_string,DefNode,sizeof(ParseTreeNode));
}

ParseTreeNode* tanalysis_SubAndFun_lookup(const char *name)
{
	return (ParseTreeNode*)ttable_lookup(tanalysis_SubAndFun_Tab,name);
}

void tanalysis_SubAndFun_release()
{
	ttable_release(tanalysis_SubAndFun_Tab);
}

/*=========================================================================*/
void tanalysis_TypeDefine_create()
{
	tanalysis_TypeDefine_Tab = ttable_create(MAX_TAB);
}

int tanalysis_TypeDefine_insert(ParseTreeNode* TypeNode)
{
	ParseTreeNode* TypeMemberKNode;
	ParseTreeNode* DimVarNode;
	int addr = 0; /* member address based on type address */

	if(tanalysis_TypeDefine_lookup(TypeNode->attr_string))
	{
		char msg[MAX_MSG];
		sprintf(msg,"this type struct has already been defined : %s \r\n",TypeNode->attr_string);
		tcode_printError(msg,TypeNode);
		return 0;
	}

	/* 1.calculate the memory size of the type structure 
	   2.calculate the address of each member variable */
	TypeNode->attr_int = 0; // memory size of the type structure
	TypeMemberKNode = TypeNode->child[0];
	while(TypeMemberKNode != NULL)
	{
		DimVarNode = TypeMemberKNode->child[0];
		while(DimVarNode != NULL)
		{
			DimVarNode->address = addr;
			addr += sizeofData((DataType)DimVarNode->attr_int);
			TypeNode->attr_int += sizeofData((DataType)DimVarNode->attr_int);

			DimVarNode = DimVarNode->sibling;
		}
		TypeMemberKNode = TypeMemberKNode->sibling;
	}

	return ttable_insert(tanalysis_TypeDefine_Tab,TypeNode->attr_string,TypeNode,sizeof(ParseTreeNode));
}

ParseTreeNode* tanalysis_TypeDefine_lookup(const char *TypeName)
{
	return (ParseTreeNode*)ttable_lookup(tanalysis_TypeDefine_Tab,TypeName);
}

ParseTreeNode* tanalysis_TypeDefine_findmember(const char *TypeName,const char *MemberName)
{
	ParseTreeNode* TypeNode;
	ParseTreeNode* TypeMemberKNode;
	ParseTreeNode* DimVarNode;

	TypeNode = tanalysis_TypeDefine_lookup(TypeName);
	if(TypeNode == NULL)
		return NULL;

	TypeMemberKNode = TypeNode->child[0];
	while(TypeMemberKNode != NULL)
	{
		DimVarNode = TypeMemberKNode->child[0];
		while(DimVarNode != NULL)
		{
			if(str_nocasecmp(MemberName,DimVarNode->attr_string) == 0)
			{
				return DimVarNode;
			}
			DimVarNode = DimVarNode->sibling;
		}
		TypeMemberKNode = TypeMemberKNode->sibling;
	}

	return NULL;
}

int tanalysis_TypeDefine_memsize(const char *TypeName,int *size)
{
	ParseTreeNode* TypeNode;
	TypeNode = tanalysis_TypeDefine_lookup(TypeName);
	if(TypeNode != NULL)
	{
		*size = TypeNode->attr_int;
		return 1;
	}
	else
	{
		*size = -1;
		return 0;
	}
}

void tanalysis_TypeDefine_release()
{
	ttable_release(tanalysis_TypeDefine_Tab);
}

void tanalysis_GlobalVar_create()
{
	tanalysis_GlobalVar_Tab = ttable_create(MAX_TAB);
}

int tanalysis_GlobalVar_push(ParseTreeNode* DimVarNode)
{

	if(tanalysis_GlobalVar_lookup(DimVarNode->attr_string))
	{
		char msg[MAX_MSG];
		sprintf(msg,"global variable has already been defined. %s\r\n",DimVarNode->attr_string);
		tcode_printError(msg,DimVarNode);
		return 0;
	}

	if(tanalysis_GetVarDimension(DimVarNode) > 0)
		tanalysis_GlobalVar_TopIndexInByte += sizeofData(PtrType);
	else
		tanalysis_GlobalVar_TopIndexInByte += sizeofData((DataType)DimVarNode->attr_int);
	DimVarNode->address             = tanalysis_GlobalVar_TopIndexInByte; // record the local variable index in stack frame
	tanalysis_GlobalVar_TopIndexInByte += sizeofData(datatypeType);      // update the local variable index pointer
	return ttable_insert(tanalysis_GlobalVar_Tab,DimVarNode->attr_string,DimVarNode,sizeof(ParseTreeNode)); // insert to table
}

ParseTreeNode* tanalysis_GlobalVar_lookup(const char *name)
{
	return (ParseTreeNode*)ttable_lookup(tanalysis_GlobalVar_Tab,name);
}

void tanalysis_GlobalVar_release()
{
	ttable_release(tanalysis_GlobalVar_Tab);
}

void tanalysis_LocalVar_create()
{
	tanalysis_LocalVar_Stack = tstack_create(SMART_STACK_SIZE);
	tanalysis_LocalVar_TopIndexInByte = 0;
}

int tanalysis_LocalVar_push(ParseTreeNode* DimVarNode)
{
	if(tanalysis_LocalVar_lookup(DimVarNode->attr_string))
	{
		char msg[MAX_MSG];
		sprintf(msg,"local variable has already been defined. %s\r\n",DimVarNode->attr_string);
		tcode_printError(msg,DimVarNode);
		return 0;
	}

	if(tanalysis_GetVarDimension(DimVarNode) >0 )
		tanalysis_LocalVar_TopIndexInByte += sizeofData(PtrType);
	else
		tanalysis_LocalVar_TopIndexInByte += sizeofData((DataType)DimVarNode->attr_int);
	DimVarNode->address            = tanalysis_LocalVar_TopIndexInByte; // record the local variable index in stack frame
	tanalysis_LocalVar_TopIndexInByte += sizeofData(datatypeType);      // update the local variable index pointer
	
	tstack_PushBuffer(tanalysis_LocalVar_Stack,DimVarNode,sizeof(ParseTreeNode)); // push to stack
	return 1;
}

ParseTreeNode* tanalysis_LocalVar_lookup(const char *name)
{
	long i;
	ParseTreeNode* Node;
	for(i = 0; i<tanalysis_LocalVar_getCount(); i++)
	{
		Node = (ParseTreeNode*)tstack_Get(tanalysis_LocalVar_Stack,i,sizeof(ParseTreeNode));
		if(str_nocasecmp(name,Node->attr_string) == 0)
			return Node;
	}
	return NULL;
}

ParseTreeNode* tanalysis_LocalVar_pop()
{
	if(tstack_IsEmpty(tanalysis_LocalVar_Stack))
		return NULL;

	return (ParseTreeNode*)tstack_PopBuffer(tanalysis_LocalVar_Stack,sizeof(ParseTreeNode));
}

int tanalysis_LocalVar_getCount()
{
	return (int)(tstack_getTop(tanalysis_LocalVar_Stack) / sizeof(ParseTreeNode));
}

ParseTreeNode* tanalysis_LocalVar_getTopLocal()
{
	return (ParseTreeNode*)tstack_Get(tanalysis_LocalVar_Stack,0,sizeof(ParseTreeNode));
}

void tanalysis_LocalVar_release()
{
	tstack_release(tanalysis_LocalVar_Stack);
}

void tanalysis_ArgDef_create()
{
	tanalysis_ArgDef_Stack = tstack_create(SMART_STACK_SIZE);
	tanalysis_ArgDef_TopIndexInByte = 0;
}

int tanalysis_ArgDef_push(ParseTreeNode* ArgDefNode)
{
	if(tanalysis_ArgDef_lookup(ArgDefNode->attr_string))
	{
		char msg[MAX_MSG];
		sprintf(msg,"argument variable has already been defined. %s\r\n",ArgDefNode->attr_string);
		tcode_printError(msg,ArgDefNode);
		return 0;
	}

	if(tanalysis_GetVarDimension(ArgDefNode) >0 )
		tanalysis_ArgDef_TopIndexInByte += sizeofData(PtrType);
	else
		tanalysis_ArgDef_TopIndexInByte += sizeofData((DataType)ArgDefNode->attr_int); 
	ArgDefNode->address          = tanalysis_ArgDef_TopIndexInByte;
	tanalysis_ArgDef_TopIndexInByte += sizeofData(datatypeType); 
	tstack_PushBuffer(tanalysis_ArgDef_Stack,ArgDefNode,sizeof(ParseTreeNode));

	return 1;
}

ParseTreeNode* tanalysis_ArgDef_lookup(const char *name)
{ 
	long i;
	ParseTreeNode* Node;
	for(i = 0; i<(long)(tstack_getTop(tanalysis_ArgDef_Stack) / sizeof(ParseTreeNode)); i++)
	{
		Node = (ParseTreeNode*)tstack_Get(tanalysis_ArgDef_Stack,i,sizeof(ParseTreeNode));
		if(str_nocasecmp(name,Node->attr_string) == 0)
			return Node;
	}
	return NULL;
}

void tanalysis_ArgDef_release()
{
	tstack_release(tanalysis_ArgDef_Stack);
}

int tanalysis_ArgDef_GetTopIndexInByte()
{
	return tanalysis_ArgDef_TopIndexInByte;
}

/*=========================================================================*/
int tanalysis_collectGlobalNodes(ParseTreeNode* programKNode)
{
	int ret = 1;
	int function_index = 1;
	ParseTreeNode* childNode;
	ParseTreeNode* DimVarKNode;

	childNode = programKNode->child[0];
	while(childNode != NULL)
	{
		switch(childNode->nodekind)
		{
		case DimK: /* global variables */
			{
				DimVarKNode = childNode->child[0];
				while(DimVarKNode != NULL)
				{
					if(!tanalysis_GlobalVar_push(DimVarKNode))
						ret = 0;
					DimVarKNode = DimVarKNode->sibling;
				}
				break;
			}
		case FunctionK:
		case SubK:
			{
				/* record the function index for calling */
				if(str_nocasecmp(childNode->attr_string,ENTRY_FUNC_NAME) == 0)
					childNode->address = 0;
				else
				{	
					childNode->address = function_index;
					function_index++;
				}
				if(!tanalysis_SubAndFun_insert(childNode))
					ret = 0;
				break;
			}
		case DeclareFunctionK:
		case DeclareSubK:
			{
				if(childNode->modifier == NativeModifier)
				{
					if(!tanalysis_SubAndFun_insert(childNode))
						ret = 0;
				}
				break;
			}
		case TypeK:
			{
				if(!tanalysis_TypeDefine_insert(childNode))
					ret = 0;
				break;
			}
		}
		childNode = childNode->sibling;
	}
	return ret;
}

int tanalysis_collectArgList(ParseTreeNode* funKNode)
{
	int ret = 1;
	ParseTreeNode* ArgListNode = funKNode->child[0];
	ParseTreeNode* ArgDefNode = ArgListNode->child[0];

	while(ArgDefNode != NULL)
	{
		if(!tanalysis_ArgDef_push(ArgDefNode))
			ret = 0;
		ArgDefNode = ArgDefNode->sibling;
	}
	return ret;
}

/*=========================================================================*/
ParseTreeNode* tanalysis_GetVariableNode(const char *name,int *kind)
{
	ParseTreeNode* DimVarKNode;

	DimVarKNode = tanalysis_LocalVar_lookup(name);
	if(DimVarKNode != NULL)
	{
		*kind = LocalVariableKind;
		return DimVarKNode;
	}
	DimVarKNode = tanalysis_ArgDef_lookup(name);
	if(DimVarKNode != NULL)
	{
		*kind = ArgVariableKind;
		return DimVarKNode;
	}
	DimVarKNode = tanalysis_GlobalVar_lookup(name);
	if(DimVarKNode != NULL)
	{
		*kind = GlobalVariableKind;
		return DimVarKNode;
	}
	return NULL;
}

int tanalysis_GetGlobalVarAddress(ParseTreeNode* DimVarKNode)
{
	return DimVarKNode->address;
}

int tanalysis_GetLocalVarAddress(ParseTreeNode* DimVarKNode)
{
	return DimVarKNode->address;
}

int tanalysis_GetArgVarAddress(ParseTreeNode* DimVarKNode)
{
	return -(tanalysis_ArgDef_TopIndexInByte - DimVarKNode->address + SIZE_STACK_FRAME_RECORD);
}

int tanalysis_GetVarDataType(ParseTreeNode* DimVarKNode)
{
	return DimVarKNode->attr_int;
}

int tanalysis_GetVarDimension(ParseTreeNode* DimVarKNode)
{
	return DimVarKNode->dimension;
}

int tanalysis_GetVariable(const char *name,int *pIsGlobal,int *pAddr,int *type,int *dimension)
{
	int kind;
	ParseTreeNode* DimVarKNode;

	DimVarKNode = tanalysis_GetVariableNode(name,&kind);
	if(DimVarKNode == NULL)
		return 0;

	*pIsGlobal = kind == GlobalVariableKind;
	*type = tanalysis_GetVarDataType(DimVarKNode);
	*dimension = tanalysis_GetVarDimension(DimVarKNode);
	switch(kind)
	{
	case GlobalVariableKind:
		*pAddr = tanalysis_GetGlobalVarAddress(DimVarKNode);
		break;
	case LocalVariableKind: 
		*pAddr = tanalysis_GetLocalVarAddress(DimVarKNode);
		break;
	case ArgVariableKind:
		*pAddr = tanalysis_GetArgVarAddress(DimVarKNode);
		break;
	}
	return 1;
}

int tanalysis_GetVariableTypeName(const char *varname,char* TypeName)
{
	ParseTreeNode* DimVarKNode;
	int kind;

	DimVarKNode = tanalysis_GetVariableNode(varname,&kind);
	if(DimVarKNode == NULL)
		return 0;
	strcpy(TypeName,DimVarKNode->attr_string2);
	return 1;
}

void tanalysis_UsedNativeFunc_init()
{
	tanalysis_UsedNativeFunc_Num = 0;
}

int tanalysis_UsedNativeFunc_insert(const char* name)
{
	int i;
	/* check if it is already inserted */
	for(i=0; i< tanalysis_UsedNativeFunc_Num;i++)
	{
		if(str_nocasecmp(name,tanalysis_UsedNativeFunc_Tab[i]) == 0)
			return i;
	}
	tanalysis_UsedNativeFunc_Tab[i] = str_copynew(name);
	tanalysis_UsedNativeFunc_Num++;
	return i; /* Get and Set native invoke reserved */
}

int tanalysis_UsedNativeFunc_get(int index,char *name)
{
	if(index < tanalysis_UsedNativeFunc_Num)
	{
		strcpy(name,tanalysis_UsedNativeFunc_Tab[index]);
		return 1;
	}
	else
		return 0;
}

int tanalysis_UsedNativeFunc_count()
{
	return tanalysis_UsedNativeFunc_Num;
}

void tanalysis_UsedNativeFunc_release()
{
	int i;
	for(i = 0; i<tanalysis_UsedNativeFunc_Num; i++)
	{
		free(tanalysis_UsedNativeFunc_Tab[i]);
	}
}


