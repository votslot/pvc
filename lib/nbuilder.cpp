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
	return (dx*dx + dy * dy + dz * dz);
}

void FindBestNormal(uint64_t *pLocationMap, uint *pEnum, RenderPoint *pPt, int num) 
{
	const int sww = 3;
	const int nw = sww*2 + 1;
	int num_eq = 0;
	float nx, ny, nz;
	RenderPoint pTmp[nw];
	float pDist[nw];
	for (int i = 0; i < nw; i++)
	{
		int ia = pEnum[i];
		pTmp[i] = pPt[ia];
	}
	int tn = 0;
    // 0-1-2  3  4-5-6
	for (int i = sww; i < num - sww; i++)
	{
		pTmp[tn] = pPt[pEnum[i + sww]];
		tn++;
		if (tn >= nw) tn = 0;

		// Collect  dist
		int  ia = pEnum[i];
		int kk = 0, kMin[2] = { -1,-1 };
		for (int k = 0; k < nw; k++)
		{
			pDist[k] = Dist(pTmp[k], pPt[ia]);
			if ((pDist[k] > 0) && (kk<2) ) {
				kMin[kk++] = k;
			}
		}
		if ((kMin[0] == -1) && (kMin[1] == -1)) 
		{
			continue;
		}

		//find min
		float dMin[2];
		dMin[0] = pDist[kMin[0]]; dMin[1] = pDist[kMin[1]];
		for (int k = 0; k < nw; k++) 
		{
			if ((k == kMin[0]) || (k==kMin[1]))
			{
				continue;
			}
			if (pDist[k] > 0.0f) {
				if ((pDist[k] < dMin[0]) || (pDist[k] < dMin[1])) {
					int l = (dMin[0] > dMin[1]) ? 0 : 1;
					dMin[l] = pDist[k];
					kMin[l] = k;
				}
			}
		}

		int v0 = kMin[0];
		int v2 = kMin[1];
		if (GetN3(pTmp[v0], pPt[ia], pTmp[v2], nx, ny, nz))
		{
			uint vr = (uint)(fabs(nx) * 31.0f);
			uint vg = (uint)(fabs(ny) * 31.0f);
			uint vb = (uint)(fabs(nz) * 31.0f);
			if (vb < 10) vb = 10;
			pPt[ia].w = vr | (vg << 5) | (vb << 10);
		}
		else {
			pPt[ia].w = 31;
		}

#if 0
		int ib = pEnum[i + 1];
		if (pLocationMap[ia] == pLocationMap[ib]) {
			RenderPoint eqa = pPt[ia];
			RenderPoint eqb = pPt[ib];
			num_eq++;
		}
	
		
		int k0 = pEnum[i - 1];
		int k1 = pEnum[i - 0];
		int k2 = pEnum[i + 1];
		RenderPoint p0 = pPt[k0];
		RenderPoint p1 = pPt[k1];
		RenderPoint p2 = pPt[k2];
		float d01 = Dist(p0, p1);
		float d12 = Dist(p2, p1);
		
		//if ((d01 < 21.0f) && (d12 < 21.0f))
		if(1)
		{
			if (GetN3(p0, p1, p2, nx, ny, nz))
			{
				uint vr = (uint)(fabs(nx) * 31.0f);
				uint vg = (uint)(fabs(ny) * 31.0f);
				uint vb = (uint)(fabs(nz) * 31.0f);
				if (fabs(nz) > 0.9f)
				{
					pPt[k1].w = 31;
				}
				else {
					pPt[k1].w = 31 << 5;
				}
				pPt[k1].w = vr | (vg << 5) | (vb << 10);
			}
			else {
				pPt[k1].w = 31;
			}
		}
		else {
			pPt[k1].w = 0;
		}
		
		if (i < 64) {
			//printf("%llx (%d,%d) \n", pLocationMap[ia], (uint)pPt[ia].x, (uint)pPt[ia].y);
		}
		//std::cout << "(" << pPt[ia].x << "," << pPt[ia].y << ")" << std::endl;
#endif
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
	double sx = double(0xFFFFFFFF) / ((double)Bd.xMax - (double)Bd.xMin);
	double sy = double(0xFFFFFFFF) / ((double)Bd.yMax - (double)Bd.yMin);
	double sz = double(0xFFFFFFFF) / ((double)Bd.zMax - (double)Bd.zMin);
	for (int i = 0; i < num; i++)
	{
		uint xi = (uint)(((double)pPt[i].x - (double)Bd.xMin) * sx);
		uint yi = (uint)(((double)pPt[i].y - (double)Bd.yMin) * sy);
		uint zi = (uint)(((double)pPt[i].z - (double)Bd.zMin) * sz);
		pLocationMap[i] = CombineBits(xi, yi, zi, 20);
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
