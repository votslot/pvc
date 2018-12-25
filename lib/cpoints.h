
#ifndef _CPOINTS_H
#define _CPOINTS_H
struct CPoint 
{
	float x;
	float y;
    float z;
	float attr;
};

CPoint *GetPoints(int &num);

#endif