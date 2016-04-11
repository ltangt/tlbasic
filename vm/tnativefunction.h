#ifndef _TLBASIC_NATIVEFUNCTION_H_
#define _TLBASIC_NATIVEFUNCTION_H_

#include "tbytecode.h"
#include "tRunstack.h"
#include "tcodefile.h"

#define MAX_NATIVE_FUNC 128

typedef void (*tNativeFunPtr)(void);

#define tNativeFunction_Invoke(func_index) API_FuncTab[func_index]()

u8      tNativeFunction_PopDataType(void);
s32     tNativeFunction_PopParamAsInteger(void);
u8      tNativeFunction_PopParamAsByte(void);
s32     tNativeFunction_PopParamAsLong(void);
f64     tNativeFunction_PopParamAsDouble(void);
u8*     tNativeFunction_PopParamAsString(void);
void*   tNativeFunction_PopParamAsPtr(void);
void*   tNativeFunction_PopParamAsType(void);

void    tNativeFunction_PushReturnAsNull(void);
void    tNativeFunction_PushReturnAsInteger(s32 integer);
void    tNativeFunction_PushReturnAsByte(u8 byte_value);
void    tNativeFunction_PushReturnAsLong(s32 long_value);
void    tNativeFunction_PushReturnAsDouble(f64 double_value);
void    tNativeFunction_PushReturnAsString(u8* string);
void    tNativeFunction_PushReturnAsPtr(void* ptr);
void    tNativeFunction_PushReturnAsType(void* Type);

#endif
