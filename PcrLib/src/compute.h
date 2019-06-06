#ifndef _COMPUTE_H
#define _COMPUTE_H

unsigned int ComputeInit(int sw, int sh);
void ComputeRun(int sw, int sh);
void Compute_AddPoint(float x, float y, float z, float w);
void Compute_DoneAddPoints();

#endif