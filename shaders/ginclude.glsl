

#ifndef _G_INCLUDE_GLSL
#define _G_INCLUDE_GLSL

#ifndef FOR_SHADER
#define FOR_SHADER 1
#endif

const std::string cs_glversion = 
R""(
	#version 430 core 
)"";

#include "ginclude-shared.h"

#ifdef FOR_SHADER
#undef FOR_SHADER 
#endif


#endif

