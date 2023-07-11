#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sstream>
#include "GeoIdHeight.h"
#include <errno.h>
#include <sys/stat.h>

#define MATH_DREEG_TO_RADIAN 0.01745329251994329576923690768489


//////////////////////////////////////////////////////////
GeoidPROJ::GeoidPROJ()
{
	m_Contex = nullptr;
	m_dst = nullptr;
	m_pGeoidHeight= nullptr;
}
GeoidPROJ::~GeoidPROJ()
{
	Clear();
}
void GeoidPROJ::Clear()
{
	if (m_dst != nullptr)
	{
		proj_destroy(m_dst);
		m_dst = nullptr;
	}
	if (m_Contex != nullptr)
	{
		proj_context_destroy(m_Contex);
		m_Contex = nullptr;
	}
	proj_cleanup();
	//m_pGeoidHeight = nullptr;
}
bool GeoidPROJ::FileExists(const std::string& name) const
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}
bool GeoidPROJ::Create(GeoidHeight* pGeoidHeight, const std::string& strLocalPath, const std::string& strCfgFile)
{
    m_pGeoidHeight= pGeoidHeight;
	if (strCfgFile.empty())
		return false;
	Clear();
	m_Contex = proj_context_create();
	if (m_Contex == nullptr)
		return false;
	proj_context_set_enable_network(m_Contex, 0);
	proj_grid_cache_set_enable(m_Contex, false);
	proj_grid_cache_set_filename(m_Contex, nullptr);
	const char* path = strLocalPath.c_str();
	proj_context_set_search_paths(m_Contex, 1, &path);

	m_strLocalPath = strLocalPath;
	
	FILE* fp = fopen(strCfgFile.c_str(), "rt");
	if (fp == NULL)
		return false;
	m_mapEPSG.clear();
	char buff[4096];
	unsigned int nLine = 0;
	unsigned int nChar, i, nSize;
	std::vector<unsigned int> vecPos;
	while (fgets(buff, 4096, fp))
	{
		nChar = strlen(buff);
		if (nChar > 0 && buff[0] !='#')
		{
			vecPos.clear();
			for (i = 0; i < nChar; i++)
			{
				if (buff[i] == ';')
				{
					vecPos.push_back(i);
					buff[i] = 0;
				}
			}
			if (vecPos.size() > 2)
			{
				std::string strEPSG = std::string(buff + vecPos[0] + 1);
				std::string strFile= std::string(buff + vecPos[1] + 1);
				m_mapEPSG[strEPSG] = strFile;
			}
		}
	}
	fclose(fp);
	return true;
}
bool GeoidPROJ::setGeoid(const std::string& strEPSG)
{
	if (strEPSG == "3900" || strEPSG=="N2000")
	{
		m_strEPSG = "N2000";
		return true;
	}
	else if (strEPSG == "5717" || strEPSG=="N60")
	{
		m_strEPSG = "N60";
		return true;
	}
	if (strEPSG.empty() || m_Contex==nullptr)
		return false;
	m_strEPSG = strEPSG;
	if (m_dst != nullptr)
	{
		proj_destroy(m_dst);
		m_dst = nullptr;
	}
	std::string strCode = strEPSG;
	if (strEPSG == "EGM96")
		strCode = "5773";
	else if (strEPSG == "EGM2008")
		strCode = "3855";

	std::map<std::string, std::string>::const_iterator it = m_mapEPSG.find(strCode);

	if (it != m_mapEPSG.end())
	{
		//if (!S3Download(it->second))
			return false;
		//std::string strDest = "+proj=vgridshift +grids=";
		//strDest += it->second;
		//+proj = longlat + datum = WGS84 + no_defs + to + proj = vgridshift + grids = us_nga_egm96_15.tif
		//m_dst = proj_create(m_Contex, strDest.c_str());
		//if (m_dst == nullptr)		
			//return false;
		//return true;
	}
	return false;
}
bool GeoidPROJ::getHeight(bool isFWD,double lat, double lon, double& dHeight) const
{
	if (m_strEPSG == "N2000" || m_strEPSG == "N60")
	{
		if (m_pGeoidHeight!= nullptr && m_pGeoidHeight->getHeightLinear(lat, lon, m_strEPSG, dHeight))
		{
			if (isFWD)
				dHeight = -dHeight;
			return true;
		}
		return false;
	}
	if (m_dst == nullptr || m_Contex==nullptr)
		return false;
	dHeight = 0.0;
	double dLon = lon * MATH_DREEG_TO_RADIAN ;
	double dLat = lat * MATH_DREEG_TO_RADIAN;
	proj_trans_generic(m_dst, isFWD ? PJ_FWD : PJ_INV, &dLon, sizeof(double), 1, &dLat, sizeof(double), 1, &dHeight, sizeof(double), 1, nullptr, 0, 0);
	if (isnan(dHeight))
		return false;
	return true;
}
bool GeoidPROJ::getHeight(bool isFWD,double lat, double lon,const std::string& strEPSG,double& dHeight) const
{
	std::string strCode;
	if (strEPSG == "3900" || strEPSG == "N2000")
		strCode = "N2000";
	else if (strEPSG == "5717" || strEPSG == "N60")
		strCode = "N60";
	if(!strCode.empty())
	{
		if (m_pGeoidHeight != nullptr && m_pGeoidHeight->getHeightLinear(lat, lon, strCode, dHeight))
		{
			if(isFWD)
				 dHeight=-dHeight;
			return true;
		}
		return false;
	}
	if (strEPSG.empty() || m_Contex==nullptr)
		return false;
	
	strCode = strEPSG;
	if (strEPSG == "EGM96")
		strCode = "5773";
	else if (strEPSG == "EGM2008")
		strCode = "3855";

	std::map<std::string, std::string>::const_iterator it= m_mapEPSG.find(strCode);
	if (it != m_mapEPSG.end())
	{		
		//if (!S3Download(it->second))
			return false;
		//std::string strDest = "+proj=vgridshift +grids=";
		//strDest += it->second;
		//+proj = longlat + datum = WGS84 + no_defs + to + proj = vgridshift + grids = us_nga_egm96_15.tif
		//PJ*  mdst = proj_create(m_Contex, strDest.c_str());
		//if (mdst == nullptr)
		//{
			//strerror(strDest.c_str());
			//return false;
		//}
		//dHeight = 0.0;
		//double dLon= lon* MATH_DREEG_TO_RADIAN;
		//double dLat = lat * MATH_DREEG_TO_RADIAN;
		//proj_trans_generic(mdst, isFWD? PJ_FWD:PJ_INV, &dLon, sizeof(double), 1, &dLat,sizeof(double), 1, &dHeight, sizeof(double), 1,	nullptr, 0, 0);
		
		//if (isnan(dHeight))
			//return false;
		//proj_destroy(mdst);
		//return true;
	}
	return false;
}

