# Install script for directory: D:/work_xing/project/eview-commondriver/source/drivers

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/eview-commondriver")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/snmpdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/dbdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/pingdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/iec104drv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/modbustcp/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/modbusrtu/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/mitubishifxdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/abcipdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/abdf1drv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/omronfinsdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/simenss7drv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/eviewdatadrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/sampledrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/commondrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/bacnetdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/gwmqttdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/mnmqttdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/exceldrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/csvdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/libnodavetest/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/opc/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/pdfdrv/cmake_install.cmake")
  include("D:/work_xing/project/eview-commondriver/cmake-temp/drivers/dhvideoinfo/cmake_install.cmake")

endif()

