#include "storage.h"
#include "cbuff.h"
#include "..\shaders\ginclude.h"



static bool GetN3(const RenderPoint &A, const RenderPoint &B, const RenderPoint &C, float &x, float &y, float &z)
{
	double dx1 = (double)(A.x) - (double)B.x;
	double dy1 = (double)(A.y) - (double)B.y;
	double dz1 = (double)(A.z) - (double)B.z;
	double dx2 = (double)(C.x) - (double)B.x;
	double dy2 = (double)(C.y) - (double)B.y;
	double dz2 = (double)(C.z) - (double)B.z;
	double xx = dy1 * dz2 - dz1 * dy2;
	double yy = -dx1 * dz2 + dz1 * dx2;
	double zz = dx1 * dy2 - dy1 * dx2;
	double len = sqrt(xx * xx + yy * yy + zz * zz);
	if (len > 0.0) {
		x = (float)(xx / len);
		y = (float)(yy / len);
		z = (float)(zz / len);
		return true;
	}
	return false;
}


static float Dist(RenderPoint &A, RenderPoint &B)
{
	float dx = A.x - B.x;
	float dy = A.y - B.y;
	float dz = A.z - B.z;
	return sqrtf(dx*dx + dy * dy + dz * dz);
}

static void PintDist(uint64_t *pLocationMap, uint *pEnum, RenderPoint *pPt, int num)
{
	for (int i = 1; i < num - 1; i++)
	{
		int ia = pEnum[i-1];
		int ib = pEnum[i];
		float dd = Dist(pPt[ia], pPt[ib]);
		printf("%f\n", dd);
	}
}

void FindBestNormal(uint64_t *pLocationMap, uint *pEnum, RenderPoint *pPt, int num) 
{
	//PintDist(pLocationMap, pEnum, pPt, num);
	
	const int sww = 3;
	const int nw = sww*2 + 1;
	int num_eq = 0;
	float nx, ny, nz;
   
	RenderPoint D[4];
	float dd[4];
	for (int i = sww; i < num - sww; i++)
	{
		int ia = pEnum[i];
		RenderPoint T0 = pPt[pEnum[i]];
		D[0] = pPt[pEnum[i - 2]];
		D[1] = pPt[pEnum[i - 1]];
		D[2] = pPt[pEnum[i + 1]];
	    D[3] = pPt[pEnum[i + 2]];
		for( int i = 0; i<4; i++) dd[i] = Dist(D[i], T0);
		float d_min = FLT_MAX;
		int k_min1,k_min2;
		for (int i = 0; i < 4; i++)
		{
			if (dd[i] < d_min) {
				d_min = dd[i];
				k_min1= i;
			}
		}
		d_min = FLT_MAX;
		for (int i = 0; i < 4; i++)
		{
			if (i == k_min1) continue;
			if (dd[i] < d_min) {
				d_min = dd[i];
				k_min2 = i;
			}
		}
		RenderPoint P0 = D[k_min1];
		RenderPoint P1 = D[k_min2];

		/*
		RenderPoint S0 = pPt[pEnum[i - 2]];
		RenderPoint P0 = pPt[pEnum[i - 1]];
		RenderPoint P1 = pPt[pEnum[i + 1]];
		RenderPoint P2 = pPt[pEnum[i + 2]];
		float ds0 = Dist(S0, T0);
		float df0 = Dist(P0, T0);
		float df1 = Dist(P1, T0);
		float df2 = Dist(P2, T0);
		*/
		float df0 = Dist(P0, T0);
		float df1 = Dist(P1, T0);
		float dfMax = (df0 > df1) ? df0 : df1;
		if ((T0.y > 299.0f) &&(T0.y > 321.0f))
		{
			//printf("%f\n", dfMax);
		}
		


		if (GetN3(P0, T0, P1, nx, ny, nz))
		{
			uint vr = (uint)(fabs(nx) * 31.0f);
			uint vg = (uint)(fabs(ny) * 31.0f);
			uint vb = (uint)(fabs(nz) * 31.0f);
			//pPt[ia].w =  vr | (vg << 5) | (vb << 10);
			pPt[ia].w = vb | (vb << 5) | (vb << 10);
		}
		else {
			pPt[ia].w = 31;
		}

	}
	std::cout << "EQPINTS=" << num_eq << std::endl;

}

