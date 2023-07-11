#ifndef _QuadTreeObjectSizeBuilder_h__
#define _QuadTreeObjectSizeBuilder_h__
#include "LandXml.Reader.Types.h"
#include <forward_list>
#include "ObjectInterface.h"
#include "QuadTreeNodeTriangle.h"
#include "QuadTreeNodeTriangle.cpp"
//#include "LandXmlToglTF.h"
#include "LasToolToPnts.h"
#include "Base64encoder.h"

#include "QuadTreeInstanceNode.h"
#include "QuadTreeInstanceNode.h"
#include "QuadTreeInstanceNode.cpp"
#include "LandXmlObjectInterface.h"
#include "GeoIdHeight.h"
#include "PointCloudModel.h"
#define CS_MAX_DEPTH_QUAD_TREE 12
struct ParamDataInstanceObjectSize
{
    unsigned int indexFile;
    std::stringstream sstream;
    std::map<std::string, QuadTreeInstanceNode<ObjectIntance>*> mapObject;
};
class QuadTreeObjectSizeBuilder
{
public:
    QuadTreeObjectSizeBuilder(GeoidPROJ* pGeoidPROJ,PointCloudModel* pPointCloudModel,double cellSize,double scaleGeometricError) : m_isPointGlobal(false), m_pPointCloudModel(pPointCloudModel),
        _cellSize(cellSize), _scaleGeometricError(scaleGeometricError), m_pGeoidPROJ(pGeoidPROJ)
    {
    }
    virtual ~QuadTreeObjectSizeBuilder();

    void setCRC(const std::string& crcCode, const std::string& heightCode)
    {
        m_crcCode = "epsg:" + crcCode;
        m_heightCode = heightCode;
    }
    inline const std::string& getJsonBox() { return m_JsonBox; }
    inline const std::string& getLastErro() { return m_strLastErro; }
    int writeLandXmlObjectsToTileset(const std::string& localTile, const std::string& localPathB3dm);

private:
    bool getBoundingBoxFromPipe(const Vec3d& vecOrg, BoundingBox& mBox);
    
    double calGeometricError(double sRadius) const;

    std::string encodeExtras(double geoidHeight, const BoundingBox& mModelBox) const;
    bool ConvertCoordinate(const Vec3d& vec, double& lat, double& lon, double& height, const std::string& strSource, const std::string& strDest);
    
    bool writeSpatialIndexTitleset(const std::string& strTitle, const BoundingBox& mModelBox, const std::string& strJSONI3DM, double lat, double lon, double height, bool hasWGS84);
    
    bool writeLandXmlPnts(const std::string& localPnts, const std::forward_list<PointInstance*>& pListObjectPoints)const;

    unsigned int _maxTreeDepth;
    unsigned int _maxTriangleInFile;
    int totalPNTS=0;
    double _cellSize; // minimun edge box
    double _scaleGeometricError;
    PointCloudModel* m_pPointCloudModel;

    std::string m_JsonBox;
    std::string m_crcCode;
    std::string m_heightCode;
    std::string m_strLastErro;

    Vec3d m_RefPoint;
    Vec3d m_InsertPoint;
    Vec3d m_pointGlobal;
    bool m_isPointGlobal;
    bool m_isDraco;

    // for outrput
    Vec3d m_vecWorldOrg;
    BoundingBox m_worldBox, m_localBox;
    std::vector<BoundingBox> m_vecBox;
    GeoidPROJ* m_pGeoidPROJ;

};
#endif