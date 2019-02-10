
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
 layout(local_size_x = 32,local_size_y =1) in;   
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer dbg  {  float debugOut[]; }; 
 layout(std430,binding = 2) buffer pt   {  float inputPoints[]; }; 
 layout(std430,binding = 3) buffer zm   {  uint zMap[]; }; 
 layout(std430,binding = 4) buffer vv   {  mat4 World2View; };
 layout(std430,binding = 5) buffer pp   {  Partition partitions[]; }; 

 void main()                           
 {  
    int grp_size = int(globs.wrkLoad);
	//float zScale = 16777215.0/(zFar -zNear);
	float scm = min(globs.screenX,globs.screenY);
    int loc = 0;
	for( loc = 0; loc< grp_size; loc++)
	{
	    const uint offset = (gl_GlobalInvocationID.x*grp_size + loc)*4;
		float xIn = float(inputPoints[offset + 0]);
		float yIn = float(inputPoints[offset + 1]);
		float zIn = float(inputPoints[offset + 2]);
		uint color = uint (inputPoints[offset + 3]);

		vec4 inCam = World2View * vec4(xIn,yIn,zIn,1.0) ;
		if( (inCam.z > globs.zNear) && (inCam.z < globs.zFar)  )
		{
			float prd = scm*globs.zNear/(inCam.z + globs.zNear);
			float xf = inCam.x*prd + 0.5*globs.screenX;
			float yf = inCam.y*prd + 0.5*globs.screenY;

			uint xx = uint(xf + 0.5);
			uint yy = uint(yf + 0.5);
			uint wOut =  uint(globs.screenX); 
			uint hOut =  uint(globs.screenY);
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