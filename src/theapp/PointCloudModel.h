#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <forward_list>
#include <list>
#include "LasToolToPnts.h"
#include "lasreader.hpp"
#include "lastransform.hpp"
#include "Vec3d.h"
#include "GeoIdHeight.h"

using namespace std;

class PointCloudModel
{
public:
	PointCloudModel(string epsg, string geoId);
	~PointCloudModel();
	int ReadPointCloud(const std::string& iFile, const BoundingBox& iBox, const GeoidPROJ* geo);

	string getEpsgCode();
	string getVerticalSystemName();

	void getBoundingBox(BoundingBox& worldBox) { worldBox.expandBy(box); };
	void getLocalBox(BoundingBox& worldBox, BoundingBox& localBox) { localBox.set(worldBox._min - box.center(), worldBox._max - box.center()); PointLocal(); };
	void getVecorg(Vec3d& vecOrg) { vecOrg.set(box.center()); };
	bool tranform(const Vec3d& vSource, Vec3d& vDest, const Tranform& mToECEF, const Tranform& mToWGS84,const GeoidPROJ* pGeoidPROJ) const;

	uint8_t ShortToChar(short RGB16) {
		double RGB64 = double(RGB16) / 65535;
		uint8_t RGB8 = uint8_t(round(RGB64 * 255));
		return RGB8;
	}
	void PointLocal();
	std::forward_list<PointInstance> m_forwardListPointCloud;
	std::list<CgPoint> m_listPointCloud;
	double a;
private:
	double minx=MAXFLOAT, miny= MAXFLOAT, minz= MAXFLOAT, maxx=-MAXFLOAT, maxy=-MAXFLOAT, maxz=-MAXFLOAT;
	string EpsgCode;
	string GeoId;
	BoundingBox box;
};


class PointCloudProcess {
public:
	PointCloudProcess();
	~PointCloudProcess();
	void getBoundingBox();
	void getPoint();
};