#ifndef __CESIUMPRO_H__
#define __CESIUMPRO_H__

#include <string>
#include <proj.h>
#include "Vec3d.h"
#include "EllipsoidModel.h"

#define RAD_TO_DEG    57.295779513082321
#define DEG_TO_RAD   .017453292519943296
class CesiumPro
{
public:
	CesiumPro();
	virtual ~CesiumPro(void);
	bool Create(const std::string& strSource, const std::string& strDest = "epsg:4326");
	void Clear();
	bool ConvertCoordinate(const Vec3d& vecSource, Vec3d& vecDest) const;
	void setVerticalName(const std::string& crcVerticalName);
	bool ProjConvertCoordinate(const Vec3d& vecSource, Vec3d& vecDest) const;
private:
	PJ_CONTEXT* m_Contex;
	PJ* m_src;
	PJ* m_dst;
	PJ* m_transformation;
	CesiumEllipsoidModel  m_ellipsoidModel;
	std::string m_VerticalName;
};
#endif