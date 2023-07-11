#include "OctreeNode.h"
template <class T>
int OctreeNode<T>::m_maxLevel = 15;

template <class T>
float OctreeNode<T>::calGeometricError(float xSize,float ySize,float zSize, float screenPixels) const
{
    return sqrt(xSize * xSize + ySize * ySize + zSize * zSize);
    //return (3.1415926f* 0.25f * (xSize * xSize + ySize * ySize + zSize * zSize))/ (screenPixels);
    //return (3.1415926 * radius * radius) / (screenPixels);// *10;
}

template <class T>
void OctreeNode<T>::InitTree(int maxLevel)
{
    OctreeNode<T>::m_maxLevel = maxLevel;    
}

template <class T>
OctreeNode<T>::OctreeNode(float _x, float _y, float _z, float _xSize, float _ySize, float _zSize, OctreeType _octreeNodeType, int _level) :
    x(_x),
    y(_y),
    z(_z),
    xSize(_xSize),
    ySize(_ySize),
    zSize(_zSize),
    octreeNodeType(_octreeNodeType),
    level(_level)    
{
    bottom_left_front_node = nullptr;
    bottom_right_front_node = nullptr;
    bottom_left_back_node = nullptr;
    bottom_right_back_node = nullptr;
    top_left_front_node = nullptr;
    top_right_front_node = nullptr;
    top_left_back_node = nullptr;
    top_right_back_node = nullptr;
    isPath = false;
}

template <class T>
OctreeNode<T>::~OctreeNode()
{
    if (bottom_left_back_node != nullptr)
    {
        delete bottom_left_back_node;
        bottom_left_back_node = nullptr;
    }
    if (bottom_right_front_node != nullptr)
    {
        delete bottom_right_front_node;
        bottom_right_front_node = nullptr;
    }
    if (bottom_left_back_node != nullptr)
    {
        delete bottom_left_back_node;
        bottom_left_back_node = nullptr;
    }
    if (bottom_right_back_node != nullptr)
    {
        delete bottom_right_back_node;
        bottom_right_back_node = nullptr;
    }

    if (top_left_front_node != nullptr)
    {
        delete top_left_front_node;
        top_left_front_node = nullptr;
    }
    if (top_right_front_node != nullptr)
    {
        delete top_right_front_node;
        top_right_front_node = nullptr;
    }
    if (top_left_back_node != nullptr)
    {
        delete top_left_back_node;
        top_left_back_node = nullptr;
    }
    if (top_right_back_node != nullptr)
    {
        delete top_right_back_node;
        top_right_back_node = nullptr;
    }
}

template <class T>
bool OctreeNode<T>::IsContain(float px, float py, float pz, float x_size, float y_size, float z_size, T* object) const
{
    if (object->xMin() >= px
        && object->xMin() + object->xSizeBox() <= px + x_size
        && object->yMin() >= py
        && object->yMin() + object->ySizeBox() <= py + y_size
        && object->zMin() >= pz
        && object->zMin() + object->zSizeBox() <= pz + z_size)
        return true;
    return false;
}

template <class T>
bool OctreeNode<T>::IsContain(float px, float py, float pz, float x_size, float y_size, float z_size, OctreeNode<T>* octreeNode) const
{
    if (octreeNode->x >= px
        && octreeNode->x + octreeNode->xSize <= px + x_size
        && octreeNode->y >= py
        && octreeNode->y + octreeNode->ySize <= py + y_size
        && octreeNode->z >= pz
        && octreeNode->z + octreeNode->zSize <= pz + z_size)
        return true;
    return false;
}

template <class T>
bool OctreeNode<T>::IsInterSect(float px, float py, float pz, float x_size, float y_size, float z_size, OctreeNode<T>* octreeNode) const
{
    if (octreeNode->x > px + x_size
        || octreeNode->x + xSize<px
        || octreeNode->y>py + y_size
        || octreeNode->y + ySize < py
        || octreeNode->z + zSize<pz
        || octreeNode->z>pz + z_size)
        return false;
    return true;
}

