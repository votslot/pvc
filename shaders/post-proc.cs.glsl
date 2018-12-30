
const std::string cs_postproc = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer pm   {  int params[]; }; 
 layout(std430,binding = 1) buffer zmi  {  uint zMapIn[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint zMapOut[];}; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	int border = 1;
	if(( xx < params[0]-border) && (yy< params[1]-border) &&( xx > border) && ( yy> border)) 
	{
	    uint shift = xx +  params[0]* yy;
		uint val0 = zMapIn[shift];
		uint val_ini = val0;
		uint col = val0 & 0xFF;
		
		/*
		if( val0==0xFFFFFFFF){
		    uint valr = zMapIn[shift+1];
			uint vall = zMapIn[shift-1];
			uint valt = zMapIn[shift-params[0]];
			uint valb = zMapIn[shift+params[0]];
			if( (valr !=  0xFFFFFFFF) || (vall !=  0xFFFFFFFF) || ( valt !=  0xFFFFFFFF) || ( valb !=  0xFFFFFFFF) )
			{
				val0 = 0;
			}
		}
		zMapOut[shift] = val0;
		*/
		
		 uint valr = zMapIn[shift+1];
		 uint vall = zMapIn[shift-1];
		 uint valt = zMapIn[shift-params[0]];
		 uint valb = zMapIn[shift+params[0]];
		 if(( val0 >= valr) && (val0 >= vall) && (val0 >= valt) && (val0 >= valb))
		 {
		    if( valr < val0)  val0 = valr;
		    if( vall < val0)  val0 = vall;
		    if( valt < val0)  val0 = valt;
		    if( valb < val0)  val0 = valb;
		 }
		 zMapOut[shift] = val0;
		
	}
 }       
)"";


const std::string cs_postproc_w = 
R""(
 #version 430 core 
 layout(local_size_x = 32,local_size_y =32) in; 
 layout(std430,binding = 0) buffer pm   {  int params[]; }; 
 layout(std430,binding = 1) buffer zmi  {  uint zMapIn[]; }; 
 layout(std430,binding = 2) buffer zmo  {  uint zMapOut[];}; 

 void main()                           
 {                                     
    const uint xx = gl_GlobalInvocationID.x;
    const uint yy = gl_GlobalInvocationID.y;
	uint border = 3;
	if(( xx < params[0]-border) && (yy< params[1]-border) &&( xx > border) && ( yy> border)) 
	{
	    uint shift = xx +  params[0]* yy;
		uint val0 = zMapIn[shift];
		uint val_ini = val0;
		uint col = val0 & 0xFF;
		
		uint ww = uint(params[0]);
		uint shiftStart = shift- ww*border - border ;
		uint vv;
		uint num = border*2 + 1;
		uint xMin = 0xFFFFFFFF, yMin =  0xFFFFFFFF;
		for( uint yi = 0; yi<=num ; yi++)
		{
			for( uint xi = 0; xi<=num  ; xi++)
			{
				vv = zMapIn[shiftStart + xi + yi*ww];
				if( vv < val0) {
					val0 = vv;
					col  = val0 & 0xFF;
					xMin = xi;
					yMin = yi;
				}
			}
		}

		if( ( xMin == 0) || ( xMin == num ) || ( yMin == 0) || ( yMin == num ) )
		{
			col = 20 + col/2;
		}
		zMapOut[shift] = (val0 & 0xFFFFFF00) | col;
	}
 }       
)"";


