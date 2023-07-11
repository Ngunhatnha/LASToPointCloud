#ifndef _OctreeNode_h__
#define _OctreeNode_h__
#include "ObjectInterface.h"
#include <sstream>
#include <forward_list>
#include <map>
enum OctreeType
{
    ROOT,
    BOTTOM_LEFT_FRONT,		// 1 
    BOTTOM_RIGHT_FRONT,		// 2 
    BOTTOM_LEFT_BACK,		// 3 
    BOTTOM_RIGHT_BACK,      // 4 
    TOP_LEFT_FRONT,         // 5 
    TOP_RIGHT_FRONT,        // 6 
    TOP_LEFT_BACK,          // 7 
    TOP_RIGHT_BACK          // 8   
};

template <class T>
class OctreeNode
{
public:
    static void InitTree(int maxLevel);
    OctreeNode(float _x, float _y, float _z, float _xSize, float _ySize, float _zSize, OctreeType _octreeNodeType, int _level);
    ~OctreeNode();
public:
    void BuildTree(int level);
    void InsertObject(T* object);
    std::forward_list<T*> GetObjectsAt(float px, float py, float pz, float x_size, float y_size, float z_size);
    void RemoveObjectsAt(float px, float py, float pz, float x_size, float y_size, float z_size);
    void ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, OctreeNode<T>*>& mapObject);
    void ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, OctreeNode<T>*>& mapObject);
    void getBox(float& _x, float& _y, float& _z, float& _xSize, float& _ySize, float& _zSize) const;
    inline const std::forward_list<T*>& getObjectList() const { return objectList; }
private:
    bool IsContain(float px, float py, float pz, float x_size, float y_size, float z_size, T* object) const;
    bool IsContain(float px, float py, float pz, float x_size, float y_size, float z_size, OctreeNode<T>* octreeNode) const;
    bool IsInterSect(float px, float py, float pz, float x_size, float y_size, float z_size, OctreeNode<T>* octreeNode) const;
    float calGeometricError(float xSize, float ySize, float zSize, float screenPixels) const;
public:
    std::forward_list<T*> objectList;
    static int m_maxLevel;
    static float m_minSize;
private:
    OctreeType octreeNodeType;
    float x;
    float y;
    float z;
    float xSize;
    float ySize;
    float zSize;
    int level;    
    OctreeNode* bottom_left_front_node;
    OctreeNode* bottom_right_front_node;
    OctreeNode* bottom_left_back_node;
    OctreeNode* bottom_right_back_node;
    OctreeNode* top_left_front_node;
    OctreeNode* top_right_front_node;
    OctreeNode* top_left_back_node;
    OctreeNode* top_right_back_node;
    bool isPath;
};
#endif//OctreeNode_h