//////////////////////////////////////////////////////////
Tranform::Tranform()
{
	m_Contex = nullptr;
	m_src = nullptr;
	m_dst = nullptr;
	m_transformation = nullptr;
}
bool Tranform::Create(const std::string& strSource, const std::string& strDest)
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
Tranform::~Tranform()
{
	Clear();
}
void Tranform::Clear()
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
bool Tranform::Convert(double latSrc, double lonSrc, double& latDest, double& lonDest) const
{
	if (m_transformation == nullptr)
		return false;
	PJ_COORD coord;
	coord.xyzt.x = latSrc;
	coord.xyzt.y = lonSrc;
	coord.xyzt.z = 0.0;
	coord.xyzt.t = HUGE_VAL;
	coord = proj_trans(m_transformation, PJ_FWD, coord);
	if (isnan(coord.xyzt.x) || isnan(coord.xyzt.y) || isnan(coord.xyzt.z)||
		isinf(coord.xyzt.x) || isinf(coord.xyzt.y) || isinf(coord.xyzt.z))
		return false;
	latDest = coord.xyz.x;
	lonDest = coord.xyz.y;

	return true;
}
bool Tranform::Convert(double xSrc, double ySrc, double zSrc,double& xDest, double& yDest, double& zDest) const
{
	if (m_transformation == nullptr)
		return false;
	PJ_COORD coord;
	coord.xyzt.x = xSrc;
	coord.xyzt.y = ySrc;
	coord.xyzt.z = zSrc;
	coord.xyzt.t = HUGE_VAL;
	coord = proj_trans(m_transformation, PJ_FWD, coord);
	if (isnan(coord.xyzt.x) || isnan(coord.xyzt.y) || isnan(coord.xyzt.z) ||
		isinf(coord.xyzt.x) || isinf(coord.xyzt.y) || isinf(coord.xyzt.z))
		return false;
	xDest = coord.xyz.x;
	yDest = coord.xyz.y;
	zDest = coord.xyz.z;
	return true;
}
///////////////////////////////////////////////////////
GeoidHeightLocal::GeoidHeightLocal()
{
	m_minLat = m_maxLat = m_minLon = m_maxLon = m_resLat = m_resLon = 0.0;
	m_value = NULL;
	m_nRow = m_nCol = 0;
}
GeoidHeightLocal::~GeoidHeightLocal()
{
	clear();
}
void GeoidHeightLocal::clear()
{
	if (m_value)
		delete[]m_value;
	m_minLat = m_maxLat = m_minLon = m_maxLon = m_resLat = m_resLon = 0.0;
	m_value = NULL;
	m_nRow = m_nCol = 0;
}
bool GeoidHeightLocal::init(const char* geoid_filename)
{
	FILE* fp = fopen(geoid_filename, "rt");
	if (fp == NULL)
		return false;
	clear();
	char buff[4096];
	unsigned int nLine = 0;
	unsigned int nChar, index = 0, i, nSize;
	while (fgets(buff, 4096, fp))
	{
		nChar = strlen(buff);
		if (nChar > 4)
		{
			nLine++;
			if (nLine == 1)
			{
				if (sscanf(buff, "%lf %lf %lf %lf %lf %lf", &m_minLat, &m_maxLat, &m_minLon, &m_maxLon, &m_resLat, &m_resLon) != 6)
				{
					fclose(fp);
					return false;
				}
				m_nCol = (m_maxLon - m_minLon) / m_resLon + 1;
				m_nRow = (m_maxLat - m_minLat) / m_resLat + 1;
				nSize = m_nCol * m_nRow;
				m_value = new double[nSize];
			}
			else
			{
				std::stringstream ss(buff);
				for (i = 0; i < m_nCol; i++)
				{
					if (!ss.eof())
					{
						if (index < nSize)
						{
							ss >> m_value[index];
							index++;
						}
						else
						{
							fclose(fp);
							clear();
							return false;
						}
					}
					else
						break;
				}
			}
		}
	}
	fclose(fp);
	return true;
}
double GeoidHeightLocal::_rawval(unsigned int col, unsigned int row) const
{
	unsigned int index = row * m_nCol + col;
	return m_value[index];
}
bool GeoidHeightLocal::getHeightLinear(double lat, double lon, double& geidHeight) const
{
	double v00, v01, v10, v11;
	unsigned int ix, iy;

	if ((lon < m_minLon) || (lon > m_maxLon) ||
		(lat < m_minLat) || (lat > m_maxLat))
		return false;
	double column = (lon - m_minLon) / m_resLon;
	double row = (m_maxLat - lat) / m_resLat;

	ix = (unsigned int)(column);
	iy = (unsigned int)(row);

	if (iy == (m_nRow - 1))
		iy -= 1;

	v00 = _rawval(ix, iy);
	v01 = _rawval(ix + 1, iy);
	v10 = _rawval(ix, iy + 1);
	v11 = _rawval(ix + 1, iy + 1);

	double vx = ((ix + 1) - column) * v00 + (column - ix) * v01;
	double vy = ((ix + 1) - column) * v10 + (column - ix) * v11;
	geidHeight = ((iy + 1) - row) * vx + (row - iy) * vy;
	return true;
}
/////////////////////////////////////////////////////// 
bool GeoidHeight::getHeightLinear(double lat, double lon, std::string strVerticalName, double& geoidHeight) const
{
	if (strVerticalName.empty())
		return false;
	if (strVerticalName == "N2000")
	{
		if (!m_GeoidN2000.isEmpty())
		{
			double latETRF89, lonETRF89;
			m_Tranform.Convert(lat, lon, latETRF89, lonETRF89);
			return m_GeoidN2000.getHeightLinear(latETRF89, lonETRF89, geoidHeight);
			//geoidHeight = getHeightLinear(lat, lon);
			return true;
		}
		return false;
	}
	if (strVerticalName == "N60")
	{
		if (!m_GeoidN60.isEmpty())
		{
			double latETRF89, lonETRF89;
			m_Tranform.Convert(lat, lon, latETRF89, lonETRF89);
			return m_GeoidN60.getHeightLinear(latETRF89, lonETRF89, geoidHeight);
		}
		return false;
	}
	if (strVerticalName == "EGM2008")
	{
		geoidHeight = getHeightLinear(lat, lon);
		return true;
	}
	return false;
}
GeoidHeight::GeoidHeight()
{
	m_geoid_ctx = NULL;
	m_hasTransform = false;
}
GeoidHeight::~GeoidHeight()
{
	clear(TYPE_GEOID_EGM2008_5);
	m_GeoidN2000.clear();
	m_GeoidN60.clear();
}

