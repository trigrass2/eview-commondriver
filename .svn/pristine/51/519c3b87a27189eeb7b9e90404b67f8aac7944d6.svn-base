cmake_minimum_required(VERSION 2.6)

PROJECT (dhvideoinfo)

#Setting Source Files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
SET(SRCS ${SRCS} ${SRC_LIST} )

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/win64/include)
    LINK_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/win64/lib)
ELSE()
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/win32/include)
    LINK_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/win32/lib)
ENDIF()

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME dhvideoinfo)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

#SET(LINK_LIBS pkdrvcmn pkce dhplay dhnetsdk avnetsdk dhconfigsdk Infra Stream NetFramework StreamSvr dhdvr)
SET(LINK_LIBS pkdrvcmn pkce dhplay dhnetsdk dhconfigsdk)
  
SET(SPECOUTDIR /drivers/dhvideoinfo)
INCLUDE(../../CMakeCommonLib)
