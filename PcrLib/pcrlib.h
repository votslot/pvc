
#ifndef _PCRLIB_H_
#define _PCRLIB_H_

namespace pcrlib 
{
	typedef void(*PcrErrorHandler)(const char *pMessage);

	struct LibCallback
	{
		static LibCallback *m_cb;
		virtual void error(const char *pMsh);
		virtual void message(const char *pMsh);
		virtual void *memAlloc(size_t sz);
		virtual void memFree(void *ptr);
	};
	
	struct Camera {
		float pos[3];   //position x,y,z
		float lookAt[3];
		float up[3];
		float zNear;
		float zFar;
		float fovInGrads;
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

	enum ColorModel {
		Color_model_xyz = 0,
		Colos_model_rgb = 1,
		Color_model_intencity = 2
	};

	struct RenderParams 
	{
		ColorModel cm;
		unsigned int pointSize;
	};


    class  IPcrLib 
	{
	public:

		virtual void startAddPoints() = 0;
		virtual int  addPoint(float x, float y, float z, unsigned int w) = 0;
		virtual void doneAddPoints() = 0;
		virtual BoundBox getBoundBox() = 0;
		
		virtual int render(const Camera &cam, int destWidth, int destHeight, const RenderParams &rp) = 0;

		virtual void renderText(const char *pTxt, unsigned int x, unsigned int y, unsigned int color) = 0;
		virtual int verify() = 0;
		static IPcrLib* init(LibCallback *pCb = NULL);
		static void release(IPcrLib **ppLib);
	};
}


#endif