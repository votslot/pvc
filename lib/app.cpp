
#include <iostream>
#include <thread>
#include "../winui/ui.h"
#include "../pcloud/pcloud.h"
#include "camera.h"
#include "compute.h"


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

	unsigned int numCurr;
	unsigned int numP;
	float minX, maxX, minY, maxY, minZ, maxZ;
	CloudImpl() :numP(0) {}
	void OnStart();
	void SetNumPoints(int num);
	void SetPointValue(float x, float y, float z,float w);
	void OnDone();
	void OnErr(const char *pMsg);
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
	theCloudImpl.InitTestCloud();
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
	numP = num;
	std::cout << "NUM=" << numP << std::endl;
}

void  CloudImpl::SetPointValue(float x, float y, float z,float w)
{		
	if (x < minX) minX = x;
	if (y < minY) minY = y;
	if (z < minZ) minZ = z;
	if (x > maxX) maxX = x;
	if (y > maxY) maxY = y;
	if (z > maxZ) maxZ = z;
	Compute_AddPoint(x, y, z, w);
	numCurr++;
}

void CloudImpl::OnDone()
{
	float ddx = (maxX - minX);
	float ddy = (maxY - minY);
	float ddz = (maxZ - minZ);
	float maxD = (ddx > ddy) ? ddx : ddy;
	maxD = (maxD > ddz) ? maxD : ddz;
	float cx = (maxX + minX) *0.5f;
	float cy = (maxY + minY) *0.5f;
	float cz = (maxZ + minZ) *0.5f;

	Compute_DoneAddPoints();
	Camera *pCam = Camera::GetCamera();
	pCam->SetPivotCamera(0.0f, 0.0f, maxD*2.0f, cx, cy, cz);
	pCam->m_zFar = maxD * 4.0f;
	pCam->m_MaxDimension = maxD;
}

void CloudImpl::OnErr(const char *pMsg)
{
	theUIOutImpl.DispalyError(pMsg);
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