#include "storage.h"
#include "cbuff.h"
#include "..\shaders\ginclude.h"
#include "..\pcloud\pcloud.h"

class PointStorageImpl : public PointStorage
{
	struct pointp4
	{
		float  x, y, z, w;
	};

public:
	static const int sMaxBuffs = 32;
	static const int sMaxAllocSize = 1024 * 1024 * 128;
	int maxPointsInBuff = 0;
	int numPointsInBuff[sMaxBuffs];
	int numPartitionsInBuff[sMaxBuffs];
	SSBBuffer bufferPoints[sMaxBuffs];
	SSBBuffer bufferPartition[sMaxBuffs];
	float bbXMin = FLT_MAX;
	float bbXMax = FLT_MIN;
	float bbYMin = FLT_MAX;
	float bbYMax = FLT_MIN;
	float bbZMin = FLT_MAX;
	float bbZMax = FLT_MIN;

	int numInUse;
	bool hasPoints;

	char *pTemp = NULL;
	int numPointsInTemp = 0;

	Partition *pPartitions = NULL;
	int numPartitions = 0;

	int sizeInTemp;
	unsigned int maxBuffSz;

	void Init()
	{
		numInUse = 0;
		sizeInTemp = 0;
		numPartitions = 0;
		hasPoints = false;
		unsigned int maxSSB = SSBBuffer::getMaxSizeInBytes();
		maxBuffSz = (sMaxAllocSize < maxSSB) ? sMaxAllocSize : maxSSB;
		maxPointsInBuff = maxBuffSz / (4 * sizeof(float));
		pTemp = new char[maxBuffSz];
		pPartitions = new Partition[maxPointsInBuff];

		for (int i = 0; i < sMaxBuffs; i++)
		{
			numPointsInBuff[i] = 0;
			numPartitionsInBuff[i] = 0;
			bufferPoints[i].init();
			bufferPartition[i].init();
		}
		bbZMin = FLT_MAX;
		bbZMax = FLT_MIN;
	}

	void SetPoint(float x, float y, float z, float w)
	{
		int ptSize = sizeof(float) * 4;
		if ((sizeInTemp) >= maxBuffSz)
		{
			AddNewBuffer();
		}
		float *pDest = (float*)pTemp;
		pDest += numPointsInTemp * 4;
		pDest[0] = x;
		pDest[1] = y;
		pDest[2] = z;
		pDest[3] = w;
		if (x < bbXMin) bbXMin = x;
		if (x > bbXMax) bbXMax = x;
		if (y < bbYMin) bbYMin = y;
		if (y > bbYMax) bbYMax = y;
		if (z < bbZMin) bbZMin = z;
		if (z > bbZMax) bbZMax = z;
		numPointsInTemp++;
		sizeInTemp += ptSize;
	}

	void DoneAddPoints()
	{
		if (sizeInTemp > 0)
		{
			AddNewBuffer();
		}
		hasPoints = true;
	}

	void AddNewBuffer()
	{
		//assert(numInUse < sMaxBuffs);
		std::function<void(partitionData<float> *pD)> OnDonePartition = [=](partitionData<float> *pDt)
		{
			float dx = pDt->maxX - pDt->minX;
			float dy = pDt->maxY - pDt->minY;
			float dz = pDt->maxZ - pDt->minX;
			float dMax = (dx > dy) ? dx : dy;
			dMax = (dMax > dz) ? dMax : dz;

			pPartitions[numPartitions].cx = (pDt->maxX + pDt->minX) *0.5f;
			pPartitions[numPartitions].cy = (pDt->maxY + pDt->minY) *0.5f;
			pPartitions[numPartitions].cz = (pDt->maxZ + pDt->minZ) *0.5f;
			pPartitions[numPartitions].sz = dMax;
			pPartitions[numPartitions].ndx = numPartitions;
			if ((numPartitions & 31) == 0) std::cout << ".";
			numPartitions++;

			// shuffle points
			pointp4 *pPt = (pointp4*)pTemp;
			if (pDt->numPoints == 4096)
			{
				for (int n = 0; n < 4096; n++)
				{
					int k1 = pDt->first + (rand() & 4095);
					int k2 = pDt->first + (rand() & 4095);
					pointp4 ptTemp = pPt[k1];
					pPt[k1] = pPt[k2];
					pPt[k2] = ptTemp;
				}
			}
		};

		DoPartitionXYZW_Float(pTemp, numPointsInTemp, OnDonePartition);
		std::cout << "done" << std::endl;

		bufferPoints[numInUse].setData(pTemp, sizeInTemp);
		bufferPartition[numInUse].setData(pPartitions, numPartitions * sizeof(Partition));
		numPointsInBuff[numInUse] = numPointsInTemp;
		numPartitionsInBuff[numInUse] = numPartitions;
		numInUse++;

		sizeInTemp = 0;
		numPointsInTemp = 0;
		numPartitions = 0;
	}

	void Release()
	{

	}

	//-------------------
	float GetXMin() { return bbXMin; }
	float GetYMin() { return bbYMin; }
	float GetZMin() { return bbZMin; }
	float GetXMax() { return bbXMax; }
	float GetYMax() { return bbYMax; }
	float GetZMax() { return bbZMax; }

	bool IsReady() 
	{
		return hasPoints;
	}

	int getNumAvailableBuffers()
	{
		return sMaxBuffs;
	}

	SSBBuffer * GetPointBuffer(int n) 
	{
		return &bufferPoints[n];
	}

	SSBBuffer * GetPartitionBuffer(int n)
	{
		return &bufferPartition[n];
	}

	int GetNumPointsInBuffer(int n) 
	{
		return numPointsInBuff[n];
	}

	int GetNumBuffersInUse() 
	{
		return numInUse;
	}
	int GetZBits()
	{
		return 24;
	}
};

static  PointStorageImpl theStorage;
PointStorage  * PointStorage::GetInstatnce()
{
	return  &theStorage;
}


