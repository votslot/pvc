
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "camera.h"

static Camera theCamera;

Camera *Camera::GetCamera() 
{
	return &theCamera;
}

Camera::Camera() 
{
	m_pivot[0] = m_pivot[1] = m_pivot[2] = 0.0f;
	SetPivotCamera(60.0f * 3.1415f / 180.0f, 0.0f, 600.0f);
}

float* Camera:: GetData()
{
	return m_data;
}

float *Camera::GetPos()   { return  m_data + 0; }
float *Camera::GetRight() { return  m_data + 3; }
float *Camera::GetUp()    { return  m_data + 6; }
float *Camera::GetDir()   { return  m_data + 9; }
float *Camera::GetPivot() { return  m_pivot; }

void Camera::SetScreenPixSize(int sx, int sy)
{
	m_screenX = (float)sx;
	m_screenY = (float)sy;
	printf("ScreenX = %f ScreenY = %f\n", m_screenX, m_screenY);
}

#if 0
void Camera::Build(float pos[3], float dir[3], float up[3]) 
{
	m_data[0] = pos[0];
	m_data[1] = pos[1];
	m_data[2] = pos[2];
	//right
	m_data[3] = up[2] * dir[1] - dir[2] * up[1];
	m_data[4] = -up[0] * dir[2] + dir[0] * up[2];
	m_data[5] = up[0] * dir[1] - dir[0] * up[1];

	float *pU = GetUp();
	float *pD = GetDir();
	for (int i = 0; i < 3; i++) {
		pU[i] = up[i];
		pD[i] = dir[i];
	}
}
#endif

void Camera::RotateRight(float ang)
{
	float sinF = sin(ang);
	float cosF = cos(ang);
	float uu[3], dd[3];
	float *pU = GetUp();
	float *pD = GetDir();
	for (int i = 0; i < 3; i++){
		uu[i] =  pU[i] * cosF + pD[i] * sinF;
		dd[i] = -pU[i] * sinF + pD[i] * cosF;
	}
	for (int i = 0; i < 3; i++) {
		pU[i] = uu[i];
		pD[i] = dd[i];
	}
}

void Camera::RotateUp(float ang)
{
	float sinF = sin(ang);
	float cosF = cos(ang);
	float uu[3], dd[3];
	float *pU = GetRight();
	float *pD = GetDir();
	for (int i = 0; i < 3; i++) {
		uu[i] = pU[i] * cosF + pD[i] * sinF;
		dd[i] = -pU[i] * sinF + pD[i] * cosF;
	}
	for (int i = 0; i < 3; i++) {
		pU[i] = uu[i];
		pD[i] = dd[i];
	}
}

void Camera::RotateDir(float ang)
{
	float sinF = sin(ang);
	float cosF = cos(ang);
	float uu[3], rr[3];
	float *pR = GetRight();
	float *pU = GetUp();
	for (int i = 0; i < 3; i++) {
		uu[i] =  pU[i] * cosF + pR[i] * sinF;
		rr[i] = -pU[i] * sinF + pR[i] * cosF;
	}
	for (int i = 0; i < 3; i++) {
		pU[i] = uu[i];
		pR[i] = rr[i];
	}
}

#if 0
void Camera::Build(float pos[3], float dir[3])
{
	float right[3], up[3];
	right[0] = -dir[1];
	right[1] = dir[0];
	float len = sqrtf(right[0]*right[0] + right[1] * right[1]);
	if (len > 0.0000001f) {
		right[0] = right[0] / len;
		right[1] = right[1] / len;
		right[2] = 0.0f;
	}
	else {
		right[0] = 1.0f;
		right[1] = 0.0f;
		right[2] = 0.0f;
	}
	float *pPos = GetPos();
	pPos[0] = pos[0];
	pPos[1] = pos[1];
	pPos[2] = pos[2];
	//right
	m_data[3] = right[0];
	m_data[4] = right[1];
	m_data[5] = right[2];
	//up
	m_data[6] = right[1] * dir[2] - right[2] * dir[1];
	m_data[7] = right[2] * dir[0] - right[0] * dir[2];
	m_data[8] = right[0] * dir[1] - right[1] * dir[0];
	// z(dir)
	m_data[9]  = dir[0];
	m_data[10] = dir[1];
	m_data[11] = dir[2];
}
#endif

static void RotateAroundZ(float *pV, float ang) 
{
	float xx = pV[0];
	float yy = pV[1];
	float sinF = sin(ang);
	float cosF = cos(ang);
	pV[0] =  xx * cosF + yy * sinF;
	pV[1] = -xx * sinF + yy * cosF;
}

