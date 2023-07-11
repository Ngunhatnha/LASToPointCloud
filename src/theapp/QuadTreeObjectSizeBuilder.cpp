#include "QuadTreeObjectSizeBuilder.h"
#include "Utils.h"
#include <vector>
#include <forward_list>
#include "EllipsoidModel.h"

QuadTreeObjectSizeBuilder::~QuadTreeObjectSizeBuilder()
{
}

std::string QuadTreeObjectSizeBuilder::encodeExtras(double geoidHeight, const BoundingBox& mModelBox) const
{
    std::string strEncoder;
    Base64encoder mBase64encoder;
    double* pData = new double[9];
    Vec3d vecFileOrg = m_vecWorldOrg;
    pData[0] = vecFileOrg[0];
    pData[1] = vecFileOrg[1];
    pData[2] = vecFileOrg[2] + geoidHeight;

    pData[3] = mModelBox.xMin() + pData[0];
    pData[4] = mModelBox.yMin() + pData[1];
    pData[5] = mModelBox.zMin() + pData[2];

    pData[6] = mModelBox.xMax() + pData[0];
    pData[7] = mModelBox.yMax() + pData[1];
    pData[8] = mModelBox.zMax() + +pData[2];

    unsigned int nLen = sizeof(double) * 9;
    mBase64encoder.encode((char*)pData, nLen, strEncoder);
    delete[]pData;
    return strEncoder;
}

bool QuadTreeObjectSizeBuilder::ConvertCoordinate(const Vec3d& vec, double& lat, double& lon, double& height, const std::string& strSource, const std::string& strDest)
{
    PJ_CONTEXT* C = proj_context_create();
    PJ* src = nullptr;
    PJ* dst = nullptr;
    PJ* transformation = nullptr;
    if (C != nullptr)
    {
        try
        {
            src = proj_create(C, strSource.c_str());
        }
        catch (const std::exception&)
        {
            //std::cout << "{\"Status\":\" Error\",\"crc\":\"" << strSource << " not found\"}\n";
            m_strLastErro = "\"Message\":\"File coordinate systems is not defined correctly. Please check and fix the file coordinate system and re-try\"\n";
            return false;
        }

        if (src == nullptr)
        {
            proj_context_destroy(C);
            proj_cleanup();
            m_strLastErro = "\"Message\":\"File coordinate systems is not defined correctly. Please check and fix the file coordinate system and re-try\"\n";
            return false;
        }
        try
        {
            dst = proj_create(C, strDest.c_str());
        }
        catch (const std::exception&)
        {
            //std::cout << "{\"Status\":\" Error,\"crc\": " << strDest << " not found\"}\n";
            m_strLastErro = "\"Message\":\"File coordinate systems is not defined correctly. Please check and fix the file coordinate system and re-try\"\n";
            return false;
        }

        if (dst == nullptr)
        {
            proj_destroy(src);
            proj_context_destroy(C);
            proj_cleanup();
            m_strLastErro = "\"Message\":\"File coordinate systems is not defined correctly. Please check and fix the file coordinate system and re-try\"\n";
            return false;
        }
        try
        {
            transformation = proj_create_crs_to_crs_from_pj(C, src, dst, nullptr, nullptr);
        }
        catch (const std::exception&)
        {
            proj_destroy(src);
            proj_destroy(dst);
            proj_context_destroy(C);
            proj_cleanup();
            m_strLastErro = "\"Message\":\"File coordinate systems is not defined correctly. Please check and fix the file coordinate system and re-try\"\n";
            return false;
        }
        if (transformation == nullptr)
        {
            proj_destroy(src);
            proj_destroy(dst);
            proj_context_destroy(C);
            proj_cleanup();
            m_strLastErro = "\"Message\":\"File coordinate systems is not defined correctly. Please check and fix the file coordinate system and re-try\"\n";
            return false;
        }
        PJ_COORD coord;
        coord.xyzt.x = vec[0];
        coord.xyzt.y = vec[1];
        coord.xyzt.z = vec[2];
        coord.xyzt.t = HUGE_VAL;
        coord = proj_trans(transformation, PJ_FWD, coord);
        if (isinf(coord.xyzt.x) || isinf(coord.xyzt.y) || isinf(coord.xyzt.z))
        {
            proj_destroy(src);
            proj_destroy(dst);
            proj_destroy(transformation);
            proj_context_destroy(C);
            proj_cleanup();
            
            m_strLastErro = "\"Message\":\"Model is placed outside of project coordinate system. Please put model into project area\"\n";
            return false;
        }
        lat = coord.xyz.x * DEG_TO_RAD;
        lon = coord.xyz.y * DEG_TO_RAD;
        height = coord.xyz.z;
        proj_destroy(src);
        proj_destroy(dst);
        proj_destroy(transformation);
        proj_context_destroy(C);
        proj_cleanup();
        return true;
    }
    m_strLastErro = "\"Message\":\"Unable to load PROJ library. Please check and fix the file runtime system and re-try\"\n";
    return false;
}

