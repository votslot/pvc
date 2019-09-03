#include "stdio.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include "app-events.h"
#include "app-camera.h"
#include "app-testcloud.h"
#include "app-las.h"

using namespace pcrlib;

namespace pcrapp
{
	
	class AppEventsImpl :public IAppEvents
	{
	public:
		pcrlib::IPcrLib *m_pRLib; 
		pcrlib::LibCallback *m_cb;
		AppCamera m_camera;
		RenderParams m_renderParam;

		static const int  m_maxStrings = 32;
	    std::string m_displayStrings[m_maxStrings];
		int m_numDispStrings = 0;

		float m_maxSize;
		int m_mouseXDown = 0;
		int m_mouseYDown = 0;
		bool m_leftMouseDown = false;
		bool m_rightMouseDown = false;

		AppEventsImpl();
		void init(pcrlib::LibCallback *pCb );
		void mouseDownEvent(int x, int y, bool isLeft, bool isRight);
		void mouseUpEvent(bool isLeft, bool isRight);
		void mouseMoveEvent(int x, int y);
		void mouseWhellEvent(int val);
		void paintEvent(int sw, int sh);
		void viewModeEvent(int val);
		void exitEvent();
		void openLasFile(const char *filePath);
		void testCloud();
		void setDefCamera();
		void displayString(const char *pStr);
		//
		std::string m_filePath;
		std::recursive_mutex m_mainMutex;
		std::thread *m_threadObj;
		static void loadLasAsync(AppEventsImpl *pEnvClass);
		static void testCloudAsync(AppEventsImpl *pEnvClass);
	};

	AppEventsImpl::AppEventsImpl()
	{
		m_pRLib = NULL;
		m_cb = NULL;
		m_maxSize = 1.0f;
		m_renderParam.cm = pcrlib::Color_model_xyz;
	}

	IAppEvents * IAppEvents::getAppEvents()
	{
		return new AppEventsImpl();
	}

	void AppEventsImpl::init(pcrlib::LibCallback *pCb)
	{
		m_cb = pCb;
		m_pRLib = pcrlib::IPcrLib::init(pCb);
		m_pRLib->verify();
		m_filePath.clear();
	}

	

	void AppEventsImpl::mouseDownEvent(int x, int y, bool isLeft, bool isRight)
	{
		m_mouseXDown = x;
		m_mouseYDown = y;

		if (isLeft )
		{
			m_leftMouseDown = true;
		}
		else if (isRight)
		{
			m_rightMouseDown = true;
		}
	}

	void AppEventsImpl::mouseUpEvent(bool isLeft, bool isRight) 
	{

		if (isLeft)
		{
			m_leftMouseDown = false;
		}
		else if (isRight)
		{
			m_rightMouseDown = false;
		}

	}

	void AppEventsImpl::mouseMoveEvent(int x, int y)
	{
        //float *pivot = m_camera.GetPivot();
		int dx = x - m_mouseXDown;
		int dy = y - m_mouseYDown;
		if (m_leftMouseDown)
		{
			m_camera.RotateAroundPivot((float)dx*0.01f, (float)dy*0.01f);
		}
		else if (m_rightMouseDown)
		{
			float prd = m_maxSize *0.001f;
            m_camera.ShiftPivot((float)dx*prd, (float)dy*prd);
		}
		m_mouseXDown = x;
		m_mouseYDown = y;
	}

	void AppEventsImpl::mouseWhellEvent(int val) 
	{
		if (val == 0)
		{
			return;
		}
		float prd = 0.02f;
		float shift = (val > 0) ? prd : -prd;
		m_camera.MoveInPivotDir(shift*m_maxSize);
	}

