
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "app-camera.h"
namespace pcrapp
{
	static AppCamera::vector3 operator - (const AppCamera::vector3 &a, const AppCamera::vector3 &b)
	{
		AppCamera::vector3 ret(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
		return ret;
	}

	static AppCamera::vector3 operator + (const AppCamera::vector3 &a, const AppCamera::vector3 &b)
	{
		AppCamera::vector3 ret(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
		return ret;
	}


	static AppCamera::vector3 operator * (const AppCamera::vector3 &a, float prod)
	{
		AppCamera::vector3 ret(a.v[0] * prod, a.v[1] * prod, a.v[2] * prod);
		return ret;
	}

	static AppCamera::vector3 operator * (float prod, const AppCamera::vector3 &a)
	{
		AppCamera::vector3 ret = a * prod;
		return ret;
	}

	// dot product
	static float operator * (const AppCamera::vector3 &a, const AppCamera::vector3 &b)
	{
		return (a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2]);
	}

	// Cross product
	static AppCamera::vector3 operator ^ (const AppCamera::vector3 &a, const AppCamera::vector3 &b)
	{
		AppCamera::vector3 ret(0.0f, 0.0f, 0.0f);
		ret.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
		ret.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
		ret.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
		return ret;
	}

	// Rotate vector P around axis defined by starting point S and unit vector A ( direction).
	static AppCamera::vector3 rotate(const AppCamera::vector3 &P, const AppCamera::vector3 &S, const AppCamera::vector3 &A, float angle)
	{
		AppCamera::vector3 D = P - S;
		AppCamera::vector3 N0 = (D * A) * A;
		AppCamera::vector3 N1 = D - N0;
		AppCamera::vector3 N2 = N1 ^ A;
		AppCamera::vector3 R = S + N0 + N1 * cosf(angle) + N2 * sinf(angle);
		return R;
	}

	static AppCamera theCamera;

	AppCamera *AppCamera::GetCamera()
	{
		return &theCamera;
	}

	AppCamera::AppCamera()
	{
		m_L.v[0] = m_L.v[1] = m_L.v[2] = 0.0f;
		m_zNear = 2.0f;
		m_zFar = 10000.0f;
		SetPivotCamera(0.0f, 0.0f, 600.0f, 0.0f, 0.0f, 0.0f);
	}

	float *AppCamera::GetPivot() { return  m_L.v; }

	void AppCamera::RotateAroundPivot(float dx, float dy)
	{
		AppCamera::vector3 ZZ(0.0f, 0.0f, 0.0f);
		AppCamera::vector3 A0 = m_R;
		m_P = rotate(m_P, m_L, A0, dy);
		m_D = rotate(m_D, ZZ, A0, dy);
		m_U = rotate(m_U, ZZ, A0, dy);
		m_R = rotate(m_R, ZZ, A0, dy);

		AppCamera::vector3 A1(0.0f, 0.0f, 1.0f);
		m_P = rotate(m_P, m_L, A1, dx);
		m_D = rotate(m_D, ZZ, A1, dx);
		m_U = rotate(m_U, ZZ, A1, dx);
		m_R = rotate(m_R, ZZ, A1, dx);
	}

	void AppCamera::MoveInPivotDir(float dd)
	{
		m_P = m_P + (m_D * dd)*0.02f;
	}

	void AppCamera::ShiftPivot(float dx, float dy)
	{
		float prd = 1.0f;
		AppCamera::vector3 SH;
		SH[0] = (m_R[0] * dx + m_U[0] * dy) * prd;
		SH[1] = (m_R[1] * dx + m_U[1] * dy) * prd;
		SH[2] = (m_R[2] * dx + m_U[2] * dy) * prd;
		m_P = m_P + SH;
		m_L = m_L + SH;
	}

	void AppCamera::SetPivotCamera(float teta, float fi, float dist, float px, float py, float pz)
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

	void AppCamera::BuildPcrCamera(pcrlib::Camera &res)
	{
		res.pos[0] = m_P[0];
		res.pos[1] = m_P[1];
		res.pos[2] = m_P[2];
		res.up[0] = m_U[0];
		res.up[1] = m_U[1];
		res.up[2] = m_U[2];
		res.lookAt[0] = m_D[0];
		res.lookAt[1] = m_D[1];
		res.lookAt[2] = m_D[2];
		res.zNear = m_zNear;
		res.zFar = m_zFar;
	}
}// namespace pcrapp



