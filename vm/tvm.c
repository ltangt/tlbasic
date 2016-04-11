#include "tVM.h"

#include "tVMValue.h"
#include "tNativeFunction.h"
#include "tbuffereader.h"
#include "tRunstack.h"
#include "tcodefile.h"
#include "api.h"
#include "global.h"
#include "tmem.h"
#include "tstring.h"

#include "tbytecode.h"

/*=========================================================================*/
/* VM data structure */
tCodeFile*       tVM_codefile;
tRunstack*       tVM_runstack;
tbufferreader*   tVM_codeReader; /* IP = tVM_codeReader->ptr */
u32 FP;
u32 FI;
u32 FirstFP; 

extern tNativeFunPtr API_FuncTab[];

/*=========================================================================*/
/* VM base operations */
/* Push */
#define PushInteger(value)    \
	tRunstack_pushS32(tVM_runstack,value)
#define PushVMValue(value)    \
	tRunstack_pushVMValue(tVM_runstack,value)
#define PushBytes(Num) \
	(tVM_runstack->top += (Num))

/*=========================================================================*/
/* Pop */
#define PopInteger(value)          \
	tRunstack_popS32(tVM_runstack,value)
#define PopVMValue(value)     \
	tRunstack_popVMValue(tVM_runstack,value)
#define PopBytes(Num) \
	(tVM_runstack->top -= (Num))

/*=========================================================================*/
/* Load */
#define LoadVMValue(stackindex,value)   \
	tRunstack_loadVMValue(tVM_runstack,stackindex,value)
#define ForceLoadVMValue(stackindex,type,value) \
	tRunstack_forceloadVMValue(tVM_runstack,stackindex,type,value)

/*=========================================================================*/
/* Store */
#define StoreVMValue(stackindex,value) \
	tRunstack_storeVMValue(tVM_runstack,stackindex,value)

/*=========================================================================*/
/* Read */
#define ReadCode()            \
	tbufferreader_readU8(tVM_codeReader)
#define ReadIndex()           \
	tbufferreader_readS32(tVM_codeReader)
#define ReadDataType()        \
	tbufferreader_readU8(tVM_codeReader)
#define ReadAccessFlag()      \
	tbufferreader_readU8(tVM_codeReader)
#define ReadInteger()         \
	tbufferreader_readS32(tVM_codeReader)
#define ReadString()          \
	tbufferreader_readString(tVM_codeReader)
#define ReadVMValue(value)         \
	tbufferreader_readVMValue(tVM_codeReader,value)

/*=========================================================================*/
/* Registers */
#define getFP()   (FP)
#define getFI()   (FI)
#define getIP()   (tVM_codeReader->ptr)
#define getSP()   (tVM_runstack->top)
#define getBASE() ((u32)tVM_runstack->buffer)

#define setFP(x)  (FP = (x))
#define setFI(x)  (FI = (x))
#define setIP(x)  (tVM_codeReader->ptr = (x))

#define addFP(x)  (FP += (x))
#define addFI(x)  (FI += (x))
#define addIP(x)  (tVM_codeReader->ptr += (x))


/*=========================================================================*/
/* functions */
int  tVM_InitializeCodeFile(FILE* fp);
int  tVM_InitializeRunStack(void);
int  tVM_InitializeRegisters(void);
int  tVM_InitializeGlobalVariables(void);
int  tVM_InitializeCodeReader(void);
int  tVM_ReleaseCodeReader(void);

