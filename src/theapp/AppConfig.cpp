#include <dirent.h>
#include <unistd.h>

#include <unistd.h>
#include "AppConfig.h"
#include "Utils.h"



BEGIN_LANDXML_READER
AppConfig::AppConfig()
{	
	m_isLambda = false;
	m_strConfigFiles = std::string(get_current_dir_name());		
}
AppConfig::~AppConfig()
{
}
bool AppConfig::Create(bool isLambda)
{
		m_strConfigFiles = std::string(get_current_dir_name());
	return true;
}
END_LANDXML_READER