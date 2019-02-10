

#ifndef _G_INCLUDE_GLSL
#define _G_INCLUDE_GLSL

const std::string cs_glversion = 
R""(
	#version 430 core 
)"";

const std::string  cs_structs = 
R""(
	struct  GlobalParams
	{
		float screenX;
		float screenY;
		float zNear;
		float zFar;
		float zScale;
		float maxDimension;
		uint  wrkLoad;
	};

	struct Partition
	{
		uint first;
		uint last;
		float sz;
		float pad;
	};
)"";

#endif

