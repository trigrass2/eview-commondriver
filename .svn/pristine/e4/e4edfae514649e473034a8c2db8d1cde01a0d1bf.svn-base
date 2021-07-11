cmake_minimum_required(VERSION 2.6)

PROJECT (s7mpitest)

INCLUDE_DIRECTORIES(../include)
IF(MSVC)
	ADD_DEFINITIONS(-DBCCWIN) 
ENDIF()

#Setting Source Files
SET(SRCS ${SRCS} ../nodave.c ../openS7online.c ../protoIBH.c ../protoNLPro.c ../testMPI.c ../openSocketw.c)
IF(MSVC)
	ADD_DEFINITIONS(-DBCCWIN) 
	SET(SRCS ${SRCS} ../setportw.c )
ELSE()
	SET(SRCS ${SRCS}../setport.c)
ENDIF()

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME s7mpitest)
#Setting library type used when build a library
#SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn)

SET(SPECOUTDIR /drivers/simenss7drv)
INCLUDE(../../../CMakeCommonExec)