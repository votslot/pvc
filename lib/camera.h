
#ifndef _CAMERA_H
#define _CAMERA_H

class Camera {
	float m_data[32];
	float m_pivot[3];
	float m_screenX, m_screenY, m_devSize, m_focus;
public:
	Camera();
	static Camera *GetCamera();
	float *GetData();
	void Build(float pos[3], float dir[3]);
	void Build(float pos[3], float dir[3], float up[3]);
	void RotateRight(float ang);
	void RotateUp(float ang);
	void FromWorld(float *pWorldIn, float *pInCamOut);
	void ToWorld(float *pCamIn, float *pWorldOut);
	void ConvertTo4x4(float *pOut);
	void Print4x4(float *pDat);
	float *GetUp();
	float *GetRight();
	float *GetDir();
	float *GetPos();
	float *GetPivot();
	void SetScreenPixSize(int sx, int sy);
};

#endif