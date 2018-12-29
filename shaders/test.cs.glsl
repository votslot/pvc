
const std::string cs_clean = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =32) in;         
 layout(binding = 0) buffer pm  {  int params[]; }; 
 layout(binding = 1) buffer zm  {  uint zMap[]; }; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	if(( xx < params[0]) && (yy< params[1])) 
	{
	    uint shift = xx +  params[0]* yy;
		zMap[shift] = 0xFFFFFFFF;
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
 layout(binding = 3) buffer zm  {  uint zMap[]; }; 
 layout(binding = 4) buffer vv  {  mat4 World2View; };

 void main()                           
 {  
    const int grp_size = 32;
	float zNear = 2.0f;
	float zFar = 10000.0f;
	float zScale = 16777215.0/(zFar -zNear);
	float screenX =  params[0];
	float screenY =  params[1];
	float scm = min(screenX,screenY);
    int loc;
	for( loc = 0; loc< grp_size; loc++)
	{
		const uint offset = gl_GlobalInvocationID.x*grp_size + loc;
		vec4 pval = imageLoad(inputPoints,int(offset));
		vec4 inCam = World2View * vec4(pval.xyz,1);
		if( (inCam.z > zNear) && (inCam.z < zFar) )
		{
			float prd = scm*zNear/(inCam.z + zNear);
			float xf = inCam.x*prd + 0.5*screenX;
			float yf = inCam.y*prd + 0.5*screenY;

			uint xx = uint(xf + 0.5);
			uint yy = uint(yf + 0.5);
			uint wOut =  uint(screenX); 
			uint hOut =  uint(screenY);
			if(( xx < wOut) && (yy< hOut)  )   
			{
				uint shift = xx + yy * wOut;
				uint zAsInt = uint((inCam.z-zNear) * zScale) <<8;

				//uint zAsInt = (uint(inCam.z * 2048.0))<<8;
				uint color = uint (pval.w);
				zAsInt =  (zAsInt & 0xFFFFFF00 ) | ( color & 0xFF); // add color
				atomicMin(zMap[shift],zAsInt);
			}
		}
	}
	//debugOut[0] = float( params[0]);
	//debugOut[1] = float( params[1]);
 }       
)"";