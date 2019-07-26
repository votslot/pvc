#include "math.h"
#include "stdlib.h"
#include "../PcrLib/pcrlib.h"

namespace pcrapp
{
	void generateWave(int w, int h, pcrlib::IPcrLib* pLib)
	{
		float sz = 10.0f;
		float tz = (float)w * sz;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				float rx = 1.0f* (float)rand() / (float)(RAND_MAX);
				float ry = 1.0f* (float)rand() / (float)(RAND_MAX);
				float xx = rx + (float)x * sz;
				float yy = ry + (float)y * sz;
				float zz = 0.1f + 0.2f * tz *(float)sin(2.0f * 3.1415f* yy / tz);
				//zz += 0.2f * tz *cos(2.0f * 3.1415f* (float)(x) / (float(w)));
				//float zz = (y < h/ 2) ? y * sz : h*sz / 2;
				//float zz = y * sz;
                pLib->addPoint(xx, yy, zz, zz*10.0f);
				
			}
		}
	}
}//namespace pcrapp
