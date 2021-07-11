cmake_minimum_required(VERSION 2.6)

PROJECT (kodacatv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} catv.cpp)

#ADD_DEFINITIONS(-DMODBUSTYPE_TCP) 
#Setting Target Name (executable file name | library name)
SET(TARGET_NAME kodacatv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn)

SET(SPECOUTDIR /drivers/kodacatv)
INCLUDE(../../CMakeCommonLib)