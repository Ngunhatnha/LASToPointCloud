#ifndef _LandXmlObjectInterface_h__
#define _LandXmlObjectInterface_h__
#include "LandXml.Reader.Types.h"
#include <forward_list>
#include "ObjectInterface.h"
USING_LANDXML_READER

class Triangle : public ObjectInterface
{
public:
    Triangle(TriangleMesh* mehObject, unsigned int idxTriangle) :m_meshObject(mehObject), m_idxTriangle(idxTriangle) { setType(TRIANGLE_TYPE); }
    virtual ~Triangle() {}

    virtual inline const float xP1() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v1].x;
    }
    virtual inline const float xP2() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v2].x;
    }
    virtual inline const float xP3() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v3].x;
    }

    virtual inline const float yP1() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v1].y;
    }
    virtual inline const float yP2() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v2].y;
    }
    virtual inline const float yP3() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v3].y;
    }

    virtual inline const float zP1() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v1].z;
    }
    virtual inline const float zP2() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v2].z;
    }
    virtual inline const float zP3() const
    {
        return m_meshObject->vertices[m_meshObject->triangles[m_idxTriangle].v3].z;
    }
    inline const unsigned int getIndex() const { return m_idxTriangle; }
    inline const TriangleMesh* getMeshObject() const { return m_meshObject; }
    inline const IdxTriangle getIdxTriangle() const { return m_meshObject->triangles[m_idxTriangle]; }
private:
    TriangleMesh* m_meshObject;
    unsigned int m_idxTriangle;
};

class Line : public ObjectInterface
{
public:
    Line(Polyline* pPolyline, unsigned int idxLine) :m_pPolyline(pPolyline), m_idxLine(idxLine) { setType(LINE_TYPE); }
    virtual ~Line() {}

    virtual inline const float xP1() const
    {
        return m_pPolyline->vertices[m_pPolyline->lines[m_idxLine].v1].x;
    }
    virtual inline const float xP2() const
    {
        return m_pPolyline->vertices[m_pPolyline->lines[m_idxLine].v2].x;
    }
    virtual inline const float xP3() const
    {
        return 0.0;
    }

    virtual inline const float yP1() const
    {
        return m_pPolyline->vertices[m_pPolyline->lines[m_idxLine].v1].y;
    }
    virtual inline const float yP2() const
    {
        return m_pPolyline->vertices[m_pPolyline->lines[m_idxLine].v2].y;
    }
    virtual inline const float yP3() const
    {
        return 0.0;
    }

    virtual inline const float zP1() const
    {
        return m_pPolyline->vertices[m_pPolyline->lines[m_idxLine].v1].z;
    }
    virtual inline const float zP2() const
    {
        return m_pPolyline->vertices[m_pPolyline->lines[m_idxLine].v2].z;
    }
    virtual inline const float zP3() const
    {
        return 0.0;
    }
    inline const unsigned int getIndex() const { return m_idxLine; }
    inline const Polyline* getPolylineObject() const { return m_pPolyline; }
    inline const IdxLine getIdxLine() const { return m_pPolyline->lines[m_idxLine]; }
private:
    Polyline* m_pPolyline;
    unsigned int m_idxLine;
};

class PointInstance : public ObjectIntance
{
public:
    PointInstance(const CgPoint& point) {
        m_CgPoint = point;
        m_dScale = 1.0;
    }
    virtual ~PointInstance() {}

    virtual inline const float xMin() const {
        return m_CgPoint.point.x;
    }
    virtual inline const float yMin() const {
        return m_CgPoint.point.y;
    }
    virtual inline const float zMin() const {
        return m_CgPoint.point.z;
    }
    virtual inline const float xSizeBox() const
    {
        return 0;
    }
    virtual inline const float ySizeBox() const
    {
        return 0;
    }
    virtual inline const float zSizeBox()const
    {
        return 0;
    }
public:
    float m_dScale;
    CgPoint m_CgPoint;
};
#endif