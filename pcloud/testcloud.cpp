
#include <memory.h>
#include <math.h>
#include <stdlib.h> 
#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "pcloud.h" 
#include "..\lib\vec3d.h"
#include <time.h>

PCloudIn *gpTestCloud = 0;

static void GetSphere(float rad, float xc, float yc, float zc,int num)
{
	float prd = 1.0f / (float)RAND_MAX;
	for (int n = 0; n < num; n++) {
		float xf = prd * (float)rand() - 0.5f;
		float yf = prd * (float)rand() - 0.5f;
		float zf = prd * (float)rand() - 0.5f;
		float d = sqrtf(xf*xf + yf * yf + zf*zf);
		float di = (d > 0.000001f) ? ( rad / d) : 1.0f;
		gpTestCloud->SetPointValue(xf * di + xc, yf * di + yc, zf * di+ zc,0.0f);
	}
}


static void GetRandomPlane(float w, float h, float m,float x, float y, float z, int num)
{
	float prd = (1.0f / (float)RAND_MAX);
	float cx = x + w * 0.5f;
	float cy = y + h * 0.5f;
	float amp = w * 0.9f;
	float amp_max = w * 0.5f;
	for (int n = 0; n < num; n++)
	{
		float xf =  x  + prd *  w * (float)rand();
		float yf =  y  + prd *  h * (float)rand();
		float ddx = xf - cx;
		float ddy = yf - cy;
		float rr = sqrtf( ddx * ddx + ddy * ddy);
		float ssf = sinf(80.0f* rr / w);
		float zf = (rr == 0.0f) ? 1.0f : (ssf)/(rr);
		zf =  zf * amp;
		if (zf > amp_max) zf = amp_max;
		gpTestCloud->SetPointValue(xf, yf, zf,0.0f);
	}
}

static void GetPlane(float x, float y, float z, int numX, int numY, float stepX, float stepY, float stepZ) 
{
	for (int i = 0; i < numY; i++) 
	{
		for (int j = 0; j < numX; j++) 
		{
			//float prdx = (stepX / (float)RAND_MAX) * (float)rand();
			//float prdy = (stepY / (float)RAND_MAX) * (float)rand();
			float xf =  x + stepX * (float)i;
			float yf =  y + stepY * (float)j;
			float zf = z;// +8.0f * sinf((float)j * 10.0f*  3.1415f / (float)numX) *  sinf((float)i * 10.0f*  3.1415f / (float)numY);
			if ((i == numY-1) && (j == numX-1)) {
				gpTestCloud->SetPointValue(xf, yf, zf, 63.0f);
			}
			else {
				gpTestCloud->SetPointValue(xf, yf, zf, 64.0f);
			}
		}
	}
}

static void GetPlane3(pcv::vector3 A, pcv::vector3 B, pcv::vector3 C, int num)
{
	for (int i = 0; i < num; i++)
	{
		float r1 = (float)rand() / (float)(RAND_MAX);
	//	float r2 = (1 - r1)*((float)rand() / (float)(RAND_MAX));
		float r2 = ((float)rand() / (float)(RAND_MAX));
		pcv::vector3 T1 = B - A;
		pcv::vector3 T2 = C - A;
		pcv::vector3 U = A + (T1*r1) + (T2*r2);
		if ((r1 + r2) < 1.0f) {
			gpTestCloud->SetPointValue(U[0], U[1], U[2],0.0f);
		}
	}
}

static void GetAllPlane(int nx,int ny) 
{
	int stp[4]{ 0,-1,1,0 };
	float *pH = new float[nx*ny];
	memset(pH, 0, nx*ny * sizeof(float));

	int xs = nx/2, ys = ny/2;
	for (int i = 0; i < 5000; i++) {
		int v1 = rand() & 3;
		int v2 = rand() & 3;
		xs += stp[v1];
		ys += stp[v2];
		if ((xs < nx) && (xs > 0) && (ys < ny) && (ys > 0))
		{
			pH[xs + ys * nx] =+ 20.0f * (float)rand() / (float)(RAND_MAX);
		}
	}

	


	for (int i = 0; i < 5000; i++) 
	{
		int vx = (int)((float)nx*  (float)rand() / (float)(RAND_MAX));
		int vy = (int)((float)ny*  (float)rand() / (float)(RAND_MAX));
		if (vx > nx - 1) vx = nx - 1;
		if (vy > ny - 1) vy = ny - 1;
		pH[vx + vy * nx] = pH[vx + vy * nx] + 1.0f * (float)rand() / (float)(RAND_MAX);;
	}
	

	
	

	float dx = 10.0f;
	float dy = 10.0f;
	int cnt = 0;
	for (int y = 0; y < ny-1; y++)
	{
		for (int x = 0; x < nx-1; x++,cnt++)
		{
			int st = x + y * nx;
			float xf = (float)x * dx;
			float yf = (float)y * dy;
			float zfa = pH[st];
			float zfb = pH[st+1];
			float zfc = pH[st + nx];
			float zfd = pH[st + 1 +nx];
			pcv::vector3 A(xf, yf, zfa);
			pcv::vector3 B(xf + dx, yf, zfb);
			pcv::vector3 C(xf, yf +dy, zfc);
			pcv::vector3 D(xf+dx, yf + dy, zfd);
			GetPlane3(A, B, C, 1024);
			GetPlane3(D, B, C, 1024);
		}
	}	
	
	delete[]pH;

}

