PROJECT (opcdrv)

#Setting Source Files, including .cpp,.c,.cxx
AUX_SOURCE_DIRECTORY(. SRC_LIST)
SET(SRCS ${SRCS} ${SRC_LIST} )

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME opcdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkce pkcomm pkdrvcmn)

SET(SPECOUTDIR /drivers/opcdrv)
INCLUDE(../../../CMakeCommonLib) 
