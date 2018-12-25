
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
//uniform sampler2D TextureSampler;
layout(binding = 0) buffer inz  {  int zMap[]; };   
layout(binding = 1) buffer inp  {  int params[]; };   
precision mediump float;
void main()                                  
{  
   /*
   float cf = texture( TextureSampler, fragmentUV ).r;
   float rf = fract(cf) ;float
   cf = (cf - rf)*0.001;
   float gf = fract(cf);
   float bf = trunc(cf)*0.001;
   color = vec3(rf,cf,bf)*fragmentColor*1000.0/255.0;;
   */
   uint xx = uint(fragmentUV.x * float(params[0]));
   uint yy = uint(fragmentUV.y * float(params[1]));
   
   uint shift = xx + yy* uint(params[0]);
   int cl = zMap[shift];
   if(cl!=0x7FFFFFFF){
		//float rl = float( cl &0xFF) /255.0;
		//float rl = float( cl&0xFF) /255.0;
		/*
		float rt =  float(rl>0.5 ? 0.0: 1.0);
		float gt =  float(rl>0.5 ? 1.0: 0.0);
		float bt =  float(rl);
		color = vec3(rt,gt,bt)*fragmentColor;
		*/
		float rt =  float( cl &0xFF) /255.0;
		color = vec3(rt,rt,rt)*fragmentColor;
   }else{
		color = vec3(0.0,0.0,0.5);
   }
}   
)"";