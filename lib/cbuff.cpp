#include <iostream>
#include "GL/glew.h"
#include "cbuff.h"

int BaseBuffer::m_globCount = 0;

void BaseBuffer::checkError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		printf("Buffer operation error %d %x \n", err,err);
	}
}

BaseBuffer::BaseBuffer()
{
	m_ndx = m_globCount;
	m_globCount++;
}

void SSBBuffer::bind(int bi) const
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bi, gb);
}

void SSBBuffer::init()
{
	glGenBuffers(1, &gb);
	checkError();
}

unsigned int SSBBuffer::getMaxSizeInBytes()
{
	GLint size;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
	return size;
}

	
void SSBBuffer::setData(void *pD, unsigned int sizeInBytes)
{
	GLint maxtb = 0;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxtb);
	if (sizeInBytes > (unsigned int)maxtb)
	{
			printf("Error buff size \n");
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gb);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, pD, GL_STATIC_READ);
	sz = sizeInBytes;
	checkError();
}

void SSBBuffer::allocate(unsigned int sizeInBytes) 
{
	GLint maxtb = 0;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxtb);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gb);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, NULL, GL_DYNAMIC_COPY);
	sz = sizeInBytes;
	checkError();
}

void* SSBBuffer::allocateVram(unsigned int sizeInBytes)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gb);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, NULL, GL_DYNAMIC_COPY); checkError();
	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	checkError();
	return p;
}

void SSBBuffer::getData(unsigned int sizeInBytes, void *pOut)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gb);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeInBytes, pOut);
	checkError();
}


// TexR32f
void TexR32f::init(int width,int height)
{

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void TexR32f::bind(int bi) const
{
	glBindImageTexture(bi, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
}


// TexU32
void TexU32::init(int width, int height)
{
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	checkError();
}

void TexU32::bind(int bi) const
{
	glBindImageTexture(bi, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	checkError();
}

//another option is to attach the buffer to a buffer texture with glTextureBuffer(), 
//bind the texture to an image unit with glBindImageTexture(), 
//and access the data in the compute shader using imageLoad().

unsigned int tboType = GL_RGBA32F;
void TBOBuffer::init() 
{
	glGenTextures(1, &tex);
	glGenBuffers(1, &gb);
	checkError();
}

void TBOBuffer::bind(int bi) const
{
	glBindImageTexture(bi, tex, 0, GL_FALSE, 0, GL_READ_WRITE, tboType);
	checkError();
}

void TBOBuffer::setData(void *pD, unsigned int sizeInBytes)
{
	GLint maxtb = 0;
	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxtb);
	if (sizeInBytes > (unsigned int)maxtb) 
	{
		checkError();
	}
	//const unsigned int target = GL_TEXTURE_BUFFER_EXT;
	const unsigned int target = GL_TEXTURE_BUFFER;
	glBindBuffer(target,gb); checkError();
	glBufferData(target, sizeInBytes, NULL, GL_DYNAMIC_COPY); checkError();
	unsigned char *pMem = (unsigned char*)glMapBufferRange(target,0,sizeInBytes,GL_MAP_WRITE_BIT); checkError();
	memcpy(pMem, pD, sizeInBytes); checkError();
	glUnmapBuffer(target); checkError();
	glBindTexture(target, tex); checkError();
	glTexBuffer(target, tboType, gb); checkError();
} 

// ---  CSShader ---------------

CSShader::CSShader() 
{
	for (int i = 0; i < m_maxBuffs; i++) m_bindTable[i] = -1;
	m_szx = m_szy = m_szz = 1;
}

void CSShader::initFromSource(const char *pSrc) 
{
	extern GLuint LoadShader(GLenum type, const GLchar *shaderSrc);
	GLuint csShader = LoadShader(GL_COMPUTE_SHADER, pSrc);
	m_program = glCreateProgram();                           BaseBuffer::checkError();
	glAttachShader(m_program,csShader);                      BaseBuffer::checkError();
	glLinkProgram(m_program);                                BaseBuffer::checkError();
	int localWorkGroupSize[3];
	glGetProgramiv(m_program, GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
	m_szx = localWorkGroupSize[0];
	m_szy = localWorkGroupSize[1];
	m_szz = localWorkGroupSize[2];
	BaseBuffer::checkError();
}

void CSShader::setBufferBinding(const BaseBuffer *pBuff, int bIndex)
{
	int n = pBuff->m_ndx;
	m_bindTable[n] = bIndex;
}

void CSShader::bindBuffer(const BaseBuffer *pBuff) 
{
	int n = m_bindTable[pBuff->m_ndx];
	pBuff->bind(n);
}




