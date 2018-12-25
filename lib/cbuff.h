#include <iostream>
#include "GL/glew.h"

class BaseBuffer 
{
	static int m_globCount;
public:
	GLuint gb;
	unsigned int sz;
	int m_ndx;
	BaseBuffer();
	virtual void bind(int bi) const = 0;
	static void checkError();
};

class SSBBuffer :public BaseBuffer {
public:
	void init();
	void bind(int bi) const;
	void setData(void *pD, unsigned int sizeInBytes);
	void allocate(unsigned int sizeInBytes);
	void getData(unsigned int sizeInBytes, void *pOut);
};

class TexR32f :public BaseBuffer {
public:
	GLuint tex;
	void init(int width, int height);
	void bind(int bi) const;
};

class TexU32 :public BaseBuffer {
public:
	GLuint tex;
	void init(int width, int height);
	void bind(int bi) const;
};

class TBOBuffer :public BaseBuffer {
	GLuint tex;
public:
	void init();
	void bind(int bi) const;
	void setData(void *pD, unsigned int sizeInBytes);
};

class CSShader {
	static const int m_maxBuffs = 16;
	int m_bindTable[m_maxBuffs];
	
public:
	GLuint m_program;
	int m_szx, m_szy, m_szz;
	CSShader();
	void initFromSource(const char *pSrc);
	void setBufferBinding(const BaseBuffer *pBuff, int bIndex);
	void bindBuffer(const BaseBuffer *pBuff);
};