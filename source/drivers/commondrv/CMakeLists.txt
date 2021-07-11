cmake_minimum_required(VERSION 2.6)

PROJECT (commondrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} commondrv.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME commondrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkcomm pkdbapi)

SET(SPECOUTDIR /drivers/commondrv)
INCLUDE(../../CMakeCommonLib)
