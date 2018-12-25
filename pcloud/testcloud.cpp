
#include <memory.h>
#include <math.h>
#include <stdlib.h> 
#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "pcloud.h" 

PCloudIn *gpTestCloud = 0;

static void GetSphere(float rad, float xc, float yc, float zc,int num)
{
	float prd = 1.0f / (float)RAND_MAX;
	for (int n = 0; n < num; n++) {
		float xf = prd * (float)rand() - 0.5f;
		float yf = prd * (float)rand() - 0.5f;
		float zf = prd * (float)rand() - 0.5f;
		float d = 2.0f*sqrt(xf*xf + yf * yf + zf*zf);
		float di = (d > 0.000001f) ? ( rad / d) : 1.0f;
		gpTestCloud->SetPointValue(xf * di + xc, yf * di + yc, zf * di + zc);
	}
}

#if 0
static void GetPlane(float w, float h, float x, float y, float z, int num, CPoint *dst)
{
	float prd = (1.0f / (float)RAND_MAX);
	for (int n = 0; n < num; n++)
	{
		float xf = x  + prd *  (float)rand() -0.5f;
		float yf =  y + prd *  (float)rand() -0.5f;
		dst[n].x = xf * w ;
		dst[n].y = yf * h;
		dst[n].z = z ;
		dst[n].attr = 64.0f;
	}
}

CPoint *GetPoints(int &num)
{
	int numInPlane1 = 1024*1024 ;
	int numInPlane2 = 1024*1024;
	int numInSpere = 1024*1024*10 ;
	num = numInPlane1 + numInPlane2 + numInSpere*4;
	CPoint*pRet = new CPoint[num];
	memset(pRet, 0, sizeof(CPoint)*num);
	CPoint*pp = pRet;
	std::cout << "Start Point Generation" << std::endl;
	GetPlane(5.5f, 5.5f, 0.0f, 0.0f, 0.0f, numInPlane1, pp); pp += numInPlane1;
	GetPlane(0.5f, 0.3f, 0.3f, 0.3f, 0.5f, numInPlane2, pp); pp += numInPlane2;
	GetSphere(2.8f, 2.0f, 2.0f, 0.0f, numInSpere, pp); pp += numInSpere;
	GetSphere(2.8f, -2.0f, 2.0f, 0.0f, numInSpere, pp); pp += numInSpere;
	GetSphere(2.8f, 2.0f, -2.0f, 0.0f, numInSpere, pp); pp += numInSpere;
	GetSphere(2.8f, -2.0f, -2.0f, 0.0f, numInSpere, pp); pp += numInSpere;
	std::cout << "End Point Generation" << std::endl;
	return pRet;
}


#endif

void* PCloudIn::InitTestCloud(int numPoints)
{
	if (!gpTestCloud) {
		return NULL;
	}
	gpTestCloud->OnStart();
	gpTestCloud->SetNumPoints(numPoints);

	GetSphere(10.0f, -10.0f, -10.0f, 0.0f, numPoints / 4);
	GetSphere(10.0f, -10.0f,  10.0f, 0.0f, numPoints / 4);
	GetSphere(10.0f,  10.0f, -10.0f, 0.0f, numPoints / 4);
	GetSphere(10.0f,  10.0f,  10.0f, 0.0f, numPoints / 4);
	/*
	int numInSphere = numPoints / 16;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			GetSphere(10.0f, 10.0f*(float)x, 10.0f*(float)y,0.0, numInSphere);
		}
	}
	*/
	gpTestCloud->OnDone();
	return NULL;
}
