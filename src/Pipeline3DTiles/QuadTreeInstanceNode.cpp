#include "QuadTreeInstanceNode.h"
#ifndef __QuadTreeInstanceNode_CPP_
#define __QuadTreeInstanceNode_CPP_
template <class T>
int QuadTreeInstanceNode<T>::m_maxLevel = 15;

template <class T>
float QuadTreeInstanceNode<T>::m_minHeight = 0.0f;

template <class T>
float QuadTreeInstanceNode<T>::m_maxHeight = 1.0f;

template <class T>
unsigned int QuadTreeInstanceNode<T>::m_maxTriangleInFile = 60000;

template <class T>
float QuadTreeInstanceNode<T>::m_scaleGeometricError = 0.1f;


template <class T>
float QuadTreeInstanceNode<T>::calGeometricError(float xSize, float ySize, float zSize, float screenPixels) const
{
    return sqrt(xSize * xSize + ySize * ySize + zSize * zSize)* QuadTreeInstanceNode<T>::m_scaleGeometricError;
    //return (3.1415926f * 0.25f * (xSize * xSize + ySize * ySize + zSize * zSize)) / (screenPixels);
    //return (3.1415926 * radius * radius) / (screenPixels);// *10;
}

template <class T>
void QuadTreeInstanceNode<T>::InitTreeObjectSize(int maxLevel, float minElev, float maxElev, unsigned int maxTriangleInFile,float scaleGeometricError)
{
    QuadTreeInstanceNode<T>::m_maxLevel = maxLevel;
    QuadTreeInstanceNode<T>::m_minHeight = minElev;
    QuadTreeInstanceNode<T>::m_maxHeight = maxElev;

    QuadTreeInstanceNode<T>::m_maxTriangleInFile = maxTriangleInFile;
    QuadTreeInstanceNode<T>::m_scaleGeometricError= scaleGeometricError;
}

template <class T>
void QuadTreeInstanceNode<T>::InitTree(int maxLevel, float minElev, float maxElev)
{
    QuadTreeInstanceNode<T>::m_maxLevel = maxLevel;
    QuadTreeInstanceNode<T>::m_minHeight = minElev;
    QuadTreeInstanceNode<T>::m_maxHeight = maxElev;
}

template <class T>
QuadTreeInstanceNode<T>::QuadTreeInstanceNode(float _x, float _y, float _xSize, float _ySize, QuadTreeInstanceType _QuadTreeInstanceNodeType, int _level) :
    x(_x),
    y(_y),
    xSize(_xSize),
    ySize(_ySize),
    quadtreeInstanceNodeType(_QuadTreeInstanceNodeType),
    level(_level)
{
    bottom_left_node = nullptr;
    bottom_right_node = nullptr;
    top_left_node = nullptr;
    top_right_node = nullptr;
    isPath = false;
}

template <class T>
QuadTreeInstanceNode<T>::~QuadTreeInstanceNode()
{
    if (bottom_left_node != nullptr)
    {
        delete bottom_left_node;
        bottom_left_node = nullptr;
    }
    if (bottom_right_node != nullptr)
    {
        delete bottom_right_node;
        bottom_right_node = nullptr;
    }

    if (top_left_node != nullptr)
    {
        delete top_left_node;
        top_left_node = nullptr;
    }
    if (top_right_node != nullptr)
    {
        delete top_right_node;
        top_right_node = nullptr;
    }
}

template <class T>
bool QuadTreeInstanceNode<T>::IsContain(float px, float py, float x_size, float y_size, T* object) const
{
    if (object->xMin() >= px
        && object->xMin() + object->xSizeBox() <= px + x_size
        && object->yMin() >= py
        && object->yMin() + object->ySizeBox() <= py + y_size)
        return true;
    return false;
}

template <class T>
bool QuadTreeInstanceNode<T>::IsContain(float px, float py, float x_size, float y_size, QuadTreeInstanceNode<T>* pQuadTreeInstanceNode) const
{
    if (pQuadTreeInstanceNode->x >= px
        && pQuadTreeInstanceNode->x + pQuadTreeInstanceNode->xSize <= px + x_size
        && pQuadTreeInstanceNode->y >= py
        && pQuadTreeInstanceNode->y + pQuadTreeInstanceNode->ySize <= py + y_size)
        return true;
    return false;
}

