#ifndef _COMPUTE_H
#define _COMPUTE_H

#include "../PcrLib/pcrlib.h"

unsigned int ComputeInit(int sw, int sh);
void ComputeRun(const pcrlib::Camera &pcrCam,int sw, int sh);
void Compute_AddPoint(float x, float y, float z, float w);
void Compute_DoneAddPoints();

#endif