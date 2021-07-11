set CURDIR=%~dp0
cd %CURDIR%
rd /S /Q ..\..\eview-server
del  ..\..\eview-server*.rar

echo on

rd /q /s ..\..\repo\

rem mkdir directories
mkdir ..\..\repo\eview-server
mkdir ..\..\repo\eview-server\bin
mkdir ..\..\repo\eview-server\config
mkdir ..\..\repo\eview-server\python

echo copy config dir

xcopy ..\..\bin\pkdriver.exe ..\..\repo\eview-server\bin\ /y

echo copy drivers
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\abcipdrv\abcipdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\abdf1drv\abdf1drv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\bacnetdrv\bacnetdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\dbdrv\dbdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\gwmqttdrv\gwmqttdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\iec104drv\iec104drv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\mnmqttdrv\mnmqttdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\modbusrtu\modbusrtu.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\modbustcp\modbustcp.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\omronfinsdrv\omronfinsdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\opcdrv\opcdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\snmpdrv\snmpdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\pingdrv\pingdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\mitubishifxdrv\mitubishifxdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\simenss7drv\simenss7drv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\hostdriver\hostdriver.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\samplepythondrv\samplepythondrv.exe /y
rem copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\htmlpostdrv\htmlpostdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\exceldrv\exceldrv.exe /y
copy ..\..\bin\libxl.dll ..\..\bin\drivers\exceldrv\libxl.dll /y
copy ..\..\bin\libxl.dll ..\..\bin\drivers\csvdrv\libxl.dll /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\csvdrv\csvdrv.exe /y
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\eviewdatadrv\eviewdatadrv.exe /y

rem ´ó»ª
copy ..\..\bin\pkdriver.exe ..\..\bin\drivers\dhvideoinfo\dhvideoinfo.exe /y
xcopy ..\..\source\drivers\dhvideoinfo\win32\bin\* ..\..\bin\ /y /S /E

xcopy ..\..\source\drivers\samplepythondrv\*.py* ..\..\bin\drivers\samplepythondrv\ /y /s
xcopy ..\..\source\drivers\samplepythondrv\*.bat ..\..\bin\drivers\samplepythondrv\ /y /s
rem xcopy ..\..\source\drivers\htmlpostdrv\*.py* ..\..\bin\drivers\htmlpostdrv\ /y /s /r
rem xcopy ..\..\source\drivers\htmlpostdrv\*.pyd* ..\..\bin\drivers\htmlpostdrv\ /y /s /r
rem xcopy ..\..\source\drivers\htmlquerydrv\*.py* ..\..\bin\drivers\htmlquerydrv\ /y /s
rem xcopy ..\..\source\drivers\htmlquerydrv\*.pyd* ..\..\bin\drivers\htmlquerydrv\ /y /s

xcopy ..\..\bin\drivers\*.exe ..\..\repo\eview-server\bin\drivers\  /y /s
xcopy ..\..\bin\drivers\*.dll ..\..\repo\eview-server\bin\drivers\  /y /s
xcopy ..\..\bin\drivers\*.py ..\..\repo\eview-server\bin\drivers\  /y /s
xcopy ..\..\bin\drivers\*.pyc ..\..\repo\eview-server\bin\drivers\  /y /s
xcopy ..\..\bin\drivers\*.bat ..\..\repo\eview-server\bin\drivers\  /y /s

set CURRENT_DATE=%date:~0,4%-%date:~5,2%-%date:~8,2%
set RAR_FILENAME=eview-server-commondriver-%CURRENT_DATE%.rar
..\rar a -ep1 ../../repo/%RAR_FILENAME%  ../../repo/eview-server/bin
copy ..\..\repo\%RAR_FILENAME% ..\..\repo\eview-server-commondriver.rar /y

cd %CURDIR%