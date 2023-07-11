//*****************************************************************************
// Filename: LandXML.Reader.Types.h
//
// Author: Chu Van Huyen
// Copyright (C) 2020
//
// This file contains definitions and constants common for all data and types
// in the LandXML Reader Library.
//
//*****************************************************************************
#ifndef __LANDXML_READER_TYPES_H_
#define __LANDXML_READER_TYPES_H_

// INCLUDES
#pragma warning (push)
#pragma warning (disable: 4100 4201 4512 4251 4067 4197)
//#pragma warning (pop)
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <time.h>
#include "Vec3d.h"
#ifdef LAMBDA_CPP
#endif // LAMBDA_CPP
//*****************************************************************************
// Define the namespace
//*****************************************************************************
#define BEGIN_LANDXML_READER namespace LandXml { namespace Reader {
#define END_LANDXML_READER  } }
#define USING_LANDXML_READER using namespace LandXml::Reader;

//*****************************************************************************
// These definitions are used to resolve namespace ambiguities in a few 
// classes in the managed version of LandXMLReader.
//*****************************************************************************
#define LANDXML_READER LandXML::Reader

//*****************************************************************************
// "Global" variables.
//*****************************************************************************

BEGIN_LANDXML_READER

const double ABS_TOLERANCE_CITYGMLREDER = 0.005;
const double EPSILON_CITYGMLREDER = 1E-10;
//const double CS_UNDEFINE_ELEV = -1.79769e+308;

const double CS_PI = 3.1415926535897932384626433832795;
const double CS_PI_DIV_2 = 1.5707963267948966192313216916398;
const float CS_ZERO_DIAMETER = (float)0.0;
const double CS_CURVE_TO_SEMENT_LENGTH = 5.0;
const double CS_RADIAN_TO_DREEG = 57.295779513082320876798154814105;
const double CS_DREEG_TO_RADIAN = 0.01745329251994329576923690768489;
const double CS_TWO_PI = 6.283185307179586476925286766559;
const float CS_ALIGNMENT_ZERO_DIAMETER = (float)0.1;
const double CS_ALIGNMENT_TICK_LENGTH = 5.0;
const double CS_ALIGNMENT_TICK_WIDE = 0.4;
const double CS_ALIGNMENT_STATION_LENGTH = 20.0;
const double CS_ALTITUDE_PIPE_CROSS_TO_SEGMENT = 0.005;
const double CS_CGPOINT_SIZE = 1.0;
const double CS_UNDEFINE_ELEV = -999999.0;
const double CS_CURVE_TO_SEMENT_LENGTH_PROFILE = 2.0;
const double CS_ALIGNMENT_TO_SEGMENT_LENGTH = 2.0;
const double CS_ALTITUDE_CURVE_TO_SEGMENT = 0.005;

const double CS_LIMIT_ELEV_NEGATIVE = -10000.0;
const double CS_2D_ELEV = 0.0;

#define CS_INDEX_metLinear 0
#define CS_INDEX_metDiameter 1
#define CS_INDEX_metWidth 2
#define CS_INDEX_metHeight 3
#define CS_INDEX_radiansDirection 4 

// pipe elevation type bottom
const std::string PIPE_ELEV_TYPE_BOTTOM("vesijuoksu");
// pipe elevation type top
const std::string PIPE_ELEV_TYPE_TOP("laki");

const std::string PIPE_ELEV_CROWN_LEVEL("crown level");
const std::string PIPE_ELEV_CENTER_LEVEL("center level");
const std::string PIPE_ELEV_INVERT_LEVEL("invert level");
const std::string PIPE_ELEV_BOTTOM_LEVEL("bottom level");

enum PIPE_ELEV_TYPE_LANDXML
{
	ELEV_OTHER_LANDXML,
	ELEV_BOTTOM_LANDXML,
	ELEV_TOP_LANDXML,
	ELEV_CENTER_LANDXML,
	ELEV_INVERT_LANDXML
};


