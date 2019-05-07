cmake_minimum_required(VERSION 3.0)
#set(CMAKE_VERBOSE_MAKEFILE ON)

#####################
# Linux/Windows data
#####################
if (BUILD_WINDOWS AND NOT MSYS)
    string(REGEX REPLACE "/" "\\\\" P_DST ${CMAKE_CURRENT_BINARY_DIR}/data)
    string(REGEX REPLACE "/" "\\\\" P_SRC ${CMAKE_CURRENT_SOURCE_DIR}/data/read_only/)
    string(REGEX REPLACE "/" "\\\\" P_SRC_RW ${CMAKE_CURRENT_SOURCE_DIR}/data/read_write/)
    add_custom_target(${PROJECT_NAME}.data
            COMMAND if not exist ${P_DST} mkdir ${P_DST}
            COMMAND copy ${P_SRC}* ${P_DST})
    add_dependencies(${PROJECT_NAME}.elf ${PROJECT_NAME}.data)
else ()
    add_custom_target(${PROJECT_NAME}.data
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/data
            COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/data/read_only/. ${CMAKE_CURRENT_BINARY_DIR}/data
            COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/data/read_write/. ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(${PROJECT_NAME}.elf ${PROJECT_NAME}.data)
endif ()

#####################
# VITA target
#####################
if (BUILD_VITA)
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME}.elf)
    add_custom_target(${PROJECT_NAME}_vita_release.vpk
            DEPENDS ${PROJECT_NAME}.self
            COMMAND rm -rf ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/data
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/sce_sys
            COMMAND ${VITASDK}/bin/vita-mksfoex -s TITLE_ID="${TITLE_ID}" "${PROJECT_NAME}" ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/sce_sys/param.sfo
            COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.self ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/eboot.bin
            COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/data/vita/. ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/
            COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/data/read_only/. ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/data
            COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/data/read_write/. ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND cd ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME} && zip -r ../../${PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}_vita.vpk .
            COMMAND cd ${CMAKE_CURRENT_BINARY_DIR})
endif (BUILD_VITA)

#####################
# SWITCH target
#####################
if (BUILD_SWITCH)
    set_target_properties(${PROJECT_NAME}.elf PROPERTIES LINK_FLAGS "-specs=${DEVKITPRO}/libnx/switch.specs")
    add_custom_target(${PROJECT_NAME}.nro
            DEPENDS ${PROJECT_NAME}.elf
            COMMAND ${DEVKITPRO}/tools/bin/nacptool --create "${PROJECT_NAME}" "${PROJECT_AUTHOR}" "${VERSION_MAJOR}.${VERSION_MINOR}" ${PROJECT_NAME}.nacp
            COMMAND ${DEVKITPRO}/tools/bin/elf2nro ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${CMAKE_CURRENT_SOURCE_DIR}/data/switch/icon.jpg --nacp=${PROJECT_NAME}.nacp --romfsdir=${CMAKE_CURRENT_SOURCE_DIR}/data/read_only)
    add_custom_target(${PROJECT_NAME}_switch_release.zip
            DEPENDS ${PROJECT_NAME}.nro
            COMMAND rm -rf ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nro ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/
            COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/data/read_write/. ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/
            COMMAND cd ${CMAKE_BINARY_DIR}/release && zip -r ../${PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}_switch.zip ${PROJECT_NAME}
            COMMAND cd ${CMAKE_CURRENT_BINARY_DIR})
endif (BUILD_SWITCH)

#####################
# 3DS target
#####################
if (BUILD_3DS)
    set_target_properties(${PROJECT_NAME}.elf PROPERTIES LINK_FLAGS "-specs=${DEVKITPRO}/devkitARM/arm-none-eabi/lib/3dsx.specs")
    add_custom_target(${PROJECT_NAME}.3dsx
            DEPENDS ${PROJECT_NAME}.elf
            COMMAND ${DEVKITPRO}/tools/bin/smdhtool --create "${PROJECT_NAME}" "${PROJECT_NAME}" "${PROJECT_AUTHOR}" ${CMAKE_CURRENT_SOURCE_DIR}/data/3ds/icon.png ${PROJECT_NAME}.smdh
            COMMAND ${DEVKITPRO}/tools/bin/3dsxtool ${PROJECT_NAME}.elf ${PROJECT_NAME}.3dsx --smdh=${PROJECT_NAME}.smdh --romfs=${CMAKE_CURRENT_SOURCE_DIR}/data/read_only)
    add_custom_target(${PROJECT_NAME}_3ds_release.zip
            DEPENDS ${PROJECT_NAME}.3dsx
            COMMAND rm -rf ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}
            COMMAND cp -f ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.3dsx ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/
            COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/data/read_write/. ${CMAKE_BINARY_DIR}/release/${PROJECT_NAME}/
            COMMAND cd ${CMAKE_BINARY_DIR}/release && zip -r ../${PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}_3ds.zip ${PROJECT_NAME}
            COMMAND cd ${CMAKE_CURRENT_BINARY_DIR})
endif (BUILD_3DS)
