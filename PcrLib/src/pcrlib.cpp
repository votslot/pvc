

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "../pcrlib.h"
#include "icompute.h"
#include "storage.h"
#include "matrix-utils.h"

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
		ICBuffer  *m_bufferDebug = NULL;
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

			m_bufferZMapPost = createICBuffer();
			m_bufferZMapPost->allocate(sMaxW*sMaxH * sizeof(int));

			m_bufferMatrView4x4 = createICBuffer();
			m_bufferMatrView4x4->allocate(16 * sizeof(float));

			m_bufferView2World = createICBuffer();
			m_bufferView2World->allocate(16 * sizeof(float));

			m_bufferDebug = createICBuffer();
			m_bufferDebug->allocate(1024 * sizeof(float));

			// Write something
			int *pD = new int[sMaxW*sMaxH];
			for (int i = 0; i < sMaxW*sMaxH; i++) pD[i] = 0x00000000;  //AABBGGRR
			m_bufferZMap->setData(pD, sMaxW*sMaxH * sizeof(int));
			delete[]pD;
			pD = NULL;

            InitGLBlit();
			isInit = true;
		}

		void startAddPoints()
		{
			
		}

		int addPoint(float x, float y, float z, float w) 
		{
			m_pst->SetPoint(x, y, z, w);
			return 0;
		}

		void doneAddPoints()
		{
			m_pst->DoneAddPoints();
		}

		BoundBox getBoundBox()
		{
			BoundBox ret;
			ret.xMax = m_pst->GetXMax();
			ret.xMin = m_pst->GetXMin();
			ret.yMax = m_pst->GetYMax();
			ret.yMin = m_pst->GetYMin();
			ret.zMax = m_pst->GetZMax();
			ret.zMin = m_pst->GetZMin();
			return ret;
		}

		int render(const Camera &cam, int destWidth, int destHeight)
		{
			if (!isInit)
			{
				return 0;
			}
			m_Glob.screenX = (float)destWidth;
			m_Glob.screenY = (float)destHeight;
			m_Glob.zNear = (float)cam.zNear;
			m_Glob.zFar = (float)cam.zFar;
			m_Glob.zRange = (float)(1 << 24); //16777215.0f;// / (pCam->m_zFar - pCam->m_zNear);
			m_Glob.maxDimension = 0.0f;// (float)pCam->m_MaxDimension;
			m_Glob.wrkLoad = 64;
			m_Glob.px = cam.pos[0];
			m_Glob.py = cam.pos[1];
			m_Glob.pz = cam.pos[2];
			m_Glob.bbMinX = m_pst->GetXMin();
			m_Glob.bbMaxX = m_pst->GetXMax();
			m_Glob.bbMinY = m_pst->GetYMin();
			m_Glob.bbMaxY = m_pst->GetYMax();
			m_Glob.bbMinZ = m_pst->GetZMin();
			m_Glob.bbMaxZ = m_pst->GetZMax();
			m_Glob.scrMin = (m_Glob.screenX < m_Glob.screenY) ? m_Glob.screenX : m_Glob.screenY;
			m_bufferParams->setData((unsigned char*)(&m_Glob), sizeof(GlobalParams));

			// camera
			float matrView4x4[16];
			GetProjectionMat4x4(
				m_Glob.screenX, m_Glob.screenY, m_Glob.zNear, m_Glob.zFar,
				cam.up, cam.lookAt, cam.pos,
				matrView4x4);
			m_bufferMatrView4x4->setData(matrView4x4, 16 * sizeof(float));

			// clean dst zMap buffer
			m_csCleanRGB->execute(sMaxW / 32, sMaxH / 32, 1, { m_bufferParams, m_bufferZMap });

			// Render points
			if (m_pst->IsReady())
			{
				for (int m = 0; m < m_pst->GetNumBuffersInUse(); m++)
				{
					ICBuffer *pPoints = m_pst->GetPointBuffer(m);
					ICBuffer *pPartitions = m_pst->GetPartitionBuffer(m);
				    uint num_groups_x = 1;
					uint num_groups_y = (m_pst->GetNumPointsInBuffer(m) / m_csPointRender->getSX() / m_Glob.wrkLoad);
					m_csPointRender->execute(num_groups_x, num_groups_y, 1, { m_bufferParams,m_bufferDebug,pPoints,m_bufferZMap,m_bufferMatrView4x4,pPartitions });
				}
			}

			// post proc
			{
				float Vew2World4x4[16];
				GetVew2World4x4(cam.up, cam.lookAt, cam.pos, Vew2World4x4);
				m_bufferView2World->setData(Vew2World4x4, 16 * sizeof(float));
				m_csPostProc->execute(sMaxW / 32, sMaxH / 32, 1, { m_bufferParams ,m_bufferZMap,m_bufferZMapPost,m_bufferView2World,m_bufferDebug });
			}

			m_bufferZMapPost->blit(destWidth, destHeight);
			return 0;
		}

		void releaseInternal()
		{
			if (m_bufferZMap)			releaseICBuffer(&m_bufferZMap);
			if (m_bufferParams)			releaseICBuffer(&m_bufferParams);
			if (m_bufferZMapPost)		releaseICBuffer(&m_bufferZMapPost);
			if (m_bufferMatrView4x4)	releaseICBuffer(&m_bufferMatrView4x4);
			if (m_bufferView2World)		releaseICBuffer(&m_bufferView2World);
			if (m_bufferDebug)          releaseICBuffer(&m_bufferDebug);

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

	IPcrLib* IPcrLib::GetInstance()
	{
		return (libInstance.isInit) ? &libInstance:NULL;
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

