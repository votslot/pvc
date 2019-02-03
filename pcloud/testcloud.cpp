
#include <memory.h>
#include <math.h>
#include <stdlib.h> 
#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "pcloud.h" 
#include "..\lib\vec3d.h"

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
		gpTestCloud->SetPointValue(xf * di + xc, yf * di + yc, zf * di*5.0f + zc);
	}
}


static void GetRandomPlane(float w, float h, float m,float x, float y, float z, int num)
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

static void GetPlane(float x, float y, float z, int numX, int numY, float stepX, float stepY, float stepZ) 
{
	for (int i = 0; i < numY; i++) 
	{
		for (int j = 0; j < numX; j++) 
		{
			float prdx = (stepX / (float)RAND_MAX) * (float)rand();
			float prdy = (stepY / (float)RAND_MAX) * (float)rand();
			float xf = prdx + x + stepX * (float)i;
			float yf = prdy + y + stepY * (float)j;
			float zf = z+ 8.0f * sinf((float)j * 10.0f*  3.1415f/(float)numX) *  sinf((float)i * 10.0f*  3.1415f / (float)numY);
			gpTestCloud->SetPointValue(xf, yf, zf);
		}
	}
}

static void GetPlane2(pcv::vector3 A, pcv::vector3 B, pcv::vector3 C, pcv::vector3 D)
{

}





void* PCloudIn::InitTestCloud()
{
	if (!gpTestCloud) {
		return NULL;
	}
	gpTestCloud->OnStart();

	
	GetPlane(0.0f, 0.0f, 0.0f, 256, 256, 0.9f, 0.9f, 0.0f);

	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			GetSphere(10, 35.0*(float)x, 35.0*(float)y,  0.0f, 1024 * 1024);
		}
	}


	//GetPlane(120.0f, 120.0f, 0.0f, 2048, 2048, 0.2f, 0.1f, 0.0f);
	//GetPlane(120.0f, 240.0f, 5.0f, 2048, 2048, 0.1f, 0.1f, 0.0f);

	gpTestCloud->OnDone();
	return NULL;
}
