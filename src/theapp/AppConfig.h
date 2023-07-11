#ifndef __LANDXML_READER_AppConfig_H
#define __LANDXML_READER_AppConfig_H
#include <stdio.h>
#include <string>
#include "LandXml.Reader.Types.h"
USING_LANDXML_READER
BEGIN_LANDXML_READER


class AppConfig
{
public:
	AppConfig();
	virtual ~AppConfig(void);
	bool Create(bool isLambda);
	inline const std::string& getPathConfig() const 
	{
		return m_strConfigFiles;
	}
	//inline const bool isLambda() const { return m_isLambda; }	
	inline const std::string getPathN2000() const
	{
		return m_strConfigFiles + "/" + "FIN2000_block.asc";
	}
	inline const std::string getPathN60() const
	{
		return m_strConfigFiles + "/" + "FIN2005N00.asc";
	}
	inline const std::string getPathCRC() const
	{
		return m_strConfigFiles + "/" + "crc_db.txt";
	}	
	inline const std::string getPathInstance() const
	{
		return m_strConfigFiles + "/" + "Instance_db.txt";
	}	
	inline const std::string getCatalogPROJ() const
	{
		return m_strConfigFiles + "/" + "geoid.ini";
	}
	inline const bool isLambda() const
	{
		return m_isLambda;
	}

private:	
	std::string m_strConfigFiles;
	bool m_isLambda;
};
END_LANDXML_READER
#endif