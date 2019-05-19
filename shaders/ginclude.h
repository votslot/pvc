
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

#if 0
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
	};
	
	/*
	struct Partition
	{
		float cx;
		float cy;
		float cz;
		float sz;
		unsigned int first;
		unsigned int last;
		unsigned int ndx;
		float pad1;
	};
	*/
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

	struct ColorizeData 
	{
		float xMin;
		float xMax;
		float yMin;
		float yMax;
		float zMin;
		float zMaz;
	};
#endif

#endif

