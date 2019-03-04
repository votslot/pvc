
#include <memory.h>
#include <math.h>
#include <stdlib.h> 
#include<functional>
#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "pcloud.h" 

typedef int(*compFuncType) (const void*a, const void*b);
template<typename T> static int CompX(const void*a, const void*b)
{
	return ((((T*)a)->x < ((T*)b)->x)) ? -1 : 1;
}

template<typename T> static int CompY(const void*a, const void*b)
{
	return ((((T*)a)->y < ((T*)b)->y)) ? -1 : 1;
}

template<typename T> static int CompZ(const void*a, const void*b)
{
	return ((((T*)a)->z < ((T*)b)->z)) ? -1 : 1;
}


static int depthMax = 0;
template<typename T, typename E>
unsigned int Separate(T *pD, unsigned int shift, E mid, unsigned int first, unsigned int last) 
{
	int cnt = first;
	for (unsigned int k = first; k <= last; k++) 
	{
		E *pv = (E*)((char*)(pD + k) + shift);
		if (pv[0] + pv[0] <= mid) {
			T tmp = pD[cnt];
			pD[cnt] = pD[k];
			pD[k] = tmp;
			cnt++;
		}
	}
#if 1
	for (unsigned int i = first; i <=last; i++) {
		E *pv = (E*)((char*)(pD + i) + shift);
		if (i < cnt) {
			if (pv[0] + pv[0] > mid) {
				std::cout <<  pv[0] << std::endl;
			}
		}
		else {
			if (pv[0] + pv[0] <= mid) {
				std::cout << pv[0] << std::endl;
			}
		}
	}
#endif

	return cnt;
}

template<typename T, typename E>
void DoPartition(T * pData, unsigned int first, unsigned int last, unsigned int depth, std::function<void(int a, int b)> func)
{
	auto minX = pData[first].x;
	auto maxY = pData[first].y;
	auto minZ = pData[first].z;
	auto maxX = pData[first].x;
	auto minY = pData[first].y;
	auto maxZ = pData[first].z;

	for (unsigned int k = first; k <= last; k++) 
	{
		if (pData[k].x < minX)  minX = pData[k].x;
		if (pData[k].y < minY)  minY = pData[k].y;
		if (pData[k].z < minZ)  minZ = pData[k].z;
		if (pData[k].x > maxX)  maxX = pData[k].x;
		if (pData[k].y > maxY)  maxY = pData[k].y;
		if (pData[k].z > maxZ)  maxZ = pData[k].z;
	}
	auto dx = maxX - minX;
	auto dy = maxY - minY;
	auto dz = maxZ - minZ;

	int numPoints = last - first + 1;

	if (depth > 500)
	{
		std::cout << depth << std::endl;
	}
	if ((numPoints <=256) || (depth> 255)) {
		if (depth > depthMax)
		{
			depthMax = depth;
		}
		func(first, last);
		pData[first].w = 1.0f;
		return;
	}

	unsigned int ret = first;
	if ((dx >= dy) && (dx >= dz)) {
		ret = Separate<T, E>(pData, offsetof(class T, x), (maxX + minX) , first, last);
	}
	else if ((dy >= dx) && (dy >= dz)) {
		ret = Separate<T, E>(pData, offsetof(class T, y), (maxY + minY) , first, last);
	}
	else  if ((dz >= dx) && (dz >= dy)) {
		ret = Separate<T,E>(pData, offsetof(class T, z) , (maxZ + minZ) , first, last);
	}

	DoPartition<T,E>(pData, first, ret - 1, depth + 1,func);
	DoPartition<T,E>(pData, ret, last, depth + 1,func);
}




template<typename T,typename P>
void BuildGroups( const T * pData, unsigned int first,  unsigned int last ,compFuncType *compFunc, std::function<void(partitionData<P> *pD)> donePartitionFunc)
{
	auto minX = pData[first].x;
	auto maxY = pData[first].y;
	auto minZ = pData[first].z;
	auto maxX = pData[first].x;
	auto minY = pData[first].y;
	auto maxZ = pData[first].z;

	for (unsigned int k = first; k <= last; k++)
	{
		if (pData[k].x < minX)  minX = pData[k].x;
		if (pData[k].y < minY)  minY = pData[k].y;
		if (pData[k].z < minZ)  minZ = pData[k].z;
		if (pData[k].x > maxX)  maxX = pData[k].x;
		if (pData[k].y > maxY)  maxY = pData[k].y;
		if (pData[k].z > maxZ)  maxZ = pData[k].z;
	}
	auto dx = maxX - minX;
	auto dy = maxY - minY;
	auto dz = maxZ - minZ;
	int numPoints = last - first + 1;
	//std::cout << "num= " << numPoints << std::endl;
	if (numPoints <= 4096)
	{
		partitionData<P> part;
		part.numPoints = numPoints;
		part.first = first;
		part.minX = minX;
		part.minY = minY;
		part.minZ = minZ;
		part.maxX = maxX;
		part.maxY = maxY;
		part.maxZ = maxZ;

		donePartitionFunc(&part);
		if (numPoints != 4096) 
		{
			std::cout << "num= " << numPoints << std::endl;
		}
		return;
	}

	if ((dx >= dy) && (dx >= dz)) {
		std::qsort((void*)(pData + first), last - first + 1, sizeof(T), compFunc[0]);
	}
	else if ((dy >= dx) && (dy >= dz)) {
		std::qsort((void*)(pData + first), last - first + 1, sizeof(T), compFunc[1]);
	}
	else  if ((dz >= dx) && (dz >= dy)) {
		std::qsort((void*)(pData + first), last - first + 1, sizeof(T), compFunc[2]);
	}
	
	//unsigned int mid = (first + last) / 2;
	BuildGroups<T>(pData, first,                first - 1 + numPoints/2, compFunc, donePartitionFunc);
	BuildGroups<T>(pData, first + numPoints / 2, last,                   compFunc, donePartitionFunc);

}

void DoPartitionXYZW_Float(void *pData, unsigned int num, std::function<void(partitionData<float> *pD)> func)
{
	
	struct point4f {
		float x, y, z ,w;
	};
	std::cout << "building group" << std::endl;
	static compFuncType compFuncXYZ[3] = { CompX<point4f>,CompY<point4f>,CompZ<point4f> };
	BuildGroups<point4f>((point4f*)pData, 0, num-1, compFuncXYZ,func);

#if 0
	point4f *pT = (point4f*)pData;
	for (int k = 0; k < num; k++) 
	{
		int ng =  k / 4096;
		pT[k].w = (float) ( ng &7) + 1.0f;
	
	}
#endif
	

	//DoPartition<point4f,float>((point4f*)pData, 0, num - 1,0,func);
}