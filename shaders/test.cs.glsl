
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
    int grp_size = int(globs.wrkLoad);
	float scm = min(globs.screenX,globs.screenY);
	uint wOut =  uint(globs.screenX); 
	uint hOut =  uint(globs.screenY);
    int loc = 0;
	uint offset = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_WorkGroupSize.x * globs.wrkLoad;
	for( loc = 0; loc< globs.wrkLoad; loc++, offset += gl_WorkGroupSize.x)
	{
		vec4 pt  = inputPoints[offset];
		uint color = uint(pt.w);
		vec4 inCam = World2View * vec4(pt.x, pt.y, pt.z, 1.0) ;
		if( (inCam.z > globs.zNear) && (inCam.z < globs.zFar)  )
		{
			float prd = scm*globs.zNear/(inCam.z + globs.zNear);
			float xf = inCam.x*prd + 0.5*globs.screenX;
			float yf = inCam.y*prd + 0.5*globs.screenY;
			uint xx = uint(xf + 0.5);
			uint yy = uint(yf + 0.5);
			if(( xx < wOut) && (yy< hOut) )   
			{
				uint shift = xx + yy * wOut;
				uint zAsInt = uint((inCam.z-globs.zNear) * globs.zScale) <<8;
				zAsInt =  (zAsInt & 0xFFFFFF00 ) | ( color & 0xFF); // add color
				atomicMin(zMap[shift],zAsInt);
			}
		}
	}
	//debugOut[0] =  partitions[0].sz;
	//debugOut[1] =  partitions[0].pad;
 }       
)"";