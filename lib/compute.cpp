#include <iostream>
#include <thread>
#include <assert.h>
#include "GL\\glew.h"

#include "cpoints.h"
#include "cbuff.h"
#include "camera.h"
#include "..\pcloud\pcloud.h"
#include "..\shaders\ginclude.h"


extern void WaveTest_Init();
extern void WaveTest_Run();


class PointStorage 
{
	struct pointp4
	{
		float  x, y, z, w;
	};

public:
	static const int sMaxBuffs = 32;
	static const int sMaxAllocSize = 1024 * 1024 * 128;
	int maxPointsInBuff = 0;
	int numPointsInBuff[sMaxBuffs];
	int numPartitionsInBuff[sMaxBuffs];
	SSBBuffer bufferPoints[sMaxBuffs];
	SSBBuffer bufferPartition[sMaxBuffs];
	float bbZMin = FLT_MAX;
	float bbZMax = FLT_MIN;

	int numInUse;
	bool hasPoints;

	char *pTemp = NULL;
	int numPointsInTemp = 0;

	Partition *pPartitions = NULL;
	int numPartitions = 0;

	int sizeInTemp;
	unsigned int maxBuffSz;

	void Init() 
	{
	
		numInUse = 0;
		sizeInTemp = 0;
		numPartitions = 0;
		hasPoints = false;
		unsigned int maxSSB = SSBBuffer::getMaxSizeInBytes();
		maxBuffSz = (sMaxAllocSize < maxSSB) ? sMaxAllocSize : maxSSB;
		maxPointsInBuff = maxBuffSz / (4 * sizeof(float));
		pTemp = new char[maxBuffSz];
		pPartitions = new Partition[maxPointsInBuff];

		for (int i = 0; i < sMaxBuffs; i++) 
		{
			numPointsInBuff[i] = 0;
			numPartitionsInBuff[i] = 0;
			bufferPoints[i].init();
			bufferPartition[i].init();
		}
		bbZMin = FLT_MAX;
		bbZMax = FLT_MIN;
	}

	void SetPoint(float x, float y, float z, float w) 
	{
		int ptSize = sizeof(float) * 4;
		if ((sizeInTemp ) >= maxBuffSz)
		{
			AddNewBuffer();
		}
		float *pDest = (float*)pTemp;
		pDest += numPointsInTemp * 4;
		pDest[0] = x;
		pDest[1] = y;
		pDest[2] = z;
		pDest[3] = w;
		if (z < bbZMin) bbZMin = z;
		if (z > bbZMax) bbZMax = z;
		numPointsInTemp++;
		sizeInTemp += ptSize;
	}

	void DoneAddPoits() 
	{
		if (sizeInTemp > 0) 
		{
			AddNewBuffer();
		}
		hasPoints = true;
	}
	
	void AddNewBuffer() 
	{
		assert(numInUse < sMaxBuffs);
		std::function<void(partitionData<float> *pD)> OnDonePartition = [=](partitionData<float> *pDt)
		{ 
			float dx = pDt->maxX - pDt->minX;
			float dy = pDt->maxY - pDt->minY;
			float dz = pDt->maxZ - pDt->minX;
			float dMax = (dx > dy) ? dx : dy;
			dMax = (dMax > dz) ? dMax : dz;

			pPartitions[numPartitions].cx = (pDt->maxX + pDt->minX) *0.5f;
			pPartitions[numPartitions].cy = (pDt->maxY + pDt->minY) *0.5f;
			pPartitions[numPartitions].cz = (pDt->maxZ + pDt->minZ) *0.5f;
			pPartitions[numPartitions].sz = dMax;
			pPartitions[numPartitions].ndx = numPartitions;
			if ((numPartitions & 31) == 0) std::cout << ".";
			numPartitions++;

			// shuffle points
			pointp4 *pPt = (pointp4*)pTemp;
			if (pDt->numPoints == 4096) 
			{
				for (int n = 0; n < 4096; n++) 
				{
					int k1 = pDt->first + (rand() & 4095);
					int k2 = pDt->first + (rand() & 4095);
					pointp4 ptTemp = pPt[k1];
					pPt[k1] = pPt[k2];
					pPt[k2] = ptTemp;
				}
			}
		};

		DoPartitionXYZW_Float(pTemp, numPointsInTemp, OnDonePartition);
		std::cout << "done" << std::endl;

		bufferPoints[numInUse].setData(pTemp, sizeInTemp);
		bufferPartition[numInUse].setData(pPartitions, numPartitions*sizeof(Partition));
		numPointsInBuff[numInUse] = numPointsInTemp;
		numPartitionsInBuff[numInUse] = numPartitions;
		numInUse++;

		sizeInTemp = 0;
		numPointsInTemp = 0;
		numPartitions = 0;
	}

