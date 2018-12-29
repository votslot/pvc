
#ifndef _CAMERA_H
#define _CAMERA_H

class Camera {
	float m_data[32];
	float m_pivot[3];
	float m_screenX, m_screenY, m_devSize, m_focus;
public:
	Camera();
	static Camera *GetCamera();	
	void RotateRight(float ang);
	void RotateUp(float ang);
	void RotateDir(float ang);
	void FromWorld(float *pWorldIn, float *pInCamOut);
	void ToWorld(float *pCamIn, float *pWorldOut);
	void SetPivotCamera(float teta, float fi, float dist);
	void RotateAroundPivot(float dx, float dy);
	void MoveInPivotDir(float dd);
	void ShiftPivot(float dx, float dy);
	void ConvertTo4x4(float *pOut);
	void SetScreenPixSize(int sx, int sy);
	float *GetUp();
	float *GetRight();
	float *GetDir();
	float *GetPos();
	float *GetPivot();
	int GetScreenX();
	int GetScreenY();
	void Print4x4(float *pDat);
};

#endif