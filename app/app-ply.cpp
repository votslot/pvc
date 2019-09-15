#include "stdio.h"
#include <iostream>
#include <fstream>  
#include <sstream>
#include <vector>
#include <map>
#include "app-events.h"
#include "app-ply.h"
#include <string>


namespace pcrapp
{

	struct PlyRecordI 
	{
		static std::string m_line;
		static size_t  m_shift;
		static char *m_pData;
		static int m_recordSize;

		static void begin_line(bool isAscii,int recordSz, std::ifstream &fs)
		{
			if (isAscii) {
				std::getline(fs, m_line);
			}
			else {
				if(m_pData==NULL)  m_pData  = new char[recordSz];
				fs.read(m_pData, recordSz);
			}
			m_shift = 0;
		}

		static void done_reading()
		{
			if (m_pData != NULL) delete[]m_pData;
		}

		void read_line(float *pDest, bool isAscii, bool isBigEndian)
		{
			if (isAscii) {
				read_ascii(pDest);
			}
			else {
				read_bin(pDest, isBigEndian);
			}

		}

		virtual  void read_ascii(float *pDest) = 0;
		virtual  void read_bin(float *pDest, bool isBigEndian) = 0;
		virtual size_t get_size() = 0;
	};

	std::string PlyRecordI::m_line;;
	size_t  PlyRecordI::m_shift;
	int PlyRecordI::m_recordSize;
	char* PlyRecordI::m_pData = NULL;

	template<int N, typename T>
	struct PlyRecord : public PlyRecordI
	{
		void read_ascii(float *pDest)
		{
			size_t sz;
			float res;
			int ndx = N;
			res = std::stof(m_line.c_str() + m_shift, &sz);
			m_shift += sz;
			if (N != -1) pDest[N] = (float)res;
		}

		void read_bin(float *pDest, bool isBigEndian)
		{
			if (N != -1) {
				float res;
				char * pD = m_pData + m_shift;
				if (isBigEndian) {
					char v[4];
					v[0] = pD[3]; v[1] = pD[2];  v[2] = pD[1]; v[3] = pD[0];
					res = *((float*)(v));
				}
				else {
					res = *(float*)(pD);
				}
				pDest[N] = (float)res;
			}
			m_shift += sizeof(T);
		}

		size_t get_size()
		{
			return sizeof(T);
		}

	};

	template<int N>
	PlyRecordI *  getReader(const std::string & t)
	{
		if (t == "int8" || t == "char")           return new  PlyRecord<N, char>();
		else if (t == "uint8" || t == "uchar")    return new  PlyRecord<N, unsigned char>();
		else if (t == "int16" || t == "short")    return new  PlyRecord<N, short>();
		else if (t == "uint16" || t == "ushort")  return new  PlyRecord<N, unsigned short>();
		else if (t == "int32" || t == "int")      return new  PlyRecord<N, int>();
		else if (t == "uint32" || t == "uint")    return new  PlyRecord<N, unsigned int>();
		else if (t == "float32" || t == "float")  return new  PlyRecord<N, float>();
		else if (t == "float64" || t == "double") return new  PlyRecord<N, double>();
		return NULL;
	}


	int readPlyFile(const char *pPath, pcrlib::IPcrLib *pLib, pcrlib::LibCallback *pCb)
	{
	
		pCb->message("Opening ply file...\n ");
		std::ifstream fs(pPath, std::ifstream::binary);
		if (!fs.is_open())
		{
			pCb->message("Error opening ply file\n");
			return -1;
		}
		// Read header
		std::string line;
		std::vector<std::string> hdr;
		while (std::getline(fs, line))
		{
			if (line.back() == '\r')  line.erase(line.length() -1);
			hdr.push_back(line);
			if (line == "end_header") break;
		}
		int data_start = fs.tellg();
		//Sanity check
		if (hdr[0] != "ply") 
		{
			pCb->message("Can not find magic ply\n");
			fs.close();
			return -1;
		}
		if (hdr.back() != "end_header")
		{
			pCb->message("Can not find end_header\n");
			fs.close();
			return -1;
		}

		//fs.close();

		//Parse header
		int num_verts = -1;
		std::vector<PlyRecordI*> propV;
		bool isVertElement = false,isError = false,isAscii = false,isBigEndian = false;
		for (std::vector<std::string>::size_type i = 0; i != hdr.size(); i++)
		{
			std::stringstream sts(hdr[i]);
			std::vector<std::string> words;
			std::string word;
			while (std::getline(sts, word, ' ')) { if (word.length() >0) words.push_back(word); }
			if (words[0] == "format")
			{
				if (words[1] == "ascii") isAscii = true;
				else if (words[1] == "binary_big_endian")     isBigEndian = true;
				else if (words[1] == "binary_little_endian")  isBigEndian = false;
				else { 
					pCb->message(("ERROR: unknown format " + words[1]).c_str());
					isError = true;
				}
			}
			else if (words[0] == "element")
			{
				if (words[1] == "vertex") {
					num_verts = std::stoi(words[2], NULL, 10);
					isVertElement = true;
				}
				else { 
					isVertElement = false;
				}
			}
			else if(  (words[0] == "property") && (isVertElement) )
			{
				PlyRecordI *plir = NULL;
				if (words[2] == "x")              { plir = getReader<0>(words[1]); }
				else if (words[2] == "y")         { plir = getReader<1>(words[1]); }
				else if (words[2] == "z")         { plir = getReader<2>(words[1]); }
				else if (words[2] == "intensity") { plir = getReader<3>(words[1]); }
				else                              { plir = getReader<-1>(words[1]);}
				if (plir == NULL) {
					pCb->message(("ERROR: unknown vertex type " + words[1]).c_str());
					isError = true;
				}
				else {
					propV.push_back(plir);
				}
			}
		}

		if (isError) {
			fs.close();
			return -1;
		}

		float res[4];
		size_t ss = 0;
		for (std::vector<PlyRecordI*>::iterator it = propV.begin(); it != propV.end(); ++it) ss += ((PlyRecordI*)(*it))->get_size();
		for (int n = 0; n < num_verts; n++) 
		{
			PlyRecordI::begin_line(isAscii, (int)ss, fs);
			for (std::vector<PlyRecordI*>::iterator it = propV.begin(); it != propV.end(); ++it)
			{
				PlyRecordI *pr = *it;
				pr->read_line(res,isAscii, isBigEndian);
			}
			if((n % 1023)==0) pCb->message(("\rProcessing:" + std::to_string(n) ).c_str());
			pLib->addPoint(10.0f*res[0], 10.0f*res[1], 10.0f*res[2], (unsigned int)(512.0f* res[3]));
		}

		fs.close();
		for (std::vector<PlyRecordI*>::iterator it = propV.begin(); it != propV.end(); ++it) 
		{
			PlyRecordI *pr = *it;
			delete pr;
		}
		pCb->message("\nDone reading ply\n");
		return 0;
	}
}//namespace pcrapp
