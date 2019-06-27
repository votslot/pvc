
#ifndef _PCRLIB_H_
#define _PCRLIB_H_



namespace pcrlib 
{
    class  IPcrLib
	{
	public:
		virtual void runTest() = 0;
		static IPcrLib* Init();
	};
}


#endif