#define NODETYPE(_t_) LD_ ## _t_
// CityGML node types
enum LandXmlNodeType
{
	NODETYPE(Unknown) = 0,

	// core
	NODETYPE(LandXML),
	NODETYPE(Units),
	NODETYPE(Imperial),
    NODETYPE(Metric),
	NODETYPE(CoordinateSystem),
	NODETYPE(Start),

	NODETYPE(FeatureDictionary),
	NODETYPE(Project),
	NODETYPE(Application),
	NODETYPE(CgPoints),
	NODETYPE(CgPoint),
	NODETYPE(Survey),
	NODETYPE(Surfaces),
	NODETYPE(Surface),
	NODETYPE(Definition),
	NODETYPE(Pnts),
	NODETYPE(Point),
	NODETYPE(Faces),
	NODETYPE(Face),
	
	NODETYPE(SourceData),
	NODETYPE(Breaklines),
	NODETYPE(Breakline),
	NODETYPE(PntList2D),
	NODETYPE(PntList3D),

	NODETYPE(DataPoints),
	NODETYPE(Contours),
	NODETYPE(Contour),
	NODETYPE(Boundaries),
	NODETYPE(Boundary),
	NODETYPE(Watersheds),
	NODETYPE(Watershed),
	NODETYPE(SurfVolumes),
	NODETYPE(SurfVolume),


	//CoordinateSystem
	NODETYPE(name),
	NODETYPE(epsgCode),
	NODETYPE(rotationAngle),
	NODETYPE(verticalCoordinateSystemName),
	//FeatureDictionary
	NODETYPE(version),
	NODETYPE(DocFileRef),
	NODETYPE(location),
	//Project
	NODETYPE(desc),
	NODETYPE(Feature),
	NODETYPE(code),
	NODETYPE(source),
	NODETYPE(Property),
	NODETYPE(label),
	NODETYPE(value),
	//Application
	NODETYPE(manufacturer),
	NODETYPE(manufacturerURL),
	NODETYPE(timeStamp),
	NODETYPE(Author),
	NODETYPE(createdBy),
	NODETYPE(createdByEmail),
	NODETYPE(company),
	NODETYPE(companyURL),
	//CgPoints
	//Survey
	NODETYPE(SurveyHeader),
	// pipe networks
	NODETYPE(PipeNetworks),
	NODETYPE(PipeNetwork),
	NODETYPE(Structs),
	NODETYPE(Struct),
	NODETYPE(Center),
	NODETYPE(Invert),
	NODETYPE(StructFlow),
	NODETYPE(CircStruct),
	NODETYPE(RectStruct),
	NODETYPE(InletStruct),
	NODETYPE(OutletStruct),
	NODETYPE(Connection),

	NODETYPE(Pipes),
	NODETYPE(Pipe),
	NODETYPE(PipeFlow),
	NODETYPE(CircPipe),
	NODETYPE(EggPipe),
	NODETYPE(ElliPipe),
	NODETYPE(RectPipe),
	NODETYPE(Channel),
	// PlanFeature
	NODETYPE(PlanFeatures),
	NODETYPE(PlanFeature),
	NODETYPE(CoordGeom),
	NODETYPE(Line),
	NODETYPE(IrregularLine),
	NODETYPE(Curve),	
	NODETYPE(End),
    NODETYPE(PI),
	NODETYPE(Spiral),
	NODETYPE(Chain),
	NODETYPE(Location),
	// Alignments
    NODETYPE(Alignments),
	NODETYPE(Alignment),
	NODETYPE(Profile),
	NODETYPE(ProfAlign),
	NODETYPE(PVI),
	NODETYPE(ParaCurve),
	NODETYPE(UnsymParaCurve),
	NODETYPE(CircCurve)
};
enum LandXmlObjectsType
{
	LDT_GenericLandXmlObject = 1 << 0,
	LDT_Units = 1 << 1,
	LDT_CoordinateSystem = 1 << 2,
	LDT_FeatureDictionary = 1 << 3,
	LDT_Project = 1 << 4,
	LDT_Application = 1 << 5,
	LDT_CgPoints = 1 << 6,
	LDT_Survey = 1 << 7,
	LDT_Surfaces = 1 << 8,
	LDT_All = 0xFFFFFF
};

