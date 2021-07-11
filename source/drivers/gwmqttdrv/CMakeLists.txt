cmake_minimum_required(VERSION 2.6)

PROJECT (gwmqttdrv)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
SET(SRCS ${SRCS} ${SRC_LIST} )

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME gwmqttdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkce pkcomm jsoncpp mosquitto)

SET(SPECOUTDIR /drivers/gwmqttdrv)
INCLUDE(../../CMakeCommonLib)