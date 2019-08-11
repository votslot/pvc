

#include <stdio.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include "../pcrlib.h"
#include "icompute.h"
#include "storage.h"
#include "matrix-utils.h"
#include "font.h"

#include "OpenGL/wave-test.cs.glsl"
#include "OpenGL/shaders/render-points.cs.glsl"
#include "OpenGL/shaders/post-proc.cs.glsl"
#include "OpenGL/shaders/ginclude.h"

namespace pcrlib 
{
extern int InitGLBlit();

	
	class ThePcrLib :public IPcrLib, ICErrorCallBack
	{
	public:

		static const int sMaxW = 2048;
		static const int sMaxH = 2048;
		LibCallback *m_pcallback;
		bool isInit = false;
		PointStorage  * m_pst = NULL;
		GlobalParams m_Glob;
		//font
		IFontRenderer *m_font;
		// buffres
		std::vector<ICBuffer*> m_bufferList;
		ICBuffer  *m_bufferZMap = NULL;
		ICBuffer  *m_bufferParams = NULL;
		ICBuffer  *m_bufferZMapPost = NULL;
		ICBuffer  *m_bufferMatrView4x4  = NULL;
		ICBuffer  *m_bufferView2World = NULL;
		ICBuffer  *m_bufferDebug = NULL;
		//shaders
		std::vector<ICShader*> m_shaderList;
		ICShader *m_csPointRender = NULL;
		ICShader *m_csCleanRGB  = NULL;
		ICShader *m_csPostProcRgb = NULL;
		ICShader *m_csPostProcXyz = NULL;
		ICShader *m_csPostProcInt = NULL;

		ThePcrLib() 
		{
			m_pcallback = new LibCallback();
		}

		ICShader *initShaderInternal(const char *pSrc) 
		{
			ICShader *pRet = createICShader();
			pRet->initFromSource(pSrc);
			m_shaderList.push_back(pRet);
			return pRet;
		}

		ICShader * getPostProcShaderInternal(const RenderParams &rp)
		{
			switch(rp.cm)
			{
			case ColorModel::Color_model_intencity: return m_csPostProcInt;
			case ColorModel::Color_model_xyz:       return m_csPostProcXyz;
			case ColorModel::Colos_model_rgb:       return m_csPostProcRgb;
			default: m_pcallback->error("Unknown color mode\n");
			}
			return NULL;
		}

		void initInternal()
		{
			if (isInit) 
			{
				return;
			}
			ICShader::m_err = this;
			
			// init font
			m_font = IFontRenderer::getInstance();

			// init storage
            m_pst = PointStorage::GetInstatnce(m_pcallback);
            m_pst->Init();
			// shaders
			m_csCleanRGB    =  initShaderInternal(cs_clean.c_str());
			m_csPointRender =  initShaderInternal(cs_render_points.c_str());
			m_csPostProcXyz = initShaderInternal(cs_postproc_xyz.c_str());
			m_csPostProcInt = initShaderInternal(cs_postproc_int.c_str());
			m_csPostProcRgb = initShaderInternal(cs_postproc_rgb.c_str());

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

			if (InitGLBlit() != 0)
			{
				m_pcallback->error("OpenGL blit initialization failed");
			}
			isInit = true;
		}

	
		void startAddPoints()
		{
		}

		int addPoint(float x, float y, float z, unsigned int w)
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

		int render(const Camera &cam, int destWidth, int destHeight, const RenderParams &rp)
		{
			if (!isInit)
			{
				return 0;
			}
			m_Glob.screenX = (float)destWidth;
			m_Glob.screenY = (float)destHeight;
			m_Glob.zNear = (float)cam.zNear;
			m_Glob.zFar = (float)cam.zFar;
			m_Glob.zRange = (float)(1 << 24); 
			m_Glob.maxDimension = 0.0f;
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
					m_csPointRender->execute(
						num_groups_x, num_groups_y, 1,
						{ m_bufferParams,m_bufferDebug,pPoints,m_bufferZMap,m_bufferMatrView4x4,pPartitions });
				}
			}

			// post proc
			{
				float Vew2World4x4[16];
				GetVew2World4x4(cam.up, cam.lookAt, cam.pos, Vew2World4x4);
				m_bufferView2World->setData(Vew2World4x4, 16 * sizeof(float));

				ICShader *pPostProc = getPostProcShaderInternal(rp);
				pPostProc->execute(sMaxW / 32, sMaxH / 32, 1, { m_bufferParams ,m_bufferZMap,m_bufferZMapPost,m_bufferView2World,m_bufferDebug });
			}
			// font
			if(m_font) m_font->renderFont(m_bufferZMapPost, m_bufferParams);

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

			for (std::vector<ICShader*>::iterator it = m_shaderList.begin(); it != m_shaderList.end(); ++it)
			{
				ICShader* ps = *it;
				if(ps) releaseICShader(&ps);
			}
			m_shaderList.clear();
	
			isInit = false;
		}

		int verify();

		//from ICErrorCallBack
		void error(const char *pMsg) 
		{
			m_pcallback->error(pMsg);
		}

	};//class ThePcrLib

	IPcrLib* IPcrLib::init(LibCallback *pCb)
	{
		ThePcrLib *pInst = new ThePcrLib();
		if (pCb) pInst->m_pcallback = pCb;
		pInst->initInternal();
		return  pInst;
	}

	void IPcrLib::release(IPcrLib** ppLib)
	{
		ThePcrLib *pInst = static_cast<ThePcrLib*>(*ppLib);
		pInst->releaseInternal();
	}

	
	// Test
	int ThePcrLib::verify()
	{
		m_pcallback->message("Run verification test...\n");
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
				m_pcallback->message("Verification test failed\n");
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
		m_pcallback->message("Verification test success\n");
        return ret;
	}
	
	void  LibCallback::error(const char *pMsg) 
	{
		std::cout <<"Error: "<< pMsg;
	}

	void  LibCallback::message(const char *pMsg) 
	{ 
		std::cout << pMsg ;
	}

	void* LibCallback::memAlloc(size_t sz) 
	{ 
		return malloc(sz);
	}
	void  LibCallback::memFree(void *ptr) 
	{
		if (ptr) free(ptr);
	}
	

}