void GeoidHeight::clear(GeoidType type)
{
	if (type == TYPE_GEOID_EGM2008_5)
	{
		if (m_geoid_ctx)
		{
			munmap(m_geoid_ctx->base, m_geoid_ctx->total_len);
			free(m_geoid_ctx);
			m_geoid_ctx = NULL;
		}
	}
	else if (type == TYPE_GEOID_N2000)
		m_GeoidN2000.clear();
	else if (type == TYPE_GEOID_N60)
		m_GeoidN60.clear();
}
bool GeoidHeight::init(const char* geoid_pgm_filename, GeoidType type)
{
	if (!m_hasTransform)
		m_hasTransform = m_Tranform.Create();

	if (type == TYPE_GEOID_NONE)
		return false;
	else if (type == TYPE_GEOID_N2000)
		return m_GeoidN2000.init(geoid_pgm_filename);
	else if (type == TYPE_GEOID_N60)
		return m_GeoidN60.init(geoid_pgm_filename);

	if (m_geoid_ctx != NULL)
		clear(TYPE_GEOID_EGM2008_5);

	int fd;
	struct stat sbuf;
	char* p, * q, * r;
	size_t remain;
	unsigned long int have_line;
	unsigned int line_number;
	unsigned int expect_depth = 0;
	char line[150];
	size_t linelen;


	if ((fd = open(geoid_pgm_filename, O_RDONLY)) < 0)
	{
		fprintf(stderr, "geoid: cannot open \"%s\": %s\n", geoid_pgm_filename, strerror(errno));
		return false;
	}

	if (fstat(fd, &sbuf) < 0) {
		fprintf(stderr, "geoid: stat \"%s\" failed: %s\n", geoid_pgm_filename, strerror(errno));
		close(fd);
		return false;
	}
	if (sbuf.st_size < 30) {
		fprintf(stderr, "geoid: \"%s\" file wrong format (too small)\n", geoid_pgm_filename);
		close(fd);
		return false;
	}
	if (!(m_geoid_ctx = (geoid_ctx*)malloc(sizeof(*m_geoid_ctx)))) {
		fprintf(stderr, "geoid: malloc failed: %s\n", strerror(errno));
		close(fd);
		return false;
	}
	m_geoid_ctx->total_len = sbuf.st_size;
	m_geoid_ctx->offset = 0.0;
	m_geoid_ctx->scale = 1.0;
	if ((m_geoid_ctx->base = mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "geoid: mmap \"%s\" failed: %s\n", geoid_pgm_filename, strerror(errno));
		close(fd);
		free(m_geoid_ctx);
		return false;
	}
	close(fd);
	p = (char*)(m_geoid_ctx->base);
	remain = sbuf.st_size;
	if ((p[0] != 'P') || (p[1] != '5')) {
	nopgmheader:
		fprintf(stderr, "geoid: \"%s\": No PGM header\n", geoid_pgm_filename);
	error:
		munmap(m_geoid_ctx->base, sbuf.st_size);
		free(m_geoid_ctx);
		return false;
	}
	if ((p[2] == 13) && (p[3] == 10)) {
		p += 4;
		remain -= 4;
	}
	else if (p[2] == 10) {
		p += 3;
		remain -= 3;
	}
	else {
		goto nopgmheader;
	}
	line_number = 1;
	for (;;) {
		linelen = 0;
		have_line = 0;
		line_number++;
		while (remain) {
			if (linelen == (sizeof(line) - 1)) {
				fprintf(stderr, "geoid: \"%s\" line %d: Line too long in header\n", geoid_pgm_filename, line_number);
				goto error;
			}
			if ((remain > 1) && (p[0] == 13) && (p[1] == 10)) {
				p += 2;
				remain -= 2;
				have_line = 1;
				break;
			}
			else if (p[0] == 10) {
				p++;
				remain--;
				have_line = 1;
				break;
			}
			line[linelen++] = *(p++);
			remain--;
		}
		if (!have_line) {
			fprintf(stderr, "geoid: \"%s\" line %d: Reached EOF before end of line in header\n", geoid_pgm_filename, line_number);
			goto error;
		}
		line[linelen] = 0;

		if (expect_depth) {
			have_line = strtoul(line, &q, 10);
			if ((q == (&(line[0]))) || (*q)) {
				fprintf(stderr, "geoid: \"%s\" line %d: expected depth (single unsigned int)\n", geoid_pgm_filename, line_number);
				goto error;
			}
			if (have_line != 65535) {
				fprintf(stderr, "geoid: \"%s\": only PGM files with depth 65535 supported\n", geoid_pgm_filename);
				goto error;
			}
			break;
		}
		if (strncmp(line, "# Offset ", 9) == 0) {
			m_geoid_ctx->offset = strtod(line + 9, &q);
			if ((q == (&(line[9]))) || (*q)) {
				fprintf(stderr, "geoid: \"%s\" line %d: expected offset (float)\n", geoid_pgm_filename, line_number);
				goto error;
			}
			continue;
		}
		else if (strncmp(line, "# Scale ", 8) == 0) {
			m_geoid_ctx->scale = strtod(line + 8, &q);
			if ((q == (&(line[8]))) || (*q)) {
				fprintf(stderr, "geoid: \"%s\" line %d: expected scale (float)\n", geoid_pgm_filename, line_number);
				goto error;
			}
			continue;
		}
		else if (line[0] == '#') {
			continue;
		}
		m_geoid_ctx->width = (unsigned int)(strtoul(line, &q, 10));
		if ((q == (&(line[0]))) || (*q != ' ')) {
		whproblem:
			fprintf(stderr, "geoid: \"%s\" line %d: expected \"width height\"\n", geoid_pgm_filename, line_number);
			goto error;
		}
		r = q + 1;
		m_geoid_ctx->height = (unsigned int)(strtoul(r, &q, 10));
		if ((q == r) || (*q)) goto whproblem;
		expect_depth = 1;
	}
	if ((m_geoid_ctx->width * m_geoid_ctx->height * 2) != remain) {
		fprintf(stderr, "geoid: \"%s\": expected %d bytes after header, have %lu\n",
			geoid_pgm_filename,
			m_geoid_ctx->width * m_geoid_ctx->height * 2, (unsigned long)remain
		);
		goto error;
	}
	m_geoid_ctx->raw = (unsigned char*)p;
	m_geoid_ctx->lonres = m_geoid_ctx->width / 360.0;
	m_geoid_ctx->latres = (m_geoid_ctx->height - 1) / 180.0;
	return true;
}
unsigned int GeoidHeight::_rawval(int x, int y) const
{
	unsigned char* p;
	p = m_geoid_ctx->raw + ((y * m_geoid_ctx->width + x) << 1);
	return (p[0] << 8) | p[1];
}
double GeoidHeight::getHeightLinear(double lat, double lon) const
{
	double fx, fy, ixf, iyf;
	unsigned int v00, v01, v10, v11;
	unsigned int ix, iy;

	if (lon < 0.0) lon += 360.0;
	fy = modf((90 - lat) * m_geoid_ctx->latres, &iyf);
	fx = modf(lon * m_geoid_ctx->lonres, &ixf);
	ix = (unsigned int)ixf; iy = (unsigned int)iyf;
	if (iy == (m_geoid_ctx->height - 1)) iy -= 1;

	v00 = _rawval(ix, iy);
	v01 = _rawval(ix + 1, iy);
	v10 = _rawval(ix, iy + 1);
	v11 = _rawval(ix + 1, iy + 1);

	return m_geoid_ctx->offset + m_geoid_ctx->scale * (
		(1 - fy) * ((1 - fx) * v00 + fx * v01) +
		fy * ((1 - fx) * v10 + fx * v11)
		);
}
bool GeoidHeight::getHeightLinear(double lat, double lon, GeoidType type, double& geoidHeight) const
{
	switch (type)
	{
	case TYPE_GEOID_NONE:
		return false;
	case TYPE_GEOID_EGM2008_5:
		geoidHeight = getHeightLinear(lat, lon);
		return true;
	case TYPE_GEOID_N2000:
		return m_GeoidN2000.getHeightLinear(lat, lon, geoidHeight);
	case TYPE_GEOID_N60:
		return m_GeoidN60.getHeightLinear(lat, lon, geoidHeight);
	default:
		return false;
	}
}
#ifdef GEOID_HEIGHT_SUPPORT_CUBIC_INTERPOLATION
double GeoidHeight::getHeightCubic(double lat, double lon) const
{
	static const double c0 = 240.0;
	static const int c3[] = {
		  9, -18, -88,    0,  96,   90,   0,   0, -60, -20,
		 -9,  18,   8,    0, -96,   30,   0,   0,  60, -20,
		  9, -88, -18,   90,  96,    0, -20, -60,   0,   0,
		186, -42, -42, -150, -96, -150,  60,  60,  60,  60,
		 54, 162, -78,   30, -24,  -90, -60,  60, -60,  60,
		 -9, -32,  18,   30,  24,    0,  20, -60,   0,   0,
		 -9,   8,  18,   30, -96,    0, -20,  60,   0,   0,
		 54, -78, 162,  -90, -24,   30,  60, -60,  60, -60,
		-54,  78,  78,   90, 144,   90, -60, -60, -60, -60,
		  9,  -8, -18,  -30, -24,    0,  20,  60,   0,   0,
		 -9,  18, -32,    0,  24,   30,   0,   0, -60,  20,
		  9, -18,  -8,    0, -24,  -30,   0,   0,  60,  20,
	};
	static const double c0n = 372.0;
	static const int c3n[] = {
		  0, 0, -131, 0,  138,  144, 0,   0, -102, -31,
		  0, 0,    7, 0, -138,   42, 0,   0,  102, -31,
		 62, 0,  -31, 0,    0,  -62, 0,   0,    0,  31,
		124, 0,  -62, 0,    0, -124, 0,   0,    0,  62,
		124, 0,  -62, 0,    0, -124, 0,   0,    0,  62,
		 62, 0,  -31, 0,    0,  -62, 0,   0,    0,  31,
		  0, 0,   45, 0, -183,   -9, 0,  93,   18,   0,
		  0, 0,  216, 0,   33,   87, 0, -93,   12, -93,
		  0, 0,  156, 0,  153,   99, 0, -93,  -12, -93,
		  0, 0,  -45, 0,   -3,    9, 0,  93,  -18,   0,
		  0, 0,  -55, 0,   48,   42, 0,   0,  -84,  31,
		  0, 0,   -7, 0,  -48,  -42, 0,   0,   84,  31,
	};
	static const double c0s = 327.0;
	static const int c3s[] = {
		 18,  -36, -122,   0,  120,  135, 0,   0,  -84, -31,
		-18,   36,   -2,   0, -120,   51, 0,   0,   84, -31,
		 36, -165,  -27,  93,  147,   -9, 0, -93,   18,   0,
		210,   45, -111, -93,  -57, -192, 0,  93,   12,  93,
		162,  141,  -75, -93, -129, -180, 0,  93,  -12,  93,
		-36,  -21,   27,  93,   39,    9, 0, -93,  -18,   0,
		  0,    0,   62,   0,    0,   31, 0,   0,    0, -31,
		  0,    0,  124,   0,    0,   62, 0,   0,    0, -62,
		  0,    0,  124,   0,    0,   62, 0,   0,    0, -62,
		  0,    0,   62,   0,    0,   31, 0,   0,    0, -31,
		-18,   36,  -64,   0,   66,   51, 0,   0, -102,  31,
		 18,  -36,    2,   0,  -66,  -51, 0,   0,  102,  31,
	};
	double fx, fy, ixf, iyf;
	int v[12];
	double t[10];
	int ix, iy, i, j;
	int acc;
	double c0x;
	const int* c3x;

	if (lon < 0.0) lon += 360.0;
	fy = modf((90 - lat) * m_geoid_ctx->latres, &iyf);
	fx = modf(lon * m_geoid_ctx->lonres, &ixf);
	ix = ixf; iy = iyf;
	if (iy == (m_geoid_ctx->height - 1)) iy -= 1;

	v[0] = _rawval(ix, iy - 1);
	v[1] = _rawval(ix + 1, iy - 1);
	v[2] = _rawval(ix - 1, iy);
	v[3] = _rawval(ix, iy);
	v[4] = _rawval(ix + 1, iy);
	v[5] = _rawval(ix + 2, iy);
	v[6] = _rawval(ix - 1, iy + 1);
	v[7] = _rawval(ix, iy + 1);
	v[8] = _rawval(ix + 1, iy + 1);
	v[9] = _rawval(ix + 2, iy + 1);
	v[10] = _rawval(ix, iy + 2);
	v[11] = _rawval(ix + 1, iy + 2);

	if (iy == 0) {
		c3x = c3n;
		c0x = c0n;
	}
	else if (iy == (m_geoid_ctx->height - 2)) {
		c3x = c3s;
		c0x = c0s;
	}
	else {
		c3x = c3;
		c0x = c0;
	}

	for (i = 0; i < 10; i++) {
		acc = 0;
		for (j = 0; j < 12; j++) acc += v[j] * c3x[j * 10 + i];
		t[i] = ((double)acc) / c0x;
	}

	return m_geoid_ctx->offset + m_geoid_ctx->scale * (
		t[0] +
		fx * (t[1] + fx * (t[3] + fx * t[6])) +
		fy * (
			t[2] + fx * (t[4] + fx * t[7]) +
			fy * (t[5] + fx * t[8] + fy * t[9])
			)
		);
}
#endif