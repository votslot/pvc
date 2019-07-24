#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "app-events.h"
#include "app-las.h"


namespace pcrapp
{

#pragma pack (1)
	struct LasHeader
	{
		char magic[4];
		short fileSourceId;
		short globalEncoding;
		int   giud1;
		short guid2;
		short guid3;
		char  guid4[8];
		char verMajor;
		char verMinor;
		char sysId[32];
		char getSoft[32];
		short CreationDay;
		short CretionYear;
		short headerSize;
		unsigned int pointOfst;
		unsigned int numOfVars;
		unsigned char pointDataFormatId;
		unsigned short poitDataRecordLength;
		unsigned int numOfPointRecords;
		unsigned int NumberofPointByReturn[5];
		double xScale;
		double yScale;
		double zScale;
		double xOffset;
		double yOffset;
		double zOffset;
		double maxX;
		double maxY;
		double maxZ;
		double minX;
		double minY;
		double minZ;
	};

	struct PointDataXYZ
	{
		int x;
		int y;
		int z;
		unsigned short val;
	};

	static void ReadLas(const std::string &path, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb)
	{
		std::fstream fs;
		fs.open(path.c_str(), std::fstream::in | std::fstream::binary);
		if (fs.fail())
		{
			if (pCb) pCb->error("Failed to open las file");
			return;
		}
		fs.seekg(0, fs.end);
		int fileLength = (int)fs.tellg();
		fs.seekg(0, fs.beg);

		LasHeader lasH;
		int hdSise = sizeof(lasH);
		fs.read((char*)&lasH, sizeof(LasHeader));

		int pointArea = fileLength - lasH.pointOfst;
		int calcSize = lasH.poitDataRecordLength * lasH.numOfPointRecords;

		//Sanity check :  we expect calcSize == pointArea
		//if (gpLasCloud) gpLasCloud->OnStart();
		//if (gpLasCloud) gpLasCloud->SetNumPoints(lasH.numOfPointRecords);

		fs.seekg(0, fs.beg);
		fs.seekp(lasH.pointOfst);
		char *pPoinRecord = new char[lasH.poitDataRecordLength];
		unsigned short minV = 0;
		for (unsigned int k = 0; k < lasH.numOfPointRecords; k++)
		{
			fs.read(pPoinRecord, lasH.poitDataRecordLength);
			PointDataXYZ *pXYZ = (PointDataXYZ *)pPoinRecord;
			if (pXYZ->val > minV)
			{
				minV = pXYZ->val;
			}
			pLib->addPoint((float)pXYZ->x*float(lasH.xScale), (float)pXYZ->y*float(lasH.yScale), (float)pXYZ->z*float(lasH.zScale), (float)pXYZ->val);
		}
		fs.close();
		delete[]pPoinRecord;
	}



	void readLasFile(const char *pPath, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb)
	{
		std::string str(pPath);
		if (pCb) pCb->message((std::string("Opening las file  ") + str +"\n").c_str() );
		ReadLas(str,pLib, pCb);
	}
}//namespace pcrapp