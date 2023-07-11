#include "QuadTreeNodeTriangle.h"
#ifndef __QuadTreeNodeTriangle_CPP__
#define __QuadTreeNodeTriangle_CPP__

template <class T>
int QuadTreeTriangle<T>::m_maxLevel = 15;
template <class T>
float QuadTreeTriangle<T>::m_minHeight = 0.0f;
template <class T>
float QuadTreeTriangle<T>::m_maxHeight = 1.0f;

template <class T>
unsigned int QuadTreeTriangle<T>::m_maxTriangleInFile = 0; // unlimit triangles in file

template <class T>
double QuadTreeTriangle<T>::m_scaleGeometric = 0.1f; // scale geometric erro

template <class T>
float QuadTreeTriangle<T>::calGeometricError(float xSize, float ySize, float zSize, float screenPixels) const
{
    return sqrt(xSize * xSize + ySize * ySize + zSize * zSize)* QuadTreeTriangle<T>::m_scaleGeometric;

   // return (3.1415926f * 0.25f * (xSize * xSize + ySize * ySize + zSize * zSize)) / (screenPixels);
    //return (3.1415926 * radius * radius) / (screenPixels);// *10;
}
template <class T>
float QuadTreeTriangle<T>::getGeometricError() const
{
    float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
    return calGeometricError(xSize, ySize, zSize, 0.5f);    
}
template <class T>
void QuadTreeTriangle<T>::InitTree(int maxLevel,float minElev,float maxElev)
{
    QuadTreeTriangle<T>::m_maxLevel = maxLevel;
    QuadTreeTriangle<T>::m_minHeight = minElev;
    QuadTreeTriangle<T>::m_maxHeight = maxElev;
    QuadTreeTriangle<T>::m_maxTriangleInFile = 0;
    QuadTreeTriangle<T>::m_scaleGeometric = 0.1;
}

template <class T>
void QuadTreeTriangle<T>::InitTreeObjectSize(int maxLevel, float minElev, float maxElev,unsigned int maxTriangleInFile,double scaleGeometric)
{
    QuadTreeTriangle<T>::m_maxLevel = maxLevel;
    QuadTreeTriangle<T>::m_minHeight = minElev;
    QuadTreeTriangle<T>::m_maxHeight = maxElev;
    QuadTreeTriangle<T>::m_maxTriangleInFile= maxTriangleInFile;
    QuadTreeTriangle<T>::m_scaleGeometric= scaleGeometric;

}
template <class T>
QuadTreeTriangle<T>::QuadTreeTriangle(float _x, float _y, float _xSize, float _ySize, QuadTreeTriangleType _QuadTreeTriangleType, int _level) :
    x(_x),
    y(_y),
    xSize(_xSize),
    ySize(_ySize),
    quadtreetriangleType(_QuadTreeTriangleType),
    level(_level)
{
    bottom_left_node = nullptr;
    bottom_right_node = nullptr;
    top_left_node = nullptr;
    top_right_node = nullptr;
    isPath = false;
}

template <class T>
QuadTreeTriangle<T>::~QuadTreeTriangle()
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
bool QuadTreeTriangle<T>::IsContain(float px, float py, float x_size, float y_size, T* object) const
{
    ObjectType mObjectType = object->getType();
    if (mObjectType == TRIANGLE_TYPE)
    {
        if (object->xP1() >= px
            && object->xP1() <= px + x_size
            && object->yP1() >= py
            && object->yP1() <= py + y_size
            && object->xP2() >= px
            && object->xP2() <= px + x_size
            && object->yP2() >= py
            && object->yP2() <= py + y_size
            && object->xP3() >= px
            && object->xP3() <= px + x_size
            && object->yP3() >= py
            && object->yP3() <= py + y_size
            )
            return true;
    }
    else if (mObjectType == LINE_TYPE)
    {
        if (object->xP1() >= px
            && object->xP1() <= px + x_size
            && object->yP1() >= py
            && object->yP1() <= py + y_size
            && object->xP2() >= px
            && object->xP2() <= px + x_size
            && object->yP2() >= py
            && object->yP2() <= py + y_size            
            )
            return true;
    }
    else if (mObjectType == POINT_TYPE)
    {
        if (object->xP1() >= px
            && object->xP1() <= px + x_size
            && object->yP1() >= py
            && object->yP1() <= py + y_size            
            )
            return true;
    }
    return false;
}

template <class T>
bool QuadTreeTriangle<T>::IsContain(float px, float py, float x_size, float y_size, QuadTreeTriangle<T>* QuadTreeTriangle) const
{
    if (QuadTreeTriangle->x >= px
        && QuadTreeTriangle->x + QuadTreeTriangle->xSize <= px + x_size
        && QuadTreeTriangle->y >= py
        && QuadTreeTriangle->y + QuadTreeTriangle->ySize <= py + y_size)
        return true;
    return false;
}

template <class T>
bool QuadTreeTriangle<T>::IsInterSect(float px, float py, float x_size, float y_size, QuadTreeTriangle<T>* QuadTreeTriangle) const
{
    if (QuadTreeTriangle->x > px + x_size
        || QuadTreeTriangle->x + xSize<px
        || QuadTreeTriangle->y>py + y_size
        || QuadTreeTriangle->y + ySize < py)
        return false;
    return true;
}

