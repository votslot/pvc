#include "math.h"
#include "stdlib.h"
#include "../PcrLib/pcrlib.h"

namespace pcrapp
{
#if 0
	void generateWave(int w, int h, pcrlib::IPcrLib* pLib)
	{
	//	float sz = 10.0f;
        float step_u = 1.0f/(float)w;
        float step_v = 1.0f/(float)h;
		float nf = 2.0f;
        for (int vv = -h/2;  vv< h/2; vv++)
		{
            for (int uu = -w/2; uu < w/2; uu++)
			{
                float u = step_u * (float)uu;
                float v = step_v * (float)vv;
                float up = 2.0f * 3.1415f* u;
                float vp = 2.0f * 3.1415f* v;
                float r = 100.0f*(float)( 0.2f + cos(up*nf)*cos(up*nf) *sin(vp*1.1f) * sin(vp*1.1f));
 				r = (float)fabs(r);
                float xx  =  r* (float) ( sin(vp) * cos(up));
                float yy  =  r* (float) ( sin(vp) * sin(up));
                float zz  =  r * (float)cos(vp);
                //unsigned char c = (unsigned char)(255.0f * cos(vp*nf)*cos(vp*nf));
				unsigned short c555 =  (31 << 5) ;
                pLib->addPoint(xx, yy, zz*0.5f, c555);
			}
		}
	}
#endif
	void generateWave(int w, int h, pcrlib::IPcrLib* pLib)
	{
		float xc = 0.5f*(float)(w - 1);
		float yc = 0.5f*(float)(h - 1);
	
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				unsigned int c =(x>w/2) ? 255 :65535;
				float zf = (y == 0) ? 0.1f : 0.0f;
				pLib->addPoint((float)x - xc, (float)y- yc, zf, c);
			}
		}
	
	
	}


}//namespace pcrapp
