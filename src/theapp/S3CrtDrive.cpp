#include <dirent.h>
#include <unistd.h>

#include "S3CrtDrive.h"
#include "Utils.h"
#include "LasToLas.hpp"
// 4Gb buffer doload file from S3
#define CS_MAX_BUFFER 4000000000
S3CrtDrive::S3CrtDrive(bool isLambda)
{
    m_isLambda = isLambda;
}
S3CrtDrive::~S3CrtDrive(void)
{

}

bool S3CrtDrive::CreateFromLink(const std::string& strLink)
{
    m_bucketName.clear(); m_objectName.clear(); m_regionName.clear(); m_objectFile.clear(); m_extFile.clear();
    if (strLink.empty())
        return false;
    // 
    size_t nEnd = strLink.rfind(".");
    size_t nStart = strLink.rfind("/");
    if (std::string::npos == nStart)
        return false;       
    m_strFileName = strLink.substr(nStart+1, nEnd - nStart-1);
    m_extFile = strLink.substr(nEnd, 4);
    return true;
}
bool S3CrtDrive::ProcessLASToLAS(const AppConfig m_AppConfig, const std::vector<std::string>& vecURL, const std::string& curDir, const std::string& strCRC,const std::string& source_geoId, const std::vector<std::string>& crop, const std::string& epsgDest, const std::string& geoId)
{
    size_t nFile = vecURL.size();
    if (nFile < 1)
        return false;
    std::string sourceLAS, destLAS, urlFileLAS, metaLAS, urlMeta, metaDestLAS, urlMetaDest;
    bool bRet = false;
    for (size_t i = 0; i < nFile; i++)
    {
        if (CreateFromLink(vecURL[i]))
        {
            sourceLAS = vecURL[i];
            if (m_extFile == ".las" || m_extFile == ".LAS" || m_extFile == ".laz" || m_extFile == ".LAZ")
            {
                std::string strPathUp = curDir + "/Output/" + m_strFileName;
                mkdir(strPathUp.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

                if (strcmp(epsgDest.c_str(), "") != 0) { //Transform to epsg destination
                    destLAS = strPathUp + "/" + m_strFileName + "_epsg_" + epsgDest + ".laz";
                    metaLAS = strPathUp + "/" + m_strFileName + ".inf";
                    metaDestLAS = strPathUp + "/" + m_strFileName + "_epsg_" + epsgDest + ".inf";
                }
                else if (crop.size() == 4) { //Cropping 
                    destLAS = strPathUp + "/" + m_strFileName + "_crop" + ".laz";
                    metaLAS = strPathUp + "/" + m_strFileName + ".inf";
                    metaDestLAS = strPathUp + "/" + m_strFileName + "_crop"+".inf";
                }
                else { //Add epsg source or get inf file
                    destLAS = strPathUp + "/" + m_strFileName + "_epsg_" + strCRC + ".laz";
                    metaLAS = strPathUp + "/" + m_strFileName + ".inf";
                }

                size_t fileSize = 0;
                
                if (Utils::FileExists(sourceLAS))
                {
                        int argc = 9;
                        if (strcmp(epsgDest.c_str(), "")) argc += 2;
                        if (crop.size() == 4) argc += 5;

                        char* argv[argc];
                        std::string str[argc];
                        str[0] = curDir;
                        argv[0] = (char*)(str[0].c_str()); // path LAS_CVS file

                        str[1] = std::string("-i");
                        argv[1] = (char*)(str[1].c_str());

                        str[2] = sourceLAS; // source file
                        argv[2] = (char*)(str[2].c_str());

                        str[3] = std::string("-epsg");
                        argv[3] = (char*)(str[3].c_str());

                        str[4] = strCRC;       // epsg tranform
                        argv[4] = (char*)(str[4].c_str());

                        str[5] = std::string("-source_geoId");
                        argv[5] = (char*)(str[5].c_str());

                        str[6] = source_geoId; // geo_id source ****************************************************************
                        argv[6] = (char*)(str[6].c_str());

                        str[7] = std::string("-o");
                        argv[7] = (char*)(str[7].c_str());

                        str[8] = destLAS; // dest file
                        argv[8] = (char*)(str[8].c_str());

                        if (strcmp(epsgDest.c_str(), "") != 0) {
                            str[9] = std::string("-target_epsg");
                            argv[9] = (char*)(str[9].c_str());

                            str[10] = epsgDest; // epsg Destination
                            argv[10] = (char*)(str[10].c_str());
                        }

                        if (crop.size() == 4) {
                            str[9] = std::string("-crop");
                            argv[9] = (char*)(str[7].c_str());

                            str[10] = crop[0]; // xmin
                            argv[10] = (char*)(str[8].c_str());

                            str[11] = crop[1]; // ymin
                            argv[11] = (char*)(str[9].c_str());

                            str[12] = crop[2]; // xmax
                            argv[12] = (char*)(str[10].c_str());

                            str[13] = crop[3]; // ymax
                            argv[13] = (char*)(str[11].c_str());
                        }

                        // str[9] = std::string("-v");
                        // argv[9] = (char*)(str[9].c_str());

                        LasToLas mLasToLas;
                        unsigned int nEPSG;
                        std::string strCS;
                        bool isLAS = (m_extFile == ".LAS") || (m_extFile == ".las");
                        bool isCreateLAZ = false;
                        int iRet;

                        if (strcmp(geoId.c_str(), "") != 0)
                            iRet = mLasToLas.Create(m_AppConfig, argc, argv, metaLAS.c_str(), isLAS, nEPSG, strCS, isCreateLAZ, "N" + geoId, metaDestLAS.c_str());
                        else
                            iRet = mLasToLas.Create(m_AppConfig, argc, argv, metaLAS.c_str(), isLAS, nEPSG, strCS, isCreateLAZ, geoId, metaDestLAS.c_str());
                        if (iRet == 0)
                        {
                            bRet = true;
                            if (epsgDest != "" || crop.size() == 4)
                                if (nEPSG > 0)
                                    m_strJSON = "\"EPSG\":\"" + std::to_string(nEPSG) + "\"";
                            if (!strCS.empty())
                                m_strJSON += ",\"CS Name\":\"" + strCS + "\"";
                        }
                }
            }
        }
    }
    return bRet;
}

bool S3CrtDrive::ProcessLasToTileset(const AppConfig m_AppConfig, const std::vector<std::string>& vecURL, const std::string& curDir, const std::string& strCRC, const std::string& source_geoId, const std::string& cell_size, const std::string& geometry_scale) {
    size_t nFile = vecURL.size();
    if (nFile < 1)
        return false;
    std::string sourceLAS, destLAS, urlFileLAS, inputFile;
    std::vector<std::string> outputFileVec;
    bool bRet = false;
    for (size_t i = 0; i < nFile; i++)
    {
        if (CreateFromLink(vecURL[i]))
        {
            if (m_extFile == ".las" || m_extFile == ".LAS" || m_extFile == ".laz" || m_extFile == ".LAZ")
            {
                std::string strPathUp = curDir + "/Output/" + m_strFileName;
                mkdir(strPathUp.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                
                size_t fileSize = 0;
                sourceLAS = vecURL[i];
                if (Utils::FileExists(sourceLAS))
                {
                        LasToLas mLasToLas;
                        int iRet;
                        
                        iRet = mLasToLas.LasToTileset(m_AppConfig,sourceLAS,strPathUp,strCRC,"N" + source_geoId, stod(cell_size), stod(geometry_scale));
                        if (iRet > 0)
                        {
                            //    destLAS = outputFileVec[0];
                                bRet = true;
                                m_strJSON = "\"Total PNTS file\":\"" + std::to_string(iRet) + "\"";
                        }
                }
            }
        }
    }
    return bRet;
}

bool S3CrtDrive::Process(const AppConfig m_AppConfig, const std::vector<std::string>& vecURL, const std::string& strOutFile, EnumOut typeOut, const std::string& curDir,const std::string& strCRC, const std::string& source_geoId, bool exportAtributes, const std::vector<std::string>& crop, const std::string& epsgDest, const std::string& geoId,const std::string& cell_size,const std::string& geometry_scale)
{
    if (typeOut == TYPE_LAS_TOOL && epsgDest.empty())
        return ProcessLASToLAS(m_AppConfig, vecURL, curDir, strCRC, source_geoId, std::vector<std::string>());
    else if (typeOut == TYPE_LAS_TOOL && !epsgDest.empty())
        return ProcessLASToLAS(m_AppConfig, vecURL, curDir, strCRC, source_geoId, std::vector<std::string>(), epsgDest, geoId);
    else if (typeOut == TYPE_LAS_TOOL_GEO)
        return ProcessLASToLAS(m_AppConfig, vecURL, curDir, strCRC, source_geoId, std::vector<std::string>(), epsgDest, geoId);
    else if (typeOut == TYPE_LAS_TOOL_CROP)
        return ProcessLASToLAS(m_AppConfig, vecURL, curDir, strCRC, source_geoId, crop);
    else if (typeOut == TYPE_LAS_TOOL_PNTS)
        return ProcessLasToTileset(m_AppConfig, vecURL, curDir, strCRC, source_geoId, cell_size, geometry_scale);
    return false;
}





