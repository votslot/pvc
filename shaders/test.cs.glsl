//////////////
#if 0
const std::string vs_source1_aa = 
R""(
 #version 430 core 
 layout(local_size_x = 64) in;         
 layout(binding = 0) buffer in0  {  float camera[]; };                    
 layout(binding = 1) buffer in1  {  int params[]; }; 
 layout(binding = 2) buffer in2  {  float points[]; }; 
 layout(rgba32f,binding = 3) uniform image2D destination;
 layout(binding = 4) buffer dbg  {  int debugOut[]; }; 

 void main()                           
 {                                     
    const uint offset = gl_GlobalInvocationID.x;
	if(offset <64)
	{
		debugOut[offset] = int(camera[offset]);
	}
	if(offset < 512*512)
	{
    uint aa =  gl_GlobalInvocationID.x;
	uint yy = aa>>9;
	uint xx = aa & 511;
	float rr = 0.0;
	float gg = 0.0;

	if( (yy & 31)==0 || (xx & 31)==0 )
	{
	  rr = 1.0;
	  gg = 1.0;
	}
	
	vec4 color = vec4(rr, gg, 0.0, 1.0) ;
	imageStore(destination, ivec2(xx,yy), color);
	}
 }       
)"";
#endif



const std::string cs_clean = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =32) in;         
 layout(binding = 0) buffer zm  {  int zMap[]; }; 
 layout(binding = 1) buffer pm  {  int params[]; }; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	if(( xx < params[0]) && (yy< params[1])) 
	{
	    uint shift = xx +  params[0]* yy;
		zMap[shift] = 0x7FFFFFFF;
	}
 }       
)"";


const std::string cs_render_points = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =1) in;   
 layout(binding = 0) buffer in1  {  int params[]; }; 
 layout(binding = 1) buffer dbg  {  float debugOut[]; }; 
 layout(rgba32f,binding = 2) uniform readonly imageBuffer inputPoints;
 layout(binding = 3) buffer zm  {  int zMap[]; }; 
 layout(binding = 4) buffer vv  {  mat4 World2View; };

 void main()                           
 {  
    const int grp_size = 32;
	float screenX =  params[0];
	float screenY =  params[1];
	float scm = min(screenX,screenY);
    int loc;
	for( loc = 0; loc< grp_size; loc++)
	{
		const uint offset = gl_GlobalInvocationID.x*grp_size + loc;
		vec4 pval = imageLoad(inputPoints,int(offset));
		vec4 inCam = World2View * vec4(pval.xyz,1);
		if(inCam.z > 0.0f)
		{
			float focus = 2.0f;
			float prd = scm*focus/(inCam.z + focus);
			float xf = inCam.x*prd + 0.5*screenX;
			float yf = inCam.y*prd + 0.5*screenY;

			uint xx = uint(xf);
			uint yy = uint(yf);
		 
			uint wOut =  uint(screenX); 
			uint hOut =  uint(screenY);
			if(( xx < wOut) && (yy< hOut))   
			{
				uint shift = xx + yy * wOut;
				int zAsInt = (int(inCam.z * 10000.0))<<8;
				zAsInt =  zAsInt | (int (pval.w)); // add color
				atomicMin(zMap[shift],zAsInt);
			
				/*
				int k, m;
				for(m = -4; m<4; m++){
					for(k = -4; k<4; k++){
						atomicMin(zMap[shift+k + m*wOut],zAsInt);
					}
				}
				*/
			}
		}
	}
	//debugOut[0] = float( params[0]);
	//debugOut[1] = float( params[1]);
 }       
)"";