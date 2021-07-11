cmake_minimum_required(VERSION 2.6)

PROJECT (simenss7drv)
############FOR_MODIFIY_BEGIN#######################

ADD_DEFINITIONS(-DDAVE_HAVE_MEMCPY)
IF(UNIX)
ADD_DEFINITIONS(-DHAVE_SELECT -DLINUX)
MESSAGE("HAVE_SELECT LINUX defined")
ENDIF()

#Setting Source Files
SET(SRCS ${SRCS} nodave.cpp nodave_TCP.cpp nodave_PPI.cpp nodave_MPI.cpp nodave_MPI2.cpp nodave_MPI3.cpp protoIBH.c protoNLPro.c AutoGroup_BlkDev.cpp S7Drv.cpp openS7online.c S7Device.cpp)

#aux_source_directory(. SRC_LIST)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME simenss7drv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkcomm)

SET(SPECOUTDIR /drivers/simenss7drv)
INCLUDE(../../CMakeCommonLib)
