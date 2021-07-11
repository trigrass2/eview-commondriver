cmake_minimum_required(VERSION 2.6)

PROJECT (pingdrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} pkping.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME pingdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkce pkdrvcmn)

SET(SPECOUTDIR /drivers/pingdrv)
INCLUDE(../../CMakeCommonLib)