template <class T>
void OctreeNode<T>::BuildTree(int level)
{
    if (level == OctreeNode<T>::m_maxLevel)
        return;

    bottom_left_front_node = new OctreeNode(x, y, z, xSize / 2, ySize / 2, zSize / 2, BOTTOM_LEFT_FRONT, level + 1);
    bottom_right_front_node = new OctreeNode(x + xSize / 2, y, z, xSize / 2, ySize / 2, zSize / 2, BOTTOM_RIGHT_FRONT, level + 1);
    bottom_left_back_node = new OctreeNode(x, y + ySize / 2, z, xSize / 2, ySize / 2, zSize / 2, BOTTOM_LEFT_BACK, level + 1);
    bottom_right_back_node = new OctreeNode(x + xSize / 2, y + ySize / 2, z, xSize / 2, ySize / 2, zSize / 2, BOTTOM_RIGHT_BACK, level + 1);
    top_left_front_node = new OctreeNode(x, y, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, TOP_LEFT_FRONT, level + 1);
    top_right_front_node = new OctreeNode(x + xSize / 2, y, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, TOP_RIGHT_FRONT, level + 1);
    top_left_back_node = new OctreeNode(x, y + ySize / 2, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, TOP_LEFT_BACK, level + 1);
    top_right_back_node = new OctreeNode(x + xSize / 2, y + ySize / 2, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, TOP_RIGHT_BACK, level + 1);

    bottom_left_front_node->BuildTree(level + 1);
    bottom_right_front_node->BuildTree(level + 1);
    bottom_left_back_node->BuildTree(level + 1);
    bottom_right_back_node->BuildTree(level + 1);
    top_left_front_node->BuildTree(level + 1);
    top_right_front_node->BuildTree(level + 1);
    top_left_back_node->BuildTree(level + 1);
    top_right_back_node->BuildTree(level + 1);
}

template <class T>
void OctreeNode<T>::InsertObject(T* object)
{
    isPath = true;
    if (level == OctreeNode<T>::m_maxLevel)
    {
        objectList.push_front(object);
        return;
    }
    //1
    if (bottom_left_front_node && IsContain(x, y, z, xSize / 2, ySize / 2, zSize / 2, object))
    {
        bottom_left_front_node->InsertObject(object);
        return;
    }
    //2
    if (bottom_right_front_node && IsContain(x + xSize / 2, y, z, xSize / 2, ySize / 2, zSize / 2, object))
    {
        bottom_right_front_node->InsertObject(object);
        return;
    }
    //3
    if (bottom_left_back_node && IsContain(x, y + ySize / 2, z, xSize / 2, ySize / 2, zSize / 2, object))
    {
        bottom_left_back_node->InsertObject(object);
        return;
    }
    //4
    if (bottom_right_back_node && IsContain(x + xSize / 2, y + ySize / 2, z, xSize / 2, ySize / 2, zSize / 2, object))
    {
        bottom_right_back_node->InsertObject(object);
        return;
    }
    //5
    if (top_left_front_node && IsContain(x, y, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, object))
    {
        top_left_front_node->InsertObject(object);
        return;
    }
    //6
    if (top_right_front_node && IsContain(x + xSize / 2, y, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, object))
    {
        top_right_front_node->InsertObject(object);
        return;
    }
    //7
    if (top_left_back_node && IsContain(x, y + ySize / 2, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, object))
    {
        top_left_back_node->InsertObject(object);
        return;
    }
    //8
    if (top_right_back_node && IsContain(x + xSize / 2, y + ySize / 2, z + zSize / 2, xSize / 2, ySize / 2, zSize / 2, object))
    {
        top_right_back_node->InsertObject(object);
        return;
    }
    objectList.push_front(object);
}

template <class T>
std::forward_list<T*> OctreeNode<T>::GetObjectsAt(float px, float py, float pz, float x_size, float y_size, float z_size)
{
    if (level == OctreeNode<T>::m_maxLevel)
        return objectList;
    std::forward_list<T*> resObjects;
    //1
    if (bottom_left_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_left_front_node))
    {
        std::forward_list<T*> childObjects1 = bottom_left_front_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects1.begin(), childObjects1.end());
    }
    //2
    if (bottom_right_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_right_front_node))
    {
        std::forward_list<T*> childObjects2 = bottom_right_front_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects2.begin(), childObjects2.end());
    }
    //3
    if (bottom_left_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_left_back_node))
    {
        std::forward_list<T*> childObjects3 = bottom_left_back_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects3.begin(), childObjects3.end());
    }
    //4
    if (bottom_right_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_right_back_node))
    {
        std::forward_list<T*> childObjects4 = bottom_right_back_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects4.begin(), childObjects4.end());
    }
    //5
    if (top_left_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_left_front_node))
    {
        std::forward_list<T*> childObjects5 = top_left_front_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects5.begin(), childObjects5.end());
    }
    //6
    if (top_right_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_right_front_node))
    {
        std::forward_list<T*> childObjects6 = top_right_front_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects6.begin(), childObjects6.end());
    }
    //7
    if (top_left_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_left_back_node))
    {
        std::forward_list<T*> childObjects7 = top_left_back_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects7.begin(), childObjects7.end());
    }
    //8
    if (top_right_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_right_back_node))
    {
        std::forward_list<T*> childObjects8 = top_right_back_node->GetObjectsAt(px, py, pz, x_size, y_size, z_size);
        resObjects.insert_after(resObjects.end(), childObjects8.begin(), childObjects8.end());
    }

    return resObjects;
}

