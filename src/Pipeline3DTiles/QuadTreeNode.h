#ifndef _QuadTreeNode_h__
#define _QuadTreeNode_h__
#include "ObjectInterface.h"
#include <sstream>
#include <forward_list>
#include <map>
enum QuadTreeType
{
    ROOT,
    BOTTOM_LEFT,	// 1 
    BOTTOM_RIGHT,	// 2     
    TOP_LEFT,       // 3 
    TOP_RIGHT,      // 4       
};

template <class T>
class QuadTreeNode
{
public:
    static void InitTree(int maxLevel);
    QuadTreeNode(float _x, float _y, float _xSize, float _ySize, QuadTreeType _quadtreeNodeType, int _level);
    ~QuadTreeNode();
public:
    void BuildTree(int level);
    void InsertObject(T* object);
    std::forward_list<T*> GetObjectsAt(float px, float py, float x_size, float y_size);
    void RemoveObjectsAt(float px, float py,float x_size, float y_size);
    void ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeNode<T>*>& mapObject);
    void ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeNode<T>*>& mapObject);
    void getBox(float& _x, float& _y, float& _xSize, float& _ySize) const;
    inline const std::forward_list<T*>& getObjectList() const { return objectList; }
private:
    bool IsContain(float px, float py, float x_size, float y_size, T* object) const;
    bool IsContain(float px, float py, float x_size, float y_size, QuadTreeNode<T>* octreeNode) const;
    bool IsInterSect(float px, float py, float x_size, float y_size, QuadTreeNode<T>* octreeNode) const;
    float calGeometricError(float xSize, float ySize, float zSize,float screenPixels) const;
public:
    std::forward_list<T*> objectList;
    static int m_maxLevel;
    static float m_minSize;
private:
    QuadTreeType quadtreeNodeType;
    float x;
    float y;   
    float xSize;
    float ySize;    
    int level;
    QuadTreeNode* bottom_left_node;
    QuadTreeNode* bottom_right_node;    
    QuadTreeNode* top_left_node;
    QuadTreeNode* top_right_node;

    bool isPath;
};
#endif//OctreeNode_h
