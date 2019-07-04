
#ifndef _ICOMPUTE_H_
#define _ICOMPUTE_H_

#include <algorithm>

namespace pcrlib 
{
	typedef void (*ICErrorHandler)(const char *pMessage);
	ICErrorHandler setICErrorHandler(ICErrorHandler errh);

	class ICBuffer 
	{
	public:
		virtual void allocate(unsigned int sizeInBytes) = 0;
		virtual unsigned int getMaxSizeInBytes() = 0;
		virtual void setData(void *pD, unsigned int sizeInBytes) = 0;
		virtual void getData(unsigned int sizeInBytes, void *pOut) = 0;
		virtual void blit(int destW, int destH) = 0;
	};

	ICBuffer * createICBuffer();
	void releaseICBuffer(ICBuffer **ppBuffer);

	class ICShader 
	{
	public:
		virtual void initFromSource(const char *pSrc) = 0;
		virtual int getSX() = 0;
		virtual int getSY() = 0;
		virtual int getSZ() = 0;
		virtual void execute(int x, int y, int z, std::initializer_list <ICBuffer*> inputs) = 0;
	};

	ICShader * createICShader();
	void releaseICShader(ICShader **ppShader);

	
}


#endif