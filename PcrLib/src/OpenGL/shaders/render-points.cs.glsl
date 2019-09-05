

#include "ginclude.glsl"

const std::string cs_clean = cs_glversion + cs_structs +
R""(
 layout(local_size_x = 32,local_size_y =32) in;   
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer zm  {  uint zMap[]; }; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	if(( xx < uint(globs.screenX)) && (yy< uint(globs.screenY)) ) 
	{
	    uint shift = xx +  uint(globs.screenX)* yy;
		zMap[shift] = 0xFFFFFF00; 
		//zMap[shift] = (xx>yy) ? 0xFF: 0xFF00;
	}
 }       
)"";


const std::string cs_render_points = cs_glversion + cs_const_val+ cs_struct_point + cs_structs + cs_structs_partition+
R""(
 precision mediump float;
 layout(local_size_x = 64,local_size_y =1) in;   
 layout(std430,binding = 0) buffer in1  {  GlobalParams globs; };
 layout(std430,binding = 1) buffer dbg  {  float debugOut[]; }; 
 layout(std430,binding = 2) buffer ptt  {  RenderPoint inputPoints[]; }; 
 layout(std430,binding = 3) buffer zm   {  uint zMap[]; }; 
 layout(std430,binding = 4) buffer vv   {  mat4 World2View; };
 layout(std430,binding = 5) buffer pp   {  Partition partitions[]; }; 
 void main()                           
 {  
    const uint msk_z = (1 << cZbuffBits) - 1;
    const uint msk_v = (1 << (32 - cZbuffBits)) -1 ;
    Partition part = partitions[gl_GlobalInvocationID.y];

    uint lodLevel = globs.wrkLoad;
    vec4 vc = World2View  * vec4(part.cx, part.cy, part.cz, 1.0) ;
    if( (vc.z > 0.0) && (vc.z < 1.0) )
    {
        vec4 tt = vec4(part.cx + globs.camUpx * part.sz , part.cy + globs.camUpy * part.sz, part.cz + globs.camUpz * part.sz, 1.0);
        vec4 vb  =  World2View  * tt ;
        float xa = vc.x/vc.w;
        float ya = vc.y/vc.w;
        float xb = vb.x/vb.w;
        float yb = vb.y/vb.w;
        float ddx =xa- xb;
        float ddy =ya-yb;
        uint dd = uint( sqrt(ddx*ddx + ddy*ddy));
        lodLevel = clamp( dd, 1, uint(globs.wrkLoad));
    }

    uint offset = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_WorkGroupSize.x * globs.wrkLoad;
    for( int loc = 0; loc < lodLevel; loc++, offset += gl_WorkGroupSize.x)
    {
        RenderPoint pt = inputPoints[offset] ;
        uint color = pt.w;
        vec4 vf =    World2View  * vec4(pt.x, pt.y, pt.z, 1.0) ;
        if( (vf.z > 0.0) && (vf.z < 1.0) && ( vf.x < globs.screenX *vf.w ) && ( vf.y < globs.screenY * vf.w) &&(vf.x>0.0) &&(vf.y>0.0)  )
        {
            uint xx = uint(vf.x/vf.w);
            uint yy = uint(vf.y/vf.w);
            uint shift = xx + yy * ( uint(globs.screenX));
            uint zAsInt = uint(vf.z* float(msk_z)) <<(32-cZbuffBits);
            zAsInt =  (zAsInt & (~ msk_v ) ) | ( color &  msk_v); // add color
            atomicMin(zMap[shift],zAsInt);
        }
        // debug
        /*
        if(color==63)
        {
            debugOut[0] =  pt.z;
        }
        */
        
    }
 }       
)"";
