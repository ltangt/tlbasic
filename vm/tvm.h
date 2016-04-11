#ifndef _TLBASIC_VM_H_
#define _TLBASIC_VM_H_
#include <stdio.h>
#include "tbytecode.h"
#include "tVMValue.h"

int        tVM_Initialize(FILE* fp); /* Initialize Virtual Machine */
int        tVM_Execute(); /* execute a basic function */
int        tVM_Exit(void); /* exit Virtual Machine */

#endif

