#ifndef _QuadTreeInstanceNode_h__
#define _QuadTreeInstanceNode_h__
#include "ObjectInterface.h"
#include <sstream>
#include <forward_list>
#include <map>
enum QuadTreeInstanceType
{
    ROOT_INSTANCE,
    BOTTOM_LEFT_INSTANCE,	// 1 
    BOTTOM_RIGHT_INSTANCE,	// 2     
    TOP_LEFT_INSTANCE,       // 3 
    TOP_RIGHT_INSTANCE,      // 4       
};

template <class T>
class QuadTreeInstanceNode
{
public:
    static void InitTreeObjectSize(int maxLevel, float minElev, float maxElev, unsigned int maxTriangleInFile, float scaleGeometricError);
    static void InitTree(int maxLevel, float minElev, float maxElev);
    QuadTreeInstanceNode(float _x, float _y, float _xSize, float _ySize, QuadTreeInstanceType _quadtreeNodeType, int _level);
    ~QuadTreeInstanceNode();
public:
    void BuildTree(int level);
    void InsertObject(T* object);
    std::forward_list<T*> GetObjectsAt(float px, float py, float x_size, float y_size);
    void RemoveObjectsAt(float px, float py, float x_size, float y_size);
    void ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeInstanceNode<T>*>& mapObject);
    void ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeInstanceNode<T>*>& mapObject);
    void getBox(float& _x, float& _y, float& _xSize, float& _ySize) const;
    inline const std::forward_list<T*>& getObjectList() const { return objectList; }
    BoundingBox getBoundingBox() const;
    void SetMinMaxElev(T* object);

private:
    bool IsContain(float px, float py, float x_size, float y_size, T* object) const;
    bool IsContain(float px, float py, float x_size, float y_size, QuadTreeInstanceNode<T>* octreeNode) const;
    bool IsInterSect(float px, float py, float x_size, float y_size, QuadTreeInstanceNode<T>* octreeNode) const;
    float calGeometricError(float xSize, float ySize, float zSize, float screenPixels) const;
public:
    std::forward_list<T*> objectList;
    static int m_maxLevel;
    static float m_minSize;    
    static float m_minHeight;
    static float m_maxHeight;
    static unsigned int m_maxTriangleInFile;
    static float m_scaleGeometricError;
private:
    QuadTreeInstanceType quadtreeInstanceNodeType;
    float x;
    float y;
    float xSize;
    float ySize;
    int level;
    float zmin=MAXFLOAT, zmax = -MAXFLOAT;
    QuadTreeInstanceNode* bottom_left_node;
    QuadTreeInstanceNode* bottom_right_node;
    QuadTreeInstanceNode* top_left_node;
    QuadTreeInstanceNode* top_right_node;

    bool isPath;
};
#endif//QuadTreeInstanceNode_h
