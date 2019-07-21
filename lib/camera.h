
#ifndef _CAMERA_H
#define _CAMERA_H

//#include "..\lib\vec3d.h"
#include "../PcrLib/pcrlib.h"

class Camera {
public:
	struct vector3 {
		float v[3];
		vector3() { v[0] = v[1] = v[2] = 0.0f; }
		vector3(float x, float y, float z) { v[0] = x; v[1] = y; v[2]=z; }
		float& operator[] (int i) { return v[i]; }
	};
	vector3 m_P,m_D,m_U,m_R,m_L; // position, dir, up, right, look-at.
	float m_zNear, m_zFar, m_Fov;// m_MaxDimension;
	Camera();
	static Camera *GetCamera();	
	void SetPivotCamera(float teta, float fi, float dist, float px, float py, float pz);
	void RotateAroundPivot(float dx, float dy);
	void MoveInPivotDir(float dd);
	void ShiftPivot(float dx, float dy);
	void BuildPcrCamera(pcrlib::Camera &res);
	float *GetPivot();
};

#endif