#ifndef GEO_LASTOLAS_HPP
#define GEO_LASTOLAS_HPP
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lastransform.hpp"
#include "geoprojectionconverter.hpp"
#include "bytestreamout_file.hpp"
#include "bytestreamin_file.hpp"
#include "lasquadtree.hpp"
#include "lasvlrpayload.hpp"
#include "lasindex.hpp"
#include <string>

#include "GeoIdHeight.h"
#include "AppConfig.h"

#include "QuadTree.h"
#include "LasToolToPnts.h"
#include "QuadTreeObjectSizeBuilder.h"
#include <sys/stat.h>
class LasToLas
{
public:
	LasToLas();
	~LasToLas();
	bool tranform(const Vec3d& vSource, Vec3d& vDest, const Tranform& mToECEF, const Tranform& mToWGS84, const std::string& GeoId , const GeoidPROJ* pGeoidPROJ) const;
	int Create(const AppConfig m_AppConfig, int argc, char* argv[], const char* pMetaFileName,const bool isLAS, unsigned int& nEPSG, std::string& strCS, bool& isCreateLAZ,const std::string& geoId ="", const char* pMetaDestFileName = "") const;
	int getInfor(LASreader* lasreader, GeoProjectionConverter* pGeoProjectionConverter, const char* fileName, unsigned int& nEPSG, std::string& strCS) const;
	int LasToTileset(const AppConfig m_AppConfig, const std::string& input,const std::string& output, const std::string& EPSG, const std::string& geo_id, const double cell_size, const double geometry_scale) const;
private:
	bool m_isLambda;
	AppConfig m_AppConfig;
	int lidardouble2string(char* string, double value) const;
	I32 lidardouble2string(char* string, double value, double precision) const;
	bool valid_resolution(F64 coordinate, F64 offset, F64 scale_factor) const;
	void usage(bool error = false, bool wait = false) const;
	void byebye(bool error = false, bool wait = false) const;
	double taketime() const;
	bool save_vlrs_to_file(const LASheader* header) const;
	bool load_vlrs_from_file(LASheader* header) const;	
};
#endif