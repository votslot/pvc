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
//static float pCameraData[32];
static int pParams[32];
static int pParams2[32];
static int sNumOfPoints = 0;
static void *pPoints;
static float matrView4x4[16];

static const int sMaxW = 2048;
static const int sMaxH = 2048;

//static   SSBBuffer bufferCamera;
static   SSBBuffer bufferParams;
static   SSBBuffer bufferDebug;
static   TBOBuffer bufferTbo;
static   SSBBuffer bufferZMap;
static   SSBBuffer bufferMatrView4x4;

static CSShader csPointRender;
static CSShader csCleanRGB; 

int gHasPoints = 0; 
int gRunWaveTest = 0;

#include "..\shaders\test.cs.glsl"

GLuint ComputeInit(int sw,int sh)
{
    // clean shader
	csCleanRGB.initFromSource(cs_clean.c_str());
	csCleanRGB.setBufferBinding(&bufferZMap, 0);
	csCleanRGB.setBufferBinding(&bufferParams, 1);

	// render points shader
	csPointRender.initFromSource(cs_render_points.c_str());
	csPointRender.setBufferBinding(&bufferParams,  0);
	csPointRender.setBufferBinding(&bufferDebug,   1);
	csPointRender.setBufferBinding(&bufferTbo,     2);
	csPointRender.setBufferBinding(&bufferZMap,    3);
	csPointRender.setBufferBinding(&bufferMatrView4x4, 4);

	pTest = new float[1024*1024*4];
	for (int i = 0; i < 1024*1024*4; i++) pTest[i] = 0.0;

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//bufferCamera.init();
	bufferParams.init();
	//
	bufferDebug.init();
	bufferTbo.init();

	//
	bufferZMap.init();
	bufferZMap.allocate(sMaxW*sMaxH *sizeof(int));
	//
	bufferMatrView4x4.init();
	bufferMatrView4x4.allocate(16 * sizeof(float));
	
	//Set params
	pParams[0] = 512;
	pParams[1] = 512;
	pParams[2] = sMaxW;
	pParams[3] = sMaxH;
	bufferParams.setData(pParams, 32 * sizeof(int));

	//Set debug data
	bufferDebug.setData(pTest, 1024 * 1024 * 4 * sizeof(float));
	if (gRunWaveTest) {
		WaveTest_Init();
	}
	return 0;
}

GLuint GetSrcBuff() 
{
	return  bufferZMap.gb;
}
GLuint GetParamsBuff() 
{
	return bufferParams.gb;
}

void ComputeRun(int sw, int sh)
{
	static int n_call = 0;
	// params
	pParams[0] = sw;
	pParams[1] = sh;
	bufferParams.setData((unsigned char*)pParams, 32 * sizeof(int));
	// camera
	Camera::GetCamera()->ConvertTo4x4(matrView4x4);
	bufferMatrView4x4.setData(matrView4x4, 16 * sizeof(float));
	//Camera::GetCamera()->Print4x4(matrView4x4);

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
		glUseProgram(0);
		//GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		//int ret = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
		SSBBuffer::checkError();
		//std::cout << std::this_thread::get_id() << std::endl;
	}

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
		/*1
		CPoint *pp = (CPoint*)pTest;
		for (int y = 0; y < 1024; y++)
		{
			for (int x = 0; x < 1024; x++,pp++)
			{
				int xx = (int)pp->z;
				int yy = (int)pp->attr;
				if( ((xx != x) || (yy != y) ) &&( xx == -1234))
				{
					printf("Err x = %d y = %d  ( %d %d)\n", x, y, xx, yy);

				}

				for (int i = 0; i < 100; i++)  pp->z += 1.0f;
				
			}
		}
		*/
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





