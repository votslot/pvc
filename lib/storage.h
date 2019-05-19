#ifndef _STORAGE_H_
#define  _STORAGE_H_

class SSBBuffer;

class PointStorage 
{
public:
	virtual void Init() = 0;
	virtual void Release() = 0;
	virtual void SetPoint(float x, float y, float z, float w) = 0;
	virtual void DoneAddPoints() = 0;
	virtual int getNumAvailableBuffers() = 0;
	virtual int GetNumBuffersInUse() = 0;
	virtual SSBBuffer * GetPointBuffer(int n) = 0;
	virtual SSBBuffer * GetPartitionBuffer(int n) = 0;
	virtual int GetNumPointsInBuffer(int n) = 0;
	virtual bool IsReady() = 0;
	virtual float GetXMin() = 0;
	virtual float GetYMin() = 0;
	virtual float GetZMin() = 0;
	virtual float GetXMax() = 0;
	virtual float GetYMax() = 0;
	virtual float GetZMax() = 0;
	virtual int GetZBits() = 0;
	static PointStorage  * GetInstatnce();
};



#endif