static uint64_t CombineBits(uint x_in, uint y_in, uint z_in, int nn)
{
	uint64_t  res = 0;
	uint64_t x = x_in;
	uint64_t y = y_in;
	uint64_t z = z_in;
	int m = 63;
	for (int i = 0; i < nn; i++)
	{
		uint64_t ii = 31 - i;
		uint64_t msk = 1LL << ii;
		uint64_t xb = (x & msk) >> ii;
		uint64_t yb = (y & msk) >> ii;
		uint64_t zb = (z & msk) >> ii;
		res |= (xb << m); m--;
		res |= (yb << m); m--;
		res |= (zb << m); m--;
	}
	return res;
}

static void BuildLocationMap(RenderPoint *pPt, int num, const BdBox<float>& Bd, uint64_t *pLocationMap)
{
	double dxf = (double)Bd.xMax - (double)Bd.xMin;
	double dyf = (double)Bd.yMax - (double)Bd.yMin;
	double dzf = (double)Bd.zMax - (double)Bd.zMin;
	double df_max = (dxf > dyf) ? dxf : dyf;
	df_max = (dzf > df_max) ? dzf : df_max;
	
	double sx = double(0xFFFFFFFF) / df_max;
	double sy = double(0xFFFFFFFF) / df_max;
	double sz = double(0xFFFFFFFF) / df_max;

	for (int i = 0; i < num; i++)
	{
		uint xi = (uint)(((double)pPt[i].x - (double)Bd.xMin) * sx);
		uint yi = (uint)(((double)pPt[i].y - (double)Bd.yMin) * sy);
		uint zi = (uint)(((double)pPt[i].z - (double)Bd.zMin) * sz);
		pLocationMap[i] = CombineBits(xi, yi, zi, 21);
		pPt[i].w = 0;
	}
}


void BuildNormals(RenderPoint *pPt, int num, const BdBox<float>& Bd)
{
	std::cout << "Build Normals box=" << Bd.xMax - Bd.xMin << ","<<Bd.yMax - Bd.yMin << "," << Bd.zMax - Bd.zMin;
	uint64_t *pLocationMap = new uint64_t[num];
	uint *pEnum = new uint[num];

	BuildLocationMap(pPt, num, Bd, pLocationMap);
	for (int i = 0; i < num; i++) pEnum[i] = i;

	std::sort(pEnum, pEnum + num, [=](uint a, uint b) {return pLocationMap[a] < pLocationMap[b]; });
	FindBestNormal(pLocationMap, pEnum, pPt, num);

	delete[] pLocationMap;
	delete[] pEnum;

}

void BuildValues(float *pF,  int num)
{
	RenderPoint *pPoint = (RenderPoint*)pF;

	unsigned int *pVals = new unsigned int[65536];
	for (int i = 0; i < 65536; i++) pVals[i] = 0;
	float minV = FLT_MAX;
	float maxV = FLT_MIN;
	for (int i = 0; i < num; i++)
	{
		float *ppF = (float*)(pPoint + i);
		float vf = ppF[3];
		if (vf > maxV) maxV = vf;
		if (vf < minV) minV = vf;
		int vi = (int)vf;
		if (vi >= 65535) vi = 65535;
		pVals[vi]++;
	}

	int t90 = (int)((float)num * 0.9f);
	int t_first = 0, t_last = 65535,total = num;
	for(;;)
	{
		if ( (total <= t90) || (t_first>= t_last))
		{
			break;
		}
		if (pVals[t_first] < pVals[t_last])
		{
			total -= pVals[t_first];
			t_first++;
		}
		else
		{
			total -= pVals[t_last];
			t_last--;
		}
	}
	float sc = 1.0f;
	int range = t_last - t_first;
	if (range > 0) {
		sc = 31.0f / (float)range;
	}

	for (int i = 0; i < num; i++)
	{
		float *ppF = (float*)(pPoint + i);
		unsigned int vfi = (ppF[3] - minV)*sc;
		if (vfi > 31) vfi = 31;
		pPoint[i].w = vfi | (vfi << 5) | (vfi << 10);
	}

}
