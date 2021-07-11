cmake_minimum_required(VERSION 2.6)

PROJECT (exceldrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} exceldrv.cpp TableDevice.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME exceldrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn libxl pkcomm )

SET(SPECOUTDIR /drivers/exceldrv)
INCLUDE(../../CMakeCommonLib)