#ifndef _S3CRTDRIVE_H__
#define _S3CRTDRIVE_H__
#include <string>
#include "LandXml.Reader.Types.h"
#include "AppConfig.h"
USING_LANDXML_READER

class S3CrtDrive
{
   public:
    S3CrtDrive(bool isLambda);
    ~S3CrtDrive(void);
    // new function
    //bool ProcessLASToLAS(const std::vector<std::string>& vecURL, const std::string& curDir, const std::string& strCRC);
    //bool ProcessLASToLASDest(const std::vector<std::string>& vecURL, const std::string& curDir, const std::string& strCRC, const std::string& epsgDest);
    bool ProcessLASToLAS(const AppConfig m_appConfig, const std::vector<std::string>& vecURL, const std::string& curDir, const std::string& strCRC,const std::string& source_geoId,const std::vector<std::string>& crop, const std::string& epsgDest = "", const std::string& geoId = "");
    bool Process(const AppConfig m_appConfig,const std::vector<std::string>& vecURL, const std::string& strOutFile, EnumOut typeOut, const std::string& curDir, const std::string& strCRC,const std::string& source_geoId, bool exportAtributes,const std::vector<std::string>& crop, const std::string& epsgDest, const std::string& geoId,const std::string& cell_size, const std::string& geometry_scale);
  
   bool CreateFromLink(const std::string& strLink);
   //bool DownLoad(const std::string& strLocal, const size_t& maxBuffer, size_t &fileSize) const;
  // bool setAclRead(const std::string& objectName) const;
   bool ProcessLasToTileset(const AppConfig m_appConfig, const std::vector<std::string>& vecURL, const std::string& curDir, const std::string& strCRC, const std::string& source_geoId, const std::string& cell_size, const std::string& geometry_scale);

   const std::string& getBucketName() const {return m_bucketName;}
   const std::string& getObjectName() const { return m_objectName; }
   const std::string& getRegionName() const { return m_regionName; }
  
   const std::string& getObjectFile() const { return m_objectFile; }
   const std::string& getFileName() const { return m_strFileName; }
   const std::string& getExtFile() const { return m_extFile; }
   const std::string& getLastErro() const { return m_strLastErro; }  
   const std::string& getRefPointJSON()
   {
       return m_strJSON;
   }
  // bool InvokeFunction(std::string functionName, const std::string strParam) const;

   protected:
       std::string m_bucketName;
       std::string m_objectName;
       std::string m_regionName;
      
       std::string m_objectFile;
       std::string m_strFileName;
       std::string m_extFile;
       std::string m_strLastErro;  
       bool m_isLambda;
       std::string m_strCurrPath;
       std::string m_strJSON;
       
};
#endif