	void Release()
	{
	
	}
};

static  PointStorage theStorage;

void Compute_AddPoint(float x, float y, float z, float w)
{
	theStorage.SetPoint(x, y, z, w);
}

void Compute_DoneAddPoits()
{
	theStorage.DoneAddPoits();
}


float *pTest    = NULL;
static int sNumOfPoints = 0;
static void *pPoints;
static float clutData[256 * 4];
//static float matrView4x4[16];

static const int sMaxW = 2048;
static const int sMaxH = 2048;
GlobalParams pGlob;


static   SSBBuffer bufferParams;
static   SSBBuffer bufferDebug;
static   SSBBuffer bufferZMap;
static   SSBBuffer bufferZMapPost;
static   SSBBuffer bufferMatrView4x4;
static   SSBBuffer bufferClut;
static   SSBBuffer bufferView2World;

static CSShader csPointRender;
static CSShader csCleanRGB;
static CSShader csPostProc;

int gHasPoints = 0; 
int gRunWaveTest = 0;

#include "..\shaders\test.cs.glsl"
#include "..\shaders\post-proc.cs.glsl"

GLuint ComputeInit(int sw,int sh)
{
    // clean shader
	csCleanRGB.initFromSource(cs_clean.c_str());
	csCleanRGB.setBufferBinding(&bufferParams, 0);
	csCleanRGB.setBufferBinding(&bufferZMap, 1);

	// render points shader
	theStorage.Init();
	csPointRender.initFromSource(cs_render_points.c_str());
	csPointRender.setBufferBinding(&bufferParams,  0);
	csPointRender.setBufferBinding(&bufferDebug,   1);
	for (int m = 0; m < theStorage.sMaxBuffs; m++)
	{
		csPointRender.setBufferBinding(&theStorage.bufferPoints[m], 2);
		csPointRender.setBufferBinding(&theStorage.bufferPartition[m], 5);
	}
	csPointRender.setBufferBinding(&bufferZMap, 3);
	csPointRender.setBufferBinding(&bufferMatrView4x4, 4);

	//post process shader
	csPostProc.initFromSource(cs_postproc_w.c_str());
	csPostProc.setBufferBinding(&bufferParams, 0);
	csPostProc.setBufferBinding(&bufferZMap, 1);
	csPostProc.setBufferBinding(&bufferZMapPost, 2);
	csPostProc.setBufferBinding(&bufferView2World, 3);

	bufferParams.init();
	bufferView2World.init();
	//
	bufferZMap.init();
	bufferZMap.allocate(sMaxW*sMaxH *sizeof(int));
	bufferZMapPost.init();
	bufferZMapPost.allocate(sMaxW*sMaxH * sizeof(int));
	//
	bufferMatrView4x4.init();
	bufferMatrView4x4.allocate(16 * sizeof(float));
	bufferView2World.init();
	bufferView2World.allocate(16 * sizeof(float));
	bufferParams.setData(&pGlob, sizeof(GlobalParams));

	//clut
	for (int i = 0; i < 1024; i+=4) 
	{
		int nn = i ;
		clutData[nn + 0] =  (float)nn / 1024.0f;
		clutData[nn + 1] = (float)(nn) / 1024.0f;
		clutData[nn + 2] = (float)(nn) / 1024.0f;
		clutData[nn+3] = (float)nn / 1024.0f;
	}
	clutData[4] = 0.0f; clutData[5] = 1.0f; clutData[6] = 0.0f;
	clutData[8] = 1.0f; clutData[9] = 0.0f; clutData[10] = 0.0f;
	clutData[12] = 0.0f; clutData[13] = 0.0f; clutData[14] = 1.0f;
	clutData[16] = 1.0f; clutData[17] = 1.0f; clutData[18] = 0.0f;
	clutData[20] = 0.5f; clutData[21] = 0.0f; clutData[22] = 0.0f;
	clutData[24] = 0.0f; clutData[25] = 0.5f; clutData[26] = 0.0f;
	clutData[28] = 0.5f; clutData[29] = 0.5f; clutData[30] = 0.0f;
	clutData[32] = 0.9f; clutData[33] = 0.9f; clutData[34] = 0.9f;
	bufferClut.init();
	void *pd = bufferClut.allocateVram(4 * 256 * sizeof(float));
	memcpy(pd, clutData, 4 * 256 * sizeof(float));
	//bufferClut.setData(clutData, 4 * 256 * sizeof(float));

	//Set debug data
	bufferDebug.init();
	pTest = new float[1024 * 1024 *4];
	for (int i = 0; i < 1024 * 1024 * 4; i++) pTest[i] = 0.0;
	bufferDebug.setData(pTest, 1024 * 1024 * 4 * sizeof(float));

	if (gRunWaveTest) {
		WaveTest_Init();
	}
	return 0;
}

