
#include <assert.h>
#include <iostream>
#include "float.h"
#include "storage.h"
#include "icompute.h"
#include "partition.h"
#include "OpenGL/shaders/ginclude.h"



namespace pcrlib
{
	//extern void BuildNormals(RenderPoint *pPt, int num, const BdBox<float>& Bd);
	void BuildValues(float *pF, int num);

	namespace pvc {

		class PointStorageImpl : public PointStorage
		{
		public:
			static const int sMaxBuffs = 32;
			static const int sMaxAllocSize = 1024 * 1024 * 128;
			LibCallback *m_cb = new LibCallback();
			int maxPointsInBuff = 0;
			int numPointsInBuff[sMaxBuffs];
			int numPartitionsInBuff[sMaxBuffs];
			ICBuffer* bufferPoints[sMaxBuffs];
			ICBuffer* bufferPartition[sMaxBuffs];
			BdBox<float>  bdGlob;
			BdBox<float>  bdBuff;
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

			void* getMem(size_t sz)
			{
				void * ptr = malloc(sz);
				return ptr;
			}

			void releaseMem(void *ptr)
			{
				free(ptr);
			}

			void Init()
			{
				numInUse = 0;
				sizeInTemp = 0;
				numPartitions = 0;
				hasPoints = false;
				unsigned int maxSSB = ICBuffer::getMaxSizeInBytes();
				maxBuffSz = (sMaxAllocSize < maxSSB) ? sMaxAllocSize : maxSSB;
				maxPointsInBuff = maxBuffSz / (4 * sizeof(float));
				pTemp = new char[maxBuffSz];
				pPartitions = new Partition[maxPointsInBuff];

				for (int i = 0; i < sMaxBuffs; i++)
				{
					numPointsInBuff[i] = NULL;
					numPartitionsInBuff[i] = NULL;
					bufferPoints[i]    = createICBuffer();
					bufferPartition[i] = createICBuffer();
				}
				bdBuff.Reset();
				bbZMin = FLT_MAX;
				bbZMax = FLT_MIN;
			}

			void SetPoint(float x, float y, float z, float w)
			{
				bdBuff.Add(x, y, z);
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
				bdGlob.Add(x, y, z);
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
					if ((numPartitions & 31) == 0) m_cb->message(".");
					numPartitions++;

					// shuffle points

					RenderPoint *pPt = (RenderPoint*)pTemp;
					if (pDt->numPoints == 4096)
					{
						for (int n = 0; n < 4096; n++)
						{
							int k1 = pDt->first + (rand() & 4095);
							int k2 = pDt->first + (rand() & 4095);
							RenderPoint ptTemp = pPt[k1];
							pPt[k1] = pPt[k2];
							pPt[k2] = ptTemp;
						}
					}

				};

				// BuildNormals((RenderPoint *)pTemp, numPointsInTemp, bdBuff);
				BuildValues((float*)pTemp, numPointsInTemp);
				DoPartitionXYZW_Float(pTemp, numPointsInTemp, OnDonePartition);
				m_cb->message("done\n");

				bufferPoints[numInUse]->allocate(sizeInTemp);
				bufferPoints[numInUse]->setData(pTemp, sizeInTemp);

				bufferPartition[numInUse]->allocate(numPartitions * sizeof(Partition));
				bufferPartition[numInUse]->setData(pPartitions, numPartitions * sizeof(Partition));

				numPointsInBuff[numInUse] = numPointsInTemp;
				numPartitionsInBuff[numInUse] = numPartitions;
				numInUse++;

				sizeInTemp = 0;
				numPointsInTemp = 0;
				numPartitions = 0;
				bdBuff.Reset();
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

			ICBuffer * GetPointBuffer(int n)
			{
				return bufferPoints[n];
			}

			ICBuffer * GetPartitionBuffer(int n)
			{
				return bufferPartition[n];
			}

			int GetNumPointsInBuffer(int n)
			{
				return numPointsInBuff[n];
			}

			int GetNumBuffersInUse()
			{
				return numInUse;
			}

		};
	}

	void BuildValues(float *pF, int num)
	{
		RenderPoint *pPoint = (RenderPoint*)pF;

		unsigned int *pVals = new unsigned int[65536];
		for (int i = 0; i < 65536; i++) pVals[i] = 0;
		float minV = FLT_MAX;
		float maxV = FLT_MIN;
		for (int i = 0; i < num; i++)
		{
			float *ppF = (float*)(pPoint + i);
			float vf = ppF[3];
			if (vf > maxV) maxV = vf;
			if (vf < minV) minV = vf;
			unsigned int vi = (unsigned int)vf;
			if (vi >= 65535) vi = 65535;
			pVals[vi]++;
		}

		int t90 = (int)((float)num * 0.9f);
		int t_first = 0, t_last = 65535, total = num;
		for (;;)
		{
			if ((total <= t90) || (t_first >= t_last))
			{
				break;
			}
			if (pVals[t_first] < pVals[t_last])
			{
				total -= pVals[t_first];
				t_first++;
			}
			else
			{
				total -= pVals[t_last];
				t_last--;
			}
		}
		float sc = 1.0f;
		int range = t_last - t_first;
		if (range > 0) {
			sc = 31.0f / (float)range;
		}

		for (int i = 0; i < num; i++)
		{
			float *ppF = (float*)(pPoint + i);
			unsigned int vfi = (ppF[3] - minV)*sc;
			if (vfi > 31) vfi = 31;
			pPoint[i].w = vfi | (vfi << 5) | (vfi << 10);
		}

	}


	static  pvc::PointStorageImpl theStorage;
	PointStorage  * PointStorage::GetInstatnce(LibCallback *pCB)
	{
		theStorage.m_cb = pCB;
		return  &theStorage;
	}
}//namespace pcrlib



