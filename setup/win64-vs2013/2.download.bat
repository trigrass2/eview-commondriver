set CURDIR=%~dp0
cd %CURDIR%
del /S /Q ..\..\thirdparty
mkdir ..\..\thirdparty

echo open ftp.peakinfo.cn > _download.ftp
echo user peak peak >> _download.ftp
echo binary >> _download.ftp
echo literal pasv >> _download.ftp
echo prompt off >> _download.ftp
echo lcd ../../thirdparty >> _download.ftp
echo cd /release/cccomps/win64-vs2013/ >> _download.ftp
echo mget cmake.rar pkcomm.rar pkdbapi.rar pkmqtt.rar jsoncpp.rar pkce.rar snmp_pp.rar libdes.rar libxl.rar pklog.rar python2.7.rar >> _download.ftp
echo cd /release/eview-server/win64-vs2013/ >> _download.ftp
echo mget eview-server-driversdk.rar eview-pkdata.rar  eview-server-base.rar>> _download.ftp
echo close >> _download.ftp
echo bye >> _download.ftp

ftp -n -s:_download.ftp
