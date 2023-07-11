#ifndef __CRC_DATABSE__H
#define __CRC_DATABSE__H
#include <string>
#include <vector>
#include <map>
struct CrcData
{
	std::string epsgKey;
	std::string epsgName;
	std::string epsgValue;
};
class CrcDatabase
{
public:
	CrcDatabase();
	~CrcDatabase();
	size_t Create(const std::string& strFile);
	const int getCrcValue(const std::string& epsgTag) const;
	const int getCrcValue(const std::string& epsgName,const std::string& CoordinateSystemName) const;
	const CrcData* getCrcData(int index) const;
	bool isLoad() const { return m_isLoaddData; }
private:
	bool m_isLoaddData;
	std::vector<CrcData> m_vecData;
	std::map<std::string, unsigned int> map_epsgTag;	
};

class CrcDatabaseCache
{
protected:
	CrcDatabaseCache() {}
public:
	static CrcDatabase& GetInstance()
	{
		static CrcDatabase vCrcDatabase;
		return vCrcDatabase;
	}
	CrcDatabaseCache(const CrcDatabaseCache&) = delete;
	CrcDatabaseCache(CrcDatabaseCache&&) = delete;
	CrcDatabaseCache& operator=(const CrcDatabaseCache&) = delete;
	CrcDatabaseCache& operator=(const CrcDatabaseCache&&) = delete;
};
#endif
