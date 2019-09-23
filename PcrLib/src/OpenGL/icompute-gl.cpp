

#include "../icompute.h"
#include "wrapper-gl.h"
#include <string>


namespace pcrlib 
{
    extern void doGLBlit(GLint win_width, GLint win_height, GLuint destBuffer);
	class CSBuffer;

	static void errCheck(ICErrorCallBack *cb)
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			cb->error(std::string("OpenGL operation error " + std::to_string(err) + "\n").c_str());
		}
	}

    // CSBuffer declaration
	ICErrorCallBack*  ICBuffer::m_err = NULL;
	class CSBuffer :public ICBuffer, ICErrorCallBack
	{
		GLuint m_buffer;
		unsigned int m_size;

		friend class CShader;
	public:
		CSBuffer() : m_buffer(0), m_size(0)
		{
			init();
		}

		~CSBuffer() {}
		void error(const char *pMsg) {
			if (m_err) m_err->error(pMsg);
		}
		void bind(int n);
		void init();
		unsigned int getMaxSizeInBytes();
		void setData(void *pD, unsigned int sizeInBytes);
		void allocate(unsigned int sizeInBytes);
		void getData(unsigned int sizeInBytes, void *pOut);
		void delBuffer();
		void blit(int destW, int destH);
	};

	// Compute shader
	ICErrorCallBack*  ICShader::m_err = NULL;
	class CShader : public ICShader
	{
		GLuint m_program;
		GLuint m_shader;
		int m_szx, m_szy, m_szz;

		friend CSBuffer;
		GLuint LoadShader(GLenum type, const GLchar *shaderSrc)
		{
			GLuint shader;
			GLint compiled;
			shader = glCreateShader(type);

			if (shader == 0)
			{
				return 0;
			}
			glShaderSource(shader, 1, &shaderSrc, NULL);
			glCompileShader(shader);
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

			if (!compiled)
			{
				GLint infoLen = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

				if (infoLen > 1)
				{
					char* infoLog = (char*)malloc(sizeof(char) * infoLen);
					glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
					if (m_err) m_err->error((std::string("Shader compilation\n") +std::string(infoLog) ).c_str());
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
			m_shader = 0;
		}

		~CShader()
		{
		}

		void initFromSource(const char *pSrc) 
		{
			m_shader = LoadShader(GL_COMPUTE_SHADER, pSrc);
			m_program = glCreateProgram();                          
			glAttachShader(m_program, m_shader);
			glLinkProgram(m_program);                               
			int localWorkGroupSize[3];
			glGetProgramiv(m_program, GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
			m_szx = localWorkGroupSize[0];
			m_szy = localWorkGroupSize[1];
            m_szz = localWorkGroupSize[2];
			errCheck(m_err);
		}

		int getSX()
		{
			return m_szx;
		}
		int getSY() 
		{
			return m_szy;
		}
		int getSZ() 
		{
			return m_szz;
		}

		void execute(int x, int y, int z, std::initializer_list <ICBuffer*> inputs) 
		{
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glUseProgram(m_program);
			errCheck(m_err);

			int i = 0;
			for (ICBuffer* bf : inputs) 
			{
				CSBuffer *pB = static_cast<CSBuffer*>(bf);
				pB->bind(i);
				i++;
			}

			glDispatchCompute(x, y, z);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			glUseProgram(0);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			errCheck(m_err);
		}

		void delShader()
		{
            if(m_program != 0)
				glDeleteProgram(m_program);
            if (m_shader != 0)
				glDeleteShader(m_shader);
			errCheck(m_err);
		}
	};// class CSShader

	ICShader * createICShader()
	{
		return new CShader();
	}

	void releaseICShader(ICShader **ppShader) 
	{
		CShader *pS = static_cast<CShader*>(*ppShader);
		pS->delShader();
		delete pS;
		*ppShader = NULL;
	}
	

	// Compute buffer. OpenGL implementation.
	void CSBuffer::init()
	{
		glGenBuffers(1, &m_buffer);
		errCheck(this);
	}

	unsigned int ICBuffer::getMaxSizeInBytes()
	{
		GLint size;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &size);
		return size;
	}

	void CSBuffer::setData(void *pD, unsigned int sizeInBytes)
	{
		if (sizeInBytes > m_size)
		{
			if (m_err) m_err->error(std::string("CSBuffer::setData () \n Requested size is bigger than allocated.  Alloc size=" + std::to_string(m_size)).c_str());
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, pD, GL_STATIC_READ);
		errCheck(this);
	}

	void CSBuffer::allocate(unsigned int sizeInBytes)
	{
		GLint maxtb = 0;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxtb);
        if (sizeInBytes > (unsigned int)maxtb)
		{
			if (m_err) m_err->error(std::string("Requested size is bigger than max possible.  Alloc size=" + std::to_string(m_size)).c_str());
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, NULL, GL_DYNAMIC_COPY);
		errCheck(this);
		m_size = sizeInBytes;
	}
		
	void CSBuffer::getData(unsigned int sizeInBytes, void *pOut)
	{
		if (sizeInBytes > m_size)
		{
			if(m_err) m_err->error(std::string("Requested size is bigger than allocated.  Alloc size=" + std::to_string(m_size)).c_str());
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeInBytes, pOut);
		errCheck(this);
	}

	void CSBuffer::delBuffer()
	{
		glDeleteBuffers(1, &m_buffer);
		errCheck(this);
	}

	void CSBuffer::bind(int n)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, n, m_buffer);
		errCheck(this);
	}

	void CSBuffer::blit(int destW, int destH)
	{
		doGLBlit(destW, destH, m_buffer);
	}

	ICBuffer * createICBuffer()
	{
		return new CSBuffer();
	}

	void releaseICBuffer(ICBuffer **ppBuffer)
	{
		CSBuffer *pB = static_cast<CSBuffer*>(*ppBuffer);
		pB->delBuffer();
		delete *ppBuffer;
		*ppBuffer = NULL;
	}

}//namespace pcrlib



