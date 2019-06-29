

const std::string cs_wave_test = 
R""(
 #version 430 core
 layout(local_size_x = 32,local_size_y =1 ) in;         
 layout(std430,binding = 0) buffer in0 {  int pdata0[]; }; 
 layout(std430,binding = 1) buffer in1 {  int pdata1[]; };
 layout(std430,binding = 2) buffer in2 {  int pdata2[]; };
 void main()                           
 {     
    const uint xx = gl_GlobalInvocationID.x;
	pdata2[xx]  =  pdata0[xx] +  pdata1[xx];
 }       
)"";

