#ifndef __LANDXML_READER_LandXmlReaderService_H
#define __LANDXML_READER_LandXmlReaderService_H
#include <sstream>
#include <iostream>
#include <list>

#include <stdio.h>
#include <time.h>       /* time */
#include <sys/stat.h>
#include <unistd.h>     /*argument*/
#include <string.h>
#include <iomanip>
#include <dirent.h>

#include "LandXml.Reader.Types.h"
#include "S3CrtDrive.h"
#include "Utils.h"
#include "AppConfig.h"

USING_LANDXML_READER
BEGIN_LANDXML_READER

class LandXmlReaderService
{
public:
	LandXmlReaderService() {}
	virtual ~LandXmlReaderService(void){}
	bool setLambda(bool isLambda) {
		return m_AppConfig.Create(isLambda);
	}
	int CreateLambda(const std::string& strArgv);
	int Create(int argc, char* argv[]);
	int Create(const std::string& strArgv);
	inline const std::stringstream& getStreamOut() const
	{
		return m_ssOut;
	}
	int TestLASTool(std::string args);
	bool checkNum();
private:
	int ProcessLASTool();
	int ProcessLASToolGeo();
	int ProcessLASToolCrop();
	int ProcessLASToolPnts();
	std::vector<std::string> vecURL;
	std::string outFile, serverType, curDir, strCRCr, source_geoId ,epsgDest, geoId, cell_size, geometry_scale;
	EnumOut mEnumOut = TYPE_OUT_FILE;
	bool exportAtributes = false;	
	std::vector<std::string> crop;
	AppConfig m_AppConfig;
	std::stringstream m_ssOut;
};
END_LANDXML_READER
#endif