template <class T>
void QuadTreeTriangle<T>::BuildTree(int level)
{
    if (level == QuadTreeTriangle<T>::m_maxLevel)
        return;
    bottom_left_node = new QuadTreeTriangle(x, y, xSize / 2, ySize / 2, BOTTOM_LEFT, level + 1);
    bottom_right_node = new QuadTreeTriangle(x + xSize / 2, y, xSize / 2, ySize / 2, BOTTOM_RIGHT, level + 1);

    top_left_node = new QuadTreeTriangle(x, y + ySize / 2, xSize / 2, ySize / 2, TOP_LEFT, level + 1);
    top_right_node = new QuadTreeTriangle(x + xSize / 2, y + ySize / 2, xSize / 2, ySize / 2, TOP_RIGHT, level + 1);

    bottom_left_node->BuildTree(level + 1);
    bottom_right_node->BuildTree(level + 1);
    top_left_node->BuildTree(level + 1);
    top_right_node->BuildTree(level + 1);
}

template <class T>
void QuadTreeTriangle<T>::InsertObject(T* object)
{
    isPath = true;
    if (level == QuadTreeTriangle<T>::m_maxLevel)
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
std::forward_list<T*> QuadTreeTriangle<T>::GetObjectsAt(float px, float py, float x_size, float y_size)
{
    if (level == QuadTreeTriangle<T>::m_maxLevel)
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
    if (top_left_node && IsInterSect(px, py,x_size, y_size, top_left_node))
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
void QuadTreeTriangle<T>::RemoveObjectsAt(float px, float py, float x_size, float y_size)
{
    if (level == QuadTreeTriangle<T>::m_maxLevel)
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
void QuadTreeTriangle<T>::getBox(float& _x, float& _y, float& _xSize, float& _ySize) const
{
    _x = x;
    _y = y;
    _xSize = xSize;
    _ySize = ySize;
}

template <class T>
void QuadTreeTriangle<T>::ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeTriangle<T>*>& mapObject)
{
    if (bottom_left_node == nullptr &&
        bottom_right_node == nullptr &&
        top_left_node == nullptr &&
        top_right_node == nullptr &&
        !objectList.empty())
    {
        float vCenter[3];
        float vecHalf[3];
        float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);
       // geometricError = 0.0;

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];

        // calculate bounding box from objectList ????

        sstream << "{\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        unsigned int numTriangles=std::distance(objectList.begin(), objectList.end());
        std::string nameB3dm;
        if (QuadTreeTriangle<T>::m_maxTriangleInFile == 0)
           nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";           
        else
        {
            // limit triangles in file
            if (numTriangles > QuadTreeTriangle<T>::m_maxTriangleInFile)
                nameB3dm = "T_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".json";
            else
                nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";            
        }
        sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
        mapObject[nameB3dm] = this;

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
                float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
                float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];

                sstream << "{\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;

                unsigned int numTriangles = std::distance(objectList.begin(), objectList.end());
                std::string nameB3dm;
                if (QuadTreeTriangle<T>::m_maxTriangleInFile == 0)
                    nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
                else
                {
                    // limit triangles in file
                    if (numTriangles > QuadTreeTriangle<T>::m_maxTriangleInFile)
                        nameB3dm = "T_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".json";
                    else
                        nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
                }
                sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
                mapObject[nameB3dm] = this;

                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * (QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight);

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];

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
            sstream << ",\"geometricError\":" << geometricError;

            unsigned int numTriangles = std::distance(objectList.begin(), objectList.end());
            std::string nameB3dm;
            if (QuadTreeTriangle<T>::m_maxTriangleInFile == 0)
                nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
            else
            {
                // limit triangles in file
                if (numTriangles > QuadTreeTriangle<T>::m_maxTriangleInFile)
                    nameB3dm = "T_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".json";
                else
                    nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
            }

            sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}]}]}";
            mapObject[nameB3dm] = this;
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
void QuadTreeTriangle<T>::ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeTriangle<T>*>& mapObject)
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
        float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;
        
        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];
        

        sstream << "\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        unsigned int numTriangles = std::distance(objectList.begin(), objectList.end());
        std::string nameB3dm;
        if (QuadTreeTriangle<T>::m_maxTriangleInFile == 0)
            nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
        else
        {
            // limit triangles in file
            if (numTriangles > QuadTreeTriangle<T>::m_maxTriangleInFile)
                nameB3dm = "T_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".json";
            else
                nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
        }

        sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
        mapObject[nameB3dm] = this;
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
                float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
                float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];

                sstream << "\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;

                unsigned int numTriangles = std::distance(objectList.begin(), objectList.end());
                std::string nameB3dm;
                if (QuadTreeTriangle<T>::m_maxTriangleInFile == 0)
                    nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
                else
                {
                    // limit triangles in file
                    if (numTriangles > QuadTreeTriangle<T>::m_maxTriangleInFile)
                        nameB3dm = "T_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".json";
                    else
                        nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
                }

                sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
                mapObject[nameB3dm] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float zSize = QuadTreeTriangle<T>::m_maxHeight - QuadTreeTriangle<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeTriangle<T>::m_minHeight + vecHalf[2];

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
            sstream << ",\"geometricError\":" << geometricError;
            sstream << ",\"children\": [";
            sstream << "{\"boundingVolume\":{\"box\":[";
            sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
            sstream << vecHalf[0] << ",0,0,";
            sstream << "0," << vecHalf[1] << ",0,";
            sstream << "0,0," << vecHalf[2] << "]}";
            sstream << ",\"geometricError\":" << geometricError;

            unsigned int numTriangles = std::distance(objectList.begin(), objectList.end());
            std::string nameB3dm;
            if (QuadTreeTriangle<T>::m_maxTriangleInFile == 0)
                nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
            else
            {
                // limit triangles in file
                if (numTriangles > QuadTreeTriangle<T>::m_maxTriangleInFile)
                    nameB3dm = "T_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".json";
                else
                    nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(numTriangles) + ".b3dm";
            }

            sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}]}]}";
            mapObject[nameB3dm] = this;
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