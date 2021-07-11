cmake_minimum_required(VERSION 2.6)

PROJECT (iec104drv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} 104drv.cpp AutoGroup_BlkDev.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME iec104drv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkcomm)

SET(SPECOUTDIR /drivers/iec104drv)
INCLUDE(../../CMakeCommonLib)