template <class T>
bool QuadTreeInstanceNode<T>::IsInterSect(float px, float py, float x_size, float y_size, QuadTreeInstanceNode<T>* pQuadTreeInstanceNode) const
{
    if (pQuadTreeInstanceNode->x > px + x_size
        || pQuadTreeInstanceNode->x + xSize<px
        || pQuadTreeInstanceNode->y>py + y_size
        || pQuadTreeInstanceNode->y + ySize < py)
        return false;
    return true;
}

template <class T>
void QuadTreeInstanceNode<T>::SetMinMaxElev(T* object) 
{
    if (object->zMin() >= zmax)
        zmax = object->zMin();
    if (object->zMin() <= zmin)
        zmin = object->zMin();
}

template <class T>
void QuadTreeInstanceNode<T>::BuildTree(int level)
{
    if (level == QuadTreeInstanceNode<T>::m_maxLevel)
        return;
    bottom_left_node = new QuadTreeInstanceNode(x, y, xSize / 2, ySize / 2, BOTTOM_LEFT_INSTANCE, level + 1);
    bottom_right_node = new QuadTreeInstanceNode(x + xSize / 2, y, xSize / 2, ySize / 2, BOTTOM_RIGHT_INSTANCE, level + 1);

    top_left_node = new QuadTreeInstanceNode(x, y + ySize / 2, xSize / 2, ySize / 2, TOP_LEFT_INSTANCE, level + 1);
    top_right_node = new QuadTreeInstanceNode(x + xSize / 2, y + ySize / 2, xSize / 2, ySize / 2, TOP_RIGHT_INSTANCE, level + 1);

    bottom_left_node->BuildTree(level + 1);
    bottom_right_node->BuildTree(level + 1);
    top_left_node->BuildTree(level + 1);
    top_right_node->BuildTree(level + 1);
}

template <class T>
void QuadTreeInstanceNode<T>::InsertObject(T* object)
{
    isPath = true;
    SetMinMaxElev(object);
    if (level == QuadTreeInstanceNode<T>::m_maxLevel)
    {
        objectList.push_front(object);
        return;
    }
    //1
    if (bottom_left_node && IsContain(x, y, xSize / 2, ySize / 2, object))
    {
        bottom_left_node->InsertObject(object);
        return;
    }
    //2
    if (bottom_right_node && IsContain(x + xSize / 2, y, xSize / 2, ySize / 2, object))
    {
        bottom_right_node->InsertObject(object);
        return;
    }
    //3
    if (top_left_node && IsContain(x, y + ySize / 2, xSize / 2, ySize / 2, object))
    {
        top_left_node->InsertObject(object);
        return;
    }
    //4
    if (top_right_node && IsContain(x + xSize / 2, y + ySize / 2, xSize / 2, ySize / 2, object))
    {
        top_right_node->InsertObject(object);
        return;
    }
    objectList.push_front(object);
}

template <class T>
std::forward_list<T*> QuadTreeInstanceNode<T>::GetObjectsAt(float px, float py, float x_size, float y_size)
{
    if (level == QuadTreeInstanceNode<T>::m_maxLevel)
        return objectList;
    std::forward_list<T*> resObjects;
    //1
    if (bottom_left_node && IsInterSect(px, py, x_size, y_size, bottom_left_node))
    {
        std::forward_list<T*> childObjects1 = bottom_left_node->GetObjectsAt(px, py, x_size, y_size);
        resObjects.insert_after(resObjects.end(), childObjects1.begin(), childObjects1.end());
    }
    //2
    if (bottom_right_node && IsInterSect(px, py, x_size, y_size, bottom_right_node))
    {
        std::forward_list<T*> childObjects2 = bottom_right_node->GetObjectsAt(px, py, x_size, y_size);
        resObjects.insert_after(resObjects.end(), childObjects2.begin(), childObjects2.end());
    }
    //3
    if (top_left_node && IsInterSect(px, py, x_size, y_size, top_left_node))
    {
        std::forward_list<T*> childObjects3 = top_left_node->GetObjectsAt(px, py, x_size, y_size);
        resObjects.insert_after(resObjects.end(), childObjects3.begin(), childObjects3.end());
    }
    //4
    if (top_right_node && IsInterSect(px, py, x_size, y_size, top_right_node))
    {
        std::forward_list<T*> childObjects4 = top_right_node->GetObjectsAt(px, py, x_size, y_size);
        resObjects.insert_after(resObjects.end(), childObjects4.begin(), childObjects4.end());
    }
    return resObjects;
}

