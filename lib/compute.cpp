#include <iostream>
#include <thread>
#include "GL\\glew.h"

#include "cpoints.h"
#include "cbuff.h"
#include "camera.h"

extern GLuint LoadShader(GLenum type, const GLchar *shaderSrc);
extern void WaveTest_Init();
extern void WaveTest_Run();

float *pTest    = NULL;
static int pParams[32];
static int sNumOfPoints = 0;
static void *pPoints;
static float clutData[256 * 4];
static float matrView4x4[16];

static const int sMaxW = 2048;
static const int sMaxH = 2048;

static   SSBBuffer bufferParams;
static   SSBBuffer bufferDebug;
static   TBOBuffer bufferTbo;
static   SSBBuffer bufferZMap;
static   SSBBuffer bufferZMapPost;
static   SSBBuffer bufferMatrView4x4;
static   SSBBuffer bufferClut;

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
	csPointRender.initFromSource(cs_render_points.c_str());
	csPointRender.setBufferBinding(&bufferParams,  0);
	csPointRender.setBufferBinding(&bufferDebug,   1);
	csPointRender.setBufferBinding(&bufferTbo,     2);
	csPointRender.setBufferBinding(&bufferZMap,    3);
	csPointRender.setBufferBinding(&bufferMatrView4x4, 4);

	//post process shader
	csPostProc.initFromSource(cs_postproc_w.c_str());
	csPostProc.setBufferBinding(&bufferParams, 0);
	csPostProc.setBufferBinding(&bufferZMap, 1);
	csPostProc.setBufferBinding(&bufferZMapPost, 2);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	bufferParams.init();
	//
	bufferDebug.init();
	bufferTbo.init();

	//
	bufferZMap.init();
	bufferZMap.allocate(sMaxW*sMaxH *sizeof(int));
	bufferZMapPost.init();
	bufferZMapPost.allocate(sMaxW*sMaxH * sizeof(int));
	//
	bufferMatrView4x4.init();
	bufferMatrView4x4.allocate(16 * sizeof(float));
	
	//Set params
	pParams[0] = 512;
	pParams[1] = 512;
	pParams[2] = sMaxW;
	pParams[3] = sMaxH;
	bufferParams.setData(pParams, 32 * sizeof(int));

	//clut
	for (int i = 0; i < 1024; i+=4) 
	{
		int nn = i ;
		clutData[nn+0] = (float)nn / 1024.0f;
		clutData[nn+1] = (float)nn / 1024.0f;
		clutData[nn+2] = (float)nn / 1024.0f;
		clutData[nn+3] = (float)nn / 1024.0f;
	}
	clutData[0] = 0.0f; clutData[1] = 0.5f; clutData[2] = 0.0f;
	clutData[4] = 0.0f; clutData[5] = 1.0f; clutData[6] = 0.0f;
	clutData[8] = 0.0f; clutData[9] = 0.0f; clutData[10] = 1.0f;
	bufferClut.init();
	bufferClut.setData(clutData, 4 * 256 * sizeof(float));

	//Set debug data
	pTest = new float[1024 * 1024 * 4];
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
	// params
	Camera *pCam = Camera::GetCamera();
	pParams[0] = pCam->GetScreenX();
	pParams[1] = pCam->GetScreenY();
	bufferParams.setData((unsigned char*)pParams, 32 * sizeof(int));
	// camera
	pCam->ConvertTo4x4(matrView4x4);
	bufferMatrView4x4.setData(matrView4x4, 16 * sizeof(float));

	// clean dst zMap buffer
	glUseProgram(csCleanRGB.m_program);
	csCleanRGB.bindBuffer(&bufferZMap);
	csCleanRGB.bindBuffer(&bufferParams);
	glDispatchCompute(sMaxW /32, sMaxH/32, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	if (gHasPoints) {
	  // Render points
		glUseProgram(csPointRender.m_program);
		csPointRender.bindBuffer(&bufferParams);
		csPointRender.bindBuffer(&bufferDebug);
		csPointRender.bindBuffer(&bufferTbo);
		csPointRender.bindBuffer(&bufferZMap);
		csPointRender.bindBuffer(&bufferMatrView4x4);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		GLuint num_groups_x = sNumOfPoints / 32 / 32;  // max 65535
		GLuint num_groups_y = 1;
		glDispatchCompute(num_groups_x, num_groups_y, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		SSBBuffer::checkError();

	
		SSBBuffer::checkError();

		glUseProgram(0);
		//GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		//int ret = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
		SSBBuffer::checkError();
		//std::cout << std::this_thread::get_id() << std::endl;
	}

	// post proc
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(csPostProc.m_program);

	for (int m = 0; m < 1; m++)
	{
		csPostProc.setBufferBinding(&bufferZMap, 1);
		csPostProc.setBufferBinding(&bufferZMapPost, 2);
		csPostProc.bindBuffer(&bufferParams);
		csPostProc.bindBuffer(&bufferZMap);
		csPostProc.bindBuffer(&bufferZMapPost);
		glDispatchCompute(sMaxW / 32, sMaxH / 32, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		/*
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		csPostProc.setBufferBinding(&bufferZMap, 2);
		csPostProc.setBufferBinding(&bufferZMapPost, 1);
		csPostProc.bindBuffer(&bufferParams);
		csPostProc.bindBuffer(&bufferZMapPost);
		csPostProc.bindBuffer(&bufferZMap);
		glDispatchCompute(sMaxW / 32, sMaxH / 32, 1);
		*/
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	glUseProgram(0);


	if (gRunWaveTest) {
		WaveTest_Run();
	}

	// test 
	if ( 0)
	{
		csPointRender.bindBuffer(&bufferDebug);
		bufferDebug.getData(32*sizeof(float), pTest);
		for (int y = 0; y < 4; y++) {
			printf("%f\n", pTest[y]);
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	n_call++;
}

void SetPointData(void *pData, int num)
{
	sNumOfPoints = num;
	bufferTbo.setData(pData, num* sizeof(CPoint));
	gHasPoints = 1;
}





