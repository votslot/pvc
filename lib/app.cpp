
#include <iostream>
#include <thread>
#include "../winui/ui.h"
#include "../pcloud/pcloud.h"
#include "camera.h"


class UIOutImpl :public UIOut
{
	void OnFileOpen(const char *pPath);
	void OnTestRun();
	void OnExit();
	void OnPaint();
};

class CloudImpl : public  PCloudIn
{
public:
	unsigned char *pMem;
	unsigned int numCurr;
	unsigned int numP;
	float minX, maxX, minY, maxY, minZ, maxZ;
	CloudImpl() :pMem(NULL), numP(0) {}
	void OnStart();
	void SetNumPoints(int num);
	void SetPointValue(float x, float y, float z);
	void OnDone();
	void OnErr(const char *pMsg);
	char* GetMemory(size_t sz);
};

static UIOutImpl theUIOutImpl;
static CloudImpl theCloudImpl;

//-------------------UI-----------------------------
void UIOutImpl::OnFileOpen(const char *pPath)
{
	std::cout << std::this_thread::get_id() << std::endl;
	theCloudImpl.ReadLasFile(pPath);
}

void UIOutImpl::OnTestRun()
{
	extern void SetPointData(void *pData, int num);
	int num = 1024 ;
	theCloudImpl.InitTestCloud(num);
}

void UIOutImpl::OnPaint()
{
}

void UIOutImpl::OnExit() {}

//-------------------PCloud-----------------------------
void CloudImpl::OnStart()
{
	minX = minY = minZ = FLT_MAX;
	maxX = maxY = maxZ = FLT_MIN;
	numCurr = 0;
}

void  CloudImpl::SetNumPoints(int num)
{
	pMem = new unsigned char[num*4*sizeof(float)];
	numP = num;
	std::cout << "NUM=" << numP << std::endl;
}

void  CloudImpl::SetPointValue(float x, float y, float z)
{
	if (numCurr < numP) {
		float *ptr = (float*)(pMem + 4 * numCurr * sizeof(float));
		ptr[0] = x;
		ptr[1] = y;
		ptr[2] = z;
		ptr[3] = 128.f;
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (z < minZ) minZ = z;
		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
		if (z > maxZ) maxZ = z;
		numCurr++;
	}
}

void CloudImpl::OnDone()
{
	extern void SetPointData(void *pData, int num);
	//float prd = 256.0f;
	float ddx = (maxX - minX);
	float ddy = (maxY - minY);
	float ddz = (maxZ - minZ);
	float maxD = (ddx > ddy) ? ddx : ddy;
	maxD = (maxD > ddz) ? maxD : ddz;

	float cx = (maxX + minX) *0.5f;
	float cy = (maxY + minY) *0.5f;
	float cz = (maxZ + minZ) *0.5f;

	float *ptr = (float*)(pMem);
	for (unsigned int i = 0; i < numP; i++) {
		ptr[3] =  255.0f * (ptr[2] - minZ) / ddz; // color
		ptr += 4;
	}


	DoPartitionXYZW_Float(pMem, numP);

	SetPointData(pMem, numP);
	Camera *pCam = Camera::GetCamera();
	pCam->SetPivotCamera(0.0f, 0.0f, maxD*2.0f, cx, cy, cz);
	pCam->m_zFar = maxD * 4.0f;
	pCam->m_MaxDimension = maxD;
	if (pMem != NULL) 
	{
		delete[] pMem;
	}
}

void CloudImpl::OnErr(const char *pMsg)
{
	theUIOutImpl.DispalyError(pMsg);
}

char* CloudImpl::GetMemory(size_t sz) 
{
	return new char[sz];
}

class InitBindings
{
public:
	InitBindings()
	{
		extern PCloudIn *gpTestCloud;
		extern PCloudIn *gpLasCloud;

		UIOut::SetInstance(&theUIOutImpl);
		gpLasCloud  = &theCloudImpl;
		gpTestCloud = &theCloudImpl;
	}
};
static InitBindings initB;