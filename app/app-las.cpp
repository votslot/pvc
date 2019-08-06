#include "stdio.h"
#include <iostream>
#include <fstream>  
#include "app-events.h"
#include "app-las.h"
#include <string>


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

	struct PointFromat1
	{
		int x;
		int y;
		int z;
		unsigned short val;
	};

#pragma pack (1)
	struct PointFromat3
	{
		int x;
		int y;
		int z;
		unsigned short val;
		unsigned char  rets;
		unsigned char Classification;
		unsigned char scanAngle;
		unsigned char userData;
		unsigned short  pointSourceID;
		double gpsTime;
		unsigned short red;
		unsigned short green;
		unsigned short blue;
	};

	static int ReadLas(const std::string &path, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb)
	{
		std::fstream fs;
		fs.open(path.c_str(), std::fstream::in | std::fstream::binary);
		if (fs.fail())
		{
			if (pCb) pCb->error("Failed to open las file");
			return -1;
		}
		fs.seekg(0, fs.end);
		int fileLength = (int)fs.tellg();
		fs.seekg(0, fs.beg);

		LasHeader lasH;
		int hdSise = sizeof(lasH);
		fs.read((char*)&lasH, sizeof(LasHeader));

		int pointArea = fileLength - lasH.pointOfst;
		int calcSize = lasH.poitDataRecordLength * lasH.numOfPointRecords;
		if (pCb) 
		{
			pCb->message(std::string("Las version:" + std::to_string(lasH.verMajor) + std::to_string(lasH.verMinor)+ "\n").c_str());
			pCb->message(std::string("Las point format:" + std::to_string(lasH.pointDataFormatId) + "\n").c_str());
		}
		std::cout << "yofst=" << lasH.yOffset << std::endl;

		//Sanity check :  we expect calcSize == pointArea
		fs.seekg(0, fs.beg);
		fs.seekp(lasH.pointOfst);
		char *pPoinRecord = new char[lasH.poitDataRecordLength];
		unsigned short minV = 0;
		for (unsigned int k = 0; k < lasH.numOfPointRecords; k++)
		{
			fs.read(pPoinRecord, lasH.poitDataRecordLength);
			unsigned int rr,gg,bb, cc;

			PointFromat1 *pXYZ1 = (PointFromat1*)pPoinRecord;

			if (lasH.pointDataFormatId ==3)
			{
				PointFromat3 *pXYZ3 = (PointFromat3 *)pPoinRecord;
				rr = pXYZ3->red >> 11;
				gg = pXYZ3->green >> 11;
				bb = pXYZ3->blue >> 11;
				cc = rr | (gg << 5) | (bb << 10);
			}
			else 
			{
				cc = pXYZ1->val;
			}
	
			float xf = (float)(pXYZ1->x )*float(lasH.xScale) + lasH.xOffset;
			float yf = (float)(pXYZ1->y )*float(lasH.yScale) + lasH.yOffset;
			float zf = (float)(pXYZ1->z )*float(lasH.zScale) + lasH.zOffset;
			pLib->addPoint(xf, yf, zf, cc);
		}
		fs.close();
		delete[]pPoinRecord;
		return lasH.pointDataFormatId;
	}



	int readLasFile(const char *pPath, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb)
	{
		std::string str(pPath);
		if (pCb) pCb->message((std::string("Opening las file  ") + str +"\n").c_str() );
		return ReadLas(str,pLib, pCb);
	}
}//namespace pcrapp