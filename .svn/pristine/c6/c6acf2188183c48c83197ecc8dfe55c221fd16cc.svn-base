cmake_minimum_required(VERSION 2.6)

PROJECT (modbustcp)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} modbusdrv.cpp AutoGroup_BlkDev.cpp)

ADD_DEFINITIONS(-DMODBUSTYPE_TCP) 
#Setting Target Name (executable file name | library name)
SET(TARGET_NAME modbustcp)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkce pkcomm)

SET(SPECOUTDIR /drivers/modbustcp)
INCLUDE(../../CMakeCommonLib)
#MESSAGE("CMakeCommon:${SPECOUTDIR}")  
#INCLUDE(../../CMakeSpecOutPath)

#MESSAGE("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")  
#MESSAGE("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")  
#MESSAGE("${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")  