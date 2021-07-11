cmake_minimum_required(VERSION 2.6)

PROJECT (csvdrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/../exceldrv)
SET(SRCS ${SRCS} ../exceldrv/exceldrv.cpp ../exceldrv/TableDevice.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME csvdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn libxl pkcomm )

SET(SPECOUTDIR /drivers/csvdrv)
INCLUDE(../../CMakeCommonLib)