

#include "ginclude.glsl"

const std::string cs_render_font8x8 = cs_glversion + cs_structs +cs_struct_font_chars +
R""(
 layout(local_size_x = 64,local_size_y =1) in;   
 layout(std430,binding = 0) buffer in1 {  GlobalParams globs; };
 layout(std430,binding = 1) buffer ft  {  uint fontImg[]; }; 
 layout(std430,binding = 2) buffer li  {  FontChars letterIndex[]; }; 
 layout(std430,binding = 3) buffer zm  {  uint destBuff[]; }; 

 void main()                           
 { 
  
   uint posX = letterIndex[gl_GlobalInvocationID.y].x;
   uint posY = letterIndex[gl_GlobalInvocationID.y].y;
   uint letter = letterIndex[gl_GlobalInvocationID.y].letter;
   uint usx =  uint(globs.screenX);
   uint usy =  uint(globs.screenY);
   uint left = usx  -posX;
   uint top  = usy  -posY;
   uint locX = gl_GlobalInvocationID.x & 7;
   uint locY = gl_GlobalInvocationID.x >>3;
   uint scrColor  = fontImg[locX + locY*8 + letter*64];
   uint line   = top  + locX;
   uint column = left + locY;
   uint shift = column +  usx * line ;
   destBuff[shift] = scrColor; 

   /*
    const uint xx = uint(globs.screenX) - gl_GlobalInvocationID.x -100;
    const uint yy = uint(globs.screenY) - gl_GlobalInvocationID.y -100;
	uint px = uint(globs.screenX);
	uint py = 0;
	if(( xx < uint(globs.screenX)) && (yy< uint(globs.screenY)) ) 
	{
	    uint line   = gl_GlobalInvocationID.x >> 3;
		uint column =  gl_GlobalInvocationID.x & 3;
	    uint shift = column +  uint(globs.screenX)* line;
		zMap[shift] = 0xFF; 
		//zMap[shift] = (xx>yy) ? 0xFF: 0xFF00;
	}
	*/
 }       
)"";

