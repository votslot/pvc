

#include "../cshader.h"

#ifdef _VSBUILD // visual studio build
#include "GL/glew.h"
#elif QT_BUILD
//#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
extern QOpenGLFunctions *pGLFunc ;
#define glCreateProgram()                  pGLFunc->glCreateProgram()
#define glCreateShader(_var)               pGLFunc->glCreateShader(_var)
#define glShaderSource(_a, _b, _c, _d)     pGLFunc->glShaderSource(_a, _b, _c, _d)
#define glCompileShader(_a)                pGLFunc->glCompileShader(_a)
#define glGetShaderiv(_a, _b, _c)          pGLFunc->glGetShaderiv(_a, _b, _c)
#define glGetShaderInfoLog(_a, _b, _c, _d) pGLFunc->glGetShaderInfoLog(_a, _b, _c, _d)
#define glDeleteShader(_a)                 pGLFunc->glDeleteShader(_a)
#define glAttachShader(_a, _b)             pGLFunc->glAttachShader(_a, _b)
#define glLinkProgram(_a)                  pGLFunc->glLinkProgram(_a)
#define glGetProgramiv(_a, _b, _c)         pGLFunc->glGetProgramiv(_a, _b, _c)
#endif



namespace pcrlib 
{

	static void errCheck()
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			printf("Buffer operation error %d %x \n", err, err);
		}
	}

	class CShader : public ICShader
	{
		GLuint m_program;
		int m_szx, m_szy, m_szz;

		GLuint LoadShader(GLenum type, const GLchar *shaderSrc)
		{
			GLuint shader;
			GLint compiled;
			// Create the shader object
			shader = glCreateShader(type);

			if (shader == 0)
			{
				//std::cerr << "Could not create OpenGL shader " << std::endl;
				return 0;
			}

			// Load the shader source
			glShaderSource(shader, 1, &shaderSrc, NULL);

			// Compile the shader
			glCompileShader(shader);

			// Check the compile status
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

			if (!compiled)
			{
				GLint infoLen = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

				if (infoLen > 1)
				{
					char* infoLog = (char*)malloc(sizeof(char) * infoLen);
					glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
					printf("ERROR\n %s\n", infoLog);
					free(infoLog);
				}
				glDeleteShader(shader);
				return 0;
			}
			return shader;
		}

	public:

		CShader() 
		{
			m_szx = m_szy = m_szz = 0;
			m_program = 0;
		}

		void initFromSource(const char *pSrc) 
		{
			GLuint csShader = LoadShader(GL_COMPUTE_SHADER, pSrc);
			m_program = glCreateProgram();                          
			glAttachShader(m_program, csShader);                     
			glLinkProgram(m_program);                               
			int localWorkGroupSize[3];
			glGetProgramiv(m_program, GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
			m_szx = localWorkGroupSize[0];
			m_szy = localWorkGroupSize[1];
            m_szz = localWorkGroupSize[2];
			errCheck();
		}

		int GetSX()
		{
			return m_szx;
		}
		int GetSY() 
		{
			return m_szy;
		}
		int GetSZ() 
		{
			return m_szz;
		}

		void execute(int x, int y, int z, std::initializer_list <ICBuffer*> inputs) 
		{
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glUseProgram(m_program);
			errCheck();

			int i = 0;
			for (ICBuffer* bf : inputs) 
			{
				bf->bind(i);
				i++;
			}

			glDispatchCompute(x, y, z);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glUseProgram(0);
			errCheck();
		}
	};// class CSShader


	ICShader *ICShader::GetNew() 
	{
		return new CShader();
	}




	class CSBuffer :public ICBuffer
	{
		GLuint m_buffer;
	public:
		CSBuffer()
		{
			m_buffer = 0;
			init();
		}

		void init()
		{
			glGenBuffers(1, &m_buffer);
			errCheck();
		}

		unsigned int getMaxSizeInBytes()
		{
			GLint size;
			glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
			errCheck();
			return size;
		}

		void setData(void *pD, unsigned int sizeInBytes)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, pD, GL_STATIC_READ);
			errCheck();
		}

		void allocate(unsigned int sizeInBytes)
		{
			GLint maxtb = 0;
			glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxtb);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, NULL, GL_DYNAMIC_COPY);
			errCheck();
		}

		void bind(int n) 
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, n, m_buffer);
			errCheck();
		}
	
		void getData(unsigned int sizeInBytes, void *pOut)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeInBytes, pOut);
			errCheck();
		}

		void delBuffer()
		{
			glDeleteBuffers(1, &m_buffer);
			errCheck();
		}
	};//class CSBuffer

	ICBuffer *ICBuffer::GetNew()
	{
		return new CSBuffer();
	}

	void ICBuffer::release(ICBuffer **pBuff)
	{
		(*pBuff)->delBuffer();
		delete *pBuff;
		*pBuff = NULL;
	}

}//namespace pcrlib



