
#include <iostream>
#include "GL\\glew.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#undef main
#include <SDL_opengl.h>

#include "windows.h"
#include <chrono>
#include "CommCtrl.h"
#include <SDL_syswm.h>
#include "compute.h"
#include "camera.h"
#include "../winui/ui.h"
#include "../pcloud/pcloud.h"

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512
SDL_Window* gWindow = NULL;
SDL_GLContext gGlContext;

extern GLuint  InitQuad(int sw, int sh );
extern void Draw(GLint width, GLint height, GLuint texDest);
extern void initQuadVerts();
extern void AddUI(HWND hwnd);
//static void InitBindings();

void SetOpenGLVersion()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
}


static std::chrono::high_resolution_clock::time_point sStartTime;
static int sDiff = 0;
static int sNtimes = 256;
static int sleepTime = 0;
void OnStartLoop() 
{
	sStartTime = std::chrono::high_resolution_clock::now();
  // Sleep(16);
}

void OnEndLoop() 
{
	std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
	int df = (int)std::chrono::duration_cast<std::chrono::microseconds>(finish - sStartTime).count();
	sDiff += df;
	sNtimes--;
	if (sNtimes <= 0) {
		std::cout << ((float)sDiff/256000.0f) << "ms\n";
		sNtimes = 256;
		sDiff = 0;
	}
	
}

#if 0
void OnKeyEvent(int key)
{
	static float  dist = 600.0f;
	float step = 0.01f * 256.0f,sig =  0.0f;
	Camera *pCam = Camera::GetCamera();
	switch (key) 
	{
		case SDLK_RIGHT:
			pCam->RotateUp(0.01f);
		break;
		case SDLK_LEFT:
			pCam->RotateUp(-0.01f);
		break;
		case SDLK_DOWN:
			pCam->RotateRight(0.01f);
		break;
		case SDLK_UP:
			pCam->RotateRight(-0.01f);
		break;
		case SDLK_PAGEUP:
			dist += step;
			sig = 1.0f;
		break;
		case SDLK_PAGEDOWN:
			dist -= step;
			sig = -1.0f;
		break;
	}

	float *pPos = pCam->GetPos();
	float *pDir = pCam->GetDir();
	
	pPos[0] = -pDir[0] * dist;
	pPos[1] = -pDir[1] * dist;
	pPos[2] = -pDir[2] * dist;
	
	/*
	pPos[0] += pDir[0] * step * sig;
	pPos[1] += pDir[1] * step * sig;
	pPos[2] += pDir[2] * step * sig;
	*/
	
}
#endif

/*-----------------------------------------------------------------*/
static int sMouseXDown = 0;
static int sMouseYDown = 0;
static bool sLeftMouseDown = false;
static bool sRightMouseDown = false;
static bool sHasEvent = true;
void OnMouseDownEvent(SDL_MouseButtonEvent *pMouseDown)
{
	sMouseXDown = pMouseDown->x;
	sMouseYDown = pMouseDown->y;
	if (pMouseDown->button == 1) 
	{
		sLeftMouseDown = true;
	}
	else if (pMouseDown->button == 3) 
	{
		sRightMouseDown = true;
	}
	sHasEvent = true;
}

void OnMouseUpEvent(SDL_MouseButtonEvent *pMouseUp)
{
	if (pMouseUp->button == 1)
	{
		sLeftMouseDown = false;
	}
	else if (pMouseUp->button == 3)
	{
		sRightMouseDown = false;
	}
	sHasEvent = true;
}

void OnMouseMoveEvent(SDL_MouseMotionEvent *pMotion)
{
	Camera *pCam = Camera::GetCamera();
	float *pivot = pCam->GetPivot();
	int dx = pMotion->x - sMouseXDown; 
	int dy = pMotion->y - sMouseYDown;
	if (sLeftMouseDown)
	{
		pCam->RotateAroundPivot((float)dx*0.01f, (float)dy*0.01f);
	}
	else if (sRightMouseDown) 
	{
		pCam->ShiftPivot((float)dx,(float)dy);
	}
	sMouseXDown = pMotion->x;
	sMouseYDown = pMotion->y;
	sHasEvent = true;
}