static void* BuildTreeInstanceThreadFunc(void* pData)
{
    StackData* pStackData = (StackData*)pData;
    bool bRet = true;
    pthread_attr_t tattr;
    if (pthread_attr_init(&tattr) == 0)
    {
        size_t actualStackSize;
        pthread_getattr_np(pthread_self(), &tattr);
        if (pthread_attr_getstacksize(&tattr, &actualStackSize) == 0)
        {
            if (actualStackSize == pStackData->dStackSize)
            {
                //printf("ThreadFunc:  Stack size is as expected:  %zu\n", actualStackSize);
                QuadTreeInstanceNode<ObjectIntance>* quadtree = (QuadTreeInstanceNode<ObjectIntance>*)pStackData->pObject;
                quadtree->BuildTree(1);
            }
            else
            {
                bRet = false;
                printf("ThreadFunc:  ERROR, wrong stack size!  (Requested:  %zu, got: %zu)\n", pStackData->dStackSize, actualStackSize);
            }
        }
        else
        {
            bRet = false;
            perror("pthread_attr_getstacksize");
        }
    }
    else
    {
        bRet = false;
        perror("pthread_attr_init(2)");
    }
    return (void*)bRet;
}
static void* ExportRootToJsonInstanceThreadFunc(void* pData)
{
    StackData* pStackData = (StackData*)pData;
    bool bRet = true;
    pthread_attr_t tattr;
    if (pthread_attr_init(&tattr) == 0)
    {
        size_t actualStackSize;
        pthread_getattr_np(pthread_self(), &tattr);
        if (pthread_attr_getstacksize(&tattr, &actualStackSize) == 0)
        {
            if (actualStackSize == pStackData->dStackSize)
            {
                //printf("ThreadFunc:  Stack size is as expected:  %zu\n", actualStackSize);
                QuadTreeInstanceNode<ObjectIntance>* quadtree = (QuadTreeInstanceNode<ObjectIntance>*)pStackData->pObject;
                ParamDataInstanceObjectSize* pParamData = (ParamDataInstanceObjectSize*)(pStackData->pParam);
                quadtree->ExportRootToJson(pParamData->sstream, pParamData->indexFile, pParamData->mapObject);
            }
            else
            {
                bRet = false;
                printf("ThreadFunc:  ERROR, wrong stack size!  (Requested:  %zu, got: %zu)\n", pStackData->dStackSize, actualStackSize);
            }
        }
        else
        {
            bRet = false;
            perror("pthread_attr_getstacksize");
        }
    }
    else
    {
        bRet = false;
        perror("pthread_attr_init(2)");
    }
    return (void*)bRet;
}
#define BIG_STACK 1

