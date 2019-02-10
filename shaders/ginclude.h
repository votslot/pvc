
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
	};
	
	struct Partition
	{
		unsigned int first;
		unsigned int last;
		float sz;
		float pad;
	};
	

#endif

