#include "storage.h"
#include "cbuff.h"
#include "..\shaders\ginclude.h"
#include "..\pcloud\pcloud.h"

#if 0
	void * operator new(size_t size)
	{
		void * p = malloc(size);
		return p;
	}

	void operator delete(void * p)
	{
		free(p);
	}
#endif


namespace pvc {
	template <typename T>
	struct BdBox {
		T xMin, xMax, yMin, yMax, zMin, zMax;
		bool isReset;
		BdBox()
		{
			Reset();
		}
		void Reset() { isReset = true; }
		void Add(T x, T y, T z)
		{
			if (isReset) {
				xMin = x; xMax = x; yMin = y; yMax = y; zMin = z; zMax = z;
			}
			if (x < xMin) xMin = x;
			if (y < yMin) yMin = y;
			if (z < zMin) zMin = z;
			if (x > xMax) xMax = x;
			if (y > yMax) yMax = y;
			if (z > zMax) zMax = z;
			isReset = false;
		}
	};

	class PointStorageImpl : public PointStorage
	{

	public:
		static const int sMaxBuffs = 32;
		static const int sMaxAllocSize = 1024 * 1024 * 128;
		int maxPointsInBuff = 0;
		int numPointsInBuff[sMaxBuffs];
		int numPartitionsInBuff[sMaxBuffs];
		SSBBuffer bufferPoints[sMaxBuffs];
		SSBBuffer bufferPartition[sMaxBuffs];
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
				if ((numPartitions & 31) == 0) std::cout << ".";
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

			BuildNormals((RenderPoint *)pTemp, numPointsInTemp, bdBuff);

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
			bdBuff.Reset();
		}

		//// Normals ///
		uint CombineBits(uint x, uint y, uint z)
		{
			uint res = 0;
			for (int i = 0, t = 0; i < 10; i++, t += 3)
			{
				uint xb = x & 1;
				uint yb = y & 1;
				uint zb = z & 1;
				uint tmp = (xb) | (yb << 1) | (zb << 2);
				res |= (tmp << t);
				x >>= 1;
				y >>= 1;
				z >>= 1;
			}
			return res;
		}

	
		bool GetN3(const RenderPoint &A, const RenderPoint &B, const RenderPoint &C, float &x, float &y, float &z)
		{
			double dx1 = (double)(A.x) - (double)B.x;
			double dy1 = (double)(A.y) - (double)B.y;
			double dz1 = (double)(A.z) - (double)B.z;
			double dx2 = (double)(C.x) - (double)B.x;
			double dy2 = (double)(C.y) - (double)B.y;
			double dz2 = (double)(C.z) - (double)B.z;
			double xx =  dy1 * dz2 - dz1 * dy2;
			double yy = -dx1 * dz2 + dz1 * dx2;
			double zz =  dx1 * dy2 - dy1 * dx2;
			double len = sqrt(xx * xx + yy * yy + zz * zz);
			if (len > 0.0) {
				x = (float)(xx / len);
				y = (float)(yy / len);
				z = (float)(zz / len);
				return true;
			}
			return false;
		}

		void BuildNormals(RenderPoint *pPt, int num, const BdBox<float>& Bd)
		{
			uint r = 2;
			uint g = 30;
			uint b = 0;
			uint col = r | (g << 5) | (b << 10);
			float sx = 1024.0f / (Bd.xMax - Bd.xMin);
			float sy = 1024.0f / (Bd.yMax - Bd.yMin);
			float sz = 1024.0f / (Bd.zMax - Bd.zMin);

			uint *pLocationMap = (uint*)getMem(num * sizeof(uint));
			uint *pEnum = (uint*)getMem(num * sizeof(uint));

			for (int i = 0; i < num; i++) pEnum[i] = i;

			for (int i = 0; i < num; i++)
			{
				uint xi = (uint)((pPt[i].x - Bd.xMin) * sx);
				uint yi = (uint)((pPt[i].y - Bd.yMin) * sy);
				uint zi = (uint)((pPt[i].z - Bd.zMin) * sz);
				pLocationMap[i] = CombineBits(xi, yi, zi);
				pPt[i].w = col;// CombineBits(xi, yi, zi);
			}

			for (int i = 1; i < num-1; i++)
			{
				float nx, ny, nz;
				if (GetN3(pPt[i - 1], pPt[i], pPt[i + 1], nx, ny, nz))
				{
					
					uint vr = (uint)(fabs(nx) * 31.0f);
					uint vg = (uint)(fabs(ny) * 31.0f);
					uint vb = (uint)(fabs(nz) * 31.0f);
					if (vr > 31) vr = 31;
					if (vg > 31) vg = 31;
					if (vb > 31) vb = 31;
					if (vb > 10) {
						pPt[i].w = vb | (vb << 5) | (vb << 10);
					}
					else 
					{
						uint vbb = vb * 8;
						if (vbb > 31) vbb = 31;
						pPt[i].w = 5 | (vbb << 5) | (5 << 10);
					}
					
					/*
					float r1 = 0.0, g1 = 1.0f, b1 = 0.0f;
					float r2 = 1.0, g2 = 1.0f, b2 = 1.0f;
					float wt = fabs(nz);
					float rr = r1 * (1.0f - wt) + r2 * wt;
					float gg = g1 * (1.0f - wt) + g2 * wt;
					float bb = b1 * (1.0f - wt) + b2 * wt;
					float len = sqrtf(rr * rr + gg * gg + bb * bb);
					uint ri = (uint)(31.0f * rr / len);
					uint gi = (uint)(31.0f * gg / len);
					uint bi = (uint)(31.0f * bb / len);
					pPt[i].w = ri | (gi << 5) | (bi << 10);
					*/

				}
			}

			releaseMem(pLocationMap);
			releaseMem(pEnum);
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

	};
}
	static  pvc::PointStorageImpl theStorage;
	PointStorage  * PointStorage::GetInstatnce()
	{
		return  &theStorage;
	}



