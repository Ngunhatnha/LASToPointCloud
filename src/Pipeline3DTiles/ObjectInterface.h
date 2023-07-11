#ifndef _OBJECT_h__
#define _OBJECT_h__
#include "Vec3d.h"
class Object
{
public:
    Object();
    ~Object();

    virtual inline const float xMin() const = 0;
    virtual inline const float yMin() const = 0;
    virtual inline const float zMin() const = 0;

    virtual inline const float xSizeBox() const = 0;
    virtual inline const float ySizeBox() const = 0;
    virtual inline const float zSizeBox() const = 0;
};
class ObjectIntance
{
public:
    ObjectIntance() {};
    ~ObjectIntance() {};

    virtual inline const float xMin() const = 0;
    virtual inline const float yMin() const = 0;
    virtual inline const float zMin() const = 0;

    virtual inline const float xSizeBox() const = 0;
    virtual inline const float ySizeBox() const = 0;
    virtual inline const float zSizeBox() const = 0;
};
enum ObjectType
{
    POINT_TYPE,
    TRIANGLE_TYPE,
    LINE_TYPE
};
class ObjectInterface
{
public:
    ObjectInterface() {};
    ~ObjectInterface() {};

    virtual inline const float xP1() const = 0;
    virtual inline const float xP2() const = 0;
    virtual inline const float xP3() const = 0;

    virtual inline const float yP1() const = 0;
    virtual inline const float yP2() const = 0;
    virtual inline const float yP3() const = 0;

    virtual inline const float zP1() const = 0;
    virtual inline const float zP2() const = 0;
    virtual inline const float zP3() const = 0;

    inline void setType(const ObjectType mObjectType) { m_ObjectType = mObjectType; }
    inline const ObjectType getType() const {return m_ObjectType;}
public:
    ObjectType m_ObjectType;
};


#endif//_OBJECT_h__
