#ifndef APP_EVENTS_H
#define APP_EVENTS_H

class IAppEvents
{
	virtual void OnMouseDownEvent(int x, int y) = 0;
	virtual void OnMouseUpEvent(bool isLeft, bool isRight) = 0;
	virtual void OnMouseMoveEvent(int x, int y) = 0;
	virtual void OnMouseWhellEvevt(int val) = 0;
	virtual void OnOpenLasFile(const char *filePath) = 0;
	virtual void OnTestCloud() = 0;
};


#endif