template <class T>
void QuadTreeInstanceNode<T>::RemoveObjectsAt(float px, float py, float x_size, float y_size)
{
    if (level == QuadTreeInstanceNode<T>::m_maxLevel)
    {
        if (IsContain(px, py, x_size, y_size, this))
            objectList.clear();
        return;
    }
    //1
    if (bottom_left_node && IsInterSect(px, py, x_size, y_size, bottom_left_node))
        bottom_left_node->RemoveObjectsAt(px, py, x_size, y_size);
    //2
    if (bottom_right_node && IsInterSect(px, py, x_size, y_size, bottom_right_node))
        bottom_right_node->RemoveObjectsAt(px, py, x_size, y_size);
    //3
    if (top_left_node && IsInterSect(px, py, x_size, y_size, top_left_node))
        top_left_node->RemoveObjectsAt(px, py, x_size, y_size);
    //4
    if (top_right_node && IsInterSect(px, py, x_size, y_size, top_right_node))
        top_right_node->RemoveObjectsAt(px, py, x_size, y_size);
}
template <class T>
void QuadTreeInstanceNode<T>::getBox(float& _x, float& _y, float& _xSize, float& _ySize) const
{
    _x = x;
    _y = y;
    _xSize = xSize;
    _ySize = ySize;
}

template <class T>
BoundingBox QuadTreeInstanceNode<T>::getBoundingBox() const
{
    BoundingBox box(x, y, zmin, x + xSize, y + ySize, zmax);
    return box;
}

