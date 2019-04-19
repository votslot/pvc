
#include "ginclude.glsl"


#if 0
const std::string cs_postproc = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer pm   {  int params[]; }; 
 layout(std430,binding = 1) buffer zmi  {  uint zMapIn[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint zMapOut[];}; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	int border = 1;
	if(( xx < params[0]-border) && (yy< params[1]-border) &&( xx > border) && ( yy> border)) 
	{
	    uint shift = xx +  params[0]* yy;
		uint val0 = zMapIn[shift];
		uint val_ini = val0;
		uint col = val0 & 0xFF;
		
		/*
		if( val0==0xFFFFFFFF){
		    uint valr = zMapIn[shift+1];
			uint vall = zMapIn[shift-1];
			uint valt = zMapIn[shift-params[0]];
			uint valb = zMapIn[shift+params[0]];
			if( (valr !=  0xFFFFFFFF) || (vall !=  0xFFFFFFFF) || ( valt !=  0xFFFFFFFF) || ( valb !=  0xFFFFFFFF) )
			{
				val0 = 0;
			}
		}
		zMapOut[shift] = val0;
		*/
		
		 uint valr = zMapIn[shift+1];
		 uint vall = zMapIn[shift-1];
		 uint valt = zMapIn[shift-params[0]];
		 uint valb = zMapIn[shift+params[0]];
		 if(( val0 >= valr) && (val0 >= vall) && (val0 >= valt) && (val0 >= valb))
		 {
		    if( valr < val0)  val0 = valr;
		    if( vall < val0)  val0 = vall;
		    if( valt < val0)  val0 = valt;
		    if( valb < val0)  val0 = valb;
		 }
		 zMapOut[shift] = val0;
		
	}
 }       
)"";
#endif

#if 0
const std::string cs_postproc_w = cs_glversion + 
R""(
 //#version 430 core 
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer in1 
 {  
	float screenX;
	float screenY;
	float zNear;
	float zFar;
	float zScale;
	float maxDimension;
 }; 
 layout(std430,binding = 1) buffer zmi  {  uint zMapIn[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint zMapOut[];}; 

 void main()                           
 {    
    // float prd = scm*zNear/(inCam.z + zNear);
	// uint zAsInt = uint((inCam.z-zNear) * zScale) <<8;
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	uint border = 3;
	uint ww = uint(screenX);
	uint hh = uint(screenY);

	if(( xx < ww-border) && (yy< hh-border) &&( xx > border) && ( yy> border)) 
	{
	    uint shift = xx +  ww* yy;
		uint  val0  = zMapIn[shift];
		uint  val_z = val0 & 0xFFFFFF00;
		float colf = float(val0 & 0xFF);
		uint shiftStart = shift- ww*border - border ;
		uint vv;
		uint num = border*2 + 1;
	    float lodFar  = 0.5f *maxDimension/( zFar - zNear);
		float lodNear = lodFar*0.01;
		
		for( uint yi = 0; yi<=num ; yi++)
		{
			for( uint xi = 0; xi<=num  ; xi++)
			{
				vv = zMapIn[shiftStart + xi + yi*ww];
				uint vv_z = vv & 0xFFFFFF00;
				uint vv_c = vv & 0xFF;
				if(vv_z < val_z )
				{
				    uint br = xi * (num-xi) * yi * (num-yi);
				    float zz = float(vv>>8)/16777215.0;
					zz = (zz-lodNear)/( lodFar -lodNear );
					zz  = clamp( zz*zz, 0.0, 1.0);
					float colIn = (br==0)? 0.7*float(vv_c): float(vv_c); // border color
					colf = (1.0 -zz) * colIn + zz * colf;
					val_z  = vv_z;
				}
			}
		}
		zMapOut[shift] = (val_z & 0xFFFFFF00) | uint(colf);
	}
	
 }  
 #endif

 // pGlob.zScale = 16777215.0 / (pCam->m_zFar - pCam->m_zNear);

 const std::string cs_postproc_w = cs_glversion + cs_structs +
R""(
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer zmi  {  uint zMapIn[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint zMapOut[];}; 
 layout(std430,binding = 3) buffer mt4  {  mat4 View2World;}; 
 layout(std430,binding = 4) buffer dbg  {  float dbOut[];}; 

 void main()                           
 {    
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	uint ww = uint(globs.screenX);
	uint hh = uint(globs.screenY);
	if(( xx < ww) && (yy< hh)) 
	{
	    uint shift = xx +  ww* yy;
		if(zMapIn[shift] !=  0xFFFFFF00)
		{
		    uint col =  zMapIn[shift] & 0xFF;
		    uint zi  = (zMapIn[shift]>>8) & 0x00FFFFFF;
			/*
			float zff =  float(zi)/16777215.0;
			float zf  =  (globs.zFar - globs.zNear )*zff;
			*/
			float zf  =  (globs.zFar - globs.zNear )*float(zi)/16777215.0;
			float xf = (float(xx) - globs.screenX * 0.5) /globs.scrMin;
			float yf = (float(yy) - globs.screenY * 0.5) /globs.scrMin;
			vec4 rt =  vec4( View2World[0][0],View2World[0][1],View2World[0][2],1);
			vec4 up =  vec4( View2World[1][0],View2World[1][1],View2World[1][2],1);
			vec4 dr =  vec4( View2World[2][0],View2World[2][1],View2World[2][2],1);
			vec4 pos = vec4( View2World[3][0],View2World[3][1],View2World[3][2],1);
			vec4 res = pos + ( rt * xf + up * yf + globs.zNear * dr) * (zf/ globs.zNear);
			float colorV =128 + 128.0 * (res.z - globs.bbMinZ)/(globs.bbMaxZ - globs.bbMinZ);
			uint colorI = uint(colorV);
			zMapOut[shift] = colorI | (colorI<<8) | ( colorI<<16);
			//zMapOut[shift] = 0x001010;
		}else
		{
			zMapOut[shift] = 0x800000;
		}
	}
 }  
)"";


