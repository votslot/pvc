
const std::string vert_shader_source = 
R""(
#version 430 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;
out vec3 fragmentColor;
out vec2 fragmentUV;
uniform highp mat4 matrix;
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
#version 430 core
in vec3 fragmentColor;
in vec2 fragmentUV;
out vec3 color;
layout(std430,binding = 0) buffer inz  {  uint zMap[]; };   
uniform  int screenSizeX;
uniform  int screenSizeY;
precision mediump float;
void main()                                  
{  
   uint xx = uint(fragmentUV.x * float(screenSizeX));
   uint yy = uint(fragmentUV.y * float(screenSizeY));
   uint shift = xx + yy* uint(screenSizeX);
   uint cl = zMap[shift];
   float rt = cl & 0x000000FF;
   float gt = (cl & 0x0000FF00)>>8;
   float bt = (cl & 0x00FF0000)>>16;
   color = vec3(rt/255.0,gt/255.0,bt/255.0)*fragmentColor;
}   
)"";