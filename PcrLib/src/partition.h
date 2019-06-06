
#ifndef _PARTITION_H_
#define _PARTITION_H_
#include<functional>

namespace pcrlib
{
	template<typename P>
	struct partitionData
	{
		int numPoints;
		int first;
		float  minX;
		float  minY;
		float  minZ;
		float  maxX;
		float  maxY;
		float  maxZ;
	};

	void DoPartitionXYZW_Float(void *pData, unsigned int num, std::function<void(partitionData<float> *pD)> func);
}

#endif