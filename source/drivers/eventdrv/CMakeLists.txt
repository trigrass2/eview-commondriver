cmake_minimum_required(VERSION 2.6)

PROJECT (eventdrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} eventdrv.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME eventdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkdbapi pkcomm )

SET(SPECOUTDIR /drivers/eventdrv)
INCLUDE(../../CMakeCommonLib)