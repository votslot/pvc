
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
#include "../app/app-events.h"
#include "../app/app-camera.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 512
SDL_Window* gWindow = NULL;
SDL_GLContext gGlContext;
static pcrapp::IAppEvents *sApp = NULL;

extern void AddUI(HWND hwnd);
static void ErrHnd(const char *pMsg) 
{
	MessageBoxA(NULL, pMsg, "Error", MB_OK);
	exit(0);
}

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

static bool sHasEvent = true;

void OnMouseDownEvent(SDL_MouseButtonEvent *pMouseDown)
{
	if (sApp)
	{
		sApp->mouseDownEvent(pMouseDown->x, pMouseDown->y, pMouseDown->button == 1, pMouseDown->button == 3);
	}
	sHasEvent = true;
}

void OnMouseUpEvent(SDL_MouseButtonEvent *pMouseUp)
{
	if (sApp) 
	{
		sApp->mouseUpEvent(pMouseUp->button == 1, pMouseUp->button == 3);
	}
	sHasEvent = true;
}

void OnMouseMoveEvent(SDL_MouseMotionEvent *pMotion)
{
	if (sApp) 
	{
		sApp->mouseMoveEvent(pMotion->x, pMotion->y);
	}
	sHasEvent = true;
}

void OnMouseWhellEvevt(SDL_MouseWheelEvent *pWheel)
{
	if (sApp)
	{
		sApp->mouseWhellEvevt(pWheel->y);
	}
	sHasEvent = true;
}

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

	sApp  = pcrapp::IAppEvents::getAppEvents();
	sApp->init();

	int sw = SCREEN_WIDTH;
	int sh = SCREEN_HEIGHT;
	bool quit = false; 
	SDL_Event sdlEvent;
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
			sApp->paintEvent(sw, sh);
			SDL_GL_SwapWindow(gWindow);
			OnEndLoop();
			sHasEvent = false;
		}
	}
	int t = GL_SYNC_GPU_COMMANDS_COMPLETE;
	// clear resource
	if(sApp) sApp->exitEvent();
	close();
	return 0;
}