void Camera::RotateAroundPivot(float dx, float dy)
{
	float pivotInCam[3], pivotInWorld[3];
	FromWorld(m_pivot, pivotInCam);
	RotateRight(dy);
	ToWorld(pivotInCam, pivotInWorld);
	float *pPos = GetPos();
	pPos[0] -= pivotInWorld[0];
	pPos[1] -= pivotInWorld[1];
	pPos[2] -= pivotInWorld[2];
	RotateAroundZ(pPos, dx);
	RotateAroundZ(GetUp(), dx);
	RotateAroundZ(GetRight(), dx);
	RotateAroundZ(GetDir(), dx);
	pPos[0] += m_pivot[0];
	pPos[1] += m_pivot[1];
	pPos[2] += m_pivot[2];
}

void Camera::MoveInPivotDir(float dd) 
{
	float *pPos = GetPos();
	pPos[0] = (m_pivot[0] - pPos[0])*dd + pPos[0];
	pPos[1] = (m_pivot[1] - pPos[1])*dd + pPos[1];
	pPos[2] = (m_pivot[2] - pPos[2])*dd + pPos[2];
}

void Camera::ShiftPivot(float dx, float dy) 
{
	float sh[3],dd[3],len,prd,near = 2.0f;
	float scm = (m_screenX < m_screenY) ? m_screenX : m_screenY;
	float *pR = GetRight();
	float *pU = GetUp();
	float *pC = GetPos();
	float *pP = GetPivot();
	dd[0] = pP[0] - pC[0];
	dd[1] = pP[1] - pC[1];
	dd[2] = pP[2] - pC[2];
	len = sqrtf(dd[0] * dd[0] + dd[1] * dd[1] * dd[2] * dd[2]);
	prd = 1.0f;// 0.5f *(scm * near) / (len + near);

	sh[0] = (pR[0] * dx + pU[0] * dy) * prd;
	sh[1] = (pR[1] * dx + pU[1] * dy) * prd;
	sh[2] = (pR[2] * dx + pU[2] * dy) * prd;
	pC[0] += sh[0];
	pC[1] += sh[1];
	pC[2] += sh[2];
	pP[0] += sh[0];
	pP[1] += sh[1];
	pP[2] += sh[2];
}

void Camera::SetPivotCamera(float teta, float fi, float dist) 
{
	float rr[3];
	rr[0] = sin(teta) * cos(fi);
	rr[1] = sin(teta) * sin(fi);
	rr[2] = cos(teta);
	float *pPos = GetPos();
	pPos[0] = m_pivot[0] + rr[0] * dist;
	pPos[1] = m_pivot[1] + rr[1] * dist;
	pPos[2] = m_pivot[2] + rr[2] * dist;
	float *pDir = GetDir();
	pDir[0] = -rr[0];
	pDir[1] = -rr[1];
	pDir[2] = -rr[2];
	float *pR = GetRight();
	float lenR = sqrt(pDir[0] * pDir[0] + pDir[1] * pDir[1]);
	pR[0] = -pDir[1] / lenR;
	pR[1] =  pDir[0] / lenR;
	pR[2] = 0.0f;
	float *pU = GetUp();
	pU[0] =  pDir[1] * pR[2] - pDir[2] * pR[1];
	pU[1] = -pDir[0] * pR[2] + pDir[2] * pR[0];
	pU[2] =  pDir[0] * pR[1] - pDir[1] * pR[0];
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

void Camera::Print4x4(float *pDat)
{
	printf("matrix\n");
	float *pT = pDat;
	for (int i = 0; i < 4; i++, pT += 4)
	{
		printf(" %f %f %f %f \n", pT[0], pT[1], pT[2], pT[3]);
	}
}

/*
 W2V matrix memry layout

 Rx Ux Dx 0
 Ry Uy Dy 0
 Rz Uz Dz 0
 Tx Ty Tz 0
*/

void Camera::ConvertTo4x4(float *pOut)
{
	memset(pOut, 0, 16 * sizeof(float));
	float *pC = GetPos();
	float *pR = GetRight();
	float *pU = GetUp();
	float *pD = GetDir();
	for(int i = 0,k=0; i < 3; i++,k+=4) {
		pOut[k + 0] = pR[i];
		pOut[k + 1] = pU[i];
		pOut[k + 2] = pD[i];
		pOut[k + 3] = 0.0f;
	}
	// Tx,Ty,Tz 
	pOut[12] = -(pC[0] * pR[0] + pC[1] * pR[1] + pC[2] * pR[2]);
	pOut[13] = -(pC[0] * pU[0] + pC[1] * pU[1] + pC[2] * pU[2]);
	pOut[14] = -(pC[0] * pD[0] + pC[1] * pD[1] + pC[2] * pD[2]);
}
