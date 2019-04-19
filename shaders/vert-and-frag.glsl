
#include "ginclude.glsl"

const std::string vert_shader_source = cs_glversion +
R""(
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

const std::string frag_shader_source = cs_glversion + cs_structs + 
R""(
in vec3 fragmentColor;
in vec2 fragmentUV;
out vec3 color;
layout(std430,binding = 0) buffer inz  {  uint zMap[]; };   
layout(std430,binding = 1) buffer inp  {  GlobalParams globs; };   
layout(std430,binding = 2) buffer clt  {  float clut[]; };   
precision mediump float;
void main()                                  
{  
   uint xx = uint(fragmentUV.x * float(globs.screenX));
   uint yy = uint(fragmentUV.y * float(globs.screenY));
   uint shift = xx + yy* uint(globs.screenX);
   uint cl = zMap[shift];
 //  uint cn = (cl & 0xFF)*4;
//   float rt = clut[cn + 0];
 //  float gt = clut[cn + 1];
 //  float bt = clut[cn + 2];
     float rt = cl & 0x000000FF;
     float gt = (cl & 0x0000FF00)>>8;
     float bt = (cl & 0x00FF0000)>>16;

  
   color = vec3(rt/255.0,gt/255.0,bt/255.0)*fragmentColor;
   // color = vec3(1.0,1.0,0.0)*fragmentColor;
  
  
}   
)"";