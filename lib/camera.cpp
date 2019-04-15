
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "camera.h"

static Camera::vector3 operator - (const Camera::vector3 &a, const Camera::vector3 &b)
{
	Camera::vector3 ret(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
	return ret;
}

static Camera::vector3 operator + (const Camera::vector3 &a, const Camera::vector3 &b)
{
	Camera::vector3 ret(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
	return ret;
}


static Camera::vector3 operator * (const Camera::vector3 &a, float prod)
{
	Camera::vector3 ret(a.v[0] * prod, a.v[1] * prod, a.v[2] * prod);
	return ret;
}

static Camera::vector3 operator * (float prod, const Camera::vector3 &a)
{
	Camera::vector3 ret = a * prod ;
	return ret;
}

// dot product
static float operator * (const Camera::vector3 &a, const Camera::vector3 &b)
{
	return (a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2]);
}

// Cross product
static Camera::vector3 operator ^ (const Camera::vector3 &a, const Camera::vector3 &b)
{
	Camera::vector3 ret(0.0f, 0.0f,0.0f);
	ret.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	ret.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	ret.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	return ret;
}

// Rotate vector P around axis defined by starting point S and unit vector A ( direction).
static Camera::vector3 rotate(const Camera::vector3 &P, const Camera::vector3 &S, const Camera::vector3 &A, float angle)
{
	Camera::vector3 D = P - S;
	Camera::vector3 N0 = (D * A) * A;
	Camera::vector3 N1 = D - N0;
	Camera::vector3 N2 = N1 ^ A;
	Camera::vector3 R = S + N0 +  N1 * cosf(angle) + N2 * sinf(angle);
	return R;
}

static Camera theCamera;

Camera *Camera::GetCamera() 
{
	return &theCamera;
}

Camera::Camera()
{
	m_L.v[0] = m_L.v[1] = m_L.v[2] = 0.0f;
	m_zNear = 2.0f;
	m_zFar = 10000.0f;
	m_MaxDimension = 1.0f;
	//SetPivotCamera(60.0f * 3.1415f / 180.0f, 0.0f, 600.0f);
	SetPivotCamera(0.0f, 0.0f, 600.0f, 0.0f,0.0f,0.0f);
}

float *Camera::GetPos()   { return  m_P.v; }
float *Camera::GetRight() { return  m_R.v; }
float *Camera::GetUp()    { return  m_U.v; }
float *Camera::GetDir()   { return  m_D.v; }

float *Camera::GetPivot() { return  m_L.v; }
int Camera::GetScreenX() { return (int)m_screenX; }
int Camera::GetScreenY() { return (int)m_screenY; }

void Camera::SetScreenPixSize(int sx, int sy)
{
	m_screenX = (float)sx;
	m_screenY = (float)sy;
	printf("ScreenX = %f ScreenY = %f\n", m_screenX, m_screenY);
}


void Camera::RotateAroundPivot(float dx, float dy)
{
	Camera::vector3 ZZ (0.0f, 0.0f, 0.0f);
	Camera::vector3 A0 = m_R;
	m_P = rotate(m_P, m_L, A0, dy);
	m_D = rotate(m_D, ZZ,  A0, dy);
	m_U = rotate(m_U, ZZ,  A0, dy);
	m_R = rotate(m_R, ZZ,  A0, dy);

	Camera::vector3 A1(0.0f, 0.0f, 1.0f);
	m_P = rotate(m_P, m_L, A1, dx);
	m_D = rotate(m_D, ZZ,  A1, dx);
	m_U = rotate(m_U, ZZ,  A1, dx);
	m_R = rotate(m_R, ZZ,  A1, dx);
}

void Camera::MoveInPivotDir(float dd) 
{
	m_P = m_P + (m_D * dd)*m_MaxDimension*0.02f;
}

void Camera::ShiftPivot(float dx, float dy) 
{
	float prd = 1.0f;
	Camera::vector3 SH;
	SH[0] = (m_R[0] * dx + m_U[0] * dy) * prd;
	SH[1] = (m_R[1] * dx + m_U[1] * dy) * prd;
	SH[2] = (m_R[2] * dx + m_U[2] * dy) * prd;
	m_P = m_P + SH;
	m_L = m_L + SH;
}

void Camera::SetPivotCamera(float teta, float fi, float dist, float px, float py, float pz) 
{
	vector3 S(sinf(teta) * cosf(fi), sinf(teta) * sinf(fi), cosf(teta));
	m_L[0] = px;
	m_L[1] = py;
	m_L[2] = pz;
	m_P = m_L + S * dist;
	m_D = -1.f * S;
	float lenR = sqrtf(m_D[0] * m_D[0] + m_D[1] * m_D[1]);
	if (lenR > 0.000000f) {
		m_R[0] = -m_D[1] / lenR;
		m_R[1] = m_D[0] / lenR;
		m_R[2] = 0.0f;
	}
	else {
		m_R[0] = 1.0f;
		m_R[1] = 0.0f;
		m_R[2] = 0.0f;
	}
	m_U = m_D ^ m_R;
}

void Camera::FromWorld(float *pWorldIn, float *pInCamOut) 
{
	float dd[3];
	float *pC = GetPos();
	float *pR = GetRight();
	float *pU = GetUp();
	float *pD = GetDir();
	dd[0] = pWorldIn[0] - pC[0];
	dd[1] = pWorldIn[1] - pC[1];
	dd[2] = pWorldIn[2] - pC[2];
	pInCamOut[0] = dd[0] * pR[0] + dd[1] * pR[1] + dd[2] * pR[2];
	pInCamOut[1] = dd[0] * pU[0] + dd[1] * pU[1] + dd[2] * pU[2];
	pInCamOut[2] = dd[0] * pD[0] + dd[1] * pD[1] + dd[2] * pD[2];
}

void Camera::ToWorld(float *pCamIn, float *pWorldOut)
{
	float *pC = GetPos();
	float *pR = GetRight();
	float *pU = GetUp();
	float *pD = GetDir();
	pWorldOut[0] = pCamIn[0] * pR[0] + pCamIn[1] * pU[0] + pCamIn[2] * pD[0] + pC[0];
	pWorldOut[1] = pCamIn[0] * pR[1] + pCamIn[1] * pU[1] + pCamIn[2] * pD[1] + pC[1];
	pWorldOut[2] = pCamIn[0] * pR[2] + pCamIn[1] * pU[2] + pCamIn[2] * pD[2] + pC[2];
}

static void Mat2Mat(float *A, float *B, float *C) 
{
	int m = 0;
	for (int l = 0; l < 16; l+=4) 
	{
		for (int c = 0; c < 4; c++)
		{
			C[m++] = A[l + 0]*B[0+c] + A[l + 1]*B[4+c] + A[l + 2]*B[8+c] + A[l + 3]*B[12+c];
		}
	}
}

/*
 W2V matrix memory layout

 Rx Ux Dx 0
 Ry Uy Dy 0
 Rz Uz Dz 0
 Tx Ty Tz 0
*/

void Camera::ConvertTo4x4(float *pOut)
{
	memset(pOut, 0, 16 * sizeof(float));
	for(int i = 0,k=0; i < 3; i++,k+=4) {
		pOut[k + 0] = m_R[i];
		pOut[k + 1] = m_U[i];
		pOut[k + 2] = m_D[i];
		pOut[k + 3] = 0.0f;
	}
	// Tx,Ty,Tz 
	pOut[12] = -(m_P[0] * m_R[0] + m_P[1] * m_R[1] + m_P[2] * m_R[2]);
	pOut[13] = -(m_P[0] * m_U[0] + m_P[1] * m_U[1] + m_P[2] * m_U[2]);
	pOut[14] = -(m_P[0] * m_D[0] + m_P[1] * m_D[1] + m_P[2] * m_D[2]);
}

void Camera::GetProjectionMat4x4(float screenX, float screenY, float zNear, float zFar, float *pOut)
{
	memset(pOut, 0, 16 * sizeof(float));
	float W2V[16],SP[16];
	float toScreen[4][4];

	float scm = (screenX < screenY) ? screenX : screenY;
	toScreen[0][0] = 1.0;
	toScreen[1][0] = 0.0;
	toScreen[2][0] = 0.5*screenX / (scm*zNear);
	toScreen[3][0] = 0.5*screenX / scm;

	toScreen[0][1] = 0.0;
	toScreen[1][1] = 1.0;
	toScreen[2][1] = 0.5*screenY / (scm*zNear);
	toScreen[3][1] = 0.5*screenY / scm;

	toScreen[0][2] = 0.0;
	toScreen[1][2] = 0.0;
	toScreen[2][2] = 1.0 / (zFar - zNear);
	toScreen[3][2] = -zNear / (zFar - zNear);

	toScreen[0][3] = 0.0;
	toScreen[1][3] = 0.0;
	toScreen[2][3] = 1.0 / (scm*zNear);
	toScreen[3][3] = 1.0 / scm;
	
	memcpy(SP, toScreen, 16 * sizeof(float));
	
	ConvertTo4x4(W2V);
	Mat2Mat(W2V, SP, pOut);
}

void Camera::GetVew2World4x4(float *pOut) 
{
	pOut[0] = m_R[0];
	pOut[1] = m_R[1];
	pOut[2] = m_R[2];
	pOut[3] = 0.0f;

	pOut[4] = m_U[0];
	pOut[5] = m_U[1];
	pOut[6] = m_U[2];
	pOut[7] = 0.0f;

	pOut[8]  = m_D[0];
	pOut[9]  = m_D[1];
	pOut[10] = m_D[2];
	pOut[11] = 0.0f;

	pOut[12] = m_P[0];
	pOut[13] = m_P[1];
	pOut[14] = m_P[2];
	pOut[15] = 1.0f;

}


void Camera::Print4x4(float *pDat)
{
	printf("matrix\n");
	float *pT = pDat;
	for (int i = 0; i < 4; i++, pT += 4)
	{
		printf(" %f %f %f %f \n", pT[0], pT[1], pT[2], pT[3]);
	}
}

