#include "math.h"
#include "stdlib.h"
#include "../PcrLib/pcrlib.h"

namespace pcrapp
{
	void generateWave(int w, int h, pcrlib::IPcrLib* pLib)
	{
		float sz = 10.0f;
        float step_u = 1.0f/(float)w;
        float step_v = 1.0f/(float)h;
        for (int vv = -h/2;  vv< h/2; vv++)
		{
            for (int uu = -w/2; uu < w/2; uu++)
			{
                float u = step_u * (float)uu;
                float v = step_v * (float)vv;
                float up = 2.0f * 3.1415f* u;
                float vp = 2.0f * 3.1415f* v;
                float r = 0.2f + cos(up*6.0f)*cos(up*6.0f) *sin(vp*1.1f) * sin(vp*1.1f);
                r = sqrt(r*r);
                float xx  =  r* (float) ( sin(vp) * cos(up));
                float yy  =  r* (float) ( sin(vp) * sin(up));
                float zz  =  r *cos(vp);
               // float zz = 0.1f + 0.2f * tz *(float)sin(8.0f * 3.1415f* yy / tz);
				//zz += 0.2f * tz *cos(2.0f * 3.1415f* (float)(x) / (float(w)));
				//float zz = (y < h/ 2) ? y * sz : h*sz / 2;
				//float zz = y * sz;
                pLib->addPoint(xx, yy, zz*0.5f, zz);
				
			}
		}
	}
}//namespace pcrapp