enum GeometryType
{
	LDT_Unknown = 0,
	LDT_CgPoint,
	LDT_Surface,
	LDT_Breakline,
	LDT_PntList3D,
	LDT_P,
	LDT_F
};

enum EnumOut
{
	TYPE_NONE,
	TYPE_ERROR_INPUT,
	TYPE_UNKNOWN_OPTION,
	TYPE_CONV,
	TYPE_OUT_FILE,
	TYPE_OUT_JSON,
	TYPE_CONV_WGS84,
	TYPE_GET_POSITION,
	TYPE_GET_DISTANCE,
	TYPE_GET_INV_POSITION,
	TYPE_VERSION,
	TYPE_SPATIALINDEX_QUADTREE,
	TYPE_SPATIALINDEX_QUADTREE_INSERT,
	TYPE_SPATIALINDEX_QUADTREE_MERGE,
	TYPE_SPATIALINDEX_QUADTREE_OBJECTSIZE,
	TYPE_ONEOBJECT_ONEFILE,
	TYPE_MERGE_JSON,
	TYPE_GET_HEIGHT,
	TYPE_GET_CVPOSITION,
	TYPE_PROJECT_CENTER,
	TYPE_PROJECT_MATONPLANE,
	TYPE_PROJECT_POSONLOCAL,
	TYPE_PROJECT_POSONGLOBAL,
	TYPE_PROJECT_CVTOLOCAL,
	TYPE_PROJECT_CVTOGLOBAL,
	TYPE_PROJECT_FIRST_CENTER,
	TYPE_LAS_TOOL,
	TYPE_LAS_TOOL_GEO,
	TYPE_LAS_TOOL_CROP,
	TYPE_LAS_TOOL_PNTS
};

typedef unsigned int LandXmlObjectsTypeMask;
std::string getLandXmlObjectsClassName(LandXmlObjectsTypeMask mask);
std::vector<std::string> tokenize(const std::string& str, const std::string& delimiters = ",|& ");
bool caseInsensitiveStringCompare(const std::string& str1, const std::string& str2);
LandXmlObjectsTypeMask getLandXmlObjectsTypeMaskFromString(const std::string& stringMask);

inline std::string ltrim_copy(const std::string& str)
{
	size_t nsize = str.size();
	int istart = 0;
	// trim left
	while ((istart < nsize) && (str[istart] == ' '))
		istart++;
	if (istart == nsize)
		return std::string();
	return str.substr(istart, (nsize - istart + 1));
}

inline std::string rtrim_copy(const std::string& str)
{
	size_t nsize = str.size();
	int iend = nsize - 1;	
	// trim right
	while ((iend >= 0) && (str[iend] == ' '))
		iend--;
	if (iend <= 0)
		return std::string();
	return str.substr(0, (iend + 1));
}

inline std::string trim_copy(const std::string& str)
{
	size_t nsize = str.size();
	int istart = 0;
	int iend = nsize-1;
	// trim left
	while ((istart < nsize) && (str[istart] == ' '))
		istart++;
	if(istart == nsize)
		return std::string();
	// trim right
	while ((iend >= 0) && (str[iend] == ' '))
		iend--;
	if((iend==0) || (iend<= istart))
		return std::string();
	return str.substr(istart, (iend- istart+1));
}

/**
** @class       glTF_Point
**
** Holds a 3D point or a vector
**
** @param x X-coordinate
** @param y Y-coordinate
** @param z Z-coordinate
*/


struct Point3f
{
	float x, y, z;
};

struct Point3d
{
	double x, y, z;
};

struct RGB
{
	unsigned short R = 0, G = 0, B = 0;

};

/**
** @class       glTF_Point2D
**
** Holds a 2D point or a vector
**
** @param x X-coordinate
** @param y Y-coordinate
*/
struct Point2f
{
	float x, y;
};

