
#include <stdio.h>
#include <memory.h>
#include <math.h> 

namespace pcrlib
{

	static void Print4x4(float *pDat)
	{
		printf("matrix\n");
		float *pT = pDat;
		for (int i = 0; i < 4; i++, pT += 4)
		{
			printf(" %f %f %f %f \n", pT[0], pT[1], pT[2], pT[3]);
		}
	}

	static void Mat2Mat(float *A, float *B, float *C)
	{
		int m = 0;
		for (int l = 0; l < 16; l += 4)
		{
			for (int c = 0; c < 4; c++)
			{
				C[m++] = A[l + 0] * B[0 + c] + A[l + 1] * B[4 + c] + A[l + 2] * B[8 + c] + A[l + 3] * B[12 + c];
			}
		}
	}

	/*
	W2V matrix memory layout

	Rx Ux Dx 0
	Ry Uy Dy 0
	Rz Uz Dz 0
	Tx Ty Tz 1
	*/

	static void ConvertTo4x4(const float *m_R, const float *m_U, const float *m_D, const float *m_P,float *pOut)
	{
		memset(pOut, 0, 16 * sizeof(float));
		for (int i = 0, k = 0; i < 3; i++, k += 4) {
			pOut[k + 0] = m_R[i];
			pOut[k + 1] = m_U[i];
			pOut[k + 2] = m_D[i];
			pOut[k + 3] = 0.0f;
		}
		pOut[15] = 1.0f;
		// Tx,Ty,Tz,Tw
		pOut[12] = -(m_P[0] * m_R[0] + m_P[1] * m_R[1] + m_P[2] * m_R[2]);
		pOut[13] = -(m_P[0] * m_U[0] + m_P[1] * m_U[1] + m_P[2] * m_U[2]);
		pOut[14] = -(m_P[0] * m_D[0] + m_P[1] * m_D[1] + m_P[2] * m_D[2]);
	}

	void GetProjectionMat4x4(float screenPixX, float screenPixY, float zNear, float zFar, float fov,
							 const float *m_U, const float *m_D, const float *m_P, 
							 float *pOut)
	{
		memset(pOut, 0, 16 * sizeof(float));
		float W2V[16], SP[16];
		float toScreen[4][4];
		float screenX = screenPixX;
		float screenY = screenPixY;
		float prd = 2.0f;
		float scm = (screenX < screenY) ? screenX : screenY;
		
		float tana = tan(fov * 3.1415f / 360.0f);
		//Xs = x*scm  + ( z * tana * screenPixX) /2
		toScreen[0][0] = scm;
		toScreen[1][0] = 0.0f;
		toScreen[2][0] = tana * 0.5f*screenPixX;
		toScreen[3][0] = 0.0f;

		//Ys = y*scm  + ( z * tana * screenPixY) /2
		toScreen[0][1] = 0.0f;
		toScreen[1][1] = scm;
		toScreen[2][1] = tana * 0.5f*screenPixY;
		toScreen[3][1] = 0.0f;

		// Zs = (z-zNear)/(zFar-zNear)
		toScreen[0][2] = 0.0f;
		toScreen[1][2] = 0.0f;
		toScreen[2][2] = 1.0f / (zFar - zNear);
		toScreen[3][2] = -zNear / (zFar - zNear);

		// w = z*tana
		toScreen[0][3] = 0.0f;
		toScreen[1][3] = 0.0f;
		toScreen[2][3] = tana;
		toScreen[3][3] = 0.0f;

		float rr[3];
		rr[0] = m_U[1] * m_D[2] - m_U[2] * m_D[1];
		rr[1] = m_U[2] * m_D[0] - m_U[0] * m_D[2];
		rr[2] = m_U[0] * m_D[1] - m_U[1] * m_D[0];

		ConvertTo4x4(rr,m_U,m_D,m_P,W2V);
		Mat2Mat(W2V, (float*)toScreen, pOut);
	}

	void GetVew2World4x4(const float *m_U, const float *m_D, const float *m_P, float *pOut)
	{
		pOut[0] = m_U[1] * m_D[2] - m_U[2] * m_D[1];
		pOut[1] = m_U[2] * m_D[0] - m_U[0] * m_D[2];
		pOut[2] = m_U[0] * m_D[1] - m_U[1] * m_D[0];
		pOut[3] = 0.0f;

		pOut[4] = m_U[0];
		pOut[5] = m_U[1];
		pOut[6] = m_U[2];
		pOut[7] = 0.0f;

		pOut[8] = m_D[0];
		pOut[9] = m_D[1];
		pOut[10] = m_D[2];
		pOut[11] = 0.0f;

		pOut[12] = m_P[0];
		pOut[13] = m_P[1];
		pOut[14] = m_P[2];
		pOut[15] = 1.0f;
	}

	
}//namespace pcrlib
