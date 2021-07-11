cmake_minimum_required(VERSION 2.6)

PROJECT (dbdrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} dbdrv.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME dbdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkdbapi pkcomm)

SET(SPECOUTDIR /drivers/dbdrv)
INCLUDE(../../CMakeCommonLib)