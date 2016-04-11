#ifndef	_TLBASIC_BYTECODE_H_
#define	_TLBASIC_BYTECODE_H_


/*=========================================================================*/
#define	SIZE_STACK_FRAME_RECORD	12 /* IP + FP +FI */
/*=========================================================================*/

typedef	unsigned char	u8;
typedef	unsigned short	u16;
typedef	unsigned long	u32;
typedef	short			s16;
typedef	long			s32;
typedef	float			f32;
typedef	char*			ptr32;
typedef	double			f64;
typedef	enum 
{
	NullType	   = 0,
	ByteType	   = 1,
	ShortType	   = 2,
	IntegerType	   = 4,
	LongType	   = 5,
	DateType	   = 6,
	FloatType	   = 7,
	DoubleType	   = 8,
	StringType	   = 9,
	PtrType		   = 10,
	TypeType	   = 11,
	datatypeType   = 12,
	accessflagType = 13,
	IndexType	   = 14,
}DataType;

typedef	enum
{
	ACCESS_FLAG_GLOBAL = 1,
	ACCESS_FLAG_LOCAL  = 2,
}AccessFlag;

/*=========================================================================*/
/*	ByteCode Description Format
/* (Pop1,Pop2,...) ByteCode	Op1,Op2,...	[Push1,Push2...]
/*=========================================================================*/
typedef	enum 
{
	 C_NOP			= 0x00,	/* NOP */
	 C_CONST			  ,	/* CONST type value	*/

	 C_LOAD				  ,	/* LOAD	access	address	[value]	*/
	 C_STORE			  ,	/* (value) STORE access	address	*/
	 C_HEAP_LOAD		  ,	/* (base_addr,index) HEAP_LOAD type	[value]	*/
	 C_HEAP_STORE		  ,	/* (value,base_addr,index) HEAP_STORE type	*/
	 C_FORCE_LOAD		  ,	/* FORCE_LOAD access address type */

	 C_ADD				  ,	/* (op1,op2) ADD [result] */
	 C_SUB				  ,	/* (op1,op2) SUB [result] */
	 C_MUL				  ,	/* (op1,op2) MUL [result] */
	 C_DIV				  ,	/* (op1,op2) DIV [result] */
	 C_MOD				  ,	/* (op1,op2) MOD [result] */
	 C_OPP				  ,	/* (op1)	 OPP [result] */
	 C_AND				  ,	/* (op1,op2) AND [result] */
	 C_OR				  ,	/* (op1,op2) OR	 [result] */
	 C_NOT				  ,	/* (op1)	 NOT [result] */
	 C_LG				  ,	/* (op1,op2) LG	 [result] */
	 C_LT				  ,	/* (op1,op2) LT	 [result] */
	 C_EQ				  ,	/* (op1,op2) EQ	 [result] */
	 C_LG_EQ			  ,	/* (op1,op2) LG_EQ [result]	*/
	 C_LT_EQ			  ,	/* (op1,op2) LT_EQ [result]	*/
	 C_NOT_EQ			  ,	/* (op1,op2) NOT_EQ	[result] */

	 C_ALLOC			  ,	/* (size) ALLOC [memaddr] */
	 C_ALLOC_ARRAY		  ,	/* (size1,size2,size3,...) ALLOC_ARRAY dimension [memaddr]	*/
	 C_FREE				  ,	/* (address) FREE  */
	 C_FREE_ARRAY		  ,	/* (address) FREE_ARRAY	dimension */

	 C_PUSH				  ,	/* PUSH	type */
	 C_POP				  ,	/* POP	count */
	 C_POP_RESTOP		  ,	/* POP_RESTOP count	*/
	 C_CONVERT			  ,	/* CONVERT type	*/

	 C_FJP				  ,	/* FJP size	*/
	 C_TJP				  ,	/* TJP size	*/
	 C_JMP				  ,	/* JMP size	*/

	 C_CALL				  ,	/* CALL	function_index */
	 C_INVOKE			  ,	/* INVOKE "function	name" */
	 C_RET				  ,	/* RET */

	 C_EXIT				  ,	/* EXIT	*/

	 C_DEBUG			  ,
	 C_RESERVE			  ,
}tByteCode;

#define BYTECODE_COUNT (C_RESERVE+1)

#define	BYTE_CODE_NAMES {	 \
"NOP",          \
"CONST",        \
"LOAD",         \
"STORE",        \
"HEAP_LOAD",    \
"HEAP_STORE",   \
"FORCE_LOAD",   \
"ADD",          \
"SUB",          \
"MUL",          \
"DIV",          \
"MOD",          \
"OPP",          \
"AND",          \
"OR",           \
"NOT",          \
"LG",           \
"LT",           \
"EQ",           \
"LG_EQ",        \
"LT_EQ",        \
"NOT_EQ",       \
"ALLOC",        \
"ALLOC_ARRAY",  \
"FREE",         \
"FREE_ARRAY",   \
"PUSH",         \
"POP",          \
"POP_RESTOP",   \
"CONVERT",      \
"FJP",          \
"TJP",          \
"JMP",          \
"CALL",         \
"INVOKE",       \
"RET",          \
"EXIT",         \
"DEBUG",        \
"RESERVE",      \
}

#define  BYTE_CODE_FORMAT {   \
	"",		 /* NOP */        \
	"TV",	 /* CONST */      \
	"FI",    /* LOAD  */      \
	"FI",	 /* STORE */      \
	"T",     /* HEAP_LOAD */  \
	"T",     /* HEAP_STORE */ \
	"FIT",   /* FORCE_LOAD */ \
	"",      /* ADD */        \
	"",      /* SUB */        \
	"",		 /* MUL */        \
	"",		 /* DIV */        \
	"",		 /* MOD */        \
	"",		 /* OPP */        \
	"",		 /* AND */        \
	"",		 /* OR  */        \
	"",		 /* NOT */        \
	"",		 /* LG  */        \
	"",		 /*	LT	*/        \
	"",		 /*	EQ	*/        \
	"",		 /*	LG_EQ */      \
	"",		 /*	LT_EQ */      \
	"",		 /*	NOT_EQ */     \
	"",	     /*	ALLOC  */     \
	"I",	 /*	ALLOC_ARRAY	*/ \
	"",		 /*	FREE */       \
	"I",	 /*	FREE_ARRAY */ \
	"T",	 /*	PUSH */       \
	"I",	 /* POP */        \
	"I",     /* POP_RESTOP */ \
	"T",	 /* CONVERT */    \
	"I",	 /* FJP */        \
	"I",	 /* TJP */        \
	"I",	 /* JMP */        \
	"I",	 /* CALL */       \
	"I",	 /* INVOKE */     \
	"I",	 /* RET */        \
	"",		 /* EXIT */       \
	"",      /* DEBUG */      \
	"",      /* RESERVE */    \
}


/* define the program entry	function name */
#define	ENTRY_FUNC_NAME	  "main"

#endif
