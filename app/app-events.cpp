#include "stdio.h"
#include <iostream>
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
		void exitEvent();
		void openLasFile(const char *filePath);
		void testCloud();
		//
		void setDefCamera();
	};
	

	AppEventsImpl::AppEventsImpl()
	{
		m_pRLib = NULL;
		m_cb = NULL;
		m_maxSize = 1.0f;
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
		float *pivot = m_camera.GetPivot();
		int dx = x - m_mouseXDown;
		int dy = y - m_mouseYDown;
		if (m_leftMouseDown)
		{
			m_camera.RotateAroundPivot((float)dx*0.01f, (float)dy*0.01f);
		}
		else if (m_rightMouseDown)
		{
			m_camera.ShiftPivot((float)dx, (float)dy);
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
		float prd = 1.0f;
		float shift = (val > 0) ? prd : -prd;
		m_camera.MoveInPivotDir(shift*m_maxSize);
	}

	void AppEventsImpl::paintEvent(int sw, int sh)
	{
		if (m_pRLib)
		{
			pcrlib::Camera pcrCam;
			m_camera.BuildPcrCamera(pcrCam);
			m_pRLib->render(pcrCam, sw, sh);
		}
	}

	void AppEventsImpl::exitEvent()
	{
		if (m_pRLib)
		{
			pcrlib::IPcrLib::release(&m_pRLib);
		}
	}


	void AppEventsImpl::openLasFile(const char *filePath) 
	{
		if (m_pRLib)
		{
			m_pRLib->startAddPoints();
			readLasFile(filePath, m_pRLib, m_cb);
			m_pRLib->doneAddPoints();
			setDefCamera();
		}
	}

	void AppEventsImpl::testCloud()
	{
		if (m_pRLib)
		{
			m_pRLib->startAddPoints();
			generateWave(1024, 1024, m_pRLib);
			m_pRLib->doneAddPoints();
			setDefCamera();
		}
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
			m_camera.m_zFar = maxD * 4.0f;
			m_maxSize = maxD;
		}
	}




}// namespace pcrapp