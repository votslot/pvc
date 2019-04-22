#include <iostream>
#include <thread>
#include <assert.h>
#include "GL\\glew.h"

#include "cpoints.h"
#include "cbuff.h"
#include "camera.h"
#include "storage.h"
#include "..\pcloud\pcloud.h"
#include "..\shaders\ginclude.h"


extern void WaveTest_Init();
extern void WaveTest_Run();


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
	PointStorage  * pst = PointStorage::GetInstatnce();
    // clean shader
	csCleanRGB.initFromSource(cs_clean.c_str());
	csCleanRGB.setBufferBinding(&bufferParams, 0);
	csCleanRGB.setBufferBinding(&bufferZMap, 1);

	// render points shader
	pst->Init();
	csPointRender.initFromSource(cs_render_points.c_str());
	csPointRender.setBufferBinding(&bufferParams,  0);
	csPointRender.setBufferBinding(&bufferDebug,   1);
	//for (int m = 0; m < theStorage.sMaxBuffs; m++)
	for (int m = 0; m < pst->getNumAvailableBuffers(); m++)
	{
		csPointRender.setBufferBinding(pst->GetPointBuffer(m), 2);
		csPointRender.setBufferBinding(pst->GetPartitionBuffer(m), 5);
	}
	csPointRender.setBufferBinding(&bufferZMap, 3);
	csPointRender.setBufferBinding(&bufferMatrView4x4, 4);

	//post process shader
	csPostProc.initFromSource(cs_postproc_w.c_str());
	csPostProc.setBufferBinding(&bufferParams, 0);
	csPostProc.setBufferBinding(&bufferZMap, 1);
	csPostProc.setBufferBinding(&bufferZMapPost, 2);
	csPostProc.setBufferBinding(&bufferView2World, 3);
	csPostProc.setBufferBinding(&bufferDebug, 4);

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
	PointStorage  * pst = PointStorage::GetInstatnce();
	Camera *pCam = Camera::GetCamera();
	pGlob.screenX   = (float)pCam->GetScreenX();
	pGlob.screenY   = (float)pCam->GetScreenY();
	pGlob.zNear     = (float)pCam->m_zNear;
	pGlob.zFar      = (float)pCam->m_zFar;
	pGlob.zScale = 16777215.0f / (pCam->m_zFar - pCam->m_zNear);
	pGlob.maxDimension = (float)pCam->m_MaxDimension;
	pGlob.wrkLoad = 64;
	pGlob.px = pCam->m_P[0];
	pGlob.py = pCam->m_P[1];
	pGlob.pz = pCam->m_P[2];
	pGlob.bbMinZ = pst->GetZMin();
	pGlob.bbMaxZ = pst->GetZMax();
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

	// Render points
	if (pst->IsReady()) 
	{
		glUseProgram(csPointRender.m_program);
		csPointRender.bindBuffer(&bufferParams);
		csPointRender.bindBuffer(&bufferDebug);
		csPointRender.bindBuffer(&bufferZMap);
		csPointRender.bindBuffer(&bufferMatrView4x4);

		for (int m = 0; m < pst->GetNumBuffersInUse(); m++) {
			csPointRender.bindBuffer(pst->GetPointBuffer(m));
			csPointRender.bindBuffer(pst->GetPartitionBuffer(m));
			GLuint num_groups_x = 1;
			GLuint num_groups_y = (pst->GetNumPointsInBuffer(m) / csPointRender.m_szx / pGlob.wrkLoad);
			glDispatchCompute(num_groups_x, num_groups_y, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			SSBBuffer::checkError();
		}
		glUseProgram(0);
		SSBBuffer::checkError();
	}

	// post proc
	{
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
		csPostProc.bindBuffer(&bufferDebug);
		glDispatchCompute(sMaxW / 32, sMaxH / 32, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}

	glUseProgram(0);

	// test 
#if 0
	if (theStorage.hasPoints)
	{
		csPointRender.bindBuffer(&bufferDebug);
		bufferDebug.getData(32*sizeof(float), pTest);
		for (int y = 0; y < 8; y++)  printf("%f ", pTest[y]);
		printf("\n");
	}
#endif

	if (gRunWaveTest) {
		WaveTest_Run();
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	n_call++;
}

void Compute_AddPoint(float x, float y, float z, float w)
{
	PointStorage::GetInstatnce()->SetPoint(x, y, z, w);
}

void Compute_DoneAddPoints()
{
	PointStorage::GetInstatnce()->DoneAddPoints();
}