template <class T>
void OctreeNode<T>::RemoveObjectsAt(float px, float py, float pz, float x_size, float y_size, float z_size)
{
    if (level == OctreeNode<T>::m_maxLevel)
    {
        if (IsContain(px, py, pz, x_size, y_size, z_size, this))
            objectList.clear();
        return;
    }
    //1
    if (bottom_left_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_left_front_node))
        bottom_left_front_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //2
    if (bottom_right_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_right_front_node))
        bottom_right_front_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //3
    if (bottom_left_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_left_back_node))
        bottom_left_back_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //4
    if (bottom_right_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, bottom_right_back_node))
        bottom_right_back_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //5
    if (top_left_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_left_front_node))
        top_left_front_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //6
    if (top_right_front_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_right_front_node))
        top_right_front_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //7
    if (top_left_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_left_back_node))
        top_left_back_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
    //8
    if (top_right_back_node && IsInterSect(px, py, pz, x_size, y_size, z_size, top_right_back_node))
        top_right_back_node->RemoveObjectsAt(px, py, pz, x_size, y_size, z_size);
}
template <class T>
void OctreeNode<T>::getBox(float& _x, float& _y, float& _z, float& _xSize, float& _ySize, float& _zSize) const
{
    _x = x;
    _y = y;
    _z = z;
    _xSize = xSize;
    _ySize = ySize;
    _zSize = zSize;
}

