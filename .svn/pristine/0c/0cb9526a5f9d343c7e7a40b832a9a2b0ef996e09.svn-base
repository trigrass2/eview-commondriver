cmake_minimum_required(VERSION 2.6)

PROJECT (snmpdrv)

INCLUDE_DIRECTORIES(../../ext/snmp_pp/include)
############FOR_MODIFIY_BEGIN#######################
#Setting Source Files
SET(SRCS ${SRCS} snmpdrv.cpp AutoGroup_ObjDev_snmp.cpp)

#Setting Target Name (executable file name | library name)
SET(TARGET_NAME snmpdrv)
#Setting library type used when build a library
SET(LIB_TYPE SHARED)

IF(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    SET(LINK_LIBS snmp_pp libdes ws2_32 pkdrvcmn pkcomm pkdata jsoncpp)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    SET(LINK_LIBS snmp_pp libdes pkdrvcmn pkcomm pkdata jsoncpp)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES Windows)

SET(SPECOUTDIR /drivers/snmpdrv)
INCLUDE(../../CMakeCommonLib)
