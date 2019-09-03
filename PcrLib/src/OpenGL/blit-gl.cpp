#include <iostream>
#include "wrapper-gl.h"

void SomeFunc() {
}

//#if (USE_GL_BLIT==1)
#if 1
namespace pcrlib
{
	static GLuint programObject;
	static GLuint vertexbuffer;
	static GLuint colorbuffer;
	static GLuint uvbuffer;
	static GLint  screenXLocation;
	static GLint  screenYLocation;
	static bool sBlitIsInit = false;

	const std::string vert_shader_source =
	R""(
	#version 430 core
	layout(location = 0) in vec3 vertexPosition;
	layout(location = 1) in vec3 vertexColor;
	layout(location = 2) in vec2 vertexUV;
	out vec3 fragmentColor;
	out vec2 fragmentUV;
	uniform highp mat4 matrix;
	void main()                  
	{  
		gl_Position.xyz = vertexPosition;
		gl_Position.w = 1.0;
		fragmentColor = vertexColor;
		fragmentUV = vertexUV;
	}                            
	)"";

	const std::string frag_shader_source =
	R""(
	#version 430 core
	in vec3 fragmentColor;
	in vec2 fragmentUV;
	out vec3 color;
	layout(std430,binding = 0) buffer inz  {  uint zMap[]; };   
	uniform  int screenSizeX;
	uniform  int screenSizeY;
	precision mediump float;
	void main()                                  
	{  
		uint xx = uint(fragmentUV.x * float(screenSizeX));
		uint yy = uint(fragmentUV.y * float(screenSizeY));
		uint shift = xx + yy* uint(screenSizeX);
		uint cl = zMap[shift];
		float rt = cl & 0x000000FF;
		float gt = (cl & 0x0000FF00)>>8;
		float bt = (cl & 0x00FF0000)>>16;
		color = vec3(rt/255.0,gt/255.0,bt/255.0)*fragmentColor;
	}   
	)"";

    /*
	static GLuint  GetSrcBuff() 
	{
		return 0;
	}
    */

    /*
    static int err_check()
    {
            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR)
            {
                 return 1;
            }
            return 0;
    }
    */

    static void initQuadVerts()
	{
		static const GLfloat g_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
		};

		static const GLfloat g_color_buffer_data[] = {
			1.0,   1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0,   1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
		};

		static const GLfloat g_uv_buffer_data[] = {
			1.0,   0.0f,
			0.0f,  1.0f,
			0.0f,  0.0f,
			1.0,   0.0f,
			0.0f,  1.0f,
			1.0f,  1.0f,
		};

        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &colorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

    static GLuint LoadShader(GLenum type, const GLchar *shaderSrc)
	{
		GLuint shader;
		GLint compiled;
		// Create the shader object
		shader = glCreateShader(type);

		if (shader == 0)
		{
			std::cerr << "Could not create OpenGL shader " << std::endl;
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

	int InitGLBlit()
	{
		GLuint vertexShader;
		GLuint fragmentShader;
		GLint linked;

		vertexShader = LoadShader(GL_VERTEX_SHADER, vert_shader_source.c_str());
		if (vertexShader == 0) 
		{
			return 1;
		}

		fragmentShader = LoadShader(GL_FRAGMENT_SHADER, frag_shader_source.c_str());
		if (fragmentShader == 0)
		{
			return 1;
		}

		// Link the vertex and fragment shader into a shader program
		GLuint shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		// Bind vPosition to attribute 0
		glBindAttribLocation(shaderProgram, 0, "vPosition");
		// Link the program
		glLinkProgram(shaderProgram);
		programObject = shaderProgram;

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			return 1;
		}
		// Check the link status
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			return 1;
		}
        initQuadVerts();
        screenXLocation = glGetUniformLocation(programObject, "screenSizeX");
		screenYLocation = glGetUniformLocation(programObject, "screenSizeY");
		sBlitIsInit = true;
		return 0;
	}


	extern GLuint GetSrcBuff();

	void doGLBlit(GLint win_width, GLint win_height, GLuint destBuffer)
	{
		if (!sBlitIsInit)
		{
			return;
		}
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, win_width, win_height);
		glUseProgram(programObject);
		// verts layout(location = 0)
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//color layout(location = 1)
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//uvlayout(location = 2)
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, destBuffer);
		glUniform1i(screenXLocation, win_width);
		glUniform1i(screenYLocation, win_height);


		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, GetClutData());
		//draw
		glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		//check err
		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			printf("Error %x\n", err);
		}
	}
}//namespace pcrlib
#endif