	void AppEventsImpl::paintEvent(int sw, int sh)
	{
		static int nn = 0;
		if (m_pRLib)
		{
            m_pRLib->renderText((nn&1)? "|":"-",10,sh-10, 0);
            m_pRLib->renderText("PointSize PgUp/PgDn ",25,sh-10, 0);
			m_mainMutex.lock();
			for( int i = 0,py = 20; i<m_maxStrings; i++,py+=10){
				if ( !m_displayStrings[i].empty()) {
					m_pRLib->renderText(m_displayStrings[i].c_str(), 20, py, 0xFFFFFFFF);
				}
			}
			m_mainMutex.unlock();

 			pcrlib::Camera pcrCam;
			m_camera.BuildPcrCamera(pcrCam);
			m_pRLib->render(pcrCam, sw, sh, m_renderParam);
		}
		nn++;
	}

	void  AppEventsImpl::viewModeEvent(int val) 
	{
		switch (val)
		{
		case 0: m_renderParam.cm = pcrlib::Color_model_xyz; break;
		case 1: m_renderParam.cm = pcrlib::Colos_model_rgb; break;
		case 2: m_renderParam.cm = pcrlib::Color_model_intencity; break;
		}
	}

	void AppEventsImpl::exitEvent()
	{
		if (m_threadObj) 
		{
			m_threadObj->join();
			delete m_threadObj;
		}
		if (m_pRLib)
		{
			pcrlib::IPcrLib::release(&m_pRLib);
		}
	}

	void AppEventsImpl::testCloud()
	{
		if (m_pRLib)
		{
			m_threadObj = new std::thread(testCloudAsync, this);
		}
	}

	void AppEventsImpl::testCloudAsync(AppEventsImpl *pEnvClass)
	{
		    pEnvClass->m_pRLib->startAddPoints();
			generateWave(1024*4, 1024*4, pEnvClass->m_pRLib);
			pEnvClass->m_pRLib->doneAddPoints();
			pEnvClass->setDefCamera();
            pEnvClass->m_renderParam.cm = pcrlib::Color_model_xyz;
	}

	void AppEventsImpl::openLasFile(const char *filePath)
	{
		if (m_pRLib)
		{
			m_filePath = std::string(filePath);
			m_threadObj = new std::thread(loadLasAsync, this);
		}
	}

	void AppEventsImpl::loadLasAsync(AppEventsImpl *pEnvClass)
	{
		pEnvClass->m_pRLib->startAddPoints();
		int ret = readLasFile(pEnvClass->m_filePath.c_str(), pEnvClass->m_pRLib, pEnvClass->m_cb);
		pEnvClass->m_pRLib->doneAddPoints();
		pEnvClass->setDefCamera();
		pEnvClass->m_renderParam.cm = (ret == 1) ? pcrlib::Color_model_intencity : pcrlib::Colos_model_rgb;
	}

	void AppEventsImpl::displayString(const char *pStr) 
	{
		m_mainMutex.lock();

		if (pStr == NULL) {
			m_mainMutex.unlock();
			return;
		}

		if (pStr[0] == '\r') m_displayStrings[m_numDispStrings].clear();

		for (const char *p = pStr; *p; p++) 
		{
			if (*p != '\n') {
				if(*p != '\r')  m_displayStrings[m_numDispStrings] += *p;
			}
			else {
				m_numDispStrings++;
			}
			if (m_numDispStrings >= m_maxStrings) {
				m_displayStrings[0].clear();
				m_numDispStrings = 0;
			}
		}
		m_mainMutex.unlock();
	}

	void AppEventsImpl::setDefCamera()
	{
		if (m_pRLib) 
		{
			pcrlib::BoundBox bd = m_pRLib->getBoundBox();
			float ddx = (bd.xMax - bd.xMin);
			float ddy = (bd.yMax - bd.yMin);
			float ddz = (bd.zMax - bd.zMin);
			float maxD = (ddx > ddy) ? ddx : ddy;
			maxD = (maxD > ddz) ? maxD : ddz;
			float cx = (bd.xMax + bd.xMin) *0.5f;
			float cy = (bd.yMax + bd.yMin) *0.5f;
			float cz = (bd.zMax + bd.zMin) *0.5f;
			m_camera.SetPivotCamera(0.0f, 0.0f, maxD*2.0f, cx, cy, cz);
            m_camera.m_zFar = maxD * 10.0f;
			m_maxSize = maxD;
		}
	}




}// namespace pcrapp
