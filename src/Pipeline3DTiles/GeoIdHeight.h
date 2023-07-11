#ifndef ___GEOID_H__
#define ___GEOID_H__
#include <vector>
#include <map>
#include <string>
#include <proj.h>

// GTX file format
// https://vdatum.noaa.gov/docs/gtx_info.html
//https://www.maanmittauslaitos.fi/kartat-ja-paikkatieto/asiantuntevalle-kayttajalle/koordinaattimuunnokset
enum GeoidType
{
	TYPE_GEOID_NONE,
	TYPE_GEOID_EGM2008_5,
	TYPE_GEOID_N2000,
	TYPE_GEOID_N60
};
class Tranform
{
public:
	Tranform();
	virtual ~Tranform(void);
	bool Create(const std::string& strSource = "epsg:4326", const std::string& strDest = "epsg:9059");
	void Clear();
	bool Convert(double latSrc, double lonSrc, double& latDest, double& lonDest) const;
	bool Convert(double xSrc, double ySrc, double zSrc, double& xDest, double& yDest, double& zDest) const;
private:
	PJ_CONTEXT* m_Contex;
	PJ* m_src;
	PJ* m_dst;
	PJ* m_transformation;
};


class GeoidHeightLocal
{
public:
	GeoidHeightLocal();
	virtual ~GeoidHeightLocal();
	bool init(const char* geoid_filename);
	void clear();
	bool getHeightLinear(double lat, double lon, double& geidHeight) const;
	inline bool isEmpty() const
	{
		return (m_value == NULL);
	}
private:
	double _rawval(unsigned int ix, unsigned int iy) const;
	double m_minLat, m_maxLat, m_minLon, m_maxLon, m_resLat, m_resLon;
	double* m_value;
	unsigned int m_nRow, m_nCol;
};
class GeoidHeight
{
public:
	GeoidHeight();
	virtual ~GeoidHeight();
	bool init(const char* geoid_pgm_filename, GeoidType type);
	void clear(GeoidType type);
	bool getHeightLinear(double lat, double lon, GeoidType type, double& geoidHeight) const;
	bool getHeightLinear(double lat, double lon, std::string strVerticalName, double& geoidHeight) const;

	inline bool isEmpty(GeoidType type) const
	{
		switch (type)
		{
		case TYPE_GEOID_NONE:
			return false;
		case TYPE_GEOID_EGM2008_5:
			return (m_geoid_ctx == NULL);
		case TYPE_GEOID_N2000:
			return m_GeoidN2000.isEmpty();
		case TYPE_GEOID_N60:
			return m_GeoidN60.isEmpty();
		default:
			return false;
		}
	}
#ifdef GEOID_HEIGHT_SUPPORT_CUBIC_INTERPOLATION
	double getHeightCubic(double lat, double lon) const;
#endif
private:
	struct geoid_ctx {
		/* raw should be (uint16_t *) but it might not be aligned */
		unsigned char* raw;
		double offset;
		double scale;
		double lonres;
		double latres;
		unsigned int width;
		unsigned int height;
		void* base;
		size_t total_len;
	};
	double getHeightLinear(double lat, double lon) const;
	unsigned int _rawval(int x, int y) const;
	geoid_ctx* m_geoid_ctx;
	GeoidHeightLocal m_GeoidN2000, m_GeoidN60;
	Tranform m_Tranform;
	bool m_hasTransform;
};

class GeoidPROJ
{
public:
	GeoidPROJ();
	virtual ~GeoidPROJ(void);
public:
	bool Create(GeoidHeight* pGeoidHeight, const std::string& strLocalPath, const std::string& strCfgFile);
	bool getHeight(bool isFWD,double lat, double lon,const std::string& strEPSG, double& dHeight) const;
	bool getHeight(bool isFWD,double lat, double lon,double& dHeight) const;
	bool setGeoid(const std::string& strEPSG);	
	const std::string& getGeoid() const
	{
		return m_strEPSG;
	}
	void Clear();
private:
	bool FileExists(const std::string& name) const;
	//bool S3Download(const std::string& strFile) const;
	std::map<std::string, std::string> m_mapEPSG;
	PJ_CONTEXT* m_Contex;
	PJ* m_dst;
	std::string m_strEPSG;
	std::string m_strLocalPath,m_strPath;
	GeoidHeight* m_pGeoidHeight;
};

class GeoidPROJCache
{
protected:
	GeoidPROJCache() {}
public:
	static GeoidPROJ& GetInstance()
	{
		static GeoidPROJ vGeoidPROJ;
		return vGeoidPROJ;
	}
	GeoidPROJCache(const GeoidPROJCache&) = delete;
	GeoidPROJCache(GeoidPROJCache&&) = delete;
	GeoidPROJCache& operator=(const GeoidPROJCache&) = delete;
	GeoidPROJCache& operator=(const GeoidPROJCache&&) = delete;
};
/////////////////////////////////////


//#define GEOID_HEIGHT_SUPPORT_CUBIC_INTERPOLATION 1


//class GeoidHeightCache
//{
//protected:
//	GeoidHeightCache() {}
//public:
//	static GeoidHeight& GetInstance()
//	{
//		static GeoidHeight vGeoidHeight;
//		return vGeoidHeight;
//	}
//	GeoidHeightCache(const GeoidHeightCache&) = delete;
//	GeoidHeightCache(GeoidHeightCache&&) = delete;
//	GeoidHeightCache& operator=(const GeoidHeightCache&) = delete;
//	GeoidHeightCache& operator=(const GeoidHeightCache&&) = delete;
//};

#endif