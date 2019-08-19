
#include <assert.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <string>
#include "float.h"
#include "storage.h"
#include "icompute.h"
#include "partition.h"
#include "OpenGL/shaders/ginclude.h"



namespace pcrlib
{
	namespace pvc {

		class PointStorageImpl : public PointStorage
		{
		public:
			std::atomic_bool m_wrkStart = false;
			std::atomic_bool m_wrkDone = true;
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
			int totalProgress = 0;

			int numInUse;
			bool hasPoints;

			//char *pTemp = NULL;
			RenderPoint *pTemp = NULL;
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

				char *pTemp8 = new char[maxBuffSz];
				pTemp = (RenderPoint*)pTemp8;

				pPartitions = new Partition[maxPointsInBuff];

				for (int i = 0; i < sMaxBuffs; i++)
				{
					numPointsInBuff[i] = NULL;
					numPartitionsInBuff[i] = NULL;
					bufferPoints[i] = NULL;// createICBuffer();
					bufferPartition[i] = NULL;// createICBuffer();
				}
				bdBuff.Reset();
				bbZMin = FLT_MAX;
				bbZMax = FLT_MIN;
			}

			void SetPoint(float x, float y, float z, unsigned int w)
			{
				bdBuff.Add(x, y, z);
				int ptSize = sizeof(float) * 4;
				if ((sizeInTemp) >= maxBuffSz)
				{
					AddNewBuffer();
				}

				pTemp[numPointsInTemp].x = x;
				pTemp[numPointsInTemp].y = y;
				pTemp[numPointsInTemp].z = z;
				pTemp[numPointsInTemp].w = w;

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
					//report progress
					//if ((numPartitions & 31) == 0) 
					{
						m_cb->message(("\rProcessing:" + std::to_string(totalProgress)).c_str());
						totalProgress += 4096;
					}
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

				DoPartitionXYZW_Float(pTemp, numPointsInTemp, OnDonePartition);
				// Wait for GL thread to call initGlBuffers() 
				m_wrkDone = false;
				m_wrkStart = true;
				for (;;) {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					if (m_wrkDone) break;
				}
				//initGlBuffers();
	
				numPointsInBuff[numInUse] = numPointsInTemp;
				numPartitionsInBuff[numInUse] = numPartitions;
				numInUse++;

				sizeInTemp = 0;
				numPointsInTemp = 0;
				numPartitions = 0;
				bdBuff.Reset();
			}

			void initGlBuffers() 
			{
				if (!m_wrkStart) 
				{
					return;
				}
				//std::cout << "initGlBuffers()" << std::endl;
				bufferPoints[numInUse] = createICBuffer();
				bufferPoints[numInUse]->allocate(sizeInTemp);
				bufferPoints[numInUse]->setData(pTemp, sizeInTemp);

				bufferPartition[numInUse] = createICBuffer();;
				bufferPartition[numInUse]->allocate(numPartitions * sizeof(Partition));
				bufferPartition[numInUse]->setData(pPartitions, numPartitions * sizeof(Partition));
				m_wrkStart = false;
				m_wrkDone = true;
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



	static  pvc::PointStorageImpl theStorage;
	PointStorage  * PointStorage::GetInstatnce(LibCallback *pCB)
	{
		theStorage.m_cb = pCB;
		return  &theStorage;
	}

	void PointStorage::onGLTick()
	{
		theStorage.initGlBuffers();
	}




#if 0
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
#endif

}//namespace pcrlib



