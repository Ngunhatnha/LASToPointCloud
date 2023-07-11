#ifndef _LasToolTOPNTS_H__
#define _LasToolTOPNTS_H__
#include <vector>
#include <map>
#include "LandXml.Reader.Types.h"
#include "LandXmlObjectInterface.h"
#include "InstanceCatalog.h"
#include "Vec3d.h"

#include <vector>
#include <sstream>
#include <iostream>

#include <algorithm>
#include <fstream>
#include <forward_list>
#include <memory>
USING_LANDXML_READER

class LasToolToPnts
{
protected:
	struct pntsData
	{
		unsigned short nBatID;
		PointSetRef* pPointSetRef;		
	};
	class pntsInOut
	{
	public:
		pntsInOut() :_hasFile(0) {}
		~pntsInOut()
		{
			if (_hasFile)
				_mFile.close();
			_mSS.str("");
		}
		inline bool Create(const std::string& strFile)
		{
			_mFile.open(strFile.c_str(), std::fstream::out);
			_mFile.precision(15);
			_mFile.setf(std::ios::fixed, std::ios::floatfield);

			_mSS.str("");
			_mSS.precision(15);
			_mSS.setf(std::ios::fixed, std::ios::floatfield);

			_hasFile = _mFile.is_open();
			return _hasFile;
		}
		inline pntsInOut& operator <<  (const std::string& str)
		{
			if (_hasFile)
				_mFile << str;
			else
				_mSS << str;
			return *this;

		}
		/*inline i3dmInOut& operator <<  (const char* str)
		{
			if (_hasFile)
				_mFile << str;
			else
				_mSS << str;
			return *this;

		}*/
		inline pntsInOut& operator <<  (const double dValue)
		{
			if (_hasFile)
				_mFile << dValue;
			else
				_mSS << dValue;
			return *this;
		}
		inline pntsInOut& operator <<  (const float dValue)
		{
			if (_hasFile)
				_mFile << dValue;
			else
				_mSS << dValue;
			return *this;
		}
		inline pntsInOut& operator <<  (const unsigned int iValue)
		{
			if (_hasFile)
				_mFile << iValue;
			else
				_mSS << iValue;
			return *this;
		}
		inline pntsInOut& operator <<  (const int iValue)
		{
			if (_hasFile)
				_mFile << iValue;
			else
				_mSS << iValue;
			return *this;
		}
		inline pntsInOut& operator <<  (const size_t iValue)
		{
			if (_hasFile)
				_mFile << iValue;
			else
				_mSS << iValue;
			return *this;
		}
		inline bool is_open() const
		{
			return _hasFile;
		}
		inline void clear()
		{
			if (_hasFile)
				_mFile.close();
			else
				_mSS.str("");
		}
		inline const std::stringstream& getSSBuff() const
		{
			return _mSS;
		}
	private:
		bool _hasFile;
		std::ofstream _mFile;
		std::stringstream _mSS;
	};
public:
	LasToolToPnts();
	~LasToolToPnts();	
	unsigned int writePointCloud(FILE* fp, const BoundingBox& iBox, std::forward_list<PointInstance*>* pListPoint) const;
private:
	std::vector<pntsData> m_vecPntsData;
};

class LasToolToFilePnts : public LasToolToPnts
{
public:
	LasToolToFilePnts() {};
	~LasToolToFilePnts() { clear(); };
	std::string putPosition(BoundingBox* box,PointInstance* pPointInstance);
	bool writeCmpt(const std::string& strFile, const std::string& strData);
	void clear();
private:
	std::map<BoundingBox*, std::forward_list<PointInstance*>*> m_mapInstance;
}; 
#endif