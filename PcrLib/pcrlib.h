
#ifndef _PCRLIB_H_
#define _PCRLIB_H_



namespace pcrlib 
{
	typedef void(*PcrErrorHandler)(const char *pMessage);

	struct Camera {
		float pos[3];   //position x,y,z
		float lookAt[3];
		float up[3];
		float zNear;
		float zFar;
		float fovInRadians;
		bool isPerspective;
	};

	struct BoundBox {
		float xMin;
		float xMax;
		float yMin;
		float yMax;
		float zMin;
		float zMax;
	};

    class  IPcrLib
	{
	public:

		virtual void startAddPoints() = 0;
		virtual int  addPoint(float x, float y, float z, float w) = 0;
		virtual void doneAddPoints() = 0;
		virtual BoundBox getBoundBox() = 0;
		virtual int render(const Camera &cam, int destWidth, int destHeight) = 0;

		virtual int runTest() = 0;
		static PcrErrorHandler setErrHandler(PcrErrorHandler errh);
		static IPcrLib* Init();
		static IPcrLib* GetInstance();
	};
}


#endif