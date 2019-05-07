cmake_minimum_required(VERSION 3.7)

set(BUILD_SWITCH ON CACHE BOOL "Build with NX (SWITCH) support")

set(CMAKE_TOOLCHAIN_FILE Toolchain.cmake)
project(SDL_kitchensink C)
include(GNUInstallDirs)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(KIT_VERSION_MAJOR "1")
set(KIT_VERSION_MINOR "0")
set(KIT_VERSION_PATCH "7")
set(KIT_VERSION ${KIT_VERSION_MAJOR}.${KIT_VERSION_MINOR}.${KIT_VERSION_PATCH})
add_definitions(
    -DKIT_VERSION_MAJOR=${KIT_VERSION_MAJOR}
    -DKIT_VERSION_MINOR=${KIT_VERSION_MINOR}
    -DKIT_VERSION_PATCH=${KIT_VERSION_PATCH}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DLIBCONFIG_STATIC")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -Werror -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -ggdb -O2 -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -DNDEBUG")

option(BUILD_EXAMPLES "Build examples" OFF)
option(USE_DYNAMIC_LIBASS "Use dynamically loaded libass" OFF)
option(USE_ASAN "Use AddressSanitizer" OFF)

#set(SDL2_INCLUDE_DIRS /opt/devkitpro/portlibs/switch/include/SDL2)
#set(SDL2_LIBRARIES  /opt/devkitpro/portlibs/switch/lib/SDL2)

find_package(SDL2 REQUIRED)
message(STATUS "Found SDL2: ${SDL2_INCLUDE_DIRS}")
message(STATUS "Found SDL2_LIB: ${SDL2_LIBRARIES}")
find_package(ffmpeg COMPONENTS avcodec avformat avutil swscale swresample)

set(LIBRARIES
    ${SDL2_LIBRARIES}
    ${FFMPEG_LIBRARIES}
)
set(INCLUDES
    include/
    ${SDL2_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

if(USE_DYNAMIC_LIBASS)
    if(WIN32 OR MINGW OR MSYS)
        set(DYNAMIC_LIBASS_NAME "\"libass-9.dll\"")
    else()
        set(DYNAMIC_LIBASS_NAME "\"libass.so\"")
    endif()
    add_definitions(-DUSE_DYNAMIC_LIBASS)
    add_definitions(-DDYNAMIC_LIBASS_NAME=${DYNAMIC_LIBASS_NAME})
else()
    find_package(ass)
    set(LIBRARIES ${LIBRARIES} ${ASS_LIBRARIES})
    set(INCLUDES ${INCLUDES} ${ASS_INCLUDE_DIRS})
endif()

FILE(GLOB_RECURSE SOURCES "src/*.c")
FILE(GLOB INSTALL_HEADERS "include/kitchensink/*.h")

add_library(SDL_kitchensink SHARED ${SOURCES})
add_library(SDL_kitchensink_static STATIC ${SOURCES})

set_target_properties(SDL_kitchensink PROPERTIES VERSION ${KIT_VERSION})
set_target_properties(SDL_kitchensink PROPERTIES SOVERSION ${KIT_VERSION_MAJOR})

set_target_properties(SDL_kitchensink PROPERTIES DEBUG_POSTFIX "d")
set_target_properties(SDL_kitchensink_static PROPERTIES DEBUG_POSTFIX "d")

target_compile_definitions(SDL_kitchensink PRIVATE "KIT_DLL;KIT_DLL_EXPORTS")
target_compile_options(SDL_kitchensink PRIVATE "-fvisibility=hidden")

set_property(TARGET SDL_kitchensink PROPERTY C_STANDARD 99)
set_property(TARGET SDL_kitchensink_static PROPERTY C_STANDARD 99)

if(USE_ASAN)
    set(LIBRARIES asan ${LIBRARIES})
    target_compile_options(SDL_kitchensink PRIVATE "-fsanitize=address")
    message(STATUS "DEVELOPMENT: AddressSanitizer enabled!")
endif()

include_directories(${INCLUDES})
target_link_libraries(SDL_kitchensink ${LIBRARIES})

set(PKG_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/SDL_kitchensink.pc")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.in"
    ${PKG_CONFIG_FILE}
    @ONLY
)

if(BUILD_EXAMPLES)
    add_executable(audio examples/example_audio.c)
    add_executable(complex examples/example_complex.c)
    add_executable(simple examples/example_simple.c)
    add_executable(custom examples/example_custom.c)
    add_executable(rwops examples/example_rwops.c)

    if(MINGW)
        target_link_libraries(audio mingw32)
        target_link_libraries(complex mingw32)
        target_link_libraries(simple mingw32)
        target_link_libraries(custom mingw32)
        target_link_libraries(rwops mingw32)
    endif()

    set_property(TARGET audio PROPERTY C_STANDARD 99)
    set_property(TARGET complex PROPERTY C_STANDARD 99)
    set_property(TARGET simple PROPERTY C_STANDARD 99)
    set_property(TARGET custom PROPERTY C_STANDARD 99)
    set_property(TARGET rwops PROPERTY C_STANDARD 99)

    target_link_libraries(audio SDL_kitchensink_static ${LIBRARIES})
    target_link_libraries(complex SDL_kitchensink_static ${LIBRARIES})
    target_link_libraries(simple SDL_kitchensink_static ${LIBRARIES})
    target_link_libraries(custom SDL_kitchensink_static ${LIBRARIES})
    target_link_libraries(rwops SDL_kitchensink_static ${LIBRARIES})
endif()

# documentation target
add_custom_target(docs COMMAND doxygen WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Installation
install(FILES ${PKG_CONFIG_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${INSTALL_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kitchensink)
INSTALL(TARGETS SDL_kitchensink SDL_kitchensink_static
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
include(Targets.cmake)
