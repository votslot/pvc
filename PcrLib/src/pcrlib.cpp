

#include <stdio.h>
#include "../pcrlib.h"
#include <string>
#include "cshader.h"
#include "OpenGL/wave-test.cs.glsl"

namespace pcrlib 
{

	class ThePcrLib :public IPcrLib
	{
		void runTest();
	};


	
	IPcrLib* IPcrLib::Init()
	{
		return  new ThePcrLib();
	}

	// Test
	void ThePcrLib::runTest()
	{
		const unsigned int bfz = 1024;
		int *pDA = new int[bfz];
		int *pDB = new int[bfz];
		int *pDC = new int[bfz];

		ICShader *pCsh = ICShader::GetNew();
		pCsh->initFromSource(cs_wave_test.c_str());
		ICBuffer *pBuffA = ICBuffer::GetNew();
		ICBuffer *pBuffB = ICBuffer::GetNew();
		ICBuffer *pBuffC = ICBuffer::GetNew();
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
				break;
			}
		}

		ICBuffer::release(&pBuffA);
		ICBuffer::release(&pBuffB);
		ICBuffer::release(&pBuffC);
		delete[] pDA;
		delete[] pDB;
		delete[] pDC;
	}
	
}

