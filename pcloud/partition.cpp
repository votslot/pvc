
#include <memory.h>
#include <math.h>
#include <stdlib.h> 
#include<functional>
#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "pcloud.h" 

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

void DoPartitionXYZW_Float(void *pData, unsigned int num, std::function<void(unsigned int a, unsigned int b)> func)
{
	struct point4f {
		float x, y, z ,w;
	};
	DoPartition<point4f,float>((point4f*)pData, 0, num - 1,0,func);
}