int QuadTreeObjectSizeBuilder::writeLandXmlObjectsToTileset(const std::string& localTile, const std::string& localPathPnts)
{
    if ((m_pPointCloudModel == NULL) ||
         m_pPointCloudModel->m_listPointCloud.size() == 0)
        return false;

    m_crcCode = m_pPointCloudModel->getEpsgCode();
    m_heightCode = m_pPointCloudModel->getVerticalSystemName();

    BoundingBox bBox, bBoxAlign;
    ParamDataInstanceObjectSize mParamDataInstanceObjectSize;
    bool isBoxValid = false;
    /////////////////////////// bounding box world
    m_worldBox.init();
    m_localBox.init();
    
    std::list<CgPoint>::const_iterator itCg = m_pPointCloudModel->m_listPointCloud.begin();

   
    m_pPointCloudModel->getBoundingBox(m_worldBox);
    
    m_pPointCloudModel->getLocalBox(m_worldBox, m_localBox);
    m_pPointCloudModel->getVecorg(m_vecWorldOrg);
    
    /////////////////////// build quad tree
    Vec3d vecSize = m_localBox._max - m_localBox._min;
    double sizeMax = vecSize[0];
    if (sizeMax < vecSize[1])
        sizeMax = vecSize[1];

    int nLevelMax = 1;
    if (sizeMax > _cellSize)
    {
        while (sizeMax > _cellSize)
        {
            sizeMax *= 0.5;
            nLevelMax++;
            if (nLevelMax >= CS_MAX_DEPTH_QUAD_TREE)
                break;
        }
        if (nLevelMax > 1)
            nLevelMax--;
    }
    
    //////// process points instance
    // spatial index CgPoint
    if (m_pPointCloudModel->m_listPointCloud.size() > 0)
    {
        QuadTreeInstanceNode<ObjectIntance>::InitTreeObjectSize(nLevelMax, m_localBox._min[2], m_localBox._max[2], _maxTriangleInFile, _scaleGeometricError);
        QuadTreeInstanceNode<ObjectIntance>* quadtreeInstance = new QuadTreeInstanceNode<ObjectIntance>(m_localBox.xMin(), m_localBox.yMin(), vecSize.getX(), vecSize.getY(), ROOT_INSTANCE, 1);
#ifdef BIG_STACK
        StackData mStackData;

        mStackData.pObject = quadtreeInstance;
        mStackData.dStackSize = PAGE_SIZE; // 64Mb
        mStackData.pParam = NULL;
        if (!Utils::CreateThreadFunc(BuildTreeInstanceThreadFunc, &mStackData))
            return false;
#else
        quadtreeInstance->BuildTree(1);
#endif
        std::forward_list<PointInstance>::const_iterator it = m_pPointCloudModel->m_forwardListPointCloud.begin();
        int count = 0;
        while (it != m_pPointCloudModel->m_forwardListPointCloud.end())
        {
            quadtreeInstance->InsertObject(new PointInstance(*(it)));
            it++;
        }

        mParamDataInstanceObjectSize.indexFile = 0;
        mStackData.pObject = quadtreeInstance;
#ifdef BIG_STACK
        mStackData.pParam = &mParamDataInstanceObjectSize;
        if (!Utils::CreateThreadFunc(ExportRootToJsonInstanceThreadFunc, &mStackData))
            return false;
#else
        quadtree->ExportRootToJson(mParamDataInstanceObjectSize.sstream, mParamDataInstanceObjectSize.indexFile, mParamDataInstanceObjectSize.mapObject);
#endif
        std::map<std::string, QuadTreeInstanceNode<ObjectIntance>*>::const_iterator itMap = mParamDataInstanceObjectSize.mapObject.begin();
        QuadTreeInstanceNode<ObjectIntance>* pNode;
        std::string localPnts;
        ObjectIntance* pObjectIntance;
        PointInstance* pPointInstance;

        std::string strData = localPnts = localPathPnts + "/" + "data.dat";
        while (itMap != mParamDataInstanceObjectSize.mapObject.end())
        {
            pNode = itMap->second;
            // write PNTS
            localPnts = localPathPnts + "/" + itMap->first;
            const std::forward_list<ObjectIntance*>& pListObject = pNode->getObjectList();
            BoundingBox box(pNode->getBoundingBox());
            std::forward_list<ObjectIntance*>::const_iterator itObject = pListObject.begin();
            LasToolToFilePnts mLasToolToFilePnts;
            while (itObject != pListObject.end())
            {
                pObjectIntance = *(itObject++);
                pPointInstance = dynamic_cast<PointInstance*>(pObjectIntance);
                if (pPointInstance)
                {
                    mLasToolToFilePnts.putPosition(&box,pPointInstance);
                }
            }
            mLasToolToFilePnts.writeCmpt(localPnts, strData);
            itMap++;
            totalPNTS++;
        }
        delete quadtreeInstance;
    }
    ////////////////////////////////////////////////////////////////////////////////// write tileset ////////////////////////////////////////////////////////////////////////////////    
        double lat, lon, height;
        Matrixd local2world;
        Vec3d vCenterBox;
        //1 0 0 0, 0 1 0 0, 0 0 1 0, Tx Ty Tz 1
        // step 1 convert coordinates
        if (m_crcCode.empty())
            m_crcCode = "epsg:4978";

        // conver coordinate to WGS84
        if (!ConvertCoordinate(m_vecWorldOrg, lat, lon, height, m_crcCode, "epsg:4326"))
            return false;
        // step 2 get ellipsoid height 
        CesiumEllipsoidModel  ellipsoidModel;
        double geoidHeight = 0.0;

        std::string strGeoidConfig = m_pPointCloudModel->getVerticalSystemName(); 

        if (strGeoidConfig.empty())
            strGeoidConfig = m_heightCode;
        else
            m_heightCode = strGeoidConfig;

        if (writeSpatialIndexTitleset(localTile, m_localBox, mParamDataInstanceObjectSize.sstream.str(), lat, lon, height, true))
            return totalPNTS;
        else return -1;
    }

