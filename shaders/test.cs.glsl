
const std::string cs_clean = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =32) in;         
 layout(std430,binding = 0) buffer in1 
 {  
	float screenX;
	float screenY;
 }; 
 layout(std430,binding = 1) buffer zm  {  uint zMap[]; }; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	if(( xx < uint(screenX)) && (yy< uint(screenY)) ) 
	{
	    uint shift = xx +  uint(screenX)* yy;
		zMap[shift] = 0xFFFFFF00;
	}
 }       
)"";


const std::string cs_render_points = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =1) in;   
 layout(std430,binding = 0) buffer in1 
 {  
	float screenX;
	float screenY;
	float zNear;
	float zFar;
	float zScale;
 }; 
 layout(std430,binding = 1) buffer dbg  {  float debugOut[]; }; 
 layout(std430,binding = 2) buffer pt   {  float inputPoints[]; }; 
 layout(std430,binding = 3) buffer zm   {  uint zMap[]; }; 
 layout(std430,binding = 4) buffer vv   {  mat4 World2View; };

 void main()                           
 {  
    const int grp_size = 32;
	//float zScale = 16777215.0/(zFar -zNear);
	float scm = min(screenX,screenY);
    int loc;
	for( loc = 0; loc< grp_size; loc++)
	{
	    const uint offset = (gl_GlobalInvocationID.x*grp_size + loc)*4;
		float xIn = float(inputPoints[offset + 0]);
		float yIn = float(inputPoints[offset + 1]);
		float zIn = float(inputPoints[offset + 2]);
		uint color = uint (inputPoints[offset + 3]);

		vec4 inCam = World2View * vec4(xIn,yIn,zIn,1.0) ;
		if( (inCam.z > zNear) && (inCam.z < zFar) )
		{
			float prd = scm*zNear/(inCam.z + zNear);
			float xf = inCam.x*prd + 0.5*screenX;
			float yf = inCam.y*prd + 0.5*screenY;

			uint xx = uint(xf + 0.5);
			uint yy = uint(yf + 0.5);
			uint wOut =  uint(screenX); 
			uint hOut =  uint(screenY);
			if(( xx < wOut) && (yy< hOut) )   
			{
				uint shift = xx + yy * wOut;
				uint zAsInt = uint((inCam.z-zNear) * zScale) <<8;
				zAsInt =  (zAsInt & 0xFFFFFF00 ) | ( color & 0xFF); // add color
				atomicMin(zMap[shift],zAsInt);
			}
		}
	}
	//debugOut[0] = float( params[0]);
	//debugOut[1] = float( params[1]);
 }       
)"";