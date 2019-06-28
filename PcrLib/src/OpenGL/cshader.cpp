

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
		}

		virtual int GetSX()
		{
			return m_szx;
		}
		virtual int GetSY() 
		{
			return m_szy;
		}
		virtual int GetSZ() 
		{
			return m_szz;
		}

		virtual void execute(int x, int y, int z, std::initializer_list <ICBuffer*> inputs) 
		{
		
		}
	};

	ICShader *ICShader::GetNew() 
	{
		return new CShader();
	}

}



