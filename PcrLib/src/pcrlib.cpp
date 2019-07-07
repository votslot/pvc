

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "../pcrlib.h"
#include "icompute.h"
#include "storage.h"

#include "OpenGL/wave-test.cs.glsl"
#include "OpenGL/shaders/render-points.cs.glsl"
#include "OpenGL/shaders/post-proc.cs.glsl"
#include "OpenGL/shaders/ginclude.h"

namespace pcrlib 
{
extern int InitGLBlit();

	
	class ThePcrLib :public IPcrLib
	{
	public:

		static const int sMaxW = 2048;
		static const int sMaxH = 2048;
		static PcrErrorHandler m_ErrFunc;
		bool isInit = false;
		PointStorage  * m_pst = NULL;
		GlobalParams m_Glob;
		// buffres
		ICBuffer  *m_bufferZMap = NULL;
		ICBuffer  *m_bufferParams = NULL;
		ICBuffer  *m_bufferZMapPost = NULL;
		ICBuffer  *m_bufferMatrView4x4  = NULL;
		ICBuffer  *m_bufferView2World = NULL;
		//shaders
		ICShader *m_csPointRender = NULL;
		ICShader *m_csCleanRGB  = NULL;
		ICShader *m_csPostProc  = NULL;

		ThePcrLib() 
		{
			m_ErrFunc = defErrFunc;
			setICErrorHandler(defErrFunc);
		}

		void initInternal()
		{
			if (isInit) 
			{
				return;
			}
			// init storage
			m_pst = PointStorage::GetInstatnce();
			m_pst->Init();
			// shaders
			m_csCleanRGB = createICShader();
			m_csCleanRGB->initFromSource(cs_clean.c_str());
			
			m_csPointRender = createICShader();
			m_csPointRender->initFromSource(cs_render_points.c_str());
			
			m_csPostProc = createICShader();;
			m_csPostProc->initFromSource(cs_postproc_w.c_str());

			// buffers
			m_bufferParams = createICBuffer();
			m_bufferParams->allocate(sizeof(GlobalParams));
		
			m_bufferZMap = createICBuffer();
			m_bufferZMap->allocate(sMaxW*sMaxH * sizeof(int));

			// Write something
			int *pD = new int[sMaxW*sMaxH];
			for (int i = 0; i < sMaxW*sMaxH; i++) pD[i] = 0x00000000;  //AABBGGRR
			m_bufferZMap->setData(pD, sMaxW*sMaxH * sizeof(int));
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
			m_Glob.screenX = (float)destWidth;
			m_Glob.screenY = (float)destHeight;
			/*
			pGlob.zNear = (float)pCam->m_zNear;
			pGlob.zFar = (float)pCam->m_zFar;
			pGlob.zRange = (float)(1 << 24); 
			pGlob.maxDimension = (float)pCam->m_MaxDimension;
			pGlob.wrkLoad = 64;
			pGlob.px = pCam->m_P[0];
			pGlob.py = pCam->m_P[1];
			pGlob.pz = pCam->m_P[2];
			pGlob.bbMinX = pst->GetXMin();
			pGlob.bbMaxX = pst->GetXMax();
			pGlob.bbMinY = pst->GetYMin();
			pGlob.bbMaxY = pst->GetYMax();
			pGlob.bbMinZ = pst->GetZMin();
			pGlob.bbMaxZ = pst->GetZMax();
			pGlob.scrMin = (pGlob.screenX < pGlob.screenY) ? pGlob.screenX : pGlob.screenY;
			*/
			m_bufferParams->setData((unsigned char*)(&m_Glob), sizeof(GlobalParams));

			// clean dst zMap buffer
			m_csCleanRGB->execute(sMaxW / 32, sMaxH / 32, 1, { m_bufferParams, m_bufferZMap });

			m_bufferZMap->blit(destWidth, destHeight);
			return 0;
		}

		void releaseInternal()
		{
			if (m_bufferZMap)			releaseICBuffer(&m_bufferZMap);
			if (m_bufferParams)			releaseICBuffer(&m_bufferParams);
			if (m_bufferZMapPost)		releaseICBuffer(&m_bufferZMapPost);
			if (m_bufferMatrView4x4)	releaseICBuffer(&m_bufferMatrView4x4);
			if (m_bufferView2World)		releaseICBuffer(&m_bufferView2World);

			if (m_csPointRender)  releaseICShader(&m_csPointRender);
			if (m_csCleanRGB)	  releaseICShader(&m_csCleanRGB);
			if (m_csPostProc)	  releaseICShader(&m_csPostProc);

			isInit = false;
		}


		static void defErrFunc(const char *pMessage)
		{
			std::cout << "PcrLibError: " << pMessage << std::endl;
			assert(false);
		}

		int runTest();
	};//class ThePcrLib


	static ThePcrLib libInstance;
	
	IPcrLib* IPcrLib::Init()
	{
		libInstance.initInternal();
		return  &libInstance;
	}

	PcrErrorHandler ThePcrLib::m_ErrFunc = NULL;
	PcrErrorHandler IPcrLib::setErrHandler(PcrErrorHandler errh)
	{
		ThePcrLib::m_ErrFunc = errh;
		setICErrorHandler(errh);
		return NULL;
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

