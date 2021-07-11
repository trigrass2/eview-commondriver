cmake_minimum_required(VERSION 2.6)

PROJECT (abcipdrv)

#Setting Source Files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
SET(SRCS ${SRCS} ${SRC_LIST} ) 
   
#Setting Target Name (executable file name | library name)
SET(TARGET_NAME abcipdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkcomm pkce)

SET(SPECOUTDIR /drivers/abcipdrv)
INCLUDE(../../CMakeCommonLib)
