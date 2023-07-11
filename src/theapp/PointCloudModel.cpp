#include "PointCloudModel.h"
#include "Utils.h"
#include "EllipsoidModel.h"

PointCloudModel::PointCloudModel(string epsg,string geoid)
{
	EpsgCode = epsg;
	GeoId = geoid;
}
PointCloudModel::~PointCloudModel(void)
{
}
string PointCloudModel::getEpsgCode() {
	return EpsgCode;
}
string PointCloudModel::getVerticalSystemName() {
	return GeoId;
}
int PointCloudModel::ReadPointCloud(const std::string& iFile, const BoundingBox& iBox,const GeoidPROJ* geo) {
	CgPoint tmp;
	std::vector<CgPoint> Pnt;
	LASreadOpener lasreadopener;
	LASreader* lasreader = lasreadopener.open(iFile.c_str());
	
	Tranform toECEF,toWGS84;
	toWGS84.Create(EpsgCode, "epsg:4326");
	toECEF.Create("epsg:4326", "epsg:4978");
		
		/*
		minx = lasreader->header.min_x;
		miny = lasreader->header.min_y;
		minz = lasreader->header.min_z;
		maxx = lasreader->header.max_x;
		maxy = lasreader->header.max_y;
		maxz = lasreader->header.max_z;
		*/
	while (lasreader->read_point()) {
		Vec3d PointSourceTmp(lasreader->point.get_x(), lasreader->point.get_y(), lasreader->point.get_z());
		Vec3d PointDestTmp;
		tranform(PointSourceTmp, PointDestTmp, toECEF, toWGS84, geo);
		/*
		tmp.point.x = lasreader->point.get_x();
		tmp.point.y = lasreader->point.get_y();
		tmp.point.z = lasreader->point.get_z();
		*/
		tmp.point.x = PointDestTmp.getX();
		tmp.point.y = PointDestTmp.getY();
		tmp.point.z = PointDestTmp.getZ();

			if (tmp.point.x > maxx) maxx = tmp.point.x;
			if (tmp.point.x < minx) minx = tmp.point.x;
			if (tmp.point.y > maxy) maxy = tmp.point.y;
			if (tmp.point.y < miny) miny = tmp.point.y;
			if (tmp.point.z > maxz) maxz = tmp.point.z;
			if (tmp.point.z < minz) minz = tmp.point.z;

		tmp.rgb.R = ShortToChar(short(lasreader->point.rgb[0]));
		tmp.rgb.G = ShortToChar(short(lasreader->point.rgb[1]));
		tmp.rgb.B = ShortToChar(short(lasreader->point.rgb[2]));
		Pnt.push_back(tmp);
		m_listPointCloud.push_back(tmp);
	}
	for (int i = 0; i < Pnt.size(); i++) {
		//cout << Pnt[i].point.x << " "<< Pnt[i].point.y<<" "<< Pnt[i].point.z<<endl;
		//cout << Pnt[i].rgb.R << " " << Pnt[i].rgb.G << " " << Pnt[i].rgb.B << endl;
		PointInstance ag(Pnt[i]);
		m_forwardListPointCloud.push_front(ag);
	}

	if (EpsgCode != "epsg:4978") EpsgCode = "epsg:4978";
	
	box.set(minx, miny, minz, maxx, maxy, maxz);
	
	return Pnt.size();
}

void PointCloudModel::PointLocal() {
	Vec3d center(box.center());
	//Vec3d center(box.center().getY(), box.center().getX(), box.center().getZ());
	Vec3d tmpVec(-center);
	Matrixd translate;
	translate.makeTranslate(tmpVec);
	std::forward_list<PointInstance>::const_iterator it = m_forwardListPointCloud.begin();
	CgPoint pPointInstance;
	it = m_forwardListPointCloud.begin();
	float xyz[3];
	Vec3d newPoint;
	std::forward_list<PointInstance> tempo;
	while (it != m_forwardListPointCloud.end())
	{
		pPointInstance = (it++)->m_CgPoint;
		newPoint.set(pPointInstance.point.x, pPointInstance.point.y, pPointInstance.point.z);
		newPoint = translate.preMult(newPoint);
		CgPoint tm;
		tm.point.x = newPoint.getX();
		tm.point.y = newPoint.getY();
		tm.point.z = newPoint.getZ();

		tm.rgb.R = pPointInstance.rgb.R;
		tm.rgb.G = pPointInstance.rgb.G;
		tm.rgb.B = pPointInstance.rgb.B;
		PointInstance tmp(tm);
		tempo.push_front(tmp);
	}
	//tempo.reverse();
	m_forwardListPointCloud.swap(tempo);
}

bool PointCloudModel::tranform(const Vec3d& vSource, Vec3d& vDest, const Tranform& mToECEF, const Tranform& mToWGS84,const GeoidPROJ* pGeoidPROJ) const
{
	double xECEF, yECEF, zECEF, dLat, dLon, dHeight;
	mToWGS84.Convert(vSource.getY(), vSource.getX(), dLat, dLon);
	double dElev = vSource.getZ();
	if ((GeoId == "N2000" || GeoId == "N60") && pGeoidPROJ->getHeight(false, dLat, dLon, dHeight))
		dElev += dHeight;
	if (mToECEF.Convert(dLat, dLon, dElev, xECEF, yECEF, zECEF))
	{
		vDest = Vec3d(xECEF, yECEF, zECEF);
		return true;
	}
	return false;
}