void OnMouseWhellEvevt(SDL_MouseWheelEvent *pWheel)
{
	if (pWheel->y == 0) 
	{
		return;
	}
	Camera *pCam = Camera::GetCamera();
	float prd = 1.0f;
	float shift = (pWheel->y > 0) ? prd : -prd;
	pCam->MoveInPivotDir(shift);
	sHasEvent = true;
}
/*---------------------------------------------------------------------*/

void close()
{
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
}

int SdlEntryPoint()
{
	// initialize sdl
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL cannot init with error " << SDL_GetError() << std::endl;
		return -1;
	}
	//SetOpenGLVersion();

	// create window
	gWindow = SDL_CreateWindow("OPV", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL |SDL_WINDOW_RESIZABLE| SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		std::cout << "Cannot create window with error " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_GL_SetSwapInterval(1);

	SDL_Renderer * renderer = SDL_CreateRenderer(gWindow, -1, 0);
	if (renderer == NULL) 
	{
		std::cout << "Cannot create renderer " << SDL_GetError() << std::endl;
		return -1;
	}

	// create opengl context
	gGlContext = SDL_GL_CreateContext(gWindow);
	if (gGlContext == NULL)
	{ 
		std::cout << "Cannot create OpenGL context with error " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(gWindow, &wmInfo);
	AddUI(wmInfo.info.win.window);
	 
	// Init Glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: %s\n", glewGetErrorString(err));
	}
	
	GLint maxtb = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &maxtb);
	glGetIntegerv(GL_MINOR_VERSION, &maxtb);

	
	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxtb);
	printf("GL_MAX_TEXTURE_BUFFER_SIZE= %d MB \n", maxtb/(1024*1024));
	glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &maxtb);
	printf("GL_MAX_TEXTURE_SIZE= %d\n", maxtb);
	//glGet(GL_MAX_COMPUTE_WORK_GROUP_COUNT, &maxtb);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxtb); // 0 - x
	printf("GL_MAX_COMPUTE_WORK_GROUP_COUNT= %d\n", maxtb);
	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxtb);
	printf("GL_MAX_TEXTURE_BUFFER_SIZE= %d\n", maxtb);

	GLuint texdest = ComputeInit(SCREEN_WIDTH, SCREEN_HEIGHT);
	InitQuad(SCREEN_WIDTH, SCREEN_HEIGHT);

	int sw = SCREEN_WIDTH;
	int sh = SCREEN_HEIGHT;
	bool quit = false; 
	SDL_Event sdlEvent;
	Camera *pCam = Camera::GetCamera();
	pCam->SetScreenPixSize(sw, sh);
	while (!quit)
	{
		while (SDL_PollEvent(&sdlEvent) != 0)
		{
			switch (sdlEvent.type) 
			{
				case SDL_KEYDOWN:
					//OnKeyEvent(sdlEvent.key.keysym.sym);
					sHasEvent = true;
				break;
				case SDL_MOUSEBUTTONDOWN:
					OnMouseDownEvent(&sdlEvent.button);
				break;
				case  SDL_MOUSEBUTTONUP:
					OnMouseUpEvent(&sdlEvent.button);
				break;
				case  SDL_MOUSEMOTION:
					OnMouseMoveEvent(&sdlEvent.motion);
				break;
				case SDL_MOUSEWHEEL:
					OnMouseWhellEvevt(&sdlEvent.wheel);
				break;
				case SDL_WINDOWEVENT:
					if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) {
						sw = sdlEvent.window.data1;
						sh = sdlEvent.window.data2;
						pCam->SetScreenPixSize(sw, sh);
					}
				break;
				case SDL_QUIT:
					quit = true;
				break;
			}
		}
		if (sHasEvent)
		{
			OnStartLoop();
			// render to buffer
			ComputeRun(sw, sh);
			// render to screen
			Draw(sw, sh, texdest);
			// swap 
			SDL_GL_SwapWindow(gWindow);
			OnEndLoop();
			sHasEvent = false;
		}
	}
	int t = GL_SYNC_GPU_COMMANDS_COMPLETE;
	// clear resource
	close();
	return 0;
}




