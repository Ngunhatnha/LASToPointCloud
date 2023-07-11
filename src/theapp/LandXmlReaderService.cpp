#include "LandXmlReaderService.h"
// https://movable-type.co.uk/scripts/latlong.html

BEGIN_LANDXML_READER
int LandXmlReaderService::ProcessLASTool()
{
    if ((mEnumOut == TYPE_UNKNOWN_OPTION) || (TYPE_ERROR_INPUT == mEnumOut))
    {
        printf("{\"Status\":\"Unknown options\"}\n");
        return 0;
    }
    if (mEnumOut == TYPE_LAS_TOOL)
    {
            {
                std::string curDir = m_AppConfig.getPathConfig();
                srand(time(NULL));

                std::string  strPath = curDir + "/Output";
                Utils::DeleteFilesInDirectory(strPath, true);
                rmdir(strPath.c_str());
                mkdir(strPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                

                S3CrtDrive mS3CrtDrive(m_AppConfig.isLambda());
                if (!mS3CrtDrive.Process(m_AppConfig, vecURL, outFile, mEnumOut, curDir, strCRCr, source_geoId, exportAtributes, std::vector<std::string>(), epsgDest,geoId,"", ""))
                {
                    if (!mS3CrtDrive.getLastErro().empty())
                        m_ssOut << "{\"Status\":\"Error\"," << mS3CrtDrive.getLastErro().c_str() << "}\n";
                    else
                        m_ssOut << "{\"Status\":\"Error\"}\n";
                }
                else 
                {
                    if (mS3CrtDrive.getRefPointJSON().empty())
                        m_ssOut << "{\"Status\":\"OK\"}\n";
                    else
                        m_ssOut << "{\"Status\":\"OK\"," << mS3CrtDrive.getRefPointJSON().c_str() << "}\n";
                }
            }
        return 1;
    }
    return 0;
}

int LandXmlReaderService::ProcessLASToolPnts()
{
    if ((mEnumOut == TYPE_UNKNOWN_OPTION) || (TYPE_ERROR_INPUT == mEnumOut))
    {
        printf("{\"Status\":\"Unknown options\"}\n");
        return 0;
    }
    if (mEnumOut == TYPE_LAS_TOOL_PNTS)
    {
        {
            std::string curDir = m_AppConfig.getPathConfig();

            srand(time(NULL));
            std::string  strPath = curDir + "/Output";
            Utils::DeleteFilesInDirectory(strPath,true);
            rmdir(strPath.c_str());            
            mkdir(strPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            S3CrtDrive mS3CrtDrive(m_AppConfig.isLambda());
            if (!mS3CrtDrive.Process(m_AppConfig, vecURL, outFile, mEnumOut, curDir, strCRCr, source_geoId, exportAtributes, std::vector<std::string>(), epsgDest, geoId, cell_size, geometry_scale))
            {
                if (!mS3CrtDrive.getLastErro().empty())
                    m_ssOut << "{\"Status\":\"Error\"," << mS3CrtDrive.getLastErro().c_str() << "}\n";
                else
                    m_ssOut << "{\"Status\":\"Error\"}\n";
            }
            else
            {
                if (mS3CrtDrive.getRefPointJSON().empty())
                    m_ssOut << "{\"Status\":\"OK\"}\n";
                else
                    m_ssOut << "{\"Status\":\"OK\"," << mS3CrtDrive.getRefPointJSON().c_str() << "}\n";
            }
        }
        return 1;
    }
    return 0;
}

int LandXmlReaderService::ProcessLASToolCrop()
{
    if ((mEnumOut == TYPE_UNKNOWN_OPTION) || (TYPE_ERROR_INPUT == mEnumOut))
    {
        printf("{\"Status\":\"Unknown options\"}\n");
        return 0;
    }
    if (crop.size()!=4){
        printf("{\"Status\":\"crop need 4 parameters\"}\n");
        return 0;
    }
    if (mEnumOut == TYPE_LAS_TOOL_CROP)
    {
        {
            std::string curDir = m_AppConfig.getPathConfig();
            srand(time(NULL));
           
            std::string  strPath = curDir + "/Output";
            Utils::DeleteFilesInDirectory(strPath, true);
            rmdir(strPath.c_str());
            mkdir(strPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            S3CrtDrive mS3CrtDrive(m_AppConfig.isLambda());
            if (!mS3CrtDrive.Process(m_AppConfig, vecURL, outFile, mEnumOut, curDir, strCRCr, source_geoId,exportAtributes, crop, epsgDest, geoId,"", ""))
            {
                if (!mS3CrtDrive.getLastErro().empty())
                    m_ssOut << "{\"Status\":\"Error\"," << mS3CrtDrive.getLastErro().c_str() << "}\n";
                else
                    m_ssOut << "{\"Status\":\"Error\"}\n";
            }
            else
            {
                if (mS3CrtDrive.getRefPointJSON().empty())
                    m_ssOut << "{\"Status\":\"OK\"}\n";
                else
                    m_ssOut << "{\"Status\":\"OK\"," << mS3CrtDrive.getRefPointJSON().c_str() << "}\n";
            }
        }
        return 1;
    }
    return 0;
}

bool LandXmlReaderService::checkNum() {
    try {
        if (!strCRCr.empty()) stoi(strCRCr);
        if (!geoId.empty()) stoi(geoId);
        if (!source_geoId.empty()) stoi(source_geoId);
        if (!epsgDest.empty()) stoi(epsgDest);
        
        if (!cell_size.empty()) stod(cell_size);
        if (!geometry_scale.empty()) stod(geometry_scale);
        if (crop.size() == 4) {
            for (int j = 0; j < 4; j++)
                if (!crop[j].empty()) stod(crop[j]);
        }
    }
    catch (...) {
        m_ssOut << "Input error. Check your input!";
        mEnumOut = TYPE_ERROR_INPUT;
    }
}


int LandXmlReaderService::CreateLambda(const std::string& strArgv)
{
    mEnumOut = Utils::ProcessArgumentsLambda(strArgv, vecURL, outFile, serverType, strCRCr,source_geoId,epsgDest,geoId,crop,cell_size,geometry_scale, exportAtributes);
    checkNum();
    if (mEnumOut == TYPE_LAS_TOOL)
        return ProcessLASTool();
    else if (mEnumOut == TYPE_LAS_TOOL_PNTS)
        return ProcessLASToolPnts();
    else if (mEnumOut == TYPE_LAS_TOOL_CROP)
        return ProcessLASToolCrop();
    return 0;
}
int LandXmlReaderService::TestLASTool(std::string args)
{
    /*
    Can't use keep_xy with target_epsg:
    "-s s3 -sour 3879 -dest 4326 -kepp_xy " => WRONG
    must use
    "-s s3 -sour 3879 -dest 4326"
    then
    "-keep_xy";
    */

    //Target: -s s3 -T source_epsg,source_geoId,dest_epsg,dest_geoid  (LASTOLAS change file's epsg and geoId)
    //        -s s3 -C source_epsg,source_geoId,x_min,y_min,x_max,y_max (Cropping)
    //        -s s3 -P source_epsg,source_geoId,cell_size,geometry_scale (convert from LAS to Pnts and make tileset)
    
    //args = std::string(get_current_dir_name()) + "/rgb_673498d_epsg_3879.laz -P 3879,2000,15,0.1";
    return CreateLambda(args);
}

END_LANDXML_READER