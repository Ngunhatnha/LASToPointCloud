#include "CesiumPro.h"
#include "GeoIdHeight.h"



CesiumPro::CesiumPro()
{
	m_Contex = nullptr;
	m_src = nullptr;
	m_dst = nullptr;
	m_transformation = nullptr;
}
bool CesiumPro::Create(const std::string& strSource, const std::string& strDest)
{
	Clear();
	m_Contex = proj_context_create();
	if (m_Contex == nullptr)
		return false;
	m_src = proj_create(m_Contex, strSource.c_str());
	if (m_src == nullptr)
	{
		Clear();
		return false;
	}
	m_dst = proj_create(m_Contex, strDest.c_str());
	if (m_dst == nullptr)
	{
		Clear();
		return false;
	}
	m_transformation = proj_create_crs_to_crs_from_pj(m_Contex, m_src, m_dst, nullptr, nullptr);
	if (m_transformation == nullptr)
	{
		Clear();
		return false;
	}
	return true;
}
CesiumPro::~CesiumPro()
{
	Clear();
}
void CesiumPro::Clear()
{
	if (m_src != nullptr)
	{
		proj_destroy(m_src);
		m_src = nullptr;
	}
	if (m_dst != nullptr)
	{
		proj_destroy(m_dst);
		m_dst = nullptr;
	}
	if (m_transformation != nullptr)
	{
		proj_destroy(m_transformation);
		m_transformation = nullptr;
	}
	if (m_Contex != nullptr)
	{
		proj_context_destroy(m_Contex);
		m_Contex = nullptr;
	}
	proj_cleanup();
}


bool CesiumPro::ConvertCoordinate(const Vec3d& vecSource, Vec3d& vecDest) const
{
	if (m_transformation == nullptr)
		return false;
	PJ_COORD coord;
	coord.xyzt.x = vecSource[1];
	coord.xyzt.y = vecSource[0];
	coord.xyzt.z = vecSource[2];
	coord.xyzt.t = HUGE_VAL;
	coord = proj_trans(m_transformation, PJ_FWD, coord);
	if (isnan(coord.xyzt.x) || isnan(coord.xyzt.y) || isnan(coord.xyzt.z))	
		return false;
	double lat = coord.xyz.x;
	double lon = coord.xyz.y;
	double height = coord.xyz.z;
	double geoidHeight = 0.0;

	//if (GeoidHeightCache::GetInstance().getHeightLinear(lat, lon, m_VerticalName, geoidHeight))
	if (GeoidPROJCache::GetInstance().getHeight(false, lat, lon, m_VerticalName, geoidHeight))
		height += geoidHeight;

	m_ellipsoidModel.convertLatLongHeightToXYZ(lat * DEG_TO_RAD, lon * DEG_TO_RAD, height, vecDest[0], vecDest[1], vecDest[2]);

	return true;
}


void CesiumPro::setVerticalName(const std::string& crcVerticalName)
{
	m_VerticalName = crcVerticalName;
}
bool CesiumPro::ProjConvertCoordinate(const Vec3d& vecSource, Vec3d& vecDest) const
{
	if (m_transformation == nullptr)
		return false;
	PJ_COORD coord;
	coord.xyzt.x = vecSource[0];
	coord.xyzt.y = vecSource[1];
	coord.xyzt.z = vecSource[2];
	coord.xyzt.t = HUGE_VAL;
	coord = proj_trans(m_transformation, PJ_FWD, coord);
	if (isnan(coord.xyzt.x) || isnan(coord.xyzt.y) || isnan(coord.xyzt.z))	
		return false;
	vecDest[0] = coord.xyz.x;
	vecDest[1] = coord.xyz.y;
	vecDest[2] = coord.xyz.z;

	return true;
}