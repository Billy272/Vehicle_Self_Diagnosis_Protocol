#ifndef _RAPIDDIAGNOSIS_H
#define _RAPIDDIAGNOSIS_H

#include "../interface/protocol_define.h"

/********************************通用函数声明**************************************/
void process_readVIN_function( void* pIn, void* pOut );
void process_Car_ClearDTC( void* pIn, byte* pOut );


#endif
