#include <stdio.h>
#include "LandXmlReaderService.h"
USING_LANDXML_READER;

int main(int argc, char* argv[]) 
{
    std::string args="";

    for (int i = 1; i < argc; i++) {
        if (i != (argc - 1)) 
        {
            args.append(argv[i]);
            args.append(" ");
        }
         else args.append(argv[i]);
    }

    //std::cout << args;

    int nRet = 0;
    bool isLambda = false;
    LandXmlReaderService mLandXmlReaderService;
    //mLandXmlReaderService.setLambda(isLambda);
    nRet = mLandXmlReaderService.TestLASTool(args);
    if (!isLambda)
    {
        std::string strRet= mLandXmlReaderService.getStreamOut().str();
        std::cout << strRet;            
    }
    return nRet;
}