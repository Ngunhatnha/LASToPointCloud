#include "QuadTreeNode.h"
template <class T>
int QuadTreeNode<T>::m_maxLevel = 15;

template <class T>
float QuadTreeNode<T>::calGeometricError(float xSize, float ySize, float zSize, float screenPixels) const
{
    return sqrt(xSize * xSize + ySize * ySize + zSize * zSize);
    //return (3.1415926f * 0.25f * (xSize * xSize + ySize * ySize + zSize * zSize)) / (screenPixels);
    //return (3.1415926 * radius * radius) / (screenPixels);// *10;
}

template <class T>
void QuadTreeNode<T>::InitTree(int maxLevel)
{
    QuadTreeNode<T>::m_maxLevel = maxLevel;
}

template <class T>
QuadTreeNode<T>::QuadTreeNode(float _x, float _y, float _xSize, float _ySize, QuadTreeType _QuadTreeNodeType, int _level) :
    x(_x),
    y(_y),    
    xSize(_xSize),
    ySize(_ySize),    
    quadtreeNodeType(_QuadTreeNodeType),
    level(_level)
{
    bottom_left_node = nullptr;
    bottom_right_node = nullptr;    
    top_left_node = nullptr;
    top_right_node = nullptr;    
    isPath = false;
}

template <class T>
QuadTreeNode<T>::~QuadTreeNode()
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
bool QuadTreeNode<T>::IsContain(float px, float py, float x_size, float y_size, T* object) const
{
    if (object->xMin() >= px
        && object->xMin() + object->xSizeBox() <= px + x_size
        && object->yMin() >= py
        && object->yMin() + object->ySizeBox() <= py + y_size)
        return true;
    return false;
}

template <class T>
bool QuadTreeNode<T>::IsContain(float px, float py, float x_size, float y_size,QuadTreeNode<T>* QuadTreeNode) const
{
    if (QuadTreeNode->x >= px
        && QuadTreeNode->x + QuadTreeNode->xSize <= px + x_size
        && QuadTreeNode->y >= py
        && QuadTreeNode->y + QuadTreeNode->ySize <= py + y_size)
        return true;
    return false;
}

template <class T>
bool QuadTreeNode<T>::IsInterSect(float px, float py, float x_size, float y_size, QuadTreeNode<T>* QuadTreeNode) const
{
    if (QuadTreeNode->x > px + x_size
        || QuadTreeNode->x + xSize<px
        || QuadTreeNode->y>py + y_size
        || QuadTreeNode->y + ySize < py)
        return false;
    return true;
}

template <class T>
void QuadTreeNode<T>::BuildTree(int level)
{
    if (level == QuadTreeNode<T>::m_maxLevel)
        return;
    bottom_left_node = new QuadTreeNode(x, y, xSize / 2, ySize / 2, BOTTOM_LEFT, level + 1);
    bottom_right_node = new QuadTreeNode(x + xSize / 2, y, xSize / 2, ySize / 2, BOTTOM_RIGHT, level + 1);

    top_left_node = new QuadTreeNode(x, y + ySize / 2, xSize / 2, ySize / 2, TOP_LEFT, level + 1);
    top_right_node = new QuadTreeNode(x + xSize / 2, y + ySize / 2, xSize / 2, ySize / 2, TOP_RIGHT, level + 1);

    bottom_left_node->BuildTree(level + 1);
    bottom_right_node->BuildTree(level + 1);
    top_left_node->BuildTree(level + 1);
    top_right_node->BuildTree(level + 1);
}

template <class T>
void QuadTreeNode<T>::InsertObject(T* object)
{
    isPath = true;
    if (level == QuadTreeNode<T>::m_maxLevel)
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
std::forward_list<T*> QuadTreeNode<T>::GetObjectsAt(float px, float py, float x_size, float y_size)
{
    if (level == QuadTreeNode<T>::m_maxLevel)
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
        std::forward_list<T*> childObjects4 = top_right_node->GetObjectsAt(px, py,x_size, y_size);
        resObjects.insert_after(resObjects.end(), childObjects4.begin(), childObjects4.end());
    }
    return resObjects;
}

template <class T>
void QuadTreeNode<T>::RemoveObjectsAt(float px, float py, float x_size, float y_size)
{
    if (level == QuadTreeNode<T>::m_maxLevel)
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
void QuadTreeNode<T>::getBox(float& _x, float& _y, float& _xSize, float& _ySize) const
{
    _x = x;
    _y = y;    
    _xSize = xSize;
    _ySize = ySize;    
}

template <class T>
void QuadTreeNode<T>::ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeNode<T>*>& mapObject)
{
    if (bottom_left_node == nullptr &&
        bottom_right_node == nullptr &&        
        top_left_node == nullptr &&
        top_right_node == nullptr &&        
        !objectList.empty())
    {
        float vCenter[3];
        float vecHalf[3];
        float zSize = QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);
       // geometricError = 0.0;

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

        // calculate bounding box from objectList ????

        sstream << "{\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".b3dm";
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
                float zSize = QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight;
                float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

                sstream << "{\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;
                std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".b3dm";
                sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
                mapObject[nameB3dm] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float zSize = QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * (QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight);

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

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

            std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".b3dm";
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
void QuadTreeNode<T>::ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, QuadTreeNode<T>*>& mapObject)
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
        float zSize = QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

        sstream << "\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".b3dm";
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
                float zSize = QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight;
                float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

                sstream << "\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;
                std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".b3dm";
                sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
                mapObject[nameB3dm] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float zSize = QuadTreeNode<T>::m_maxHeight - QuadTreeNode<T>::m_minHeight;
        float geometricError = calGeometricError(xSize, ySize, zSize, 0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = QuadTreeNode<T>::m_minHeight + vecHalf[2];

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

            std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(), objectList.end())) + ".b3dm";
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