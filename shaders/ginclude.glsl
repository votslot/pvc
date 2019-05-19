

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

#if 0
 DECLARE_STRUCT_AS_STRING(cs_macro_zbits, 
	#define mZbits 24
 );

 DECLARE_STRUCT_AS_STRING(cs_structs, 
	struct  GlobalParams
	{
		float screenX;
		float screenY;
		float zNear;
		float zFar;
		float zRange;
		float maxDimension;
		unsigned int wrkLoad;
		float px;
		float py;
		float pz;
		float bbMinX;
		float bbMaxX;
		float bbMinY;
		float bbMaxY;
		float bbMinZ;
		float bbMaxZ;
		float scrMin;
		float pad0;
		float pad1;
		float pad2;
	};);

 DECLARE_STRUCT_AS_STRING(cs_structs_partition, 
	 struct Partition
	 {
		 float cx;
		 float cy;
		 float cz;
		 float sz;
		 uint first;
		 uint last;
		 uint ndx;
		 float pad1;
	 };);

 DECLARE_STRUCT_AS_STRING(cs_struct_colorize, 
	struct ColorizeData 
	{
		float xMin;
		float xMax;
		float yMin;
		float yMax;
		float zMin;
		float zMaz;
	};);


#define DECLARE_STRUCT_AS_STRING(_stringname, _v) _v
#endif

#endif

