
#ifndef _G_INCLUDE_H
#define _G_INCLUDE_H


typedef unsigned int uint;


#ifndef FOR_SHADER
#define FOR_SHADER 0
#endif

#include "ginclude-shared.h"

#ifdef FOR_SHADER
#undef FOR_SHADER 
#endif


#endif

