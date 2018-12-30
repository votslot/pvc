
const std::string vert_shader_source = 
R""(
#version 450 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;
out vec3 fragmentColor;
out vec2 fragmentUV;
void main()                  
{  
  gl_Position.xyz = vertexPosition;
  gl_Position.w = 1.0;
  fragmentColor = vertexColor;
  fragmentUV = vertexUV;
}                            
)"";

const std::string frag_shader_source = 
R""(
#version 450 core
in vec3 fragmentColor;
in vec2 fragmentUV;
out vec3 color;
layout(std430,binding = 0) buffer inz  {  uint zMap[]; };   
layout(std430,binding = 1) buffer inp  {  int params[]; };   
layout(std430,binding = 2) buffer clt  {  float clut[]; };   
precision mediump float;
void main()                                  
{  
   uint xx = uint(fragmentUV.x * float(params[0]));
   uint yy = uint(fragmentUV.y * float(params[1]));
   uint shift = xx + yy* uint(params[0]);
   uint cl = zMap[shift];
   if(cl!=0xFFFFFFFF){
        uint cn = (cl & 0xFF)*4;
		float rt = clut[cn + 0];
		float gt = clut[cn + 1];
		float bt = clut[cn + 2];
		color = vec3(rt,gt,bt)*fragmentColor;
   }else{
		color = vec3(0.0,0.0,0.0);
   }
}   
)"";