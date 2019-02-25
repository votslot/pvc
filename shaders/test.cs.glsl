
#include "ginclude.glsl"

const std::string cs_clean = cs_glversion + cs_structs +
R""(
 layout(local_size_x = 32,local_size_y =32) in;   
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer zm  {  uint zMap[]; }; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	if(( xx < uint(globs.screenX)) && (yy< uint(globs.screenY)) ) 
	{
	    uint shift = xx +  uint(globs.screenX)* yy;
		zMap[shift] = 0xFFFFFF00;
	}
 }       
)"";


const std::string cs_render_points = cs_glversion + cs_structs +
R""(
 precision mediump float;
 layout(local_size_x = 64,local_size_y =1) in;   
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer dbg  {  float debugOut[]; }; 
 layout(std430,binding = 2) buffer ptt  {  vec4 inputPoints[]; }; 
 layout(std430,binding = 3) buffer zm   {  uint zMap[]; }; 
 layout(std430,binding = 4) buffer vv   {  mat4 World2View; };
 layout(std430,binding = 5) buffer pp   {  Partition partitions[]; }; 

 void main()                           
 {  

    float scm = min(globs.screenX,globs.screenY);
    mat4 toScreen;
	toScreen[0][0] = 1.0;
 	toScreen[1][0] = 0.0;
 	toScreen[2][0] = 0.5*globs.screenX /(scm*globs.zNear);
 	toScreen[3][0] = 0.5*globs.screenX/scm;

	toScreen[0][1] = 0.0;
 	toScreen[1][1] = 1.0;
 	toScreen[2][1] = 0.5*globs.screenY /(scm*globs.zNear);
 	toScreen[3][1] = 0.5*globs.screenY/scm;

	toScreen[0][2] = 0.0;
 	toScreen[1][2] = 0.0;
 	toScreen[2][2] = 1.0/( globs.zFar - globs.zNear);
 	toScreen[3][2] = -globs.zNear/( globs.zFar - globs.zNear);

	toScreen[0][3] = 0.0;
 	toScreen[1][3] = 0.0;
 	toScreen[2][3] = 1.0/(scm*globs.zNear);
 	toScreen[3][3] = 1.0/scm;

	mat4 finMat = toScreen * World2View ;
    int grp_size = int(globs.wrkLoad);
	
 	uint offset = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_WorkGroupSize.x * globs.wrkLoad;
	for( int loc = 0; loc< globs.wrkLoad; loc++, offset += gl_WorkGroupSize.x)
	{
		vec4 pt  = inputPoints[offset];
		uint color = uint(pt.w);
		vec4 vf =   finMat * vec4(pt.x, pt.y, pt.z, 1.0) ;

		if( (vf.z > 0.0) && (vf.z < 1.0) && ( vf.x < globs.screenX *vf.w ) && ( vf.y < globs.screenY * vf.w) &&(vf.x>0.0) &&(vf.y>0.0)  )
		{
			uint xx = uint(vf.x/vf.w);
			uint yy = uint(vf.y/vf.w);
			uint shift = xx + yy * ( uint(globs.screenX));
			uint zAsInt = uint((vf.z*16777215.0)) <<8;
			zAsInt =  (zAsInt & 0xFFFFFF00 ) | ( color & 0xFF); // add color
			atomicMin(zMap[shift],zAsInt);
		}
	}
	//debugOut[0] =  partitions[0].sz;
	//debugOut[1] =  partitions[0].pad;
 }       
)"";