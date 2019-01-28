
#ifndef _PCLOUD_H_
#define _PCLOUD_H_

class PCloudIn 
{
  public:
	  virtual  void OnStart() {};
	  virtual  void OnDone() {};
	  virtual  void SetNumPoints( int num) {}
	  virtual  void SetPointValue(float x, float y, float z) {}
	  virtual  void OnErr(const char *pMsg) {}

	  void ReadLasFile(const char *pPath);
	  void* InitTestCloud();
};

void DoPartitionXYZW_Float(void *pData, unsigned int num);

#endif