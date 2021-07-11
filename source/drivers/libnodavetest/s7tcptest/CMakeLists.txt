cmake_minimum_required(VERSION 2.6)

PROJECT (s7tcptest)

INCLUDE_DIRECTORIES(../include)

#Setting Source Files
SET(SRCS ${SRCS} ../nodave.c ../openS7online.c ../protoIBH.c ../protoNLPro.c ../openSocketw.c ../testISO_TCP.c)
#aux_source_directory(. SRC_LIST)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME s7tcptest)
#Setting library type used when build a library
#SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn)

SET(SPECOUTDIR /drivers/simenss7drv)
INCLUDE(../../../CMakeCommonExec)