GLuint GetSrcBuff() 
{
	//return  bufferZMap.gb;
	return  bufferZMapPost.gb;
}
GLuint GetParamsBuff() 
{
	return bufferParams.gb;
}

GLuint GetClutData()
{
	return bufferClut.gb;
}

void ComputeRun(int sw__, int sh__)
{
	static int n_call = 0;
	Camera *pCam = Camera::GetCamera();
	pGlob.screenX   = (float)pCam->GetScreenX();
	pGlob.screenY   = (float)pCam->GetScreenY();
	pGlob.zNear     = (float)pCam->m_zNear;
	pGlob.zFar      = (float)pCam->m_zFar;
	pGlob.zScale = 16777215.0 / (pCam->m_zFar - pCam->m_zNear);
	pGlob.maxDimension = (float)pCam->m_MaxDimension;
	pGlob.wrkLoad = 64;
	pGlob.px = pCam->m_P[0];
	pGlob.py = pCam->m_P[1];
	pGlob.pz = pCam->m_P[2];
	pGlob.bbMinZ = theStorage.bbZMin;
	pGlob.bbMaxZ = theStorage.bbZMax;
	pGlob.scrMin = (pGlob.screenX < pGlob.screenY) ? pGlob.screenX : pGlob.screenY;
	bufferParams.setData((unsigned char*)(&pGlob), sizeof(GlobalParams));
	// camera
	float matrView4x4[16];
	pCam->GetProjectionMat4x4(pGlob.screenX, pGlob.screenY, pGlob.zNear, pGlob.zFar, matrView4x4);
	bufferMatrView4x4.setData(matrView4x4, 16 * sizeof(float));
	
	// clean dst zMap buffer
	glUseProgram(csCleanRGB.m_program);
	csCleanRGB.bindBuffer(&bufferZMap);
	csCleanRGB.bindBuffer(&bufferParams);
	glDispatchCompute(sMaxW /32, sMaxH/32, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	if (theStorage.hasPoints) 
	{
	    // Render points
		glUseProgram(csPointRender.m_program);
		csPointRender.bindBuffer(&bufferParams);
		csPointRender.bindBuffer(&bufferDebug);
		csPointRender.bindBuffer(&bufferZMap);
		csPointRender.bindBuffer(&bufferMatrView4x4);

		for (int m = 0; m < theStorage.numInUse; m++) {
			csPointRender.bindBuffer(&theStorage.bufferPoints[m]);
			csPointRender.bindBuffer(&theStorage.bufferPartition[m]);
			//GLuint num_groups_x = theStorage.numPointsInBuff[m] / csPointRender.m_szx/ pGlob->wrkLoad;  // max 65535
			//GLuint num_groups_y = 1;
			GLuint num_groups_x = 1;
			GLuint num_groups_y = (theStorage.numPointsInBuff[m] / csPointRender.m_szx / pGlob.wrkLoad);
			glDispatchCompute(num_groups_x, num_groups_y, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			SSBBuffer::checkError();
		}
		glUseProgram(0);
		//GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		//int ret = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
		SSBBuffer::checkError();
		//std::cout << std::this_thread::get_id() << std::endl;
	}

	// post proc
	float Vew2World4x4[16];
	pCam->GetVew2World4x4(Vew2World4x4);
	bufferView2World.setData(Vew2World4x4, 16 * sizeof(float));
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(csPostProc.m_program);
	csPostProc.setBufferBinding(&bufferZMap, 1);
	csPostProc.setBufferBinding(&bufferZMapPost, 2);
	csPostProc.bindBuffer(&bufferParams);
	csPostProc.bindBuffer(&bufferZMap);
	csPostProc.bindBuffer(&bufferZMapPost);
	csPostProc.bindBuffer(&bufferView2World);
	glDispatchCompute(sMaxW / 32, sMaxH / 32, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUseProgram(0);


	if (gRunWaveTest) {
		WaveTest_Run();
	}

	// test 
#if 0
	if (theStorage.hasPoints)
	{
		csPointRender.bindBuffer(&bufferDebug);
		bufferDebug.getData(32*sizeof(float), pTest);
		for (int y = 0; y < 4; y++)  printf("%f", pTest[y]);
		printf("\n");
	}
#endif

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	n_call++;
}

#if 0
void SetPointData(void *pData, int num)
{
	sNumOfPoints = num;
	theStorage.bufferPoints[0].setData(pData, num * sizeof(CPoint));
	theStorage.numPointsInBuff[0] = num;
	theStorage.numInUse = 1;
	gHasPoints = 1;
	SSBBuffer::checkError();
}
#endif