double QuadTreeObjectSizeBuilder::calGeometricError(double dRadius) const
{
    return 2 * dRadius * _scaleGeometricError;
}

bool QuadTreeObjectSizeBuilder::writeSpatialIndexTitleset(const std::string& strTitle, const BoundingBox& mModelBox, const std::string& strJSONPNTS, double lat, double lon, double height, bool hasWGS84)
{
    if (strJSONPNTS.empty()) return false;

    std::string strDir, strName, strExt, strTile;
    Utils::splitFilepath(strTitle, strDir, strName, strExt);

    std::ofstream mFile;
    std::ofstream mFilePnts;
    

    mFile.open(strTitle, std::fstream::out | std::fstream::trunc);
    if (!mFile.is_open())
        return false;

    if (!strJSONPNTS.empty())
    {
        std::string strTitlePnts;
        strTitlePnts = strDir + "/PNTSTileset.json";
        mFilePnts.open(strTitlePnts, std::fstream::out | std::fstream::trunc);
    }

    if (mFile.is_open() || mFilePnts.is_open())
    {
        std::stringstream stream, streamModel, streamPnts, streamTileModel;
        stream.precision(15);
        stream.setf(std::ios::fixed, std::ios::floatfield);
        streamTileModel.precision(15);
        streamTileModel.setf(std::ios::fixed, std::ios::floatfield);

        double geometricError = calGeometricError(mModelBox.radius());
        //Vec3d vCenter = mModelBox.center();
        //////////////////////////////////////// fixed model center all way 0,0,0 for landXML
        //if(vCenter.length()<0.000001)
        Vec3d vCenter;


        std::stringstream ssJson;
        ssJson.precision(15);
        ssJson.setf(std::ios::fixed, std::ios::floatfield);

        if (hasWGS84)
        {
            Matrixd local2world;
            CesiumEllipsoidModel  ellipsoidModel;
            //Vec3d vCenterBox = m_vecWorldOrg;
            //double xyz[3];
            ellipsoidModel.computeLocalToWorldTransformFromLatLongHeight(lat, lon, height, local2world);
            //ellipsoidModel.convertLatLongHeightToXYZ(lat, lon, height, xyz[0], xyz[1], xyz[2]);
            // write assert			
            stream << "{\"asset\":{\"version\":\"1.0\",\"tilesetVersion\":\"" << CSZ_VERSION << "\",\"extras\":{\"data\":";
            stream << "\"" << encodeExtras(height, mModelBox) << "\",";
            stream << "\"RefPoint\":[" << lat * RAD_TO_DEG << "," << lon * RAD_TO_DEG << "," << height << "],";
            stream << "\"epsgCode\":\"" << m_crcCode << "\",";
            stream << "\"verticalCoordinateSystemName\":\"" << m_heightCode << "\",";
            stream << "\"LocalOrg\":[" << m_vecWorldOrg[0] << "," << m_vecWorldOrg[1] << "," << m_vecWorldOrg[2] << "],";
            stream << "\"ModelCenter\":[" << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << "],";
            stream << "\"ModelBox\":[" << mModelBox.xMin() << "," << mModelBox.yMin() << "," << mModelBox.zMin() << "," << mModelBox.xMax() << "," << mModelBox.yMax() << "," << mModelBox.zMax() << "]";
            
            stream << "},";
            
            stream << "\"gltfUpAxis\":\"Z\"},";
            stream << "\"geometricError\":" << geometricError << ",\"root\":{\"refine\":\"ADD\",";

            streamModel << stream.str();
            streamPnts << stream.str();

            stream << "\"transform\":[";
            /*
            stream << local2world(0, 0) << "," << local2world(0, 1) << "," << local2world(0, 2) << "," << local2world(0, 3) << ",";
            stream << local2world(1, 0) << "," << local2world(1, 1) << "," << local2world(1, 2) << "," << local2world(1, 3) << ",";
            stream << local2world(2, 0) << "," << local2world(2, 1) << "," << local2world(2, 2) << "," << local2world(2, 3) << ",";
            stream << local2world(3, 0) << "," << local2world(3, 1) << "," << local2world(3, 2) << "," << local2world(3, 3) << "],";
            */
            
            stream << 1 << "," << 0 << "," << 0 << "," << 0 << ",";
            stream << 0 << "," << 1 << "," << 0 << "," << 0 << ",";
            stream << 0 << "," << 0 << "," << 1 << "," << 0 << ",";
            stream << m_vecWorldOrg[0] << "," << m_vecWorldOrg[1] << "," << m_vecWorldOrg[2] << "," << 1 << "],";
            
            ssJson << "\"RefPoint\":[" << lat * RAD_TO_DEG << "," << lon * RAD_TO_DEG << "," << height << "],";
            ssJson << "\"epsgCode\":\"" << m_crcCode << "\",";
            ssJson << "\"verticalCoordinateSystemName\":\"" << m_heightCode << "\",";
            ssJson << "\"LocalOrg\":[" << m_vecWorldOrg[0] << "," << m_vecWorldOrg[1] << "," << m_vecWorldOrg[2] << "],";
            ssJson << "\"ModelCenter\":[" << vCenter[0] << "," << vCenter[1] << "," << vCenter[2] << "],";
            ssJson << "\"ModelBox\":[" << mModelBox.xMin() << "," << mModelBox.yMin() << "," << mModelBox.zMin() << "," << mModelBox.xMax() << "," << mModelBox.yMax() << "," << mModelBox.zMax() << "]";
        }

        double halfX, halfY, halfZ;
        halfX = 0.5 * (mModelBox._max.x() - mModelBox._min.x());
        halfY = 0.5 * (mModelBox._max.y() - mModelBox._min.y());
        halfZ = 0.5 * (mModelBox._max.z() - mModelBox._min.z());

        stream << "\"boundingVolume\":{\"box\":[";
        streamModel << "\"boundingVolume\":{\"box\":[";
        streamPnts << "\"boundingVolume\":{\"box\":[";

        stream << vCenter.x() << "," << vCenter.y() << "," << vCenter.z() << ",";
        streamModel << vCenter.x() << "," << vCenter.y() << "," << vCenter.z() << ",";
        streamPnts << vCenter.x() << "," << vCenter.y() << "," << vCenter.z() << ",";

        stream << halfX << ",0,0,";
        streamModel << halfX << ",0,0,";
        streamPnts << halfX << ",0,0,";

        stream << "0," << halfY << ",0,";
        streamModel << "0," << halfY << ",0,";
        streamPnts << "0," << halfY << ",0,";

        stream << "0,0," << halfZ << "]}";
        streamModel << "0,0," << halfZ << "]}";
        streamPnts << "0,0," << halfZ << "]}";

        stream << ",\"geometricError\":" << geometricError << ",";
        streamModel << ",\"geometricError\":" << geometricError << ",";
        streamPnts << ",\"geometricError\":" << geometricError << ",";

        //stream << ",\"root\":{\"refine\":\"ADD\",";

        mFile << stream.str();
        stream.str("");

        // write I3dm model to json
        if (!strJSONPNTS.empty())
        {
            mFilePnts << streamPnts.str();
            streamPnts.str("");
            mFilePnts << strJSONPNTS;
            mFilePnts << "}";
            mFilePnts.close();
        }

        // process tile
         if (!strJSONPNTS.empty())
        {
            streamTileModel << "\"content\":{\"uri\":\"PNTSTileset.json\"}}}";
            stream << "\"content\":{\"uri\":\"PNTSTileset.json\"}}}";
        }
        else
        {
            streamTileModel << "\"content\":null}}";
            stream << "\"content\":null}}";         
        }
        streamTileModel.str("");

        mFile << stream.str();
        stream.str("");
        mFile.close();

        ssJson.str("");
        return true;
    }
    return false;
}