/*=========================================================================
* FUNCTION:      tVM_Initialize
* TYPE:          public interface
* OVERVIEW:      Initialize Virtual Machine 
* INTERFACE:
*   parameters:  fp FILE* : the code file structure
*   returns:     0 : Initialize wrong
*                1 : Initialize success
*=======================================================================*/
int tVM_Initialize(FILE* fp)
{
	mem_init();

	/* initialize loading code file */
	if(!tVM_InitializeCodeFile(fp))
		return 0;
	
	/* initialize Running Stack */
	if(!tVM_InitializeRunStack())
		return 0;
	
	/* initialize Registers in the VM */
	if(!tVM_InitializeRegisters())
		return 0;
	
	/* initialize global Variables */
	if(!tVM_InitializeGlobalVariables())
		return 0;
		
	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_Exit
* TYPE:          public interface
* OVERVIEW:      exit Virtual Machine
* INTERFACE:
*   returns:     0 : exit wrong
*                1 : exit success
*=======================================================================*/
int tVM_Exit()
{
	tcodefile_release(tVM_codefile);
	tRunstack_release(tVM_runstack);

	mem_exit();

	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_InitializeCodeFile
* TYPE:          private operation
* OVERVIEW:      Initialize the code file structure
* INTERFACE:
*   parameters:  fp FILE* : the code file structure
*   returns:     0 : file not exist
*                1 : 
*=======================================================================*/
int tVM_InitializeCodeFile(FILE* fp)
{
	int ret;

	tVM_codefile = tcodefile_create();
	ret = tcodefile_load(tVM_codefile,fp);

	switch(ret)
	{
	case RET_LOAD_NOTCODEFILE:
		error_out("the file is not code file for the VM\n");
		break;
	case RET_LOAD_NOTMAGICVERSION:
		error_out("the file version is not suitable for the VM\n");
		break;
	case RET_LOAD_ERROR:
		error_out("errors in loading code file in VM \n");
		break;
	}
	return ret== RET_LOAD_SUCCESS;
}

/*=========================================================================
* FUNCTION:      tVM_InitializeRunStack
* TYPE:          private operation
* OVERVIEW:      create a run stack for Virtual Machine
* INTERFACE:
*   returns:     0 : wrong
*                1 : success
*=======================================================================*/
int tVM_InitializeRunStack()
{
	tVM_runstack = tRunstack_create(SIZE_RUNSTACK);
	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_InitializeRegisters
* TYPE:          private operation
* OVERVIEW:      set initialize value to all the registers
* INTERFACE:
*   returns:     0 : error
*                1 : success
*=======================================================================*/
int tVM_InitializeRegisters()
{
	FP = 0;
	FI = 0;
	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_InitializeGlobalVariables
* TYPE:          private operation
* OVERVIEW:      Push All the Global variables into the run stack
* INTERFACE:
*   returns:     0 : wrong
*                1 : success
*=======================================================================*/
int tVM_InitializeGlobalVariables()
{
	u32  i;
	tGlobal_Variable *glvar;
	tVMValue value;
	for(i=0;i<tVM_codefile->num_global_Variables;i++)
	{
		glvar = &tVM_codefile->global_Variables[i];
		
		value.type = glvar->type;
		value.value.int_val = 0;

		PushVMValue(&value);
		addFP(sizeofData(glvar->type) + sizeofData(datatypeType)); /* add FP */
	}
	FirstFP = FP; /* record the init FP */
	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_InitializeCodeReader
* TYPE:          private operation
* OVERVIEW:      initialize the byte code buffer reader
* INTERFACE:
*   parameters:  
*   returns:     0 : error
*                1 : success
*=======================================================================*/
int tVM_InitializeCodeReader()
{
	tFunction *fun = &tVM_codefile->functions[FI];
	tVM_codeReader = tbufferreader_newFromBuf(fun->code,fun->codesize);
	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_ReleaseCodeReader
* TYPE:          private operation
* OVERVIEW:      release the byte code buffer reader
* INTERFACE:
*   parameters:  
*   returns:     0 : error
*                1 : success
*=======================================================================*/
int tVM_ReleaseCodeReader()
{
	tbufferreader_release(tVM_codeReader);
	return 1;
}

/*=========================================================================
* FUNCTION:      tVM_Execute
* TYPE:          public interface
* OVERVIEW:      execute a basic function
* INTERFACE:
*   parameters:  
*   returns:     
*                the result of the basic function
*=======================================================================*/
int tVM_Execute()
{
	int       running = 1;
	u8        bytecode;
	u8        type;
	u8        ac_flag;
	s32       integer;
	s32       stackindex,index;
	tVMValue  value1,value2,value3;
	tVMValue  retValue;// = (tVMValue*)mem_alloc(sizeof(tVMValue));

	/* initialize the running Stack FP */
	setFP(FirstFP);

	/* seek to the entry function */
	setFI(0);
	
	/* initialize the code reader */
	tVM_InitializeCodeReader();

	/* execute the byte codes in loop  */
	while(running)
	{
		bytecode = ReadCode();
		switch(bytecode)
		{
		case C_NOP:
			break;
		case C_CONST:
			{
				ReadVMValue(&value1);
				PushVMValue(&value1);
				break;
			}
		case C_LOAD:
			{
				ac_flag  =ReadAccessFlag();
				if(ac_flag == ACCESS_FLAG_GLOBAL)
					stackindex = ReadIndex();
				else
					stackindex = ReadIndex() + getFP();
				LoadVMValue(stackindex,&value1);
				PushVMValue(&value1);
				break;
			}
		case C_STORE:
			{
				ac_flag  =ReadAccessFlag();
				if(ac_flag == ACCESS_FLAG_GLOBAL)
					stackindex = ReadIndex();
				else
					stackindex = ReadIndex() + getFP();
				PopVMValue(&value1); /* pop the source value */
				StoreVMValue(stackindex,&value1);
				break;
			}
		case C_HEAP_LOAD:
			{
				type = ReadDataType();

				PopVMValue(&value2); /* Pop Addr */
				PopVMValue(&value1); /* Pop Base */	
				tVMValue_HeapLoad(value1.value.ptr_val+value2.value.int_val,type,&value3); /* load the heap memory */
				PushVMValue(&value3); /* push the loaded value */
				break;
			}
		case C_HEAP_STORE:
			{
				ptr32 addr;
				type = ReadDataType();

				PopVMValue(&value3); /* Pop Addr */
				PopVMValue(&value2); /* Pop Base */
				PopVMValue(&value1); /* Pop Value */
				addr = (ptr32)(value2.value.ptr_val + value3.value.int_val);
				if(value1.type != type)
				{
					tVMValue_ConvertType(&value1,type);
				}
				tVMValue_HeapStore(addr,&value1);
				break;
			}
		case C_FORCE_LOAD:
			{
				ac_flag  =ReadAccessFlag();
				if(ac_flag == ACCESS_FLAG_GLOBAL)
					stackindex = ReadIndex();
				else
					stackindex = ReadIndex() + getFP();
				type = ReadDataType();

				ForceLoadVMValue(stackindex,type,&value1);
				PushVMValue(&value1);
				break;
			}
		case C_ALLOC:
			{
				PopVMValue(&value1);
				value2.type = PtrType;
				value2.value.ptr_val = (ptr32)mem_alloc(value1.value.int_val);
				memset(value2.value.ptr_val,0,value1.value.int_val);
				PushVMValue(&value2);
				break;
			}
		case C_ALLOC_ARRAY:
			{
				s32   i;
				s32  dimension; 
				s32* index_ranges;
				
				dimension = ReadInteger();
				if(dimension < 1)
					break;
				index_ranges = (s32*)mem_alloc(sizeof(s32)*dimension);
				for(i=0;i<dimension;i++)
				{
					PopVMValue(&value1);
					index_ranges[dimension-i-1] = value1.value.int_val;
				}
				value1.type = PtrType;
				value1.value.ptr_val = tVMValue_HeapAllocMultiArray(dimension,index_ranges,0);
				PushVMValue(&value1);
				
				mem_free(index_ranges);
				break;
			}
		case C_FREE:
			{
				PopVMValue(&value1);
				if(value1.value.ptr_val != NULL)
					mem_free(value1.value.ptr_val);
				break;
			}
		case C_FREE_ARRAY:
			{
				break;
			}
		case C_PUSH:
			{
				value1.type  = ReadDataType();
				value1.value.int_val = 0;
				PushVMValue(&value1);
				break;
			}
		case C_POP:
			{
				s32 i;
				integer = ReadInteger();
				for(i=0;i<integer;i++)
				{
					PopVMValue(&value1);
					tVMValue_FreeSelf(&value1);
				}
				break;
			}
		case C_POP_RESTOP:
			{
				s32 i;
				integer = ReadInteger();
				PopVMValue(&value2); /* reserve top value */
				for(i=0;i<integer;i++)
				{
					PopVMValue(&value1);
					tVMValue_FreeSelf(&value1);
				}
				PushVMValue(&value2); /* push back top value */
				break;
			}
		case C_CONVERT:
			{
				u8 type = (u8)ReadDataType();
				PopVMValue(&value1);
				tVMValue_ConvertType(&value1,type);
				PushVMValue(&value1);
				break;
			}
		case C_ADD:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_Add(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_SUB:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_Sub(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_MUL:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_Mul(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_DIV:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_Div(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_MOD:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_Mod(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_OPP:
			{
				PopVMValue(&value1);
				tVMValue_Opp(&value1,&value2);
				PushVMValue(&value2);

				tVMValue_FreeSelf(&value1);
				break;
			}
		case C_AND:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_AND(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_OR:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_OR(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_EQ:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_EQ(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_NOT_EQ:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_NOTEQ(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_LT:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_LT(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_LG:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_LG(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_LT_EQ:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_LTEQ(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_LG_EQ:
			{
				PopVMValue(&value2);
				PopVMValue(&value1);
				tVMValue_LGEQ(&value1,&value2,&value3);
				PushVMValue(&value3);

				tVMValue_FreeSelf(&value1);
				tVMValue_FreeSelf(&value2);
				break;
			}
		case C_FJP:
			{
				s32 size = ReadIndex();
				PopVMValue(&value1);
				if(value1.value.int_val == 0) /* if it is false */
					addIP(size);
				break;
			}
		case C_TJP:
			{
				s32 size = ReadIndex();
				PopVMValue(&value1);
				if(value1.value.int_val != 0) /* if it is true */
					addIP(size);
				break;
			}
		case C_JMP:
			{
				s32 size = ReadIndex();
				addIP(size);
				break;
			}
		case C_CALL:
			{
				/* read function name */
				integer = ReadIndex();
				/* push the stack frame */
				PushInteger(getIP());
				PushInteger(getFI());
				PushInteger(getFP());
				/* goto the call function code */
				tVM_ReleaseCodeReader();
				setFI(integer);
				tVM_InitializeCodeReader();
				/* set new FP,RP */
				setFP(getSP());
				break;
			}
		case C_INVOKE:
			{
				/* read function name */
				index = ReadIndex();
				/* execute the native function */
				tNativeFunction_Invoke(index);
				break;
			}
		case C_RET:
			{
				u32 param_bytes = ReadIndex();

				/* get the result of the function */
				retValue.type = NullType;
				PopVMValue(&retValue); 
				
				/* if this is the start function,then exit the loop */
				if(getFP() == FirstFP)
				{
					running = 0; /* set flag to stop while */
					break;
				}

				/* restore last stack frame and return to last function code */
				tVM_ReleaseCodeReader();
				PopInteger(integer);
				setFP(integer); 
				PopInteger(integer);
				setFI(integer);
				tVM_InitializeCodeReader();
				PopInteger(integer);
				setIP(integer);
				
				/* pop the old parameters */
				PopBytes(param_bytes);

				/* push back result of last function */
				PushVMValue(&retValue);
				break;
			}
		}
	}

	/* close the code reader */
	tVM_ReleaseCodeReader();
	return 1;
}
