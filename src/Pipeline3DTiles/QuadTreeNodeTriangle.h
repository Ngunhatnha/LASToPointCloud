#ifndef _QuadTreeTriangleTriangle_h__
#define _QuadTreeTriangleTriangle_h__
#include "ObjectInterface.h"
#include <sstream>
#include <forward_list>
#include <map>
enum QuadTreeTriangleType
{
    ROOT,
    BOTTOM_LEFT,	// 1 
    BOTTOM_RIGHT,	// 2     
    TOP_LEFT,       // 3 
    TOP_RIGHT,      // 4       
};

template <class T>
class QuadTreeTriangle
{
public:
    static void InitTreeObjectSize(int maxLevel, float minElev, float maxElev, unsigned int maxTriangleInFile, double scaleGeometric);
    static void InitTree(int maxLevel, float minElev, float maxElev);
    QuadTreeTriangle(float _x, float _y, float _xSize, float _ySize, QuadTreeTriangleType _QuadTreeTriangleType, int _level);
    ~QuadTreeTriangle();
public:
    void BuildTree(int level);
    void InsertObject(T* object);
    std::forward_list<T*> GetObjectsAt(float px, float py, float x_size, float y_size);
    void RemoveObjectsAt(float px, float py, float x_size, float y_size);
    void ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeTriangle<T>*>& mapObject);
    void ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeTriangle<T>*>& mapObject);
    void getBox(float& _x, float& _y, float& _xSize, float& _ySize) const;
    inline const std::forward_list<T*>& getObjectList() const { return objectList; }
    float getGeometricError() const;
private:
    bool IsContain(float px, float py, float x_size, float y_size, T* object) const;
    bool IsContain(float px, float py, float x_size, float y_size, QuadTreeTriangle<T>* octreeNode) const;
    bool IsInterSect(float px, float py, float x_size, float y_size, QuadTreeTriangle<T>* octreeNode) const;
    float calGeometricError(float xSize, float ySize,float zSize, float screenPixels) const;
public:
    std::forward_list<T*> objectList;
    static int m_maxLevel;
    static float m_minSize;
    static float m_minHeight;
    static float m_maxHeight;
    static unsigned int m_maxTriangleInFile;
    static double m_scaleGeometric;
private:
    QuadTreeTriangleType  quadtreetriangleType;
    float x;
    float y;
    float xSize;
    float ySize;
    int level;
    QuadTreeTriangle* bottom_left_node;
    QuadTreeTriangle* bottom_right_node;
    QuadTreeTriangle* top_left_node;
    QuadTreeTriangle* top_right_node;

    bool isPath;
};
#endif//_QuadTreeTriangleTriangle_h__
