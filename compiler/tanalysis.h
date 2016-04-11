#ifndef _TLBASIC_ANALYSIS_H_
#define _TLBASIC_ANALYSIS_H_

#include "tbytecode.h"
#include "tparse.h"

typedef enum
{
	GlobalVariableKind,
	LocalVariableKind,
	ArgVariableKind,
}VariableKind;

/*=========================================================================*/
/* name the tables and stacks */
void           tanalysis_SubAndFun_create();
int            tanalysis_SubAndFun_insert(ParseTreeNode* DefNode);
ParseTreeNode* tanalysis_SubAndFun_lookup(const char *name);
void           tanalysis_SubAndFun_release();

void           tanalysis_TypeDefine_create();
int            tanalysis_TypeDefine_insert(ParseTreeNode* TypeNode);
ParseTreeNode* tanalysis_TypeDefine_lookup(const char *TypeName);
ParseTreeNode* tanalysis_TypeDefine_findmember(const char *TypeName,const char *MemberName);
int            tanalysis_TypeDefine_memsize(const char *TypeName,int *size);
void           tanalysis_TypeDefine_release();

void           tanalysis_GlobalVar_create();
int            tanalysis_GlobalVar_push(ParseTreeNode* DimVarNode);
ParseTreeNode* tanalysis_GlobalVar_lookup(const char *name);
void           tanalysis_GlobalVar_release();

void           tanalysis_LocalVar_create();
int            tanalysis_LocalVar_push(ParseTreeNode* DimVarNode);
ParseTreeNode* tanalysis_LocalVar_lookup(const char *name);
ParseTreeNode* tanalysis_LocalVar_getTopLocal();
ParseTreeNode* tanalysis_LocalVar_pop();
int            tanalysis_LocalVar_getCount();
void           tanalysis_LocalVar_release();

void           tanalysis_ArgDef_create();
int            tanalysis_ArgDef_push(ParseTreeNode* ArgDefNode);
ParseTreeNode* tanalysis_ArgDef_lookup(const char *name);
void           tanalysis_ArgDef_release();
int            tanalysis_ArgDef_GetTopIndexInByte();

void           tanalysis_UsedNativeFunc_init();
int            tanalysis_UsedNativeFunc_insert(const char* name);
int            tanalysis_UsedNativeFunc_count();
int            tanalysis_UsedNativeFunc_get(int index,char* name);
void           tanalysis_UsedNativeFunc_release();

/*=========================================================================*/
// collect the global informations into tables
int  tanalysis_collectGlobalNodes(ParseTreeNode* programKNode);
int  tanalysis_collectArgList(ParseTreeNode* funKNode);

/*=========================================================================*/
// get the variable
ParseTreeNode* tanalysis_GetVariableNode(const char *name,int *kind);
int  tanalysis_GetGlobalVarAddress(ParseTreeNode* DimVarKNode);
int  tanalysis_GetLocalVarAddress(ParseTreeNode* DimVarKNode);
int  tanalysis_GetArgVarAddress(ParseTreeNode* DimVarKNode);
int  tanalysis_GetVarDataType(ParseTreeNode* DimVarKNode);
int  tanalysis_GetVarDimension(ParseTreeNode* DimVarKNode);
int  tanalysis_GetVariable(const char *name,int *pIsGlobal,int *pAddr,int *type,int *dimension);
int  tanalysis_GetVariableTypeName(const char *varname,char* TypeName);

/*=========================================================================*/
// address stacks
int  tanalysis_InitializeAddressStacks();
int  tanalysis_ReleaseAddressStacks();
int  tanalysis_PushForLoopAddress(long start_addr,long end_addr);
int  tanalysis_PushDoLoopAddress(long start_addr,long end_addr);
int  tanalysis_PopForLoopAddress(long *start_addr,long *end_addr);
int  tanalysis_PopDoLoopAddress(long *start_addr, long *end_addr);

#endif // _TLBASIC_ANALYSIS_H_
