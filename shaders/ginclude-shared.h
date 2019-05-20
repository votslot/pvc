



#if ( FOR_SHADER==1)
#include <string>
const std::string endline = "\n";
#ifndef  DECLARE_STRUCT_AS_STRING
	#define DECLARE_STRUCT_AS_STRING(_stringname, _v)  const std::string _stringname=#_v+endline;
#endif

#else
#ifndef  DECLARE_STRUCT_AS_STRING
	#define DECLARE_STRUCT_AS_STRING(_stringname, _v) _v
#endif
#endif 
 
DECLARE_STRUCT_AS_STRING(cs_const_val,
	const uint cZbuffBits = 16;
);

 DECLARE_STRUCT_AS_STRING(cs_structs, 
	struct  GlobalParams
	{
		float screenX;
		float screenY;
		float zNear;
		float zFar;
		float zRange;
		float maxDimension;
		unsigned int wrkLoad;
		float px;
		float py;
		float pz;
		float bbMinX;
		float bbMaxX;
		float bbMinY;
		float bbMaxY;
		float bbMinZ;
		float bbMaxZ;
		float scrMin;
		float pad0;
		float pad1;
		float pad2;
	};);

 DECLARE_STRUCT_AS_STRING(cs_struct_point,
 struct RenderPoint
 {
	 float x;
	 float y;
	 float z;
	 uint w;
 };);

 DECLARE_STRUCT_AS_STRING(cs_structs_partition, 
	 struct Partition
	 {
		 float cx;
		 float cy;
		 float cz;
		 float sz;
		 uint first;
		 uint last;
		 uint ndx;
		 float pad1;
	 };);

 DECLARE_STRUCT_AS_STRING(cs_struct_colorize, 
	struct ColorizeData 
	{
		float blah;
		float xMin;
		float xMax;
		float yMin;
		float yMax;
		float zMin;
		float zMaz;
	};);

#ifdef  DECLARE_STRUCT_AS_STRING
#undef  DECLARE_STRUCT_AS_STRING
#endif 




