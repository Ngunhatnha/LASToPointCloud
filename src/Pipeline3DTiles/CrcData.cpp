#include "CrcData.h"
#include <cstring>
CrcDatabase::CrcDatabase()
{
	m_isLoaddData = false;
}

CrcDatabase::~CrcDatabase()
{
}
const int CrcDatabase::getCrcValue(const std::string& epsgValue) const
{
	std::map<std::string, unsigned int>::const_iterator it=map_epsgTag.find(epsgValue);
	if (it != map_epsgTag.end())
		return it->second;
	else
		return -1;
}
const int CrcDatabase::getCrcValue(const std::string& epsgName, const std::string& CoordinateSystemName) const
{
	size_t nSize = m_vecData.size();
	for (size_t i=0;i< nSize;i++)
	{
		if ((CoordinateSystemName.find(m_vecData[i].epsgKey) != std::string::npos) ||
			(epsgName.find(m_vecData[i].epsgKey) != std::string::npos))
			return i;
	}
	return -1;
}
const CrcData* CrcDatabase::getCrcData(int index) const
{
	if(index>=0 && index< m_vecData.size())
	  return (&(m_vecData[index]));
	return NULL;
}

size_t CrcDatabase::Create(const std::string& strFile)
{
	m_vecData.clear();
	map_epsgTag.clear();
	m_isLoaddData = false;
	FILE* fp = fopen(strFile.c_str(), "rt");
	if (fp == NULL)
		return false;
	char pBuff[255];
	char* pData=NULL;
	size_t i, nSize;
	std::vector<size_t> vecIndex;
	while (fgets(pBuff, 255, fp))
	{
		nSize = strlen(pBuff);	
		if (nSize > 1)
		{
			pBuff[nSize - 1] = 0; nSize--;
			if (pBuff[nSize - 1] == '\r')
			{
				pBuff[nSize - 1] = 0;
				nSize--;
			}
			i = 0;
			// trim left
			while ((i< nSize) && (pBuff[i] == ' ' || pBuff[i] == '\t'))
				i++;
			// blank line
			if (i == nSize - 1)
				continue;
			pData = pBuff + i;
			nSize -= i;
			//////////////// comment
			if (pData[0] == '#')
				continue;
			vecIndex.clear();
			for (i = 0; i < nSize; i++)
			{
				if (pData[i] == '\t')
				{
					pData[i] = 0;
					vecIndex.push_back(i);
				}
			}
			nSize = vecIndex.size();
			if (nSize>=2)
			{
				CrcData mCrcData;

				mCrcData.epsgName = std::string(pData);
				mCrcData.epsgValue = std::string(pData + vecIndex[0] + 1);
				mCrcData.epsgKey = std::string(pData+ vecIndex[1]+1);				
				i = m_vecData.size();
				m_vecData.push_back(mCrcData);
				map_epsgTag[mCrcData.epsgValue] = i;
			}			
		}
	}
	fclose(fp);	
	m_isLoaddData = (m_vecData.size() > 0);
	return m_vecData.size();
}