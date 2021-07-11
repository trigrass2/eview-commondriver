cmake_minimum_required(VERSION 2.6)

PROJECT (modbusrtu)

############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} ../modbustcp/modbusdrv.cpp ../modbustcp/AutoGroup_BlkDev.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME modbusrtu)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

SET(LINK_LIBS pkdrvcmn pkce pkcomm)

SET(SPECOUTDIR /drivers/modbusrtu)
INCLUDE(../../CMakeCommonLib)