link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

file(GLOB headers *.h)
add_library(Pipeline3DTiles SHARED CesiumPro.cpp EllipsoidModel.cpp GeoIdHeight.cpp Vec3d.cpp ObjectInterface.cpp OctreeNode.cpp Base64encoder.cpp CrcData.cpp QuadTreeNode.cpp QuadTreeNodeTriangle.cpp QuadTreeInstanceNode.cpp ${headers})

add_definitions(-DEXPMODULE)
target_link_libraries(Pipeline3DTiles debug /usr/local/lib/libproj.so)
target_link_libraries(Pipeline3DTiles optimized /usr/local/lib/libproj.so)
target_link_libraries(Pipeline3DTiles ${AWSSDK_LINK_LIBRARIES})