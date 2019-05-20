
#include "ginclude.glsl"


const std::string cs_colorize = cs_glversion + cs_struct_colorize +
R""(
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer in1  {  ColorizeData params; };
 layout(std430,binding = 1) buffer zmo  {  uint dest[];}; 
 layout(std430,binding = 2) buffer dbg  {  float dbOut[];}; 

 void main()                           
 {    
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	dbOut[0] = params.xMin;
	dbOut[1] = params.xMax;
	dbOut[2] = params.yMin;
	dbOut[3] = params.yMax;
 }

)"";


const std::string cs_postproc_w = cs_glversion + cs_const_val + cs_structs +
R""(
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer zmi  {  uint zMapIn[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint zMapOut[];}; 
 layout(std430,binding = 3) buffer mt4  {  mat4 View2World;}; 
 layout(std430,binding = 4) buffer dbg  {  float dbOut[];}; 

 void main()                           
 {   
    const uint msk_z = (1 << cZbuffBits) - 1;
	const uint msk_v = (1 << (32 - cZbuffBits)) -1 ;
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	uint ww = uint(globs.screenX);
	uint hh = uint(globs.screenY);
	if(( xx < ww) && (yy< hh)) 
	{
	    uint shift = xx +  ww* yy;
		if(zMapIn[shift] !=  0xFFFFFF00)
		{
		    uint zi  = (zMapIn[shift]>>(32 - cZbuffBits)) & msk_z;
			float zf  =  (globs.zFar - globs.zNear )*float(zi)/float(msk_z);
			float xf = (float(xx) - globs.screenX * 0.5) /globs.scrMin;
			float yf = (float(yy) - globs.screenY * 0.5) /globs.scrMin;
			vec4 rt =  vec4( View2World[0][0],View2World[0][1],View2World[0][2],1);
			vec4 up =  vec4( View2World[1][0],View2World[1][1],View2World[1][2],1);
			vec4 dr =  vec4( View2World[2][0],View2World[2][1],View2World[2][2],1);
			vec4 pos = vec4( View2World[3][0],View2World[3][1],View2World[3][2],1);
			vec4 res = pos + ( rt * xf + up * yf + globs.zNear * dr) * (zf/ globs.zNear);
			uint colorX = uint(  255.0 * (res.x - globs.bbMinX)/(globs.bbMaxX - globs.bbMinX));
			uint colorY = uint(  255.0 * (res.y - globs.bbMinY)/(globs.bbMaxY - globs.bbMinY));
			uint colorZ = uint( 128  + 128.0 * (res.z - globs.bbMinZ)/(globs.bbMaxZ - globs.bbMinZ));
			zMapOut[shift] =  colorX | (colorY<<8) | ( colorZ<<16);
		}else
		{
			zMapOut[shift] = 0x800000;
		}
	}
 }  
)"";