static void GetAllPlaneNew()
{
	int num = 1024 *1024*4 ;
	float *pTemp = new float[num *3];
	float *pt = pTemp;
	float  xs = 0.0f, ys = 0.0f,zs= 0.0f;
	float xMin = FLT_MAX, yMin = FLT_MAX, xMax = FLT_MIN, yMax = FLT_MIN;
	srand(time(NULL));
	for (int i = 0; i < num; i++) {
		float dx = ((float)rand() / (float)(RAND_MAX)) - 0.5f;
		float dy = ((float)rand() / (float)(RAND_MAX)) - 0.5f;
		xs += dx;
		ys += dy;
		if (xs > xMax) xMax = xs;
		if (ys > yMax) yMax = ys;
		if (xs < xMin) xMin = xs;
		if (ys < yMin) yMin = ys;
		*pt++ = xs;
		*pt++ = ys;
		*pt++ = zs;
		//gpTestCloud->SetPointValue(xs, ys, zs);
	}

	int zMapX = 1024*4,zMapY = 1024*4;
	float *pZMap = new float[zMapX*zMapY];
	memset(pZMap, 0, zMapX*zMapY* sizeof(float));

	float szx = (float)zMapX / (xMax - xMin);
	float szy = (float)zMapY / (yMax - yMin);
	pt = pTemp;
	for (int i = 0; i < num; i++,pt+=3) 
	{
		int xx = (int)( (pt[0] - xMin) * szx);
		int yy = (int)( (pt[1] - yMin) * szy);
		if ((xx >= 0) && (xx < zMapX) && (yy >= 0) && (yy < zMapY))
		{
			int t = xx + yy * zMapX;
			  pZMap[t]  += 0.1f;
		}
		//gpTestCloud->SetPointValue((float)xx,(float)yy, 2.0f);
	}

	for (int i = 0; i < zMapY; i++)
	{
		for (int j = 0; j < zMapX; j++) {
			
			float xr = (float)i + ((float)rand() / (float)(RAND_MAX)) - 0.5f;
			float yr = (float)j + ((float)rand() / (float)(RAND_MAX)) - 0.5f;
			gpTestCloud->SetPointValue(xr*0.3f, yr*0.3f, pZMap[j + i * zMapX],0.0f);
		}
	}

	delete[] pTemp;
	delete[] pZMap;
}





void* PCloudIn::InitTestCloud()
{
	if (!gpTestCloud) {
		return NULL;
	}
	gpTestCloud->OnStart();

	/*
	for (int x = 0; x < 2; x++) 
	{
		for (int y = 0; y < 2; y++) 
		{
			GetRandomPlane(100.0f, 100.0f, 0.1f,  x*100.0f, y*100.0f, 0.0f, 1024 * 1024);
		}
	}
	*/

	//GetRandomPlane(100.0f, 100.0f, 0.1f, 100.0f, 100.0f, 0.0f, 4096);
	GetSphere(100.0f, 0.0f, 0.0f, 0.0f, 1024 * 1024*4);
	
	// GetRandomPlane(100.0f, 100.0f, 0.1f, 0.0f,   0.0f,     0.0f,   1024 * 1024 * 8);
	//GetRandomPlane(100.0f, 100.0f, 0.1f, 0.0f,   100.0f,   0.0f,   1024 * 1024 * 8);
	//GetRandomPlane(100.0f, 100.0f, 0.1f, 100.0f, 0.0f,     0.0f,   1024 * 1024 * 8);
	//GetRandomPlane(100.0f, 100.0f, 0.1f, 100.0f, 100.0f,   0.0f,   1024 * 1024 * 8);

	//GetRandomPlane(100.0f, 100.0f, 0.1f, 100.0f, 200.0f, 0.0f, 1024 * 1024 * 8);
	//GetRandomPlane(100.0f, 100.0f, 0.1f, 0.0f, 200.0f, 0.0f, 1024 * 1024 );
	//GetRandomPlane(100.0f, 100.0f, 0.1f, 100.0f, 200.0f, 0.0f, 1024 * 1024);

	//GetPlane(0.0f, 0.0f, 1000.0f, 64,64, 10.0f, 10.0f, 0.0f);
	
	//GetPlane(0.0f, 0.0f, 0.0f, 1024, 1024, 0.9f, 0.9f, 0.0f);
	/*
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			GetSphere(10, 35.0*(float)x, 35.0*(float)y,  0.0f, 1024 * 1024);
		}
	}
	*/
	
	


	pcv::vector3 A(0.0,  0.0,  0.0);
	pcv::vector3 B(32.0,  20.0, 0.0);
	pcv::vector3 C(10.0, 0.0, 0.0);
	pcv::vector3 D(10.0, 10.0, 0.0);
	//pcv::vector3 D(10.0,  0.0, 5.0);
	//GetPlane3(A, B, C, 256 * 256);
	//GetPlane3(B, C, D, 1024 * 1024);
	//GetAllPlane(100, 100);
	//GetAllPlaneNew();

	//GetPlane(120.0f, 120.0f, 0.0f, 2048, 2048, 0.2f, 0.1f, 0.0f);
	//GetPlane(120.0f, 240.0f, 5.0f, 2048, 2048, 0.1f, 0.1f, 0.0f);

	gpTestCloud->OnDone();
	return NULL;
}
