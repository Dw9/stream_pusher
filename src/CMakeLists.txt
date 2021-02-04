cmake_minimum_required(VERSION 3.13)


include_directories(../3rdpart/asound/include/)
link_directories(../3rdpart/asound/libs/)


include_directories(../3rdpart/faac/include/)
link_directories(../3rdpart/faac/libs/)


set(SOURCE_FILES main.cpp CapSndPcm.cpp CapSndPcm.h kfifo.c kfifo.h AACCycleRecord.cpp AACCycleRecord.h AACEncoderT.cpp AACEncoderT.h thread.cpp thread.h)
add_executable(201  ${SOURCE_FILES})

message(${PROJECT_BINARY_DIR})

# 指定生成可执行文件存放的位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

target_link_libraries(
        201
	asound
        faac
        pthread
)
