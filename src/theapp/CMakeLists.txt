#include_directories(${CMAKE_SOURCE_DIR}/../include)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

include_directories(../LASzip/src)
include_directories(../LASlib/inc)
include_directories(${CMAKE_SOURCE_DIR}/Pipeline3DTiles)
add_executable(LASToPointCloud main.cpp LandXml.Reader.Types.cpp S3CrtDrive.cpp Utils.cpp LandXmlReaderService.cpp AppConfig.cpp geoprojectionconverter.cpp LasToLas.cpp LasToolToPnts.cpp PointCloudModel.cpp QuadTreeObjectSizeBuilder.cpp)

target_link_libraries(LASToPointCloud debug Pipeline3DTiles.so /usr/lib/x86_64-linux-gnu/libexpat.so /usr/lib/x86_64-linux-gnu/libjsoncpp.so /usr/local/lib/libproj.so /usr/lib/x86_64-linux-gnu/libuuid.so /usr/lib/x86_64-linux-gnu/libzip.so)
target_link_libraries(LASToPointCloud optimized Pipeline3DTiles.so /usr/lib/x86_64-linux-gnu/libexpat.so /usr/lib/x86_64-linux-gnu/libjsoncpp.so  /usr/local/lib/libproj.so /usr/lib/x86_64-linux-gnu/libuuid.so /usr/lib/x86_64-linux-gnu/libzip.so)
target_link_libraries(LASToPointCloud debug /usr/local/lib/libifcengine.so)
target_link_libraries(LASToPointCloud optimized /usr/local/lib/libifcengine.so)
target_link_libraries(LASToPointCloud ${AWSSDK_LINK_LIBRARIES} AWS::aws-lambda-runtime)
target_link_libraries(LASToPointCloud LASlib)