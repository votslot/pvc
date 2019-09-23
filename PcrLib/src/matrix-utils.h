#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

namespace pcrlib
{
	void GetProjectionMat4x4(float screenX, float screenY, float zNear, float zFar,float fov,
		const float *m_U, const float *m_D,  const float *m_P,
		float *pOut);

	void GetVew2World4x4(const float *m_U, const float *m_D, const float *m_P, float *pOut);
}

#endif