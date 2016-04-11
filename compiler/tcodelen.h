#ifndef _TLBASIC_CODELEN_H_
#define _TLBASIC_CODELEN_H_

#include "tparse.h"
#include "global.h"

/* ////////////////////////////////////////////////////////////////////// */
long tcodelen_ByteCode(u8 code,u8 type,const char* str);


/* ///////////////////////////////////////////////////////////////////// */
long tcodelen_StatementsK(ParseTreeNode* statementsKNode);
long tcodelen_DimK_Local(ParseTreeNode* dimKNode);
long tcodelen_AssignK(ParseTreeNode* assignKNode);
long tcodelen_ExpK(ParseTreeNode* expKNode);
long tcodelen_OpK(ParseTreeNode* opKNode);
long tcodelen_ConstNumK(ParseTreeNode* constNumKNode);
long tcodelen_ConstDoubleK(ParseTreeNode* constDoubleKNode);
long tcodelen_ConstStrK(ParseTreeNode* constStrKNode);
long tcodelen_VarnameK_Load(ParseTreeNode* varnameKNode);
long tcodelen_VarnameK_Store(ParseTreeNode* varnameKNode);
long tcodelen_PrimitiveVarnamesK_Load(ParseTreeNode* primitivesKNode);
long tcodelen_PrimitiveVarnamesK_Store(ParseTreeNode* primitivesKNode);
long tcodelen_CallK_Exp(ParseTreeNode* callKNode);
long tcodelen_CallK_Stmt(ParseTreeNode* callKNode);
long tcodelen_IfK(ParseTreeNode* ifKNode);
long tcodelen_WhileK(ParseTreeNode* whileKNode);
long tcodelen_ForK(ParseTreeNode* forKNode);
long tcodelen_DoWhileLoopK(ParseTreeNode* doloopKNode);
long tcodelen_DoLoopWhileK(ParseTreeNode* doloopKNode);
long tcodelen_ExitForK(ParseTreeNode* exitforKNode);
long tcodelen_ExitDoK(ParseTreeNode* exitdoKNode);
long tcodelen_ReturnK(ParseTreeNode* returnKNode);
long tcodelen_SelectK(ParseTreeNode* selectKNode);
long tcodelen_EraseK(ParseTreeNode* eraseKNode);
long tcodelen_FreeK(ParseTreeNode* redimKNode);
long tcodelen_NewK(ParseTreeNode* newKNode);

#endif
