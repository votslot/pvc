#ifndef _STORAGE_H_
#define  _STORAGE_H_

namespace pcrlib
{
	class ICBuffer;


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

	class PointStorage
	{
	public:
		virtual void Init() = 0;
		virtual void Release() = 0;
		virtual void SetPoint(float x, float y, float z, float w) = 0;
		virtual void DoneAddPoints() = 0;
		virtual int getNumAvailableBuffers() = 0;
		virtual int GetNumBuffersInUse() = 0;
		virtual ICBuffer * GetPointBuffer(int n) = 0;
		virtual ICBuffer * GetPartitionBuffer(int n) = 0;
		virtual int GetNumPointsInBuffer(int n) = 0;
		virtual bool IsReady() = 0;
		virtual float GetXMin() = 0;
		virtual float GetYMin() = 0;
		virtual float GetZMin() = 0;
		virtual float GetXMax() = 0;
		virtual float GetYMax() = 0;
		virtual float GetZMax() = 0;
		static PointStorage  * GetInstatnce();
	};
}



#endif