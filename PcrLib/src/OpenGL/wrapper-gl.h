#ifndef WRAPPER_GL_H
#define WRAPPER_GL_H

#ifdef _VSBUILD // visual studio build
#include "GL/glew.h"
#elif QT_BUILD  // qt build

#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QOpenGLFunctions_4_1_Core>

extern QOpenGLExtraFunctions *pGlExtra;
extern QOpenGLFunctions_4_1_Core *pGl4;

#define glCreateProgram()                             pGl4->glCreateProgram()
#define glCreateShader(_var)                          pGl4->glCreateShader(_var)
#define glShaderSource(_a, _b, _c, _d)                pGl4->glShaderSource(_a, _b, _c, _d)
#define glCompileShader(_a)                           pGl4->glCompileShader(_a)
#define glGetShaderiv(_a, _b, _c)                     pGl4->glGetShaderiv(_a, _b, _c)
#define glGetShaderInfoLog(_a, _b, _c, _d)            pGl4->glGetShaderInfoLog(_a, _b, _c, _d)
#define glDeleteShader(_a)                            pGl4->glDeleteShader(_a)
#define glAttachShader(_a, _b)                        pGl4->glAttachShader(_a, _b)
#define glLinkProgram(_a)                             pGl4->glLinkProgram(_a)
#define glGetProgramiv(_a, _b, _c)                    pGl4->glGetProgramiv(_a, _b, _c)
#define glUseProgram(_a)                              pGl4->glUseProgram(_a)
#define glDeleteProgram(_a)                           pGl4->glDeleteProgram(_a)
#define glGenBuffers(_a, _b)                          pGl4->glGenBuffers(_a, _b)
#define glBindBuffer(_a, _b)                          pGl4->glBindBuffer(_a, _b)
#define glBufferData(_a, _b, _c, _d)                  pGl4->glBufferData(_a, _b, _c, _d)
#define glBindBufferBase(_a, _b, _c)                  pGl4->glBindBufferBase(_a, _b, _c)
#define glDeleteBuffers(_a, _b)                       pGl4->glDeleteBuffers(_a, _b)
#define glGetBufferSubData(_a, _b, _c, _d)            pGl4->glGetBufferSubData(_a, _b, _c, _d)
#define glBindAttribLocation(_a, _b ,_c)              pGl4->glBindAttribLocation(_a, _b ,_c)
#define glGetUniformLocation(_a,_b)                   pGl4->glGetUniformLocation(_a,_b)
#define glEnableVertexAttribArray(_a)                 pGl4->glEnableVertexAttribArray(_a)
#define glVertexAttribPointer(_a, _b, _c, _d, _e, _f) pGl4->glVertexAttribPointer(_a, _b, _c, _d, _e, _f)
#define glUniform1i(_a, _b)                           pGl4->glUniform1i(_a, _b)
#define glDisableVertexAttribArray(_a)                pGl4->glDisableVertexAttribArray(_a)

#define glMemoryBarrier(_a)                           pGlExtra->glMemoryBarrier(_a)
#define glDispatchCompute(_a, _b, _c)                 pGlExtra->glDispatchCompute(_a, _b, _c)

#endif //#elif QT_BUILD

#endif


