#ifndef ___InstanceCatalog_H__
#define ___InstanceCatalog_H__
#include <string>
#include <map>
#include <cstring>

class InstanceCatalog
{
public:
	InstanceCatalog() {};
	virtual ~InstanceCatalog() {};
	size_t Create(const std::string& strFile)
	{
		m_mapInstancecatalog.clear();		
		FILE* fp = fopen(strFile.c_str(), "rt");
		if (fp == NULL)
			return 0;
		char pBuff[255];
		char* pData = NULL;
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
				while ((i < nSize) && (pBuff[i] == ' ' || pBuff[i] == '\t'))
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
				if (nSize >= 2)
					m_mapInstancecatalog[std::string(pData)] = std::make_pair(std::string(pData + vecIndex[0] + 1), atof(pData + vecIndex[1] + 1));		
			}
		}
		fclose(fp);		
		return m_mapInstancecatalog.size();
	}
	
	std::string getGlbFile(const std::string& strCode, double& dScale) const
	{
		std::map<std::string, std::pair<std::string, double>>::const_iterator it = m_mapInstancecatalog.find(strCode);
		if (it != m_mapInstancecatalog.end())
		{
			dScale = it->second.second;
			return it->second.first;
		}
		return std::string("");
	}
	void clear() 
	{ 
		m_mapInstancecatalog.clear(); 
	}
	inline bool isEmpty() const
	{
		return (m_mapInstancecatalog.size() == 0);
	}
private:
	std::map<std::string, std::pair<std::string, double>> m_mapInstancecatalog;
};
class InstanceCatalogCache
{
protected:
	InstanceCatalogCache() {}
public:	
	static InstanceCatalog& GetInstance()
	{
		static InstanceCatalog vInstanceCatalog;
		return vInstanceCatalog;
	}
	InstanceCatalogCache(const InstanceCatalogCache&) = delete;
	InstanceCatalogCache(InstanceCatalogCache&&) = delete;
	InstanceCatalogCache& operator=(const InstanceCatalogCache&) = delete;
	InstanceCatalogCache& operator=(const InstanceCatalogCache&&) = delete;
};
#endif