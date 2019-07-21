#ifndef APP_EVENTS_H
#define APP_EVENTS_H

namespace pcrapp
{
	class IAppEvents
	{
	public:
		virtual void init() = 0;
		virtual void mouseDownEvent(int x, int y, bool isLeft, bool isRight) = 0;
		virtual void mouseUpEvent(bool isLeft, bool isRight) = 0;
		virtual void mouseMoveEvent(int x, int y) = 0;
		virtual void mouseWhellEvevt(int val) = 0;
		virtual void paintEvent(int sw, int sh) = 0;
		virtual void exitEvent() = 0;
		virtual void openLasFile(const char *filePath) = 0;
		virtual void testCloud() = 0;
		static IAppEvents * getAppEvents();
	};
}


#endif