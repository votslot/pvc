
#ifndef _PCRLIB_H_
#define _PCRLIB_H_



namespace pcrlib 
{
	typedef void(*PcrErrorHandler)(const char *pMessage);

	struct Camera {
		float posiionX, positionY, posiztionZ;
	};

    class  IPcrLib
	{
	public:
		virtual int runTest() = 0;
		virtual int render(const Camera &cam, int destWidth, int destHeight) = 0;

		static PcrErrorHandler setErrHandler(PcrErrorHandler errh) ;
		static IPcrLib* Init();
	};
}


#endif