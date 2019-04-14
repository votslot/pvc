
#ifndef _G_INCLUDE_H
#define _G_INCLUDE_H

	struct  GlobalParams
	{
		float screenX;
		float screenY;
		float zNear;
		float zFar;
		float zScale;
		float maxDimension;
		unsigned int wrkLoad;
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
		unsigned int first;
		unsigned int last;
		unsigned int ndx;
		float pad1;
	};
#endif

