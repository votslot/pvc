

const std::string cs_wave_test = 
R""(
 #version 450 core 
 layout(local_size_x = 32,local_size_y =1) in;         
 layout(std430,binding = 0) buffer in0  {  float pdata[]; }; 
 layout(std430,binding = 1) buffer aaa  {  float patom[]; }; 
 layout(std430,binding = 2) buffer vv  {  mat4 View; };
 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
	vec4 cpos = vec4(1.0,1.0,1.0, 10.0);
	vec4 res =  View *cpos;
	
	pdata[0]  =   View[0][0];  // line
	pdata[1]  =   View[0][1];
	pdata[2]  =   View[0][2];
	pdata[3]  =   View[0][3];

    pdata[4]  =   res.x;
	pdata[5]  =   res.y;
	pdata[6]  =   res.z;
	pdata[7]  =   res.w;
 }       
)"";

