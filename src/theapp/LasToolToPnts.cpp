#include <sys/stat.h>
#include "LasToolToPnts.h"
#include "Vec3d.h"

////////////////////////////////////////////////// cmpt PNTS

void LasToolToFilePnts::clear()
{
	std::map<BoundingBox*, std::forward_list<PointInstance*>*>::const_iterator it = m_mapInstance.begin();
	std::forward_list<PointInstance*>* pPointInstance;
	while (it != m_mapInstance.end())
	{
		pPointInstance = it->second;
		while (!pPointInstance->empty())
		{
			delete pPointInstance->front();
			pPointInstance->pop_front();
		}
		it++;
	}
}
std::string LasToolToFilePnts::putPosition(BoundingBox* box,PointInstance* pPointInstance)
{
	if (pPointInstance == NULL)
		return std::string("");
	std::string strGlb;
	double dScale = 0.0;
	//strGlb = InstanceCatalogCache::GetInstance().getGlbFile(Utils::convertUTF16toUTF8(pPointInstance->m_CgPoint.code), dScale);
	/*
	strGlb = InstanceCatalogCache::GetInstance().getGlbFile(pPointInstance->m_CgPoint.code, dScale);
	if (strGlb.empty())
	{
		strGlb = InstanceCatalogCache::GetInstance().getGlbFile("DEFAULT", dScale);
		if (strGlb.empty())
		{
			strGlb = "DefaultPoint.glb";
			dScale = 1.0;
		}
	}*/
		
	pPointInstance->m_dScale = dScale;
	std::map<BoundingBox*, std::forward_list<PointInstance*>*>::const_iterator it = m_mapInstance.find(box);
	std::forward_list<PointInstance*>* pListPointInstance;
	if (it != m_mapInstance.end())
	{
		pListPointInstance = it->second;
		pListPointInstance->push_front(pPointInstance);
	}
	else
	{
		pListPointInstance= new std::forward_list<PointInstance*>;
		pListPointInstance->push_front(pPointInstance);
		m_mapInstance[box] = pListPointInstance;
	}
	return strGlb;
}
bool LasToolToFilePnts::writeCmpt(const std::string& strFile,const std::string& strData)
{
	if (strFile.empty() || strData.empty() ||
		m_mapInstance.size() == 0)
		return false;

	FILE* fp = fopen(strData.c_str(), "wb");
	if (fp == NULL)
		return false;
	
	std::map<BoundingBox*, std::forward_list<PointInstance*>*>::const_iterator it = m_mapInstance.begin();	
	while (it != m_mapInstance.end())
	{		
		// write i3dm
		LasToolToPnts mLasToolToPnts;
		mLasToolToPnts.writePointCloud(fp, *it->first,it->second);		
		it++;
	}		
	fclose(fp);
	
	fp = fopen(strFile.c_str(), "wb");
	if (fp == NULL)
		return false;

	FILE* fileData = fopen(strData.c_str(), "rb");
	if (fileData == NULL)
	{
		fclose(fp);
		return false;
	}
	unsigned char pBuff[4096];
	size_t nByte;
	do
	{
		nByte = fread(pBuff, 1, 4096, fileData);
		if (nByte > 0)
			fwrite(pBuff, 1, nByte, fp);
	} while (nByte > 0);
	fclose(fileData);
	fclose(fp);	
	return true;
}

///////////////////////////////////////////////// PNTS
LasToolToPnts::LasToolToPnts()
{
    
}
LasToolToPnts::~LasToolToPnts(void)
{

}

