#include <iostream>
#include "GL/glew.h"

static GLuint programObject;
static GLuint vertexbuffer;
static GLuint colorbuffer;
static GLuint uvbuffer;
//static GLuint textureID;

#include "..\shaders\vert-and-frag.glsl"

 void initQuadVerts()
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
}

/*
static void initQuadTextureu32(int width, int height)
{
	//unsigned int * data;
	//data = new unsigned int[width*height];
	//for (int i = 0; i < width*height; i++) data[i] = (i&1) ? 0: 0xFF80FF80;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//delete[]data;
}
*/

static void initQuadTexturer32f(int width, int height)
{
	//textureID = ComputeGetDestText1(width, height);
	//textureID = ComputeGetDestText();
	return;
	/*
	float * data ;
	data = new float[width*height];
	int n = 0;
	for (int y = 0; y < height; y++){  
		for (int x = 0; x < width; x++) {
			data[n++] = ((x >> 4) & 1) ? 128128.128f : 0.0f;
		}
	}
	*/
	/*
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//delete[]data;
	*/
}


GLuint LoadShader(GLenum type, const GLchar *shaderSrc)
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

GLuint InitQuad(int sw_, int sh_)
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLint linked;

	// Load and compile shaders
	vertexShader = LoadShader(GL_VERTEX_SHADER, vert_shader_source.c_str());
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, frag_shader_source.c_str());

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
		std::cerr << "Error" << std::endl;
	}
	// Check the link status
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		std::cerr << "Can not link " << std::endl;
	}
	initQuadVerts();

	return 0;
}


extern GLuint GetSrcBuff();
extern GLuint GetParamsBuff();

void Draw(GLint win_width, GLint win_height, GLuint texDest)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, win_width, win_height);
	glUseProgram(programObject);
	// verts layout(location = 0)
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,0,(void*)0);
 	//color layout(location = 1)
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 0,(void*)0);
	//uvlayout(location = 2)
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, texDest);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, GetSrcBuff());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GetParamsBuff());
	//draw
	glDrawArrays(GL_TRIANGLES, 0, 3*2); 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	//check err
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) 
	{
		printf("Error %x\n",err);
	}
}



