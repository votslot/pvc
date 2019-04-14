

#include "GL\\glew.h"
#include "cbuff.h"

#include "..\shaders\wave-test.cs.glsl"

static   SSBBuffer bufferWaveTest;
static   SSBBuffer bufferAtom;
static   SSBBuffer bufferMatr44;
static CSShader csWaveTest;

static const int sAtomSz = 1024;
static float atoms[sAtomSz];

static const int sValuesSz = 1024;
static float values[sValuesSz];
static float mData[16] = 
{
	1.0, 2.0, 0.0, 0.0,
	1.0, 3.0, 0.0, 0.0,
	1.0, 4.0, 0.0, 0.0,
	1.0, 5.0, 0.0, 10.0
};

void WaveTest_Init()
{
	csWaveTest.initFromSource(cs_wave_test.c_str());
	csWaveTest.setBufferBinding(&bufferWaveTest, 0);
	csWaveTest.setBufferBinding(&bufferMatr44, 1);
	bufferWaveTest.init();
	bufferMatr44.init();
	bufferWaveTest.allocate(1024 * sizeof(int));
	bufferMatr44.allocate(16 * sizeof(float));
}

void WaveTest_Run() 
{
	bufferMatr44.setData((unsigned char*)mData, 16 * sizeof(float));
	glUseProgram(csWaveTest.m_program);
	csWaveTest.bindBuffer(&bufferWaveTest);
	csWaveTest.bindBuffer(&bufferMatr44);
	glDispatchCompute(64 / csWaveTest.m_szx, 1, 1);
	glUseProgram(0);
	float *pTestRes = (float*)values;
	bufferWaveTest.getData(sValuesSz * sizeof(float), pTestRes);
	int* pFound = NULL;
	//for (int m = 0; m < 4; m++)  printf("%f ", pTestRes[m]);
	//printf("\n");

	for (int m = 0; m < 4; m++)  printf("%f ", pTestRes[m +4]);
	printf("\n");


}