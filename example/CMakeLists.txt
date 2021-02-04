

set(CMAKE_C_FLAGS "-std=gnu99")



set(SDK_SOURCE_DIR  ${SDK_Root}/source)


aux_source_directory(${MediaServer_Root}/libmov/source src_mov)
aux_source_directory(${MediaServer_Root}/libmov/test src_mov)


aux_source_directory(${MediaServer_Root}/librtp/include src_rtp)
aux_source_directory(${MediaServer_Root}/librtp/source src_rtp)
aux_source_directory(${MediaServer_Root}/librtp/payload src_rtp)
aux_source_directory(${MediaServer_Root}/librtp/test src_rtp)

aux_source_directory(${MediaServer_Root}/librtsp/source src_rtsp)
aux_source_directory(${MediaServer_Root}/librtsp/source/server/aio src_rtsp)
aux_source_directory(${MediaServer_Root}/librtsp/source/client src_rtsp)
aux_source_directory(${MediaServer_Root}/librtsp/source/push src_rtsp)

aux_source_directory(${MediaServer_Root}/librtsp/test src_rtsp_test)
aux_source_directory(${MediaServer_Root}/librtsp/source/utils src_rtsp_utils)
aux_source_directory(${MediaServer_Root}/librtsp/source/sdp src_rtsp_sdp)
aux_source_directory(${MediaServer_Root}/librtsp/source/server src_rtsp_server)
aux_source_directory(${SDK_Root}/libaio/src src_aio)
aux_source_directory(${SDK_Root}/libhttp/source src_http)

aux_source_directory(${SDK_Root}/source src_sdk)

aux_source_directory(${MediaServer_Root}/libflv/source src_flv)

aux_source_directory(${SDK_Root}/source/digest src_digest)

aux_source_directory(${MediaServer_Root}/libmpeg/source src_mpeg)

aux_source_directory(${AVCODEC_Root}/avbsf/src src_avbsf)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/faac/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/faac/libs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/asound/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/asound/libs)

# ADD_EXECUTABLE(01_read_push  01_read_push.cpp 
#         ${SDK_SOURCE_DIR}/port/ip-route.c
#         ${SDK_SOURCE_DIR}/port/aio-socket-epoll.c
#         ${src_rtsp_sdp}
#         ${src_aio} ${src_http} ${src_sdk} ${src_digest} ${src_avbsf} ${src_rtsp_server}
#         ${src_rtsp_utils} ${src_rtp} ${src_mpeg} ${src_flv} ${src_rtsp} ${src_mov} ${src_rtsp_test})
# TARGET_LINK_LIBRARIES(01_read_push pthread)



ADD_EXECUTABLE(02_cap_save  02_cap_save.cpp 
        ${SDK_SOURCE_DIR}/port/ip-route.c
        ${SDK_SOURCE_DIR}/port/aio-socket-epoll.c
        ${src_rtsp_sdp}
        ${src_aio} ${src_http} ${src_sdk} ${src_digest} ${src_avbsf} ${src_rtsp_server}
        ${src_rtsp_utils} ${src_rtp} ${src_mpeg} ${src_flv} ${src_rtsp} ${src_mov}
    ${src_base} ${src_rtsp_test})
TARGET_LINK_LIBRARIES(02_cap_save pthread faac asound)

ADD_EXECUTABLE(03_cap_push  03_cap_push.cpp 
        ${SDK_SOURCE_DIR}/port/ip-route.c
        ${SDK_SOURCE_DIR}/port/aio-socket-epoll.c
        ${src_rtsp_sdp}
        ${src_aio} ${src_http} ${src_sdk} ${src_digest} ${src_avbsf} ${src_rtsp_server}
        ${src_rtsp_utils} ${src_rtp} ${src_mpeg} ${src_flv} ${src_rtsp} ${src_mov}
    ${src_base} ${src_rtsp_test})
TARGET_LINK_LIBRARIES(03_cap_push pthread faac asound)


ADD_EXECUTABLE(04_cap_push_save  04_cap_push_save.cpp 
        ${SDK_SOURCE_DIR}/port/ip-route.c
        ${SDK_SOURCE_DIR}/port/aio-socket-epoll.c
        ${src_rtsp_sdp}
        ${src_aio} ${src_http} ${src_sdk} ${src_digest} ${src_avbsf} ${src_rtsp_server}
        ${src_rtsp_utils} ${src_rtp} ${src_mpeg} ${src_flv} ${src_rtsp} ${src_mov}
    ${src_base} ${src_rtsp_test})
TARGET_LINK_LIBRARIES(04_cap_push_save pthread faac asound)

