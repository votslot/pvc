
#ifndef _PCRLIB_H_
#define _PCRLIB_H_



namespace pcrlib 
{
	struct Camera {
		float posiionX, positionY, posiztionZ;
	};

    class  IPcrLib
	{
	public:
		virtual int runTest() = 0;
		virtual int render(const Camera &cam) = 0;
		virtual int blitGL(int w, int h) = 0;
		static IPcrLib* Init();
	};
}


#endif