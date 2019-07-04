

#include <stdio.h>
#include <iostream>
#include "../pcrlib.h"
#include "icompute.h"

#include "OpenGL/wave-test.cs.glsl"


namespace pcrlib 
{
	extern int InitGLBlit();

	class ThePcrLib :public IPcrLib
	{
		static const int sMaxW = 2048;
		static const int sMaxH = 2048;
		bool isInit = false;
		// buffres
		ICBuffer  *pbufferZMap = NULL;
		ICBuffer  *pbufferParams = NULL;
	public:
		void initInternal()
		{
			if (isInit) 
			{
				return;
			}
			pbufferZMap = createICBuffer();
			pbufferZMap->allocate(sMaxW*sMaxH * sizeof(int));

			// Write something
			int *pD = new int[sMaxW*sMaxH];
			for (int i = 0; i < sMaxW*sMaxH; i++) pD[i] = 0x00800000;  //AABBGGRR
			pbufferZMap->setData(pD, sMaxW*sMaxH * sizeof(int));
			delete[]pD;
			pD = NULL;

			InitGLBlit();
			isInit = true;
		}

		int render(const Camera &cam, int destWidth, int destHeight)
		{
			if (!isInit)
			{
				return 0;
			}
			pbufferZMap->blit(destWidth, destHeight);
			return 0;
		}

		void releaseInternal()
		{
			if(pbufferZMap) releaseICBuffer(&pbufferZMap);
			isInit = false;
		}

		int runTest();
	};

	static ThePcrLib libInstance;
	
	IPcrLib* IPcrLib::Init()
	{
		libInstance.initInternal();
		return  &libInstance;
	}
	
	// Test
	int ThePcrLib::runTest()
	{
        int ret = 0;
		const unsigned int bfz = 1024;
		int *pDA = new int[bfz];
		int *pDB = new int[bfz];
		int *pDC = new int[bfz];

		ICShader *pCsh = createICShader();
		pCsh->initFromSource(cs_wave_test.c_str());
		ICBuffer *pBuffA = createICBuffer();
		ICBuffer *pBuffB = createICBuffer();
		ICBuffer *pBuffC = createICBuffer();
		unsigned int dtsize = bfz * sizeof(unsigned int);
		pBuffA->allocate(dtsize);
		pBuffB->allocate(dtsize);
		pBuffC->allocate(dtsize);

		for (int i = 0; i < bfz; i++)
		{
			pDA[i] = i; pDB[i] = i; pDC[i] = 0;
		}

		pBuffA->setData(pDA, dtsize);
		pBuffB->setData(pDB, dtsize);
		pBuffC->setData(pDC, dtsize);

		pCsh->execute(32, 1, 1, { pBuffA,pBuffB,pBuffC });

		pBuffC->getData(dtsize, pDC);
		for (int i = 0; i < bfz; i++)
		{
			if (pDC[i] != pDA[i] + pDB[i])
			{
                ret = 1;
				break;
			}
		}

		releaseICShader(&pCsh);
		releaseICBuffer(&pBuffA);
		releaseICBuffer(&pBuffB);
		releaseICBuffer(&pBuffC);
		delete[] pDA;
		delete[] pDB;
		delete[] pDC;
        return ret;
	}
	
}

