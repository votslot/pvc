
#ifndef _CSHADER_H_
#define _CSHADER_H_

#include <algorithm>

namespace pcrlib 
{
	class ICBuffer 
	{
	public:
		virtual void allocate(unsigned int sizeInBytes) = 0;
		virtual void* allocateVram(unsigned int sizeInBytes) = 0;
		virtual void getData(unsigned int sizeInBytes, void *pOut) = 0;
		static ICBuffer *GetNew();
	};

	class ICShader 
	{
		public:
		virtual void initFromSource(const char *pSrc) = 0;
		virtual int GetSX() = 0;
		virtual int GetSY() = 0;
		virtual int GetSZ() = 0;
		virtual void execute(int x, int y, int z, std::initializer_list <ICBuffer*> inputs) = 0;
		static ICShader *GetNew();
	};
}


#endif