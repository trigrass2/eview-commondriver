set CURDIR=%~dp0
cd %CURDIR%

echo extract depends component......
rmdir /S /Q ..\..\lib
rmdir /S /Q ..\..\include
rmdir /S /Q ..\..\bin
cd ..\..\

setup\rar.exe x -y thirdparty\cmake.rar
setup\rar.exe x -y thirdparty\pklog.rar
setup\rar.exe x -y thirdparty\pkcomm.rar
setup\rar.exe x -y thirdparty\pkmqtt.rar
setup\rar.exe x -y thirdparty\python2.7.rar
setup\rar.exe x -y thirdparty\jsoncpp.rar
setup\rar.exe x -y thirdparty\pkdbapi.rar
setup\rar.exe x -inul -y thirdparty\pkce.rar
setup\rar.exe x -y thirdparty\snmp_pp.rar
setup\rar.exe x -y thirdparty\libdes.rar
setup\rar.exe x -y thirdparty\libxl.rar
setup\rar.exe x -y thirdparty\eview-server-driversdk.rar
setup\rar.exe x -y thirdparty\eview-server-base.rar.rar
setup\rar.exe x -y thirdparty\eview-pkdata.rar

cd %CURDIR%
