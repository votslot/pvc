

#include "..\cshader.h"
#include "GL/glew.h"

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


