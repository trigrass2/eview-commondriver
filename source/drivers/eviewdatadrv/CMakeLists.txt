cmake_minimum_required(VERSION 2.6)

PROJECT (eviewdatadrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
SET(SRCS ${SRCS} ${SRC_LIST} )

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME eviewdatadrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkce pkcomm pkdata jsoncpp)

SET(SPECOUTDIR /drivers/eviewdatadrv)
INCLUDE(../../CMakeCommonLib)