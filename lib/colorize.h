#ifndef _ICOLORIZE_H_
#define  _ICOLORIZE_H_

class SSBBuffer;
class PointStorage;

class IColorize 
{
public:
	virtual void Init() = 0;
	virtual void Release() = 0;
	virtual void DoColorize(PointStorage *pst) = 0;
	virtual SSBBuffer* GetColorMap() = 0;
	static IColorize* GetInstance();
};

#endif