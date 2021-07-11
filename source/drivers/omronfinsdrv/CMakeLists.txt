cmake_minimum_required(VERSION 2.6)

PROJECT (omronfinsdrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} omrondrv.cpp Message AutoGroup_BlkDev.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME omronfinsdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkcomm)

SET(SPECOUTDIR /drivers/omronfinsdrv)
INCLUDE(../../CMakeCommonLib)