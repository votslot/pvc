
#include "ginclude.glsl"


const std::string cs_postproc_ptsize = cs_glversion + cs_structs +
R""(
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer zmi  {  uint pointZ[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint pointColor[];}; 
 layout(std430,binding = 3) buffer mt4  {  uint bufferOut[];}; 
 void main()                           
 {   
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	uint ww = uint(globs.screenX);
	uint hh = uint(globs.screenY);
	const int ptSz = int(globs.pointSize);
	const int rad = (ptSz==1) ? 2:ptSz*ptSz;
	uint shift = xx +  ww* yy;
	if(( xx < ww-ptSz-1) && (yy< hh-ptSz-1) && ( xx>ptSz+1) && ( yy>ptSz+1) ) 
	{
 		 uint f_shift = shift;
		 uint zz = pointZ[shift];
		 uint shift_t,z_l;
		 for( int ky = -ptSz; ky<=ptSz; ky++)
		 {
		     for( int kx = -ptSz; kx<=ptSz; kx++)
		     {
			 	uint dd = kx*kx + ky*ky;
			 	if(dd < rad)
				{
		            shift_t = shift + kx + ky* ww;
		            z_l = pointZ[shift_t];
		            if(z_l < zz) { f_shift = shift_t; zz = z_l;}
				}
		     }
		 }
		 bufferOut[shift] = pointColor[f_shift];
	}
	
 }  
)"";


