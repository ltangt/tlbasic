#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tVM.h"

const char szVersion[] = "TL's Basic Virtual Machine alpha version: 0.5.6b update: 2005.10.08\n";
const char szUsage[] = "Usage : vm codefile\n";

int  StartVirtualMachine(FILE* codefp); /* launch basic virtual machine */

/*=========================================================================
* FUNCTION:      main
* TYPE:          program entrance
* OVERVIEW:      
* INTERFACE:
*   parameters:  arg int:
*                argv char*[]:
*   returns:     
*=======================================================================*/
int main(int argc,char *argv[])
{
	FILE* fp;
#ifdef _DEBUG
	char *debug_argv[] = {"vm.exe","out.bin"};
	argc = 2;
	argv = debug_argv;
#endif
	
	if(argc < 2)
	{
		printf(szVersion); 
		printf(szUsage);
		return 0;
	}
	/* open code file */
	if((fp = fopen(argv[1],"rb")) == NULL)
	{
		printf("Cannot open the file: %s\n",argv[1]);
		return 0;
	}
	/* start Virtual Machine */
	StartVirtualMachine(fp);
	/* close code file */
	fclose(fp);
#ifdef _DEBUG
	/* pause program for debugging */
	system("pause");
#endif
	return 0;
}

/*=========================================================================
* FUNCTION:      StartVirtualMachine
* TYPE:          public operation
* OVERVIEW:      launch the basic virtual machine
* INTERFACE:
*   parameters:  codefp FILE* : the code file structure
*   returns:     0 : error in launch the machine
*                1 : launch OK
*=======================================================================*/
int StartVirtualMachine(FILE* codefp)
{
	/* initialize the VM */
	if(!tVM_Initialize(codefp))
		return 0;
	
	/* execute start function in code in the VM */
	tVM_Execute();
	
	/* exit the VM */
	tVM_Exit();
	
	return 1;
}

