#include "app-events.h"

namespace pcrapp
{
	class AppEventsImpl : public IAppEvents
	{
		void OnMouseDownEvent(int x, int y);
		void OnMouseUpEvent(bool isLeft, bool isRight);
		void OnMouseMoveEvent(int x, int y);
		void OnMouseWhellEvevt(int val);
		void OnOpenLasFile(const char *filePath);
		void OnTestCloud();
	};

	void OnMouseDownEvent(int x, int y) {}
	void OnMouseUpEvent(bool isLeft, bool isRight) {}
	void OnMouseMoveEvent(int x, int y) {}
	void OnMouseWhellEvevt(int val) {}
	void OnOpenLasFile(const char *filePath) {}
	void OnTestCloud() {}

}