template <class T>
void QuadTreeInstanceNode<T>::ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeInstanceNode<T>*>& mapObject)
{
    if (bottom_left_node == nullptr &&
        bottom_right_node == nullptr &&
        top_left_node == nullptr &&
        top_right_node == nullptr &&
        !objectList.empty())
    {
        float vCenter[3];
        float vecHalf[3];
        float zSize = zmax - zmin;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);
        geometricError = 0.05;

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = zmin + vecHalf[2];

        // calculate bounding box from objectList ????

        sstream << "{\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        std::string namePnts = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".pnts";
        sstream << ",\"content\":{\"uri\":\"" << namePnts << "\"}}";
        mapObject[namePnts] = this;
        indexFile++;
    }
    else
    {
        unsigned char numChild = 0;
        if (bottom_left_node != nullptr && bottom_left_node->isPath)
            numChild++;
        if (bottom_right_node != nullptr && bottom_right_node->isPath)
            numChild++;
        if (top_left_node != nullptr && top_left_node->isPath)
            numChild++;
        if (top_right_node != nullptr && top_right_node->isPath)
            numChild++;

        if (numChild == 0)
        {
            if (!objectList.empty())
            {
                float vCenter[3];
                float vecHalf[3];
                float zSize = zmax -zmin;
                float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);
                geometricError = 0.05;

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = zmin + vecHalf[2];

                sstream << "{\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;
                std::string namePnts = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".pnts";
                sstream << ",\"content\":{\"uri\":\"" << namePnts << "\"}}";
                mapObject[namePnts] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float zSize = zmax - zmin;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = zmin + vecHalf[2];

        sstream << "{\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;
        sstream << ",\"children\": [";

        unsigned char iCount = 0;
        if (bottom_left_node != nullptr && bottom_left_node->isPath)
        {
            bottom_left_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_right_node != nullptr && bottom_right_node->isPath)
        {
            bottom_right_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_left_node != nullptr && top_left_node->isPath)
        {
            top_left_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_right_node != nullptr && top_right_node->isPath)
        {
            top_right_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (objectList.empty())
            sstream << "]}";
        else
        {
            sstream << ",";
            sstream << "{\"boundingVolume\":{\"box\":[";
            sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
            sstream << vecHalf[0] << ",0,0,";
            sstream << "0," << vecHalf[1] << ",0,";
            sstream << "0,0," << vecHalf[2] << "]}";
            sstream << ",\"geometricError\":" << geometricError;
            sstream << ",\"children\": [";
            sstream << "{\"boundingVolume\":{\"box\":[";
            sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
            sstream << vecHalf[0] << ",0,0,";
            sstream << "0," << vecHalf[1] << ",0,";
            sstream << "0,0," << vecHalf[2] << "]}";
            geometricError = 0.05;
            sstream << ",\"geometricError\":" << geometricError;

            std::string namePnts = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".pnts";
            sstream << ",\"content\":{\"uri\":\"" << namePnts << "\"}}]}]}";
            mapObject[namePnts] = this;
            indexFile++;
            /*
            sstream << "]";
            std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
            sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
            mapObject[nameB3dm] = this;
            indexFile++;
            */
        }
    }
}
template <class T>
void QuadTreeInstanceNode<T>::ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeInstanceNode<T>*>& mapObject)
{
    if (!isPath)
        return;
    if (bottom_left_node == nullptr &&
        bottom_right_node == nullptr &&
        top_left_node == nullptr &&
        top_right_node == nullptr &&
        !objectList.empty())
    {
        float vCenter[3];
        float vecHalf[3];
        float zSize = zmax - zmin;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);
        geometricError = 0.05;

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = zmin + vecHalf[2];

        sstream << "\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        std::string namePnts = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".pnts";
        sstream << ",\"content\":{\"uri\":\"" << namePnts << "\"}}";
        mapObject[namePnts] = this;
        indexFile++;
    }
    else
    {
        unsigned char numChild = 0;
        if (bottom_left_node != nullptr && bottom_left_node->isPath)
            numChild++;
        if (bottom_right_node != nullptr && bottom_right_node->isPath)
            numChild++;
        if (top_left_node != nullptr && top_left_node->isPath)
            numChild++;
        if (top_right_node != nullptr && top_right_node->isPath)
            numChild++;
        if (numChild == 0)
        {
            if (!objectList.empty())
            {
                float vCenter[3];
                float vecHalf[3];
                float zSize = zmax-zmin;
                float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);
                geometricError = 0.05;

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = zmin + vecHalf[2];

                sstream << "\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;
                std::string namePnts = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".pnts";
                sstream << ",\"content\":{\"uri\":\"" << namePnts << "\"}}";
                mapObject[namePnts] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float zSize = zmax - zmin;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = zmin + vecHalf[2];

        sstream << "\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;
        sstream << ",\"children\": [";

        unsigned char iCount = 0;
        if (bottom_left_node != nullptr && bottom_left_node->isPath)
        {
            bottom_left_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_right_node != nullptr && bottom_right_node->isPath)
        {
            bottom_right_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_left_node != nullptr && top_left_node->isPath)
        {
            top_left_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_right_node != nullptr && top_right_node->isPath)
        {
            top_right_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (objectList.empty())
            sstream << "]}";
        else
        {
            sstream << ",";
            sstream << "{\"boundingVolume\":{\"box\":[";
            sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
            sstream << vecHalf[0] << ",0,0,";
            sstream << "0," << vecHalf[1] << ",0,";
            sstream << "0,0," << vecHalf[2] << "]}";
            geometricError = 0.05;
            sstream << ",\"geometricError\":" << geometricError;
            sstream << ",\"children\": [";
            sstream << "{\"boundingVolume\":{\"box\":[";
            sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
            sstream << vecHalf[0] << ",0,0,";
            sstream << "0," << vecHalf[1] << ",0,";
            sstream << "0,0," << vecHalf[2] << "]}";
            sstream << ",\"geometricError\":" << geometricError;

            std::string namePnts = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".pnts";
            sstream << ",\"content\":{\"uri\":\"" << namePnts << "\"}}]}]}";
            mapObject[namePnts] = this;
            indexFile++;
            /*
            sstream << "]";
            std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
            sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
            mapObject[nameB3dm] = this;
            indexFile++;
            */

        }

    }
}
#endif