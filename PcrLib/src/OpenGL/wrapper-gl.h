#ifndef WRAPPER_GL_H
#define WRAPPER_GL_H

#ifdef _VSBUILD // visual studio build
#include "GL/glew.h"
#elif QT_BUILD  // qt uild
#include <QtGui/QOpenGLFunctions>
extern QOpenGLFunctions *pGLFunc;
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

#endif


