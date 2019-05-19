
#include "colorize.h"
#include "storage.h"
#include "cbuff.h"
#include "..\shaders\ginclude.h"
#include "..\shaders\post-proc.cs.glsl"

class Colorize : public IColorize
{
	SSBBuffer sbb_ColorizeStruct;
	SSBBuffer sbb_OutColor;
	SSBBuffer sbb_Dbg;
	CSShader csColorize;
    float pfTemp[1024];

	void Init()
	{
		csColorize.initFromSource(cs_colorize.c_str());
		//
		sbb_OutColor.init();
		sbb_OutColor.allocate(1024 * 4);
		//
		sbb_ColorizeStruct.init();
		sbb_ColorizeStruct.allocate(sizeof(ColorizeData));
		//
		sbb_Dbg.init();
		sbb_Dbg.allocate(1024 * 4);
	}

	void Release()
	{
	}

	void DoColorize( PointStorage *pst) 
	{
		ColorizeData cd;
		cd.xMin = pst->GetXMin();
		cd.xMax = pst->GetXMax();
		cd.yMin = pst->GetYMin();
		cd.yMax = pst->GetYMax();
		sbb_ColorizeStruct.setData(&cd, sizeof(ColorizeData));
		for (int m = 0; m < pst->GetNumBuffersInUse(); m++)
		{
			//csColorize.execute(32, 32, 1, { &sbb_ColorizeStruct, &sbb_OutColor,&sbb_Dbg });
		}
		sbb_Dbg.getData(32 * sizeof(float), pfTemp);
	}

	SSBBuffer *GetColorMap() 
	{
		return NULL;
	}
};

static  Colorize theColorize;

IColorize * IColorize::GetInstance()
{
	return  &theColorize;
}