template <class T>
void OctreeNode<T>::ExportToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, OctreeNode<T>*>& mapObject)
{
    if (bottom_left_front_node == nullptr &&
        bottom_right_front_node == nullptr &&
        bottom_left_back_node == nullptr &&
        bottom_right_back_node == nullptr &&
        top_left_front_node == nullptr &&
        top_right_front_node == nullptr &&
        top_left_back_node == nullptr &&
        top_right_back_node == nullptr &&
        !objectList.empty())
    {
        float vCenter[3];
        float vecHalf[3];
        float geometricError = calGeometricError(xSize, ySize, zSize,0.5f);
        // nodeb leaft have geometricError=0.0
        geometricError = 0.0f;
        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = z + vecHalf[2];

        // calculate bounding box from objectList ????

        sstream << "{\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
        sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
        mapObject[nameB3dm] = this;
        indexFile++;
    }
    else
    {
        unsigned char numChild = 0;
        if (bottom_left_front_node != nullptr && bottom_left_front_node->isPath)
            numChild++;
        if (bottom_right_front_node != nullptr && bottom_right_front_node->isPath)
            numChild++;
        if (bottom_left_back_node != nullptr && bottom_left_back_node->isPath)
            numChild++;
        if (bottom_right_back_node != nullptr && bottom_right_back_node->isPath)
            numChild++;
        if (top_left_front_node != nullptr && top_left_front_node->isPath)
            numChild++;
        if (top_right_front_node != nullptr && top_right_front_node->isPath)
            numChild++;
        if (top_left_back_node != nullptr && top_left_back_node->isPath)
            numChild++;
        if (top_right_back_node != nullptr && top_right_back_node->isPath)
            numChild++;

        if (numChild == 0)
        {
            if (!objectList.empty())
            {
                float vCenter[3];
                float vecHalf[3];
                float geometricError = xSize;
                if (geometricError < ySize)
                    geometricError = ySize;
                if (geometricError < zSize)
                    geometricError = zSize;

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = z + vecHalf[2];

                sstream << "{\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;
                std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
                sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
                mapObject[nameB3dm] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float geometricError = calGeometricError(xSize, ySize, zSize,0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = z + vecHalf[2];

        sstream << "{\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;
        sstream << ",\"children\": [";

        unsigned char iCount = 0;
        if (bottom_left_front_node != nullptr && bottom_left_front_node->isPath)
        {
            bottom_left_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_right_front_node != nullptr && bottom_right_front_node->isPath)
        {
            bottom_right_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_left_back_node != nullptr && bottom_left_back_node->isPath)
        {
            bottom_left_back_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_right_back_node != nullptr && bottom_right_back_node->isPath)
        {
            bottom_right_back_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_left_front_node != nullptr && top_left_front_node->isPath)
        {
            top_left_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_right_front_node != nullptr && top_right_front_node->isPath)
        {
            top_right_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_left_back_node != nullptr && top_left_back_node->isPath)
        {
            top_left_back_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_right_back_node != nullptr && top_right_back_node->isPath)
        {
            top_right_back_node->ExportToJson(sstream, indexFile, mapObject);
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

            std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
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
void OctreeNode<T>::ExportRootToJson(std::stringstream& sstream, unsigned int& indexFile, std::map<std::string, OctreeNode<T>*>& mapObject)
{
    if (!isPath)
        return;
    if (bottom_left_front_node == nullptr &&
        bottom_right_front_node == nullptr &&
        bottom_left_back_node == nullptr &&
        bottom_right_back_node == nullptr &&
        top_left_front_node == nullptr &&
        top_right_front_node == nullptr &&
        top_left_back_node == nullptr &&
        top_right_back_node == nullptr &&
        !objectList.empty())
    {
        float vCenter[3];
        float vecHalf[3];
        float geometricError = calGeometricError(xSize, ySize, zSize,0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = z + vecHalf[2];

        sstream << "\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;

        std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
        sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
        mapObject[nameB3dm] = this;
        indexFile++;
    }
    else
    {
        unsigned char numChild = 0;
        if (bottom_left_front_node != nullptr && bottom_left_front_node->isPath)
            numChild++;
        if (bottom_right_front_node != nullptr && bottom_right_front_node->isPath)
            numChild++;
        if (bottom_left_back_node != nullptr && bottom_left_back_node->isPath)
            numChild++;
        if (bottom_right_back_node != nullptr && bottom_right_back_node->isPath)
            numChild++;
        if (top_left_front_node != nullptr && top_left_front_node->isPath)
            numChild++;
        if (top_right_front_node != nullptr && top_right_front_node->isPath)
            numChild++;
        if (top_left_back_node != nullptr && top_left_back_node->isPath)
            numChild++;
        if (top_right_back_node != nullptr && top_right_back_node->isPath)
            numChild++;

        if (numChild == 0)
        {
            if (!objectList.empty())
            {
                float vCenter[3];
                float vecHalf[3];
                float geometricError = calGeometricError(xSize, ySize, zSize,0.5f);

                vecHalf[0] = 0.5f * xSize;
                vecHalf[1] = 0.5f * ySize;
                vecHalf[2] = 0.5f * zSize;

                vCenter[0] = x + vecHalf[0];
                vCenter[1] = y + vecHalf[1];
                vCenter[2] = z + vecHalf[2];

                sstream << "\"boundingVolume\":{\"box\":[";
                sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
                sstream << vecHalf[0] << ",0,0,";
                sstream << "0," << vecHalf[1] << ",0,";
                sstream << "0,0," << vecHalf[2] << "]}";
                sstream << ",\"geometricError\":" << geometricError;
                std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
                sstream << ",\"content\":{\"uri\":\"" << nameB3dm << "\"}}";
                mapObject[nameB3dm] = this;
                indexFile++;
            }
            return;
        }

        float vCenter[3];
        float vecHalf[3];
        float geometricError = calGeometricError(xSize, ySize, zSize,0.5f);

        vecHalf[0] = 0.5f * xSize;
        vecHalf[1] = 0.5f * ySize;
        vecHalf[2] = 0.5f * zSize;

        vCenter[0] = x + vecHalf[0];
        vCenter[1] = y + vecHalf[1];
        vCenter[2] = z + vecHalf[2];

        sstream << "\"boundingVolume\":{\"box\":[";
        sstream << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << ",";
        sstream << vecHalf[0] << ",0,0,";
        sstream << "0," << vecHalf[1] << ",0,";
        sstream << "0,0," << vecHalf[2] << "]}";
        sstream << ",\"geometricError\":" << geometricError;
        sstream << ",\"children\": [";

        unsigned char iCount = 0;
        if (bottom_left_front_node != nullptr && bottom_left_front_node->isPath)
        {
            bottom_left_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_right_front_node != nullptr && bottom_right_front_node->isPath)
        {
            bottom_right_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_left_back_node != nullptr && bottom_left_back_node->isPath)
        {
            bottom_left_back_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (bottom_right_back_node != nullptr && bottom_right_back_node->isPath)
        {
            bottom_right_back_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_left_front_node != nullptr && top_left_front_node->isPath)
        {
            top_left_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_right_front_node != nullptr && top_right_front_node->isPath)
        {
            top_right_front_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_left_back_node != nullptr && top_left_back_node->isPath)
        {
            top_left_back_node->ExportToJson(sstream, indexFile, mapObject);
            iCount++;
            if (iCount != numChild)
                sstream << ",";
        }
        if (top_right_back_node != nullptr && top_right_back_node->isPath)
        {
            top_right_back_node->ExportToJson(sstream, indexFile, mapObject);
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
            
            std::string nameB3dm = "D_" + std::to_string(level) + "_" + std::to_string(indexFile) + "_" + std::to_string(std::distance(objectList.begin(),objectList.end())) + ".b3dm";
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