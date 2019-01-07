
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


static void GetPlane(float w, float h, float m,float x, float y, float z, int num)
{
	float prd = (1.0f / (float)RAND_MAX);
	for (int n = 0; n < num; n++)
	{
		float xf =  x  + prd *  w * (float)rand();
		float yf =  y  + prd *  h * (float)rand();
		float zf  = z  + prd * m * (float)rand();
		gpTestCloud->SetPointValue(xf, yf, zf);
	}
}





void* PCloudIn::InitTestCloud(int numPoints)
{
	if (!gpTestCloud) {
		return NULL;
	}
	gpTestCloud->OnStart();
	gpTestCloud->SetNumPoints(numPoints);

	GetPlane(100.0f, 100.0f, 10.0f,0.0f, 0.0f, 0.0f, numPoints);

	//GetSphere(10000.0f, 5.0f,   1000.0f, 200.0f, numPoints / 2);
	//GetSphere(10000.0f, 100.0f, 130.0f, 200.0f, numPoints / 2);
	//GetSphere(5.0f,  5.0f, 0.0f, 7.5f, numPoints/2);
	//GetSphere(10.0f, -10.0f,  10.0f, 0.0f, numPoints / 4);
	//GetSphere(10.0f,  10.0f, -10.0f, 0.0f, numPoints / 4);
	//GetSphere(10.0f,  10.0f,  10.0f, 0.0f, numPoints / 4);
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
