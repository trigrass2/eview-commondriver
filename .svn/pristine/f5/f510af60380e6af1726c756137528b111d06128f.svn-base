set CURDIR=%~dp0
cd %CURDIR%
set CURRENT_DATE=%date:~0,4%-%date:~5,2%-%date:~8,2%
echo open ftp.peakinfo.cn > _upload.ftp
echo user peak peak >> _upload.ftp
echo binary >> _upload.ftp
echo literal pasv >> _upload.ftp
echo prompt off >> _upload.ftp
echo lcd ../../repo >> _upload.ftp
echo mkdir /release/eview-server/win32-vs2013 >> _upload.ftp
echo mkdir /release/eview-server/win32-vs2013/backup >> _upload.ftp
echo cd /release/eview-server/win32-vs2013 >> _upload.ftp
echo mput eview-server-commondriver.rar  >> _upload.ftp

echo cd /release/eview-server/win32-vs2013/backup >> _upload.ftp
echo mput eview-server-commondriver-2*.rar  >> _upload.ftp
echo close >> _upload.ftp
echo bye >> _upload.ftp

ftp -n -s:_upload.ftp

