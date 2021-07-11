cmake_minimum_required(VERSION 2.6)

PROJECT (abdf1drv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
SET(SRCS ${SRCS} ${SRC_LIST} )

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME abdf1drv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkce pkcomm)

SET(SPECOUTDIR /drivers/abdf1drv)
INCLUDE(../../CMakeCommonLib)