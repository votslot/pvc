#ifndef APP_EVENTS_H
#define APP_EVENTS_H


namespace pcrlib 
{
	struct LibCallback;
}

namespace pcrapp
{

	class IAppEvents 
	{
	public:
		virtual void init(pcrlib::LibCallback *pCb = NULL) = 0;
		virtual void mouseDownEvent(int x, int y, bool isLeft, bool isRight) = 0;
		virtual void mouseUpEvent(bool isLeft, bool isRight) = 0;
		virtual void mouseMoveEvent(int x, int y) = 0;
		virtual void mouseWhellEvent(int val) = 0;
		virtual void keyEvent(int val, bool isPress) = 0;
		virtual void paintEvent(int sw, int sh) = 0;
		virtual void viewModeEvent(int val ) = 0;
		virtual void exitEvent() = 0;
		virtual void openLasFile(const char *filePath) = 0;
		virtual void testCloud() = 0;
		virtual void displayString(const char *pStr) = 0;

		static IAppEvents * getAppEvents();
	};
}


#endif