/**
** @class       glTFIdxTriangle
**
** Holds a triangle whose vertices are in a
** glTFPoint array. v1, v2 and v3 are
** indices to that array.
**
** @param v1 Index of the first vertex
** @param v2 Index of the second vertex
** @param v3 Index of the third vertex
** @param material Index of the material. Negative idx means global material idx
**  (returned by GetVMMaterialIdx(name), positive means local index
**  (0-based index of materials added by this plugin). See VMFileLib_Material
**  documentation for details.
*/
struct IdxTriangle
{
	unsigned int v1, v2, v3;
};

/**
** @class       VMFileLib_TriangleMesh
**
** Holds a triangle mesh. Size of vertices, normals and
** texcoords arrays is numVertices. triangles holds indices
** to thosae arrays and is numTriangles long.
** infoIdxs is an array storing the indexes to the
** VMInfos of each triangle. It is is numTriangles long.
** Meshes can be used to create any arbitrary 3D object in to the virtual
** model.
**
** VMFileLib_TriangleMesh is used for defining both VM Meshes and
** VM Surfaces.
** VM Surface model means a terrain which consists of triangles elements. It
** is the base surface for all other elements in the model. VM Meshes can be
** used to create any arbitrary 3D object in to the virtual model.
**
** Parameters
**
** @param numVertices
**  Number of vertices in the mesh
**
** @param vertices
**  The vertices of the triangles in the mesh.
**
** @param normals
**  The normals of vertices of the triangles in the mesh.
**
** @param texcoords
**  The texture coordinates of vertices of the triangles in the model.
**
** @param numTriangles
**  Number of triangles in the mesh
**
** @param triangles
**  The indices of the vertices of each triangle
**
** @param groupIdx
**  The index of the group of these items.
**
** @param infoIdx
**  Info idx.
**
** @param cutMode
**  (For surface models only) Whether the models is cut into the existing surfaces.
**  Mode OnlyCutDontInsertTriangles means that a hole is cut into the original
**  surface using the border of this surfaces, but the triangles of this surface are
**  not inserted. OnlyWeldDontInsertTriangles is the same but cut is done in weld mode.
*/
struct TriangleMesh
{
	std::string name;
	std::string GUID;
	unsigned int numVertices;
	Point3f* vertices;
	Point3f* normals;
	unsigned int numTriangles;
	IdxTriangle* triangles;
	BoundingBox localBox;	
	unsigned int rgbaColor;
};

struct TriangleMeshRef
{
	const TriangleMesh* pTriangleMesh;
	std::vector<unsigned int> vertices;
	std::vector<IdxTriangle> triangles;
	BoundingBox localBox;
};
//-------------------------------------- Polyline
struct IdxLine
{
	unsigned int v1, v2;
};
struct Polyline
{
	std::string name;
	std::string GUID;
	unsigned int numVertices;
	Point3f* vertices;	
	unsigned int numLines;
	IdxLine* lines;
	BoundingBox localBox;
	unsigned int rgbaColor;
};
struct PolylineRef
{
	const Polyline* pPolyline;
	std::vector<unsigned int> vertices;
	std::vector<IdxLine> lines;
	BoundingBox localBox;
};
//-------------------------------------- Point
struct CgPoint
{
	std::string name;
	std::string GUID;	
	std::string code;
	Point3d point;		
	RGB rgb;
};
struct PointSetRef
{
	const CgPoint* pPoint;	
	BoundingBox localBox;
};
/*
static uint64_t GetTickCountMs()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
}
/// Computes the elapsed time, in milliseconds, between two 'timespec'.
inline uint32_t TimeElapsedMs(const struct timespec& tStartTime, const struct timespec& tEndTime)
{
	return 1000 * (tEndTime.tv_sec - tStartTime.tv_sec) +
		(tEndTime.tv_nsec - tStartTime.tv_nsec) / 1000000;
}
*/
END_LANDXML_READER


#endif //__LANDXML_READER_TYPES_H_
