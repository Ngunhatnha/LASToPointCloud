#ifndef __LANXML_READER_UTILS_H
#define __LANXML_READER_UTILS_H
#include <string>
#include <vector>
#include <set>
#include "LandXml.Reader.Types.h"
#include "CesiumPro.h"
#include "GeoIdHeight.h"
USING_LANDXML_READER
#define CSZ_VERSION std::string("PNTSReader 1.0")
#define PAGE_SIZE 64*1024*1024;
#define RAD_TO_DEG    57.295779513082321
#define DEG_TO_RAD   .017453292519943296
struct StackData
{
	void* pObject;
	void* pParam;
	size_t dStackSize;
};

class Utils
{
public:
	Utils();
	~Utils();

	static double getCircleSegmentDistance();
	static bool FileExists(const std::string& name);
	static int DeleteFilesInDirectory(std::string dirpath, bool recursive);
	static bool CreateThreadFunc(void* (*ThreadFunc)(void*), StackData* pStackData);

	static EnumOut ProcessArgumentsLambda(const std::string& strArgv, std::vector<std::string>& vecURL, std::string& outFile, std::string& serverType, std::string& strCRC, std::string& source_geoId, std::string& epsgDest, std::string& geoId,std::vector<std::string>& crop, std::string& cell_size, std::string& geometry_scale, bool& exportAtributes);

	static int CopyFile(const char* to, const char* from);
	static std::string validJSON(const std::string& strValue);
	static std::string validJSON(const char* strValue);
	static std::string UnicodeToJSON(const std::wstring& input);

	static unsigned int getRandomRGBAForBreakline();
	static unsigned int getRandomRGBA();
	static bool zip_directory(const std::string& srcDir, const std::string& zipFilename);
	static bool zip_directory(const std::string& inputdir, const std::string& baseFileOut, const unsigned int limitSize, std::vector<std::string>& outputVecFilename);

	static double calGeometricError(double radius, double screenPixels);
	static std::string generateGUID();
	static void getRGBA(unsigned int x, float& red, float& green, float& blue, float& alpha);
	static std::vector<std::string> split(const std::string& str, const std::string& delim);
	static std::vector<std::string> findSubdirs(const std::string& dir);
	static void splitFilepath(const std::string& filepath, std::string& dir, std::string& name, std::string& ext);
	static EnumOut ProcessArguments(int argc, char* argv[], std::vector<std::string>& vecURL, std::string& outFile, std::string& serverType, std::string& strCRC, std::string& epsgDest, std::string& geoId, std::vector<std::string>& crop, bool& exportAtributes);
	static EnumOut ProcessArguments(const std::string& strArgv, std::vector<std::string>& vecURL, std::string& outFile, std::string& serverType, std::string& strCRC, std::string& epsgDest, std::string& geoId, std::vector<std::string>& crop, bool& exportAtributes);
	static void deleteAllFiles(const std::string& strDir);
	static void deleteFile(const std::string& strFile);
	static std::wstring convertUTF8toUTF16(const std::string& s);
	static std::string convertUTF16toUTF8(const std::wstring& ws);
	static bool ChangeStackSize(size_t sizeStack);
	/// https://www.maanmittauslaitos.fi/kartat-ja-paikkatieto/asiantuntevalle-kayttajalle/koordinaattimuunnokset

	static bool getPointOnLocalCoordinate(const Vec3d& vRefPoint, const Vec3d& vPoint, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointLocal);
	static bool getPointOnGlobalCoordinate(const Vec3d& vRefPoint, const Vec3d& vPoint, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointGlobal);
	static double getDistanceOnLocalCoordinate(const Vec3d& vRefPoint1, const Vec3d& vRefPoint2, const Vec3d& vPoint1, const Vec3d& vPoint2, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointLocal1, Vec3d& pointLocal2);
	static bool getHeight(const Vec3d& vRefPoint, const std::string& strGeoid, double& dHeight);
	static bool convertPointToGlobalCoordinate(const Vec3d& vPoint, const std::string& epsgCode, const std::string& strGeoid, Vec3d& pointGlobal);
	static bool convertPointToLocalCoordinate(const Vec3d& pointGlobal, const std::string& epsgCode, const std::string& strGeoid, Vec3d& vPoint);
	//////////// merge model to project plane
	static bool getModelCenter(const Vec3d& vInsertPoint, const Matrixd& mat, Vec3d& vModelCenter);
	static bool getMatrixTransformOnPlane(const Vec3d& vRefPoint, const Vec3d& vRefPointOnPlane, const Vec3d& localOrg, const Vec3d& vModelCenter,
		double heading, double pitch, double roll, double scale, Matrixd& matTransform, Vec3d& globalOrg);
	static bool getPositionOnGlobal(const Vec3d& vRefPoint, const Vec3d& vRefPointOnPlane, const Vec3d& localPoint, Vec3d& globalPoint);
	static bool getPositionOnLocal(const Vec3d& vRefPoint, const Vec3d& vRefPointOnPlane, const Vec3d& globalPoint, Vec3d& localPoint);
};
#endif
