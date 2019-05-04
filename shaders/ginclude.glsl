

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
		float px;
		float py;
		float pz;
		float bbMinZ;
		float bbMaxZ;
		float scrMin;
		float pad0;
		float pad1;
		float pad2;
	};

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
	};
)"";

const std::string  cs_struct_colorize = 
R""(
struct ColorizeData 
	{
		float xMin;
		float xMax;
		float yMin;
		float yMax;
		float zMin;
		float zMaz;
	};
)"";

#endif

