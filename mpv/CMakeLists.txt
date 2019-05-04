cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# delete cmake cache folder before changing this options
option(BUILD_LINUX "Build with SDL2 support" OFF)
option(BUILD_WINDOWS "Build with Windows support (msys/mingw64)" OFF)
option(BUILD_VITA "Build with Ps Vita support (hardware rendering, fast)" OFF)
option(BUILD_PS3 "Build with PS3 support (hardware rendering, fast)" OFF)
option(BUILD_3DS "Build with 3DS support" OFF)
option(BUILD_SWITCH "Build with Nintendo Switch support" ON)

include(Toolchain.cmake)

project(mpv)

##############
# mpv
##############
set(MPV_INC
        .
        switch
        audio
        audio/decode
        audio/filter
        common
        demux
        filters
        input
        misc
        options
        player
        stream
        sub
        ta
        video
        video/decode
        video/filter
        video/out
        )
file(GLOB MPV_SRC
        switch/io.c
        switch/vo_tct.c
        audio/*.c
        audio/decode/*.c
        audio/filter/*.c
        common/*.c
        demux/*.c
        filters/*.c
        input/*.c
        misc/*.c
        options/*.c
        player/*.c
        stream/*.c
        sub/*.c
        ta/*.c
        )

list(APPEND MPV_SRC
        osdep/timer.c
        osdep/timer-linux.c
        osdep/terminal-dummy.c
        osdep/threads.c
        #osdep/io.c
        osdep/path-unix.c
        osdep/subprocess.c
        osdep/subprocess-dummy.c
        #osdep/subprocess-posix.c
        audio/out/ao.c
        audio/out/ao_null.c
        audio/out/ao_sdl.c
        audio/out/pull.c
        audio/out/push.c
        audio/out/ao_pcm.c
        audio/out/ao_lavc.c
        video/csputils.c
        video/fmt-conversion.c
        video/hwdec.c
        video/image_loader.c
        video/image_writer.c
        video/img_format.c
        video/mp_image.c
        video/mp_image_pool.c
        video/sws_utils.c
        video/decode/vd_lavc.c
        video/filter/refqueue.c
        video/filter/vf_format.c
        video/filter/vf_sub.c
        video/out/opengl/common.c
        video/out/opengl/context.c
        video/out/opengl/formats.c
        video/out/opengl/libmpv_gl.c
        video/out/opengl/utils.c
        video/out/opengl/ra_gl.c
        video/out/aspect.c
        video/out/bitmap_packer.c
        video/out/dither.c
        video/out/dr_helper.c
        video/out/filter_kernels.c
        video/out/vo.c
        video/out/vo_gpu.c
        video/out/vo_libmpv.c
        video/out/vo_image.c
        #video/out/vo_tct.c
        video/out/vo_lavc.c
        video/out/vo_mediacodec_embed.c
        video/out/vo_null.c
        video/out/vo_sdl.c
        video/out/win_state.c
        video/out/gpu/context.c
        video/out/gpu/video.c
        video/out/gpu/video_shaders.c
        video/out/gpu/user_shaders.c
        video/out/gpu/hwdec.c
        video/out/gpu/lcms.c
        video/out/gpu/ra.c
        video/out/gpu/shader_cache.c
        video/out/gpu/utils.c
        video/out/gpu/osd.c
        video/out/gpu/libmpv_gpu.c
        video/out/gpu/spirv.c
        )

list(REMOVE_ITEM MPV_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/filter/af_rubberband.c
        ${CMAKE_CURRENT_SOURCE_DIR}/demux/demux_libarchive.c
        ${CMAKE_CURRENT_SOURCE_DIR}/input/pipe-win32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/input/ipc-win.c
        ${CMAKE_CURRENT_SOURCE_DIR}/input/ipc-unix.c
        ${CMAKE_CURRENT_SOURCE_DIR}/player/javascript.c
        ${CMAKE_CURRENT_SOURCE_DIR}/player/lua.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/ai_alsa1x.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/ai_oss.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/ai_sndio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/dvb_tune.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_cdda.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_bluray.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_dvb.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_dvd.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_dvd_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_libarchive.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_dvdnav.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_smb.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stream/tvi_v4l2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sub/osd_dummy.c
        )

set(MPV_CFLAGS
        -DM_PI=3.14159265358979323846
        -DM_SQRT2=1.41421356237309504880
        )

set(MPV_LDFLAGS swscale swresample avformat avfilter avcodec avutil ass freetype png bz2 fribidi pthread m z)

#####################
# PLATORM SPECIFIC
#####################
if (BUILD_SWITCH)
    #####################
    # SWITCH PLATORM
    #####################
    list(APPEND MPV_CFLAGS -D__SWITCH__)
    list(APPEND MPV_INC ${DEVKITPRO}/portlibs/switch/include/SDL2 ${DEVKITPRO}/portlibs/switch/include)
    list(APPEND MPV_LDFLAGS mbedtls mbedcrypto mbedx509 nx)
elseif (BUILD_LINUX)
    #####################
    # LINUX PLATORM
    #####################
    list(APPEND MPV_INC /usr/include/SDL2)
    list(APPEND MPV_LDFLAGS lzma)
    #list(APPEND MPV_INC /usr/include/samba-4.0)
    #list(APPEND MPV_LDFLAGS smbclient)
    #list(APPEND MPV_SRC ${CMAKE_CURRENT_SOURCE_DIR}/stream/stream_smb.c)
endif ()

#####################
# pplay executable
#####################
add_library(${PROJECT_NAME} ${MPV_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${MPV_INC})
target_compile_options(${PROJECT_NAME} PRIVATE ${MPV_CFLAGS})
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_link_libraries(${PROJECT_NAME} PUBLIC ${MPV_LDFLAGS})
