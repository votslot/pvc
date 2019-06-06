
#include <memory.h>
#include <math.h>
#include <stdlib.h> 
#include<functional>
#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "partition.h" 

namespace pcrlib
{
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

	template<typename T> static void Swap(T *pData, unsigned int n, unsigned int k)
	{
		T temp = pData[k];
		pData[k] = pData[n];
		pData[n] = temp;
	}

	template<typename T> static float getV(T* pD, int k, int n)
	{
		float *pp = (float*)(&pD[k]);
		return pp[n];
	}

	unsigned int getNearestPow2(unsigned int val)
	{
		for (int k = 31; k >= 0; k--)
		{
			if (val & (1 << k))
			{
				return 1 << k;
			}
		}
		return 0;
	}

	template<typename T>
	void Separate(T *pData, unsigned int firstIn, unsigned int lastIn, unsigned int nval, unsigned int pivotIndexIn, compFuncType comp, int nn)
	{
		// test
		//printf("Input: fist=%d lastIn=%d nval=%d hint = %d nn = %d\n ",firstIn,lastIn, nval, pivotIndexIn,nn);
		//T *pData = pDataIn + firstIn;
		unsigned int cnt = 0;
		unsigned int first = firstIn;
		unsigned int last = lastIn;
		unsigned int pivotIndex = pivotIndexIn;
		for (;;)
		{
			T tempHint = pData[pivotIndex];
			Swap(pData, first, pivotIndex);
			cnt = first + 1;
			for (unsigned int k = first + 1; k <= last; k++)
			{
				if (comp(&pData[k], &tempHint) < 0)
				{
					Swap(pData, k, cnt);
					cnt++;
				}
			}
			cnt--;
			Swap(pData, first, cnt);

			if (nval < cnt)
			{
				last = cnt - 1;
				pivotIndex = (first + last) / 2;
				continue;
			}

			if (nval > cnt)
			{
				first = cnt + 1;
				pivotIndex = (first + last) / 2;
				continue;
			}
			break;
		}


		// test
#if 0
		float tstVal = getV(pData, nval, nn);
		for (unsigned int k = 0; k < lastIn; k++)
		{
			if (k < nval) {
				float vl = getV(pData, k, nn);
				if ((vl = tstVal) && (vl != tstVal)) {
					printf("k = %d vl = %f  tstVa= %f\n ", k, vl, tstVal);
				}
			}
			else
			{
				float vl = getV(pData, k, nn);
				if ((vl < tstVal) && (vl != tstVal)) {
					printf("k = %d vl = %f  tstVa= %f\n ", k, vl, tstVal);
				}

			}
		}
#endif	

	}


	template<typename T, typename P>
	void BuildGroups(T * pData, unsigned int first, unsigned int last, compFuncType *compFunc, std::function<void(partitionData<P> *pD)> donePartitionFunc)
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

		int n = 0;
		if ((dx >= dy) && (dx >= dz)) {
			n = 0;
		}
		else if ((dy >= dx) && (dy >= dz)) {
			n = 1;
		}
		else  if ((dz >= dx) && (dz >= dy)) {
			n = 2;
		}

		//std::qsort((void*)(pData + first), last - first + 1, sizeof(T), compFunc[n]);
		unsigned int numPt = last - first + 1;
		unsigned int pivot;
		if ((numPt & (numPt - 1)) == 0)// is power of 2
		{
			pivot = first + (numPt / 2) - 1;
		}
		else
		{
			pivot = first + getNearestPow2(numPt) - 1;
		}

		Separate<T>(pData, first, last, pivot, first, compFunc[n], n);
		BuildGroups<T>(pData, first, pivot, compFunc, donePartitionFunc);
		BuildGroups<T>(pData, pivot + 1, last, compFunc, donePartitionFunc);

	}

	void DoPartitionXYZW_Float(void *pData, unsigned int num, std::function<void(partitionData<float> *pD)> func)
	{

		struct point4f {
			float x, y, z, w;
		};
		static compFuncType compFuncXYZ[3] = { CompX<point4f>,CompY<point4f>,CompZ<point4f> };


		//unsigned int aa = getNearestPow2(1025);
		if (0)
		{
			int numInTest = 32000;
			point4f *pTest = new point4f[numInTest];
			for (int k = 0; k < numInTest; k++) {
				pTest[k].x = (float)rand(); pTest[k].y = 0.0f; pTest[k].z = 0.0f;
			}
			//Separate<point4f>((point4f*)pTest, numInTest-1, 32000-1, hint, compFuncXYZ[0]);
		}
		///////

		std::cout << "building group" << std::endl;
		BuildGroups<point4f>((point4f*)pData, 0, num - 1, compFuncXYZ, func);


		//DoPartition<point4f,float>((point4f*)pData, 0, num - 1,0,func);
	}
}