unsigned int LasToolToPnts::writePointCloud(FILE* fp, const BoundingBox& iBox,std::forward_list<PointInstance*>* pListPoint) const
{
	size_t numPoint = std::distance(pListPoint->begin(), pListPoint->end());
	if (numPoint == 0)
		return false;
	if (fp == NULL)
		return false;

	std::stringstream ss_pnts;
	//std::stringstream ss_i3dm;
	ss_pnts.setf(std::ios::fixed, std::ios::floatfield);
	//ss_b3dm.precision(10);

	unsigned int i, nFileSize = 0;
	unsigned int mHeader[7]; // page 66
	mHeader[0] = 0x73746E70; // pnts	             
	mHeader[1] = 1;        // version 1
	mHeader[2] = 0;        // byteLength
	mHeader[3] = 0;        // fetureTableJSONByteLength
	mHeader[4] = 0;        // fetureTableBinaryByteLength
	mHeader[5] = 0;        // batchTableJSONByteLength 
	mHeader[6] = 0;        // batchTableBinaryByteLength

	//fetureTableJSON
	unsigned int sizePosition = sizeof(float) * 3 * numPoint;
	unsigned int sizeRGB = sizeof(uint8_t) *3 * numPoint;

	Vec3d center(iBox.center());
	Vec3d tmpVec(-center);
	Matrixd translate;
	translate.makeTranslate(tmpVec);

	ss_pnts << "{\"POINTS_LENGTH\":" << numPoint << ",\"RTC_CENTER\":[" << center[0] << "," << center[1] << "," << center[2] << "],\"POSITION\":{\"byteOffset\":0},\"RGB\":{\"byteOffset\":" << sizePosition << "}}";
	std::string strFeatureTable = ss_pnts.str();
	unsigned int nLength = 0;
	unsigned int nPadding = 0;

	nLength = strFeatureTable.length() + 7 * sizeof(unsigned int);
	nPadding = nLength % 8;
	if (nPadding != 0)
	{
		nPadding = 8 - nPadding;
		std::string strAdd;
		for (i = 0; i < nPadding; i++)
			strAdd += 0x20;
		ss_pnts << strAdd;
	}
	strFeatureTable = ss_pnts.str();

	mHeader[3] = strFeatureTable.length();        // fetureTableJSONByteLength
	ss_pnts.str("");

	//featureTableBinary

	// POSITION  float32[3] A 3-component array of numbers containing x, y, and z Cartesian coordinates for the position of the instance

	unsigned int nzero = 0;
	nPadding = (sizePosition + sizeRGB) % 8;
	if (nPadding != 0)
		nzero = 8 - nPadding;
	mHeader[4] = sizePosition + sizeRGB + nzero;        // fetureTableBinaryByteLength

	std::forward_list<PointInstance*>::const_iterator it = pListPoint->begin();
	PointInstance* pPointInstance;
	//batchTableJSON
	/*
	ss_i3dm << "{\"GUID\":[";
	std::forward_list<PointInstance*>::const_iterator it = pListPoint->begin();
	PointInstance* pPointInstance;
	i = 0;
	while (it != pListPoint->end())
	{
		pPointInstance = *(it++);
		//ss_i3dm << "\"" << Utils::convertUTF16toUTF8(pPointInstance->m_CgPoint.GUID) << "\"";
		ss_i3dm << "\"" << pPointInstance->m_CgPoint.GUID << "\"";
		if (i == numPoint - 1)
			ss_i3dm << "]}";
		else
			ss_i3dm << ",";
		i++;
	}*/
	/*
	i = 0;
	ss_i3dm << "\"name\":[";
	it = pListPoint->begin();
	while (it != pListPoint->end())
	{
		pPointInstance = *(it++);
		ss_i3dm << "\"" << pPointInstance->m_CgPoint.name << "\"";
		if (i == numPoint - 1)
			ss_i3dm << "],";
		else
			ss_i3dm << ",";
		i++;
	}
	i = 0;
	ss_i3dm << "\"code\":[";
	it = pListPoint->begin();
	while (it != pListPoint->end())
	{
		pPointInstance = *(it++);
		ss_i3dm << "\"" << pPointInstance->m_CgPoint.code << "\"";
		if (i == numPoint - 1)
			ss_i3dm << "]}";
		else
			ss_i3dm << ",";
		i++;
	}
	*/
	/*
	std::string strBatchTable = ss_i3dm.str();
	nLength = strBatchTable.length();
	nPadding = nLength % 8;
	if (nPadding != 0)
	{
		nPadding = 8 - nPadding;
		std::string strAdd;
		for (i = 0; i < nPadding; i++)
			strAdd += 0x20;
		ss_i3dm << strAdd;
	}
	strBatchTable = ss_i3dm.str();
	mHeader[5] = strBatchTable.length();        // batchTableJSONByteLength
	ss_i3dm.str("");
	*/
	// get file name size of glb
	//struct stat st;
	//stat(iFileModel.c_str(), &st);
	//size_t glbSize = st.st_size;

	mHeader[2] = 28 + mHeader[3] + mHeader[4] + mHeader[5];

	nPadding = mHeader[2] % 8;
	if (nPadding != 0)
	{
		nPadding = 8 - nPadding;
		mHeader[2] += nPadding;
	}

	// write to file
	// write header
	if (fwrite(mHeader, sizeof(unsigned int), 7, fp) != 7)
	{
		fclose(fp);
		return false;
	}
	nFileSize += 28;
	// write featute table	
	if (fwrite(strFeatureTable.c_str(), 1, strFeatureTable.length(), fp) != strFeatureTable.length())
	{
		fclose(fp);
		return false;
	}
	nFileSize += strFeatureTable.length();
	strFeatureTable.clear();


	//write featureTableBinary
	it = pListPoint->begin();
	float xyz[3];
	Vec3d newPoint;
	while (it != pListPoint->end())
	{
		pPointInstance = *(it++);
		newPoint.set(pPointInstance->m_CgPoint.point.x, pPointInstance->m_CgPoint.point.y, pPointInstance->m_CgPoint.point.z);
		newPoint = translate.preMult(newPoint);
		xyz[0] = newPoint.getX();
		xyz[1] = newPoint.getY();
		xyz[2] = newPoint.getZ();
		if (fwrite(xyz, sizeof(float), 3, fp) != 3)
		{
			fclose(fp);
			return false;
		}
		nFileSize += (3 * sizeof(float));
	}

	it = pListPoint->begin();
	uint8_t rgb[3];
	while (it != pListPoint->end())
	{
		pPointInstance = *(it++);
		rgb[0] = pPointInstance->m_CgPoint.rgb.R;
		rgb[1] = pPointInstance->m_CgPoint.rgb.G;
		rgb[2] = pPointInstance->m_CgPoint.rgb.B;
		if (fwrite(rgb, sizeof(uint8_t), 3, fp) != 3)
		{
			fclose(fp);
			return false;
		}
		nFileSize += (1 * sizeof(float));
	}


	char cZero = 0;
	for (i = 0; i < nzero; i++)
	{
		if (fwrite(&cZero, 1, 1, fp) != 1)
		{
			fclose(fp);
			return false;
		}
	}
	/*
	// strBatchTable
	if (fwrite(strBatchTable.c_str(), 1, strBatchTable.length(), fp) != strBatchTable.length())
	{
		fclose(fp);
		return false;
	}
	strBatchTable.clear();
	*/
	if (nPadding > 0)
	{
		char cValue = 0;
		for (unsigned int i = 0; i < nPadding; i++)
		{
			if (fwrite(&cValue, 1, 1, fp) != 1)
			{
				fclose(fp);
				return false;
			}
		}
	}
	fflush(fp);
	return mHeader[2];
}
