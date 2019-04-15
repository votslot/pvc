
#ifndef _PCLOUD_H_
#define _PCLOUD_H_
#include<functional>

class PCloudIn 
{
  public:
	  virtual  void OnStart() {};
	  virtual  void OnDone() {};
	  virtual  void SetNumPoints( int num) {}
	  virtual  void SetPointValue(float x, float y, float z,float w) {}
	  virtual  void OnErr(const char *pMsg) {}

	  void ReadLasFile(const char *pPath);
	  void* InitTestCloud();
};


template<typename P>
struct partitionData 
{
	int numPoints;
	int first;
	float  minX;
	float  minY;
	float  minZ;
	float  maxX;
	float  maxY;
	float  maxZ;
};

void DoPartitionXYZW_Float(void *pData, unsigned int num, std::function<void(partitionData<float> *pD)> func);

#endif