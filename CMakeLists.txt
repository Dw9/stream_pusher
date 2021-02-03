project(stream_pusher)
cmake_minimum_required(VERSION 3.1.3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


add_definitions(-DOS_LINUX)


set(TARGET_ARCH x86_64)
set(CHIP intel CACHE STRING "this is default chip type" )



message("-- TARGET_ARCH: ${TARGET_ARCH}")
message("-- CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")


# 包含引用目录，源文件可以直接用 #include "repo_version.h" 包含
include_directories(${PROJECT_BINARY_DIR})



set(MediaServer_Root ${CMAKE_CURRENT_SOURCE_DIR}/src/media-server)
set(SDK_Root ${CMAKE_CURRENT_SOURCE_DIR}/src/sdk)
set(AVCODEC_Root ${CMAKE_CURRENT_SOURCE_DIR}/src/avcodec)


# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/${TARGET_ARCH}/include)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/${TARGET_ARCH}/lib)

INCLUDE_DIRECTORIES(${AVCODEC_Root}/avbsf/include)

INCLUDE_DIRECTORIES(${MediaServer_Root})


INCLUDE_DIRECTORIES(${MediaServer_Root}/librtp/include)
INCLUDE_DIRECTORIES(${MediaServer_Root}/librtsp/include)
INCLUDE_DIRECTORIES(${MediaServer_Root}/libmpeg/include)
INCLUDE_DIRECTORIES(${MediaServer_Root}/libflv/include)
INCLUDE_DIRECTORIES(${MediaServer_Root}/libmov/include)


INCLUDE_DIRECTORIES(${SDK_Root}/include)
INCLUDE_DIRECTORIES(${SDK_Root}/libaio/include)
INCLUDE_DIRECTORIES(${SDK_Root}/libhttp/include)



INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/3dparty/${TARGET_ARCH}/include)
#link_directories(${PROJECT_SOURCE_DIR}/3dparty/${TARGET_ARCH}/lib)




# 指定生成可执行文件存放的位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

aux_source_directory(${PROJECT_SOURCE_DIR}/src src_base)

#ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(example)
#ADD_SUBDIRECTORY(test)