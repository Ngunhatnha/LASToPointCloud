cmake_minimum_required(VERSION 3.5.0)

# Use shared libraries, which is the default for the AWS C++ SDK build.
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
#add_compile_options(-O3 -Wall -Wno-strict-aliasing)


#debug option
add_compile_options(-Og -Wall -Wno-strict-aliasing)
#add_compile_options(-O0 -Wall -Wno-strict-aliasing)

project(LASToPointCloud)

set (CMAKE_CXX_STANDARD 11)
# Locate the aws sdk for c++ package.
find_package(AWSSDK REQUIRED COMPONENTS s3-crt)
find_package(aws-lambda-runtime REQUIRED)


# Link to shared libraries.
#add_definitions(-DUSE_IMPORT_EXPORT)

#message(STATUS "Setting MSVC flags")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin")

add_subdirectory(Pipeline3DTiles)
add_subdirectory(theapp)
add_subdirectory(LASlib/src)
add_dependencies(LASToPointCloud Pipeline3DTiles)
target_compile_definitions(LASToPointCloud PUBLIC "LAMBDA_CPP")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LASToPointCloud)
aws_lambda_package_